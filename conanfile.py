from conan import ConanFile

class Audio2WaveformConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("boost/1.84.0")
        self.requires("ffmpeg/4.4.4")
        self.requires("nlohmann_json/3.11.3")
        self.requires("opencv/4.9.0")
        
    def build_requirements(self):
        self.tool_requires("cmake/3.28.1")
        