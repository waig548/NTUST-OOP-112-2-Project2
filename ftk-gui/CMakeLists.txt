add_executable(ftk-gui gui.h gui.cpp view.h view.cpp main.cpp)
target_include_directories(ftk-gui PRIVATE ".")
target_link_libraries(ftk-gui PRIVATE stduuid nlohmann_json cparse CRCpp color glad glfw imgui ImGuiFileDialog stb lib-ftk)

add_custom_command(TARGET ftk-gui PRE_BUILD COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/out/${PROJECT_NAME}/assets)
add_custom_command(TARGET ftk-gui POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/out/${PROJECT_NAME}/assets)