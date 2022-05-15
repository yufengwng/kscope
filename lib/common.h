#pragma once

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include <memory>
#include <string>
#include <vector>

namespace kscope {

template <class T>
using Box = std::unique_ptr<T>;

} // namespace kscope
