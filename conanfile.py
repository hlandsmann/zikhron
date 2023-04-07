from conans import ConanFile


class zikhron(ConanFile):
    name = "zikhron"
    version = "0.1"
    requires = (
        "benchmark/1.6.0",
        "catch2/3.0.1",
        "fmt/9.1.0",
        "spdlog/1.11.0",
        "ctre/3.7.2",
        "ms-gsl/4.0.0",
        "nlohmann_json/3.11.2",
    )
    generators = "cmake", "gcc", "txt", "cmake_find_package"
    include_prerelease = True
