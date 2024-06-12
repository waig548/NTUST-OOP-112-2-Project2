cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(-DUNICODE -D_UNICODE UNICODE _UNICODE)

project(lib-ftk)
project(ftk-cli)
project(ftk-gui)

add_subdirectory(externals)
add_subdirectory(lib-ftk)
add_subdirectory(ftk-cli)
add_subdirectory(ftk-gui)

set_target_properties(ftk-gui PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/ftk-gui)
set_target_properties(ftk-gui PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/out/ftk-gui)
set_target_properties(ftk-gui PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/out/ftk-gui)