add_library(kscope SHARED
  ast.cpp
  lexer.cpp
  parser.cpp
)

target_include_directories(kscope PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
