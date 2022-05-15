add_library(kscope SHARED
  ast.cpp
  emitter.cpp
  lexer.cpp
  parser.cpp
)

target_include_directories(kscope PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kscope LINK_PUBLIC LLVM)
