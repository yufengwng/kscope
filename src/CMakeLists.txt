add_executable(kscope-bin main.cpp)
target_include_directories(kscope-bin PUBLIC ${PROJECT_SOURCE_DIR/lib})
target_link_libraries(kscope-bin LINK_PUBLIC kscope)
set_target_properties(kscope-bin PROPERTIES OUTPUT_NAME kscope)
