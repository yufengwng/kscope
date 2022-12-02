# kscope

yet another implementation of the llvm [kaleidoscope][llvm-kscope] language

[llvm-kscope]: https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html

## getting started

First, install or build LLVM 15. Then setup and compile the project:

```console
$ mkdir build && cd build
$ cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=true [-DLLVM_DIR=/path/to/lib/cmake/llvm ...]
$ ninja
```
