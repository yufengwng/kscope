# kscope

yet another implementation of the llvm [kaleidoscope][llvm-kscope] language

[llvm-kscope]: https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html

## getting started

First, install or build LLVM 15. Then setup and compile the project:

```console
$ cmake -S . -B build -G Ninja [-DLLVM_DIR=/path/to/lib/cmake/llvm ...]
$ make build
```
