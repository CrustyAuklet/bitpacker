#include <iostream>
#include "bitpacker/bitpacker.hpp"

int main() {
    constexpr auto static_message = bitpacker::pack(BP_STRING("u12b1b1u14s24"), 3300, true, false, 4500, 16764793);
    std::cout << "{ ";
    for(size_t i = 0; i < static_message.size(); ++i) {
        std::cout << std::hex << "0x" << static_cast<int>(static_message[i]);
        if(i < static_message.size()-1) {
            std::cout << ", ";
        }
    }
    std::cout << " }";
}
