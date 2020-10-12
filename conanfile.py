import os

from conans import ConanFile, tools

class BitpackerConan(ConanFile):
    requires = "span-lite/0.7.0"
    name = "bitpacker"
    description = "type-safe and low-boilerplate bit level serialization"
    topics = ("bitpacker", "header-only", "serialization", "bitfield")
    url = "https://github.com/CrustyAuklet/bitpacker"
    version = "0.1"
    license = "BSL-1.0"
    author = "Ethan Slattery CrustyAuklet@gmail.com"
    no_copy_source = True
    exports_sources = ("include/*", "CMakeLists.txt")
    # No settings/options are necessary, this is header only

    # def source(self):
    #     # self.run("git clone ...") or
    #     # tools.download("url", "file.zip")
    #     # tools.unzip("file.zip" )

    def package(self):
        self.copy("*.hpp")
        self.copy("CMakeLists.txt")

    def package_id(self):
        self.info.header_only()
