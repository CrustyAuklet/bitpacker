#include "python_common.hpp"

TEST_CASE("compare to python pack: single unsigned integers", "[bitpacker::binary_compat]") {
    testPackAgainstPython(BP_STRING("u1"), 0b1U);
    testPackAgainstPython(BP_STRING("u1"), 0b0U);
    testPackAgainstPython(BP_STRING("u4"), 0b1010U);
    testPackAgainstPython(BP_STRING("u10"), 0b11'1010'0101U);
    testPackAgainstPython(BP_STRING("u12"), 0b1001'1010'0101U);
    testPackAgainstPython(BP_STRING("u30"), 0b10'1001'1010'0110'0101'0011'1100'0110U);
    testPackAgainstPython(BP_STRING("u43"), 0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U);
    testPackAgainstPython(BP_STRING("u64"), 0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python pack: single unsigned integers little-bit-endian", "[bitpacker::binary_compat]")
{
    testPackAgainstPython(BP_STRING("<u1"), 0b1U);
    testPackAgainstPython(BP_STRING("<u1"), 0b0U);
    testPackAgainstPython(BP_STRING("<u4"), 0b1010U);
    testPackAgainstPython(BP_STRING("<u10"), 0b11'1010'0101U);
    testPackAgainstPython(BP_STRING("<u12"), 0b1001'1010'0101U);
    testPackAgainstPython(BP_STRING("<u30"), 0b10'1001'1010'0110'0101'0011'1100'0110U);
    testPackAgainstPython(BP_STRING("<u43"), 0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U);
    testPackAgainstPython(BP_STRING("<u64"), 0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python pack: multiple unsigned integers", "[bitpacker::binary_compat]") {
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

TEST_CASE("compare to python pack: multiple unsigned little endian integers", "[bitpacker::binary_compat]")
{
    testPackAgainstPython(BP_STRING("<u4u4u8u3u5"), 5, 2, 0xff, 5, 0b11010U);

    testPackAgainstPython(BP_STRING("<u4u10u12"), 0b1010U, 0b11'1010'0101U, 0b1001'1010'0101U);

    testPackAgainstPython(BP_STRING("<u4u10u12u30u43u64"),
                          0b1010U,
                          0b11'1010'0101U,
                          0b1001'1010'0101U,
                          0b10'1001'1010'0110'0101'0011'1100'0110U,
                          0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U,
                          0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python pack: multiple unsigned mixed endian integers", "[bitpacker::binary_compat]")
{
    testPackAgainstPython(BP_STRING("<u4u4u8>u3u5"), 5, 2, 0xff, 5, 0b11010U);

    testPackAgainstPython(BP_STRING("<u4>u10<u12"), 0b1010U, 0b11'1010'0101U, 0b1001'1010'0101U);

    testPackAgainstPython(BP_STRING("<u4u10>u12<u30u43>u64"),
                          0b1010U,
                          0b11'1010'0101U,
                          0b1001'1010'0101U,
                          0b10'1001'1010'0110'0101'0011'1100'0110U,
                          0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U,
                          0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python pack: single signed integers", "[bitpacker::binary_compat]") {
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

TEST_CASE("compare to python pack: single signed little endian integers", "[bitpacker::binary_compat]")
{
    // -1 creates all 1's
    testPackAgainstPython(BP_STRING("<s1"), -1);
    testPackAgainstPython(BP_STRING("<s4"), -1);
    testPackAgainstPython(BP_STRING("<s10"), -1);
    testPackAgainstPython(BP_STRING("<s12"), -1);
    testPackAgainstPython(BP_STRING("<s30"), -1);
    testPackAgainstPython(BP_STRING("<s43"), -1);
    testPackAgainstPython(BP_STRING("<s64"), -1);

    // random negative values, somewhat large
    testPackAgainstPython(BP_STRING("<s4"), -5);
    testPackAgainstPython(BP_STRING("<s10"), -500);
    testPackAgainstPython(BP_STRING("<s12"), -1040);
    testPackAgainstPython(BP_STRING("<s30"), -536870911);
    testPackAgainstPython(BP_STRING("<s43"), -4398046509981ll);
    testPackAgainstPython(BP_STRING("<s64"), -9223372036742463338ll);

    // some positive values
    testPackAgainstPython(BP_STRING("<s1"), 0);
    testPackAgainstPython(BP_STRING("<s4"), 5);
    testPackAgainstPython(BP_STRING("<s10"), 500);
    testPackAgainstPython(BP_STRING("<s12"), 2'000);
    testPackAgainstPython(BP_STRING("<s30"), 536'870'000);
    testPackAgainstPython(BP_STRING("<s43"), 4'358'146'411'000ll);
    testPackAgainstPython(BP_STRING("<s64"), 8'213'362'026'654'675'200ll);
}

TEST_CASE("compare to python pack: multiple signed integers", "[bitpacker::binary_compat]") {
    testPackAgainstPython(BP_STRING("s4s4s8s3s5"), -8, -5, -115, -3, -10);

    testPackAgainstPython(BP_STRING("s4s10s12s30s43s64"), -1, -1, -1, -1, -1, -1);

    testPackAgainstPython(BP_STRING("s4s10s12s30s43s64"), 
        -5, -500, -1040, -536'870'911, -4'398'046'509'981ll, -9'223'372'036'742'463'338ll);

    testPackAgainstPython(BP_STRING("s4s10s12s30s43s64"),
        5, 500, 2'000, 536'870'000, 4'358'146'411'000ll, 8'213'362'026'654'675'200ll);

    testPackAgainstPython(BP_STRING("s4s2s10s12s30s43s64s4s10s12"),
        5, -1, 500, -2'000, 536'870'000, -4'358'146'411'000ll, 8'213'362'026'654'675'200ll, -5, 500, -2'000);
}

TEST_CASE("compare to python pack: multiple signed little endian integers", "[bitpacker::binary_compat]")
{
    testPackAgainstPython(BP_STRING("<s4s4s8s3s5"), -8, -5, -115, -3, -10);

    testPackAgainstPython(BP_STRING("<s4s10s12s30s43s64"), -1, -1, -1, -1, -1, -1);

    testPackAgainstPython(BP_STRING("<s4s10s12s30s43s64"),
                          -5, -500, -1040, -536'870'911, -4'398'046'509'981ll, -9'223'372'036'742'463'338ll);

    testPackAgainstPython(BP_STRING("<s4s10s12s30s43s64"),
                          5, 500, 2'000, 536'870'000, 4'358'146'411'000ll, 8'213'362'026'654'675'200ll);

    testPackAgainstPython(BP_STRING("<s4s2s10s12s30s43s64s4s10s12"),
                          5, -1, 500, -2'000, 536'870'000, -4'358'146'411'000ll, 8'213'362'026'654'675'200ll, -5, 500, -2'000);
}

TEST_CASE("compare to python pack: multiple signed mixed endian integers", "[bitpacker::binary_compat]")
{
    testPackAgainstPython(BP_STRING("<s4s4>s8s3s5"), -8, -5, -115, -3, -10);

    testPackAgainstPython(BP_STRING("<s4s10>s12<s30s43>s64"), -1, -1, -1, -1, -1, -1);

    testPackAgainstPython(BP_STRING("<s4s10>s12s30<s43s64"),
                          -5, -500, -1040, -536'870'911, -4'398'046'509'981ll, -9'223'372'036'742'463'338ll);

    testPackAgainstPython(BP_STRING(">s4s10<s12>s30<s43s64"),
                          5, 500, 2'000, 536'870'000, 4'358'146'411'000ll, 8'213'362'026'654'675'200ll);

    testPackAgainstPython(BP_STRING("<s4s2>s10<s12s30>s43s64<s4s10s12"),
                          5, -1, 500, -2'000, 536'870'000, -4'358'146'411'000ll, 8'213'362'026'654'675'200ll, -5, 500, -2'000);
}

TEST_CASE("compare to python pack: bool", "[bitpacker::binary_compat]")
{
    testPackAgainstPython(BP_STRING("b1"), true);
    testPackAgainstPython(BP_STRING("b1"), false);
    testPackAgainstPython(BP_STRING("b1"), 1);
    testPackAgainstPython(BP_STRING("b1"), 0);
    
    testPackAgainstPython(BP_STRING("b4"), 0b1010U);
    testPackAgainstPython(BP_STRING("b10"), 0b11'1010'0101U);
    testPackAgainstPython(BP_STRING("b12"), 0b1001'1010'0101U);
    testPackAgainstPython(BP_STRING("b30"), 0b10'1001'1010'0110'0101'0011'1100'0110U);
    testPackAgainstPython(BP_STRING("b43"), 0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U);
    testPackAgainstPython(BP_STRING("b64"), 0xDEADBEEFCAFEBABE);
}
