#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace kscope {

template <class T>
using Box = std::unique_ptr<T>;

} // namespace kscope
