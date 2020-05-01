#include "python_common.hpp"

TEST_CASE("compare to python pack_into: byte array", "[bitpacker::binary_compat]")
{
    testPackIntoAgainstPython(BP_STRING("r8"), 5, std::array<bitpacker::byte_type, 1>{'g'});
}

TEST_CASE("compare to python pack_into: byte arrays", "[bitpacker::binary_compat]")
{
    const uint8_t s[] = "gehs";
    testPackIntoAgainstPython(BP_STRING("r16"), 5, s);
    testPackIntoAgainstPython(BP_STRING("<r16"), 5, s);

    testPackIntoAgainstPython(BP_STRING("r32"), 13, s);
    testPackIntoAgainstPython(BP_STRING("<r32"), 13, s);

    const uint8_t name[] = "crustyauklet";
    testPackIntoAgainstPython(BP_STRING("r96"), 7, name);
    testPackIntoAgainstPython(BP_STRING("<r96"), 7, name);
}

TEST_CASE("compare to python pack_into: text arrays", "[bitpacker::binary_compat]")
{
    testPackIntoAgainstPython(BP_STRING("t16"), 5, "ge");
    testPackIntoAgainstPython(BP_STRING("<t16"), 5, "ge");

    testPackIntoAgainstPython(BP_STRING("t32"), 10, "gehs");
    testPackIntoAgainstPython(BP_STRING("<t32"), 10, "gehs");

    testPackIntoAgainstPython(BP_STRING("t96"), 7, "crustyauklet");
    testPackIntoAgainstPython(BP_STRING("<t96"), 7, "crustyauklet");
}

TEST_CASE("compare to python pack_into: text arrays mixed with integers", "[bitpacker::binary_compat]")
{
    testPackIntoAgainstPython(BP_STRING("u5u3t16s8"), 3, 25, 5, "ge", -45);
    testPackIntoAgainstPython(BP_STRING("u5t16s8"), 3, 25, "ge", -45);

    // string of size != modulo 8? python impl will do it. H == 0x48 so we only need upper 5 bits
    testPackIntoAgainstPython(BP_STRING("u5t21s8"), 9, 25, "geH", -45);
    testPackIntoAgainstPython(BP_STRING("u3t18s12"), 9, 7, "ge@", -45); // @ == 0x40, upper 2 bits
}

TEST_CASE("compare to python pack_into: padding", "[bitpacker::binary_compat]")
{
    testPackIntoAgainstPython(BP_STRING("u5p3u8"), 3, 25, 255);
    testPackIntoAgainstPython(BP_STRING("u5p13u8"), 7, 25, 255);
    testPackIntoAgainstPython(BP_STRING("u5P3u8"), 9, 25, 255);
    testPackIntoAgainstPython(BP_STRING("u5P13u8"), 13, 25, 255);
}