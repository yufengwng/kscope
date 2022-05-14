add_library(kscope SHARED
  lexer.cpp
)

target_include_directories(kscope PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
