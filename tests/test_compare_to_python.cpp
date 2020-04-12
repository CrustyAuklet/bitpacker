// Copied (almost) directly from https://github.com/karkason/cppystruct/blob/master/tests/binary_compatibility_test.cpp
// This file will test the implementation against Python's output
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>

#include <limits>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#define CATCH_CONFIG_ENABLE_TUPLE_STRINGMAKER
#include "test_common.hpp"

template <typename T>
std::string escapeString(const T& val) {
    std::string out = "\"";
    for(auto c : val) {
        if(isgraph(c)) {
            out.push_back(c);
        }
        else if (isprint(c)) {
            switch(c) {
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    out += c;
                    break;
            }
        }
        else {
            std::stringstream s;
            s << std::setfill('0') << std::setw(2) << static_cast<int>(c);
            out += "\\" + s.str();
        }
    }

    out += '"';
    return out;
}

template <typename T>
std::string convertToString(const T& val) {
    if constexpr (std::is_same_v<T, char>) {
        return "chr(" + std::to_string(val) + ")";
    }
    if constexpr (std::is_integral_v<T>) {
        return std::to_string(val);
    }
    if constexpr (std::is_convertible_v<T, std::string>) {
        return escapeString(val);
    }
    if constexpr (std::is_floating_point_v<T>) {
        std::stringstream ss;
        ss << std::scientific << std::setprecision(16) << val;
        return ss.str();
    }
    return Catch::StringMaker<T>::convert(val);
}


template <typename Fmt, typename... Args>
std::string buildPythonScript(Fmt, const std::string& outputPath, const Args&... toPack) {
    std::string pythonScript = R"py(
# Workaround for Catch::StringMaker<bool>
true = True
false = False
import bitstruct
bin = bitstruct.pack(")py" + std::string(Fmt::value()) + R"py(")py";

    std::string args[] = { convertToString(toPack)... };
    for(const auto& arg : args) {
        pythonScript += ", ";
        pythonScript += arg;
    }

    pythonScript +=  R"py()
with open(r')py" + outputPath + R"py(', 'wb') as f:
    f.write(bin)
    )py";

    return pythonScript;
}


template <typename Fmt, typename... Args>
auto runPythonPack(Fmt, const Args&... toPack) {
    // Create the python script
    std::string packedBinaryFilePath = std::tmpnam(nullptr);
    std::string pythonScript = buildPythonScript(Fmt{}, packedBinaryFilePath, toPack...);

    // Write the python script
    std::string pythonScriptPath = std::tmpnam(nullptr);
    auto pythonScriptFile = std::ofstream(pythonScriptPath, std::ios::binary);
    pythonScriptFile.write(pythonScript.data(), pythonScript.size());
    pythonScriptFile.close(); // Flush to disk

    std::string scriptRunCommand = "python";
    const char* pyName = getenv("CI_PYTHON_NAME");
    if(pyName != nullptr) {
        scriptRunCommand = std::string(pyName);
    }

    // Run the python script
    scriptRunCommand += " " + pythonScriptPath;
    auto retVal = system(scriptRunCommand.c_str());
    if(retVal != 0) {
        FAIL("Python run failed with error code " << retVal);
    }

    // Read the python script output
    std::ifstream inputFile(packedBinaryFilePath, std::ios::binary);
    inputFile = std::ifstream(packedBinaryFilePath, std::ios::binary);
    std::vector<char> outputBuffer(
            (std::istreambuf_iterator<char>(inputFile)),
                    (std::istreambuf_iterator<char>()));
    inputFile.close();

    // Remove script+output files
    remove(pythonScriptPath.c_str());
    remove(packedBinaryFilePath.c_str());

    return outputBuffer;
}

std::string print_data_vector(const std::vector<char> &vec) {
    std::stringstream ss;
    ss << "{";
    for (const auto &v : vec) {
        ss << " 0x" << std::hex << (static_cast<unsigned>(v) & 0xFFU) << ",";
    }
    ss << " }";
    return ss.str();
}

template <typename T>
std::string print_tuple_value(const T v) {
    std::stringstream ss;
    if constexpr (std::is_same_v<T, char> || std::is_same_v<T, unsigned char>) {
        ss << static_cast<int>(v);
    }
    else {
        ss << v;
    }
    return ss.str();
}

template <class TupType, size_t... I>
std::string print_data_tuple(const TupType &_tup, std::index_sequence<I...>) {
    std::stringstream ss;
    ss << "{ ";
    (..., (ss << (I == 0 ? "" : ", ") << std::hex
              << print_tuple_value(std::get<I>(_tup))));
    ss << " }";
    return ss.str();
}

template <class... T>
std::string print_data_tuple(const std::tuple<T...> &_tup) {
    return print_data_tuple(_tup, std::make_index_sequence<sizeof...(T)>());
}

template <typename Fmt, typename... Args>
void testPackAgainstPython(Fmt, const Args&... toPack) {
    //auto packed = bitpacker::pack(Fmt{}, toPack...);
    auto pythonPacked = runPythonPack(Fmt{}, toPack...);

    //CAPTURE(packed);
    INFO("From Python-pack: " << print_data_vector(pythonPacked));

    //REQUIRE(packed.size() == pythonPacked.size());
    //REQUIRE(std::equal(packed.begin(), packed.end(), pythonPacked.begin()));
    
    bitpacker::span<const bitpacker::byte_type> buffer(reinterpret_cast<uint8_t*>(pythonPacked.data()), pythonPacked.size() );
    auto unpacked = bitpacker::unpack(Fmt{}, buffer);

    INFO("Unpacked by Bitpacker: " << print_data_tuple(unpacked));
    INFO("Expected: " << print_data_tuple(std::make_tuple(toPack...)));
    REQUIRE( unpacked == std::make_tuple(toPack...));
}

TEST_CASE("compare to python: single unsigned integers", "[bitpacker::binary_compat]") {
    testPackAgainstPython(BP_STRING("u1"), 0b1U);
    testPackAgainstPython(BP_STRING("u1"), 0b0U);
    testPackAgainstPython(BP_STRING("u4"), 0b1010U);
    testPackAgainstPython(BP_STRING("u10"), 0b11'1010'0101U);
    testPackAgainstPython(BP_STRING("u12"), 0b1001'1010'0101U);
    testPackAgainstPython(BP_STRING("u30"), 0b10'1001'1010'0110'0101'0011'1100'0110U);
    testPackAgainstPython(BP_STRING("u43"), 0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U);
    testPackAgainstPython(BP_STRING("u64"), 0xDEADBEEFCAFEBABE);

    testPackAgainstPython(BP_STRING("u64"), 0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python: multiple unsigned integers", "[bitpacker::binary_compat]") {
    testPackAgainstPython(BP_STRING("u4u4u8u3u5"), 5, 2, 0xff, 5, 0b11010U);

    testPackAgainstPython(BP_STRING("u4u10u12"), 0b1010U, 0b11'1010'0101U, 0b1001'1010'0101U);

    testPackAgainstPython(BP_STRING("u4u10u12u30u43u64"),
            0b1010U,
            0b11'1010'0101U,
            0b1001'1010'0101U,
            0b10'1001'1010'0110'0101'0011'1100'0110U,
            0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U,
            0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python: single signed integers", "[bitpacker::binary_compat]") {
    // -1 creates all 1's
    testPackAgainstPython(BP_STRING("s1"), -1);
    testPackAgainstPython(BP_STRING("s4"), -1);
    testPackAgainstPython(BP_STRING("s10"), -1);
    testPackAgainstPython(BP_STRING("s12"), -1);
    testPackAgainstPython(BP_STRING("s30"), -1);
    testPackAgainstPython(BP_STRING("s43"), -1);
    testPackAgainstPython(BP_STRING("s64"), -1);

    // random negative values, somewhat large
    testPackAgainstPython(BP_STRING("s4"), -5);
    testPackAgainstPython(BP_STRING("s10"), -500);
    testPackAgainstPython(BP_STRING("s12"), -1040);
    testPackAgainstPython(BP_STRING("s30"), -536870911);
    testPackAgainstPython(BP_STRING("s43"), -4398046509981ll);
    testPackAgainstPython(BP_STRING("s64"), -9223372036742463338ll);

    // some positive values
    testPackAgainstPython(BP_STRING("s1"), 0);
    testPackAgainstPython(BP_STRING("s4"), 5);
    testPackAgainstPython(BP_STRING("s10"), 500);
    testPackAgainstPython(BP_STRING("s12"), 2'000);
    testPackAgainstPython(BP_STRING("s30"), 536'870'000);
    testPackAgainstPython(BP_STRING("s43"), 4'358'146'411'000ll);
    testPackAgainstPython(BP_STRING("s64"), 8'213'362'026'654'675'200ll);
}

TEST_CASE("compare to python: multiple signed integers", "[bitpacker::binary_compat]") {
    testPackAgainstPython(BP_STRING("s4s4s8s3s5"), -8, -5, -115, -3, -10);

    testPackAgainstPython(BP_STRING("s4s10s12s30s43s64"), -1, -1, -1, -1, -1, -1);

    testPackAgainstPython(BP_STRING("s4s10s12s30s43s64"), 
        -5, -500, -1040, -536'870'911, -4'398'046'509'981ll, -9'223'372'036'742'463'338ll);

    testPackAgainstPython(BP_STRING("s4s10s12s30s43s64"),
        5, 500, 2'000, 536'870'000, 4'358'146'411'000ll, 8'213'362'026'654'675'200ll);

    testPackAgainstPython(BP_STRING("s4s2s10s12s30s43s64s4s10s12"),
        5, -1, 500, -2'000, 536'870'000, -4'358'146'411'000ll, 8'213'362'026'654'675'200ll, -5, 500, -2'000);
}
