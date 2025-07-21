cmake_minimum_required(VERSION 3.22)
project(fastin)

add_executable(fastin src/main.cpp src/docker_api.cpp src/project_api.cpp src/project.cpp src/gitlab.cpp src/enum_utils.cpp)

target_include_directories(fastin PRIVATE include)

target_include_directories(fastin PRIVATE $ENV{SPDLOG_HOME}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_directories(fastin PRIVATE $ENV{SPDLOG_HOME}/build)
target_link_libraries(fastin PRIVATE spdlog)

target_include_directories(fastin PRIVATE $ENV{PUGIXML_HOME}/src)
target_link_directories(fastin PRIVATE $ENV{PUGIXML_HOME}/build)
target_link_libraries(fastin PRIVATE pugixml)

target_include_directories(fastin PRIVATE $ENV{NLOHMANN_HOME}/include)

target_include_directories(fastin PRIVATE $ENV{CLI11_HOME}/include)

target_include_directories(fastin PRIVATE $ENV{YAML_CPP_HOME}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_directories(fastin PRIVATE $ENV{YAML_CPP_HOME}/build)
if (WIN32)
    target_compile_definitions(fastin PRIVATE YAML_CPP_STATIC_DEFINE)
endif()
target_link_libraries(fastin PRIVATE yaml-cpp)