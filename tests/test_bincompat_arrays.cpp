#include "python_common.hpp"

TEST_CASE("compare to python pack: byte array", "[bitpacker::binary_compat]")
{
    testPackAgainstPython(BP_STRING("r8"), std::array<bitpacker::byte_type, 1>{'g'});
}

TEST_CASE("compare to python pack: byte arrays", "[bitpacker::binary_compat]")
{
    const uint8_t s[] = "gehs";
    testPackAgainstPython(BP_STRING("r16"), s);
    testPackAgainstPython(BP_STRING("<r16"), s);

    testPackAgainstPython(BP_STRING("r32"), s);
    testPackAgainstPython(BP_STRING("<r32"), s);

    const uint8_t name[] = "crustyauklet";
    testPackAgainstPython(BP_STRING("r96"), name);
    testPackAgainstPython(BP_STRING("<r96"), name);
}

TEST_CASE("compare to python pack: text arrays", "[bitpacker::binary_compat]")
{
    testPackAgainstPython(BP_STRING("t16"), "ge");
    testPackAgainstPython(BP_STRING("<t16"), "ge");

    testPackAgainstPython(BP_STRING("t32"), "gehs");
    testPackAgainstPython(BP_STRING("<t32"), "gehs");

    testPackAgainstPython(BP_STRING("t96"), "crustyauklet");
    testPackAgainstPython(BP_STRING("<t96"), "crustyauklet");
}

TEST_CASE("compare to python pack: text arrays mixed with integers", "[bitpacker::binary_compat]")
{
    testPackAgainstPython(BP_STRING("u5u3t16s8"), 25, 5, "ge", -45);
    testPackAgainstPython(BP_STRING("u5t16s8"), 25, "ge", -45);

    // string of size != modulo 8? python impl will do it. H == 0x48 so we only need upper 5 bits
    testPackAgainstPython(BP_STRING("u5t21s8"), 25, "geH", -45);
    testPackAgainstPython(BP_STRING("u3t18s12"), 7, "ge@", -45); // @ == 0x40, upper 2 bits
}

TEST_CASE("compare to python pack: padding", "[bitpacker::binary_compat]")
{
    testPackAgainstPython(BP_STRING("u5p3u8"), 25, 255);
    testPackAgainstPython(BP_STRING("u5p13u8"), 25, 255);
    testPackAgainstPython(BP_STRING("u5P3u8"), 25, 255);
    testPackAgainstPython(BP_STRING("u5P13u8"), 25, 255);
}