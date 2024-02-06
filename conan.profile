[settings]
os=Windows
os_build=Windows
arch=x86_64
arch_build=x86_64
compiler=Visual Studio
compiler.version=16
build_type=Release

libpq:compiler.version=17
openssl:compiler.version=17


[options]
libpq:shared=True
libpq:with_openssl=True
openssl:shared=True


[build_requires]

[env]
