add_executable (tooling_sample tooling_sample.cpp)
target_link_libraries (tooling_sample clangTooling)
install(TARGETS tooling_sample RUNTIME DESTINATION bin)
