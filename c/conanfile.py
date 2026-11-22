"""Recette Conan 2 pour la lib C arborikit.

Publication sur ConanCenter : ouvrir une PR sur
https://github.com/conan-io/conan-center-index en copiant ce dossier.
Test local :
    conan create c/ --version=0.1.0
"""
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy
import os


class ArborikitCConan(ConanFile):
    name = "arborikit"
    version = "0.1.0"
    license = "MIT"
    url = "https://github.com/jojo8356/arborikit"
    homepage = "https://github.com/jojo8356/arborikit"
    description = "General trees, unbalanced BSTs and red-black trees for C"
    topics = ("tree", "bst", "red-black", "data-structures")
    package_type = "library"
    settings = "os", "arch", "compiler", "build_type"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "cmake/*", "tests/*", "LICENSE"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["ARBORIKIT_BUILD_TESTS"] = False
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "arborikit")
        self.cpp_info.set_property("cmake_target_name", "arborikit::arborikit")
        self.cpp_info.set_property("pkg_config_name", "arborikit")
        self.cpp_info.libs = ["arborikit"]
