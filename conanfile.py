from conans import ConanFile, CMake, tools


class BitpackerConan(ConanFile):
    name = "bitpacker"
    version = "0.1"
    license = "BSL-1.0"
    author = "Ethan Slattery CrustyAuklet@gmail.com"
    url = "https://github.com/CrustyAuklet/bitpacker"
    homepage = "https://github.com/CrustyAuklet/bitpacker"
    description = "type-safe and low-boilerplate bit level serialization"
    topics = ("header-only", "serialization", "bitfield")
    no_copy_source = True
    settings = "os", "compiler", "arch", "build_type"
    generators = ["cmake_find_package", "cmake_paths"]
    scm = {
        "type": "git",
        "subfolder": "bitpacker",
        "url": "auto",
        "revision": "auto"
    }

    def package(self):
        self.copy(pattern="LICENSE", dst="licenses")
        self.copy(pattern="*.hpp", dst="include", src="bitpacker/include")

    def package_id(self):
        self.info.header_only()

    def validate(self):
        tools.check_min_cppstd(self, "11")

    def requirements(self):
        if not tools.valid_min_cppstd(self, "20"):
            self.requires("span-lite/0.10.1")

    def build_requirements(self):
        if tools.get_env("CONAN_RUN_TESTS", True) or self.develop:
            self.build_requires("catch2/2.13.7")

    def build(self):  # this is not building a library, just tests
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure(source_folder='bitpacker', build_folder='bitpacker')
        #cmake.build()
        #cmake.test()

    def package_info(self):
        self.cpp_info.names["cmake_find_package"] = "bitpacker"
        self.cpp_info.names["cmake_find_package_multi"] = "bitpacker"
