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
    std::cout << pythonScript << std::endl;

    // Write the python script
    std::string pythonScriptPath = std::tmpnam(nullptr);
    auto pythonScriptFile = std::ofstream(pythonScriptPath, std::ios::binary);
    pythonScriptFile.write(pythonScript.data(), pythonScript.size());
    pythonScriptFile.close(); // Flush to disk

    // Run the python script
    std::string scriptRunCommand = "python " + pythonScriptPath;
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

template <typename Fmt, typename... Args>
void testPackAgainstPython(Fmt, const Args&... toPack) {
    //auto packed = bitpacker::pack(Fmt{}, toPack...);
    auto pythonPacked = runPythonPack(Fmt{}, toPack...);

    //CAPTURE(packed);
    CAPTURE(pythonPacked);

    //REQUIRE(packed.size() == pythonPacked.size());
    //REQUIRE(std::equal(packed.begin(), packed.end(), pythonPacked.begin()));
    bitpacker::span<const bitpacker::byte_type> buffer(reinterpret_cast<uint8_t*>(pythonPacked.data()), pythonPacked.size() );
    auto unpacked = bitpacker::unpack(Fmt{}, buffer);
    REQUIRE( unpacked == std::make_tuple(toPack...) );
}

TEST_CASE("integers - single items", "[bitpacker::binary_compat]") {
    testPackAgainstPython(BP_STRING("u4u4u8u3u5"), 5, 2, 0xff, 5, 0b11010U);
}
