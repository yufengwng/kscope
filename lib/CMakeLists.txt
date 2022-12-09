add_library(kscope SHARED
  ast.cpp
  emitter.cpp
  executor.cpp
  lexer.cpp
  parser.cpp
)

llvm_map_components_to_libnames(llvm_libs
  core
  support
  orcjit
  native
)

target_include_directories(kscope PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kscope LINK_PUBLIC ${llvm_libs})
