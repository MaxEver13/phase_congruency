cmake_minimum_required(VERSION 2.8.3)
project(phase_congruency)

add_definitions(-std=c++11)

set(CMAKE_BUILD_TYPE "RELEASE")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g -pthread")

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

# include directories
include_directories(
  src
  ${OpenCV_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIR}
)

# link directories
link_directories(
  ${OpenCV_LIBRARY_DIRS}
)

###########
## Build ##
###########


add_library(${PROJECT_NAME} src/phase.cpp)


add_executable(${PROJECT_NAME}_test src/main.cpp)
target_link_libraries(${PROJECT_NAME}_test ${OpenCV_LIBRARIES} ${PROJECT_NAME})
