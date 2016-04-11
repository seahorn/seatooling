cmake_minimum_required(VERSION 2.8.12)
project(SeaTooling)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR )
  message (FATAL_ERROR
    "In-source builds are not allowed. Please clean your source tree and try again.")
endif()

# Add path for custom modules
list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(LLVM 3.6.0 CONFIG NO_DEFAULT_PATH)

option (WITH_CLANG "Compile LLVM with Clang" OFF)
if (NOT LLVM_FOUND)
  
  find_package(Git REQUIRED)

  set (LLVM_TAG "release_36" CACHE STRING "LLVM tag")
  
  # we use custom version of clang
  set (CLANG_TAG "seahorn_36" CACHE STRING "Clang tag")

  add_custom_target (llvm-src
    # HACK: remove all sources so that git-clone does not fail
    # HACK: a proper solution is to work arround this as in ExternalProject
    ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/ext/llvm
    COMMAND
    ${GIT_EXECUTABLE} clone --depth=1 --branch=${LLVM_TAG}
    http://llvm.org/git/llvm.git ${CMAKE_SOURCE_DIR}/ext/llvm)
  
  if (WITH_CLANG)
    add_custom_target (clang-src
      ${GIT_EXECUTABLE} clone --depth=1 --branch=${CLANG_TAG}
      https://github.com/seahorn/clang.git
      ${CMAKE_SOURCE_DIR}/ext/llvm/tools/clang
      DEPENDS llvm-src)
    add_custom_target (compiler-rt
      ${GIT_EXECUTABLE} clone --depth=1 --branch=${LLVM_TAG}
      http://llvm.org/git/compiler-rt.git 
      ${CMAKE_SOURCE_DIR}/ext/llvm/projects/compiler-rt
      DEPENDS clang-src)
  endif()

  if (WITH_CLANG)
    set (LAST_LLVM_SRC compiler-rt)
  else()
    set (LAST_LLVM_SRC llvm-src)
  endif()
  
  add_custom_target (download-llvm-src DEPENDS ${LAST_LLVM_SRC})
  
  include(ExternalProject)
  set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure build test)
  # build llvm using previously downloaded source
  ExternalProject_Add (llvm
    DEPENDS download-llvm-src
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/ext/llvm
    INSTALL_DIR ${CMAKE_BINARY_DIR}/run
    CMAKE_ARGS
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DLLVM_TARGETS_TO_BUILD:STRING=X86 -DWITH_POLY:BOOL=OFF
    -DLLVM_ENABLE_PEDANTIC=OFF
    -DLLVM_ENABLE_PIC=ON -DLLVM_REQUIRES_RTTI:BOOL=TRUE
    TEST_AFTER_INSTALL 1
    # touch CMakeLists.txt so that cmake is re-run in the next build
    TEST_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_LIST_FILE}
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1)


  ExternalProject_Get_Property (llvm INSTALL_DIR)
  set (LLVM_ROOT ${INSTALL_DIR})
  set (LLVM_DIR ${LLVM_ROOT}/share/llvm/cmake CACHE PATH
    "Forced location of LLVM cmake config" FORCE)
  if (WITH_CLANG)
    set (Clang_DIR ${LLVM_ROOT}/share/clang/cmake CACHE PATH
      "Forced location of Clang cmake config" FORCE)
  endif()
  
  message (WARNING "No suitable LLVM build found. Run \n\tcmake --build . \n to build.")
  # stop further configuration
  return()
else()
  ## LLVM was found
  
  # if Clang was requested, ensure we have it
  if (WITH_CLANG)
    find_package(Clang CONFIG NO_DEFAULT_PATH REQUIRED)
    message (STATUS "Found Clang in ${Clang_DIR}")
  endif()
  
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
  
  # We incorporate the CMake features provided by LLVM:
  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
  include(AddLLVM)
  include(HandleLLVMOptions)
  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LDFLAGS}")

  set(LLVM_REQUIRES_RTTI TRUE)
  set(LLVM_REQUIRES_EH TRUE)

  include_directories(${LLVM_INCLUDE_DIRS})
  link_directories(${LLVM_LIBRARY_DIRS})
  add_definitions(${LLVM_DEFINITIONS})

endif()

add_subdirectory(tools)
