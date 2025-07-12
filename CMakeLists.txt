cmake_minimum_required(VERSION 3.22)
project(fastin)

add_executable(fastin src/main.cpp src/dockerfile_api.cpp src/project_api.cpp src/project.cpp)

target_include_directories(fastin PRIVATE include)

target_include_directories(fastin PRIVATE $ENV{SPDLOG_HOME}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_directories(fastin PRIVATE $ENV{SPDLOG_HOME}/build)
target_link_libraries(fastin PRIVATE spdlog)

target_include_directories(fastin PRIVATE $ENV{PUGIXML_HOME}/src)
target_link_directories(fastin PRIVATE $ENV{PUGIXML_HOME}/build)
target_link_libraries(fastin PRIVATE pugixml)

target_include_directories(fastin PRIVATE $ENV{NLOHMANN_HOME}/include)