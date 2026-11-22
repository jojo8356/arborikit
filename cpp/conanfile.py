"""Recette Conan 2 pour la lib C++ arborikit (header-only).

Test local :
    conan create cpp/ --version=0.1.0
"""
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy
import os


class ArborikitCppConan(ConanFile):
    name = "arborikit-cpp"
    version = "0.1.0"
    license = "MIT"
    url = "https://github.com/VOTRE_COMPTE/arborikit"
    homepage = "https://github.com/VOTRE_COMPTE/arborikit"
    description = "General trees, unbalanced BSTs and red-black trees for C++ (header-only)"
    topics = ("tree", "bst", "red-black", "header-only", "data-structures")
    package_type = "header-library"
    settings = "os", "arch", "compiler", "build_type"
    exports_sources = "CMakeLists.txt", "include/*", "cmake/*", "tests/*"
    no_copy_source = True

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["ARBORIKIT_BUILD_TESTS"] = False
        tc.generate()

    def package_id(self):
        self.info.clear()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()
        copy(self, "LICENSE", src=self.recipe_folder + "/..", dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "arborikit_cpp")
        self.cpp_info.set_property("cmake_target_name", "arborikit::cpp")
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
