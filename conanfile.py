from conans import ConanFile

class zikhron(ConanFile):
    name = "zikhron"
    version = "0.1"
    requires = (
        "benchmark/1.6.0",
        "catch2/2.13.7",
        "fmt/8.1.1",
        "spdlog/1.9.2",
        "ctre/3.4.1",
        "ms-gsl/3.1.0",
        "nlohmann_json/3.10.4"
    )
    generators = "cmake", "gcc", "txt", "cmake_find_package"
