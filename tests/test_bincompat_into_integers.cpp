#include "python_common.hpp"

TEST_CASE("compare to python pack_into: single unsigned integers", "[bitpacker::binary_compat]") {
    testPackIntoAgainstPython(BP_STRING("u1"), 3, 0b1U);
    testPackIntoAgainstPython(BP_STRING("u1"), 3, 0b0U);
    testPackIntoAgainstPython(BP_STRING("u4"), 3, 0b1010U);
    testPackIntoAgainstPython(BP_STRING("u10"), 5, 0b11'1010'0101U);
    testPackIntoAgainstPython(BP_STRING("u12"), 7, 0b1001'1010'0101U);
    testPackIntoAgainstPython(BP_STRING("u30"), 12, 0b10'1001'1010'0110'0101'0011'1100'0110U);
    testPackIntoAgainstPython(BP_STRING("u43"), 12, 0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U);
    testPackIntoAgainstPython(BP_STRING("u64"), 12, 0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python pack_into: single unsigned integers little-bit-endian", "[bitpacker::binary_compat]")
{
    testPackIntoAgainstPython(BP_STRING("<u1") , 5,  0b1U);
    testPackIntoAgainstPython(BP_STRING("<u1") , 5,  0b0U);
    testPackIntoAgainstPython(BP_STRING("<u4") , 5,  0b1010U);
    testPackIntoAgainstPython(BP_STRING("<u10"), 5,  0b11'1010'0101U);
    testPackIntoAgainstPython(BP_STRING("<u12"), 5,  0b1001'1010'0101U);
    testPackIntoAgainstPython(BP_STRING("<u30"), 5,  0b10'1001'1010'0110'0101'0011'1100'0110U);
    testPackIntoAgainstPython(BP_STRING("<u43"), 5,  0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U);
    testPackIntoAgainstPython(BP_STRING("<u64"), 5,  0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python pack_into: multiple unsigned integers", "[bitpacker::binary_compat]") {
    testPackIntoAgainstPython(BP_STRING("u4u4u8u3u5"), 6,  5, 2, 0xff, 5, 0b11010U);

    testPackIntoAgainstPython(BP_STRING("u4u10u12"), 6, 0b1010U, 0b11'1010'0101U, 0b1001'1010'0101U);

    testPackIntoAgainstPython(BP_STRING("u4u10u12u30u43u64"), 7,
                          0b1010U,
                          0b11'1010'0101U,
                          0b1001'1010'0101U,
                          0b10'1001'1010'0110'0101'0011'1100'0110U,
                          0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U,
                          0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python pack_into: multiple unsigned little endian integers", "[bitpacker::binary_compat]")
{
    testPackIntoAgainstPython(BP_STRING("<u4u4u8u3u5"), 9, 5, 2, 0xff, 5, 0b11010U);

    testPackIntoAgainstPython(BP_STRING("<u4u10u12"), 4, 0b1010U, 0b11'1010'0101U, 0b1001'1010'0101U);

    testPackIntoAgainstPython(BP_STRING("<u4u10u12u30u43u64"), 5,
                          0b1010U,
                          0b11'1010'0101U,
                          0b1001'1010'0101U,
                          0b10'1001'1010'0110'0101'0011'1100'0110U,
                          0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U,
                          0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python pack_into: multiple unsigned mixed endian integers", "[bitpacker::binary_compat]")
{
    testPackIntoAgainstPython(BP_STRING("<u4u4u8>u3u5"), 5, 5, 2, 0xff, 5, 0b11010U);

    testPackIntoAgainstPython(BP_STRING("<u4>u10<u12"), 3, 0b1010U, 0b11'1010'0101U, 0b1001'1010'0101U);

    testPackIntoAgainstPython(BP_STRING("<u4u10>u12<u30u43>u64"), 6,
                          0b1010U,
                          0b11'1010'0101U,
                          0b1001'1010'0101U,
                          0b10'1001'1010'0110'0101'0011'1100'0110U,
                          0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U,
                          0xDEADBEEFCAFEBABE);
}

TEST_CASE("compare to python pack_into: single signed integers", "[bitpacker::binary_compat]") {
    // -1 creates all 1's
    testPackIntoAgainstPython(BP_STRING("s1") , 3, -1);
    testPackIntoAgainstPython(BP_STRING("s4") , 3, -1);
    testPackIntoAgainstPython(BP_STRING("s10"), 3, -1);
    testPackIntoAgainstPython(BP_STRING("s12"), 3, -1);
    testPackIntoAgainstPython(BP_STRING("s30"), 3, -1);
    testPackIntoAgainstPython(BP_STRING("s43"), 3, -1);
    testPackIntoAgainstPython(BP_STRING("s64"), 3, -1);

    // random negative values, somewhat large
    testPackIntoAgainstPython(BP_STRING("s4") , 9, -5);
    testPackIntoAgainstPython(BP_STRING("s10"), 9, -500);
    testPackIntoAgainstPython(BP_STRING("s12"), 9, -1040);
    testPackIntoAgainstPython(BP_STRING("s30"), 9, -536870911);
    testPackIntoAgainstPython(BP_STRING("s43"), 9, -4398046509981ll);
    testPackIntoAgainstPython(BP_STRING("s64"), 9, -9223372036742463338ll);

    // some positive values
    testPackIntoAgainstPython(BP_STRING("s1") , 7, 0);
    testPackIntoAgainstPython(BP_STRING("s4") , 7, 5);
    testPackIntoAgainstPython(BP_STRING("s10"), 7, 500);
    testPackIntoAgainstPython(BP_STRING("s12"), 7, 2'000);
    testPackIntoAgainstPython(BP_STRING("s30"), 7, 536'870'000);
    testPackIntoAgainstPython(BP_STRING("s43"), 7, 4'358'146'411'000ll);
    testPackIntoAgainstPython(BP_STRING("s64"), 7, 8'213'362'026'654'675'200ll);
}

TEST_CASE("compare to python pack_into: single signed little endian integers", "[bitpacker::binary_compat]")
{
    // -1 creates all 1's
    testPackIntoAgainstPython(BP_STRING("<s1") , 3, -1);
    testPackIntoAgainstPython(BP_STRING("<s4") , 3, -1);
    testPackIntoAgainstPython(BP_STRING("<s10"), 3, -1);
    testPackIntoAgainstPython(BP_STRING("<s12"), 3, -1);
    testPackIntoAgainstPython(BP_STRING("<s30"), 3, -1);
    testPackIntoAgainstPython(BP_STRING("<s43"), 3, -1);
    testPackIntoAgainstPython(BP_STRING("<s64"), 3, -1);

    // random negative values, somewhat large
    testPackIntoAgainstPython(BP_STRING("<s4") , 3, -5);
    testPackIntoAgainstPython(BP_STRING("<s10"), 3, -500);
    testPackIntoAgainstPython(BP_STRING("<s12"), 3, -1040);
    testPackIntoAgainstPython(BP_STRING("<s30"), 3, -536870911);
    testPackIntoAgainstPython(BP_STRING("<s43"), 3, -4398046509981ll);
    testPackIntoAgainstPython(BP_STRING("<s64"), 3, -9223372036742463338ll);

    // some positive values
    testPackIntoAgainstPython(BP_STRING("<s1") , 3, 0);
    testPackIntoAgainstPython(BP_STRING("<s4") , 3, 5);
    testPackIntoAgainstPython(BP_STRING("<s10"), 3, 500);
    testPackIntoAgainstPython(BP_STRING("<s12"), 3, 2'000);
    testPackIntoAgainstPython(BP_STRING("<s30"), 3, 536'870'000);
    testPackIntoAgainstPython(BP_STRING("<s43"), 3, 4'358'146'411'000ll);
    testPackIntoAgainstPython(BP_STRING("<s64"), 3, 8'213'362'026'654'675'200ll);
}

TEST_CASE("compare to python pack_into: multiple signed integers", "[bitpacker::binary_compat]") {
    testPackIntoAgainstPython(BP_STRING("s4s4s8s3s5"), 3, -8, -5, -115, -3, -10);

    testPackIntoAgainstPython(BP_STRING("s4s10s12s30s43s64"), 5, -1, -1, -1, -1, -1, -1);

    testPackIntoAgainstPython(BP_STRING("s4s10s12s30s43s64"), 7,
                          -5, -500, -1040, -536'870'911, -4'398'046'509'981ll, -9'223'372'036'742'463'338ll);

    testPackIntoAgainstPython(BP_STRING("s4s10s12s30s43s64"), 7,
                          5, 500, 2'000, 536'870'000, 4'358'146'411'000ll, 8'213'362'026'654'675'200ll);

    testPackIntoAgainstPython(BP_STRING("s4s2s10s12s30s43s64s4s10s12"), 4,
                          5, -1, 500, -2'000, 536'870'000, -4'358'146'411'000ll, 8'213'362'026'654'675'200ll, -5, 500, -2'000);
}

TEST_CASE("compare to python pack_into: multiple signed little endian integers", "[bitpacker::binary_compat]")
{
    testPackIntoAgainstPython(BP_STRING("<s4s4s8s3s5"), 3, -8, -5, -115, -3, -10);

    testPackIntoAgainstPython(BP_STRING("<s4s10s12s30s43s64"), 4, -1, -1, -1, -1, -1, -1);

    testPackIntoAgainstPython(BP_STRING("<s4s10s12s30s43s64"), 7,
                          -5, -500, -1040, -536'870'911, -4'398'046'509'981ll, -9'223'372'036'742'463'338ll);

    testPackIntoAgainstPython(BP_STRING("<s4s10s12s30s43s64"), 2,
                          5, 500, 2'000, 536'870'000, 4'358'146'411'000ll, 8'213'362'026'654'675'200ll);

    testPackIntoAgainstPython(BP_STRING("<s4s2s10s12s30s43s64s4s10s12"), 1,
                          5, -1, 500, -2'000, 536'870'000, -4'358'146'411'000ll, 8'213'362'026'654'675'200ll, -5, 500, -2'000);
}

TEST_CASE("compare to python pack_into: multiple signed mixed endian integers", "[bitpacker::binary_compat]")
{
    testPackIntoAgainstPython(BP_STRING("<s4s4>s8s3s5"), 3, -8, -5, -115, -3, -10);

    testPackIntoAgainstPython(BP_STRING("<s4s10>s12<s30s43>s64"), 4, -1, -1, -1, -1, -1, -1);

    testPackIntoAgainstPython(BP_STRING("<s4s10>s12s30<s43s64"), 5,
                          -5, -500, -1040, -536'870'911, -4'398'046'509'981ll, -9'223'372'036'742'463'338ll);

    testPackIntoAgainstPython(BP_STRING(">s4s10<s12>s30<s43s64"), 2,
                          5, 500, 2'000, 536'870'000, 4'358'146'411'000ll, 8'213'362'026'654'675'200ll);

    testPackIntoAgainstPython(BP_STRING("<s4s2>s10<s12s30>s43s64<s4s10s12"), 7,
                          5, -1, 500, -2'000, 536'870'000, -4'358'146'411'000ll, 8'213'362'026'654'675'200ll, -5, 500, -2'000);
}

TEST_CASE("compare to python pack_into: bool", "[bitpacker::binary_compat]")
{
    testPackIntoAgainstPython(BP_STRING("b1"), 2, true);
    testPackIntoAgainstPython(BP_STRING("b1"), 2, false);
    testPackIntoAgainstPython(BP_STRING("b1"), 2, 1);
    testPackIntoAgainstPython(BP_STRING("b1"), 2, 0);

    testPackIntoAgainstPython(BP_STRING("b4") , 5, 0b1010U);
    testPackIntoAgainstPython(BP_STRING("b10"), 5, 0b11'1010'0101U);
    testPackIntoAgainstPython(BP_STRING("b12"), 5, 0b1001'1010'0101U);
    testPackIntoAgainstPython(BP_STRING("b30"), 5, 0b10'1001'1010'0110'0101'0011'1100'0110U);
    testPackIntoAgainstPython(BP_STRING("b43"), 5, 0b110'1001'1010'0110'0101'0011'1100'0110'1001'1010'0101U);
    testPackIntoAgainstPython(BP_STRING("b64"), 5, 0xDEADBEEFCAFEBABE);
}
