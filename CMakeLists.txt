cmake_minimum_required(VERSION 3.13)  # To match googletest requirement

project(traj-reader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR})

FILE(GLOB SOURCES ${PROJECT_SOURCE_DIR}/water_pure/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
