from conans import ConanFile, CMake, tools

class DbPostgreSQLAdapterConan(ConanFile):
    name = "DbPostgreSQLAdapter"
    description = "Implementation of C++ DB adapter based on PostgreSQL"
    url = "https://github.com/systelab/cpp-postgresql-db-adapter"
    homepage = "https://github.com/systelab/cpp-postgresql-db-adapter"
    author = "CSW <csw@werfen.com>"
    topics = ("conan", "db", "sql", "postgreSQL", "adapter")
    license = "MIT"
    generators = "cmake_find_package"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "*", "!*.yml", "!*.md", "!.gitattributes", "!.gitignore", "!LICENSE"

    def requirements(self):
        self.requires("DbAdapterInterface/0.0.0@systelab/testing")
        self.requires("openssl/3.0.13@", override=True)
        self.requires("libpq/15.4@")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=".")
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst=("bin/%s" % self.settings.build_type), src="bin")
        self.copy("*.dll", dst=("bin/%s" % self.settings.build_type), src="lib")
        self.copy("*.dylib*", dst=("bin/%s" % self.settings.build_type), src="lib")
        self.copy("*.so*", dst=("bin/%s" % self.settings.build_type), src="lib")

    def package(self):
        self.copy("*DbPostgreSQLAdapter.lib", dst="lib", keep_path=False)
        self.copy("*DbPostgreSQLAdapter.pdb", dst="lib", keep_path=False)
        self.copy("*DbPostgreSQLAdapter.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
