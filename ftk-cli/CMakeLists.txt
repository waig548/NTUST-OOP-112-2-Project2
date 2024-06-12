add_executable(ftk-cli cli_util.h cli_util.cpp main.cpp)
target_include_directories(ftk-cli PRIVATE ".")
target_link_libraries(ftk-cli PRIVATE stduuid nlohmann_json cparse CRCpp color winmm.lib lib-ftk)