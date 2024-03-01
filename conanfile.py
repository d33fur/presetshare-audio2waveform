from conan import ConanFile
from conan.tools.cmake import cmake_layout

class Audio2WaveformConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("boost/1.84.0")
        # self.requires("ffmpeg/6.1")
        self.requires("nlohmann_json/3.11.3")
        
    def build_requirements(self):
        self.tool_requires("ccmake/3.28.1")
    # def configure(self):
    #     if self.settings.compiler == "gcc":
    #         self.settings.compiler.version = "10"
    #         self.settings.compiler.libcxx = "libstdc++11"

    # def build(self):
    #     cmake = CMake(self)
    #     cmake.configure()
    #     cmake.build()

    # def package(self):
    #     cmake = CMake(self)
    #     cmake.install()

    # def package_info(self):
    #     self.cpp_info.libs = tools.collect_libs(self)
