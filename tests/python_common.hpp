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

template < typename T >
std::string escapeString(const T &val)
{
    std::string out = "\"";
    for (auto c : val) {
        if (isgraph(c)) {
            out.push_back(c);
        }
        else if (isprint(c)) {
            switch (c) {
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
            s << std::setfill('0') << std::setw(2) << static_cast< int >(c);
            out += "\\" + s.str();
        }
    }

    out += '"';
    return out;
}

// used to output values for python packing
template < typename T >
std::string convertToString(const T &val)
{
    if constexpr (std::is_same_v< T, char >) {
        return "chr(" + std::to_string(val) + ")";
    }
    if constexpr (std::is_integral_v< T >) {
        return std::to_string(val);
    }
    if constexpr (std::is_convertible_v< T, std::string >) {
        return "b" + escapeString(val);
    }
    if constexpr (std::is_floating_point_v< T >) {
        std::stringstream ss;
        ss << std::scientific << std::setprecision(16) << val;
        return ss.str();
    }
    return Catch::StringMaker< T >::convert(val);
}

template < typename T , size_t N>
std::string convertToString(std::array< T, N > buff)
{
    std::stringstream ss;
    ss << "b'";
    for (const auto &v : buff) {
        ss << "\\x" << std::hex << static_cast< int >(v);
    }
    ss << "'";
    return ss.str();
}


template < typename Fmt, typename... Args >
std::string buildPythonScript(Fmt, const std::string &outputPath, const Args &... toPack)
{
    std::string pythonScript = R"py(
# Workaround for Catch::StringMaker<bool>
true = True
false = False
import bitstruct
bin = bitstruct.pack(")py" + std::string(Fmt::value()) +
                               R"py(")py";

    std::string args[] = {convertToString(toPack)...};
    for (const auto &arg : args) {
        pythonScript += ", ";
        pythonScript += arg;
    }

    pythonScript += R"py()
with open(r')py" + outputPath +
                    R"py(', 'wb') as f:
    f.write(bin)
    )py";

    return pythonScript;
}


template < typename Fmt, typename... Args >
auto runPythonPack(Fmt, const Args &... toPack)
{
    // Create the python script
    std::string packedBinaryFilePath = std::tmpnam(nullptr);
    std::string pythonScript = buildPythonScript(Fmt{}, packedBinaryFilePath, toPack...);

    // Write the python script
    std::string pythonScriptPath = std::tmpnam(nullptr);
    auto pythonScriptFile = std::ofstream(pythonScriptPath, std::ios::binary);
    pythonScriptFile.write(pythonScript.data(), pythonScript.size());
    pythonScriptFile.close();  // Flush to disk

    std::string scriptRunCommand = "python";
    const char *pyName = getenv("CI_PYTHON_NAME");
    if (pyName != nullptr) {
        scriptRunCommand = std::string(pyName);
    }

    // Run the python script
    scriptRunCommand += " " + pythonScriptPath;
    auto retVal = system(scriptRunCommand.c_str());
    if (retVal != 0) {
        FAIL("Python run failed with error code " << retVal);
    }

    // Read the python script output
    std::ifstream inputFile(packedBinaryFilePath, std::ios::binary);
    inputFile = std::ifstream(packedBinaryFilePath, std::ios::binary);
    std::vector< char > outputBuffer(
        (std::istreambuf_iterator< char >(inputFile)),
        (std::istreambuf_iterator< char >()));
    inputFile.close();

    // Remove script+output files
    remove(pythonScriptPath.c_str());
    remove(packedBinaryFilePath.c_str());

    return outputBuffer;
}

inline std::string print_data_vector(const std::vector< char > &vec)
{
    std::stringstream ss;
    ss << "{";
    for (const auto &v : vec) {
        ss << " 0x" << std::hex << (static_cast< unsigned >(v) & 0xFFU) << ",";
    }
    ss << " }";
    return ss.str();
}

template < typename T >
std::string print_tuple_value(const T v)
{
    std::stringstream ss;
    if constexpr (std::is_same_v< T, char > || std::is_same_v< T, unsigned char >) {
        ss << static_cast< int >(v);
    }
    else {
        ss << v;
    }
    return ss.str();
}

// specialization for printing 'r' format return value
template < size_t N >
std::string print_tuple_value(const std::array<bitpacker::byte_type, N> buff)
{
    std::stringstream ss;
    ss << "{ ";
    for ( const auto& v : buff ) {
        ss << std::hex << static_cast< int >(v);
    }
    ss << " }";
    return ss.str();
}

template < class TupType, size_t... I >
std::string print_data_tuple(const TupType &_tup, std::index_sequence< I... >)
{
    std::stringstream ss;
    ss << "{ ";
    (..., (ss << (I == 0 ? "" : ", ") << std::hex
              << print_tuple_value(std::get< I >(_tup))));
    ss << " }";
    return ss.str();
}

template < class... T >
std::string print_data_tuple(const std::tuple< T... > &_tup)
{
    return print_data_tuple(_tup, std::make_index_sequence< sizeof...(T) >());
}

template < typename Fmt, typename... Args >
void testPackAgainstPython(Fmt, const Args &... toPack)
{
    constexpr auto formats = bitpacker::impl::get_type_array(Fmt{});
    using FormatTypes = decltype(bitpacker::impl::FormatTypes<>(Fmt{}));
    using ReturnTypes = decltype(bitpacker::impl::ReturnTypes<>(Fmt{}));

    //auto packed = bitpacker::pack(Fmt{}, toPack...);
    auto pythonPacked = runPythonPack(Fmt{}, toPack...);

    //CAPTURE(packed);
    INFO("Format String: " << Fmt::value())
    INFO("From Python-pack: " << print_data_vector(pythonPacked));

    //REQUIRE(packed.size() == pythonPacked.size());
    //REQUIRE(std::equal(packed.begin(), packed.end(), pythonPacked.begin()));

    bitpacker::span< const bitpacker::byte_type > buffer(reinterpret_cast< uint8_t * >(pythonPacked.data()), pythonPacked.size());
    auto unpacked = bitpacker::unpack(Fmt{}, buffer);

    INFO("Unpacked by Bitpacker: " << print_data_tuple(unpacked));
    INFO("Expected (uncast): " << print_data_tuple(std::make_tuple(toPack...)));

    // explicitly creating tuple of ReturnTypes will make tests pass if the value is implicitly convertable to the expected return type
    // for example: multi-bit bool values
    REQUIRE(unpacked == ReturnTypes{toPack...});
}
