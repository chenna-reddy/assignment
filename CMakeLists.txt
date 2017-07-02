cmake_minimum_required(VERSION 3.5)
project(imc)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES ConcurrentQueue.cpp game.cpp TestConcurrentQueue.cpp)

add_executable(cq ConcurrentQueue.cpp TestConcurrentQueue.cpp)
add_executable(game game.cpp)

TARGET_LINK_LIBRARIES(cq
        pthread
)