
add_executable(tool-template
  ToolTemplate.cpp
  )

target_link_libraries(tool-template
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  )

install (TARGETS tool-template RUNTIME DESTINATION bin)
