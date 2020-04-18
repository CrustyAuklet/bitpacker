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

template <typename T>
struct LocalStringMaker {
    static std::string convert(const T& val) {
        return Catch::StringMaker< T >::convert(val);
    }
};

template < int SZ >
struct LocalStringMaker< unsigned char[SZ] > {
    static std::string convert(unsigned char const *str) {
        return "b" + std::string(Catch::StringMaker< unsigned char[SZ] >::convert(str));
    }
};

template <>
struct LocalStringMaker< char > {
    static std::string convert(char c) {
        return "chr(" + std::to_string(c) + ")";
    }
};

// used to output values for python packing
template < typename T >
std::string convertToString(const T &val)
{
    if constexpr (std::is_integral_v< T >) {
        return std::to_string(val);
    }
    if constexpr (std::is_convertible_v< T, std::string >) {
        return escapeString(val);
    }
    if constexpr (std::is_floating_point_v< T >) {
        std::stringstream ss;
        ss << std::scientific << std::setprecision(16) << val;
        return ss.str();
    }
    return LocalStringMaker< T >::convert(val);
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
    ss << "{ ";
    for (const auto &v : vec) {
        ss << "0x" << std::hex << (static_cast< unsigned >(v) & 0xFFU) << ", ";
    }
    std::string retval = ss.str();
    return retval.substr(0, retval.size() - 2) + " }";
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
template < typename T, size_t N >
std::string print_tuple_value(const std::array<T, N> buff)
{
    static_assert(std::is_same_v< T, bitpacker::byte_type > || std::is_same_v< T, char > || std::is_same_v< T, uint8_t >);
    std::stringstream ss;
    ss << "{ ";
    for ( const auto& v : buff ) {
        ss << "0x" << std::hex << static_cast< int >(v) << ", ";
    }
    std::string retval = ss.str();
    return retval.substr(0, retval.size()-2) + " }";
}

template < class TupType, size_t... I >
std::string print_data_tuple(const TupType &_tup, std::index_sequence< I... >)
{
    std::stringstream ss;
    ss << "{ ";
    (..., (ss << (I == 0 ? "" : ", ") << std::hex << print_tuple_value(std::get< I >(_tup))));
    ss << " }";
    return ss.str();
}

template < class... T >
std::string print_data_tuple(const std::tuple< T... > &_tup)
{
    return print_data_tuple(_tup, std::make_index_sequence< sizeof...(T) >());
}

template <typename T, size_t N>
bool operator==(const std::array< T, N > &lhs, const T* rhs)
{
    static_assert(std::is_same_v< T, bitpacker::byte_type > || std::is_same_v< T, char > || std::is_same_v< T, uint8_t >);
    // assuming lhs ans rhs are the same length. if not... that is bad.
    // don't want to assume rhs is null terminated
    bool are_same = true;
    for (size_t i = 0; i < N; ++i) {
        are_same &= lhs[i] == static_cast< bitpacker::byte_type >(rhs[i]);
    }
    return are_same;
}

template <typename T1, typename T2>
bool bitpacker_test_compare(const T1 &lhs, const T2 &rhs)
{
    if constexpr ( std::is_same_v<T1, bool> && std::is_convertible_v< T2, bool >) {
        return lhs == static_cast< bool >(rhs);
    }
    return lhs == rhs;
}

template < std::size_t... Items, typename T1, typename T2 >
bool compareBitpackerTuples_impl(std::index_sequence< Items... >, const T1 &lhs, const T2 &rhs)
{
    return ( bitpacker_test_compare(std::get< Items >(lhs), std::get< Items >(rhs)) && ...);
}

template < typename T1, typename T2 >
bool compareBitpackerTuples(const T1& lhs, const T2& rhs)
{
    constexpr auto leftSize = std::tuple_size_v< T1 >;
    constexpr auto rightSize = std::tuple_size_v< T2 >;
    const bool same_size = leftSize == rightSize;

    return same_size && compareBitpackerTuples_impl( std::make_index_sequence< leftSize >(), lhs, rhs );
}

template < typename Fmt, typename... Args >
void testPackAgainstPython(Fmt, const Args &... toPack)
{
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
    const bool success = compareBitpackerTuples(unpacked, std::make_tuple(toPack...));
    REQUIRE(success);
}
