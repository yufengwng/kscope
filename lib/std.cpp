// ===-----------------===
// kscope standard library
// ===-----------------===

#include <iostream>

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar a double and return 0.
extern "C" DLLEXPORT double putchard(double x) {
  fputc((char) x, stderr);
  return 0;
}

/// printd - print a double with newline and return 0.
extern "C" DLLEXPORT double printd(double x) {
  fprintf(stderr, "%f\n", x);
  return 0;
}
