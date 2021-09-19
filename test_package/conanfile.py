import os
from io import StringIO
from conans import ConanFile, CMake, tools

class BitpackerTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def build(self):
        # Current dir is "test_package/build/<build_id>" and CMakeLists.txt is in "test_package"
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        if not tools.cross_building(self.settings):
            # Current dir is "test_package/build/<build_id>"
            output = StringIO()
            bin_path = os.path.sep.join([".", "bin", "example"])
            self.run(bin_path, output=output)
            text = output.getvalue()
            print(f'Bitpacker test package output: {text}')
            assert('0xce, 0x49, 0x19, 0x4f, 0xfc, 0xf7, 0x90' in text)
