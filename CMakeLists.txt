cmake_minimum_required(VERSION 3.5) # 设置CMake最小版本

project(firstServer) # 设置工程名

add_executable(${PROJECT_NAME} main.cpp) # 生成可执行文件

# set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g3 -Wall")

# target_include_directories(${PROJECT_NAME}
# )

target_link_libraries(${PROJECT_NAME}
    pthread
)

add_subdirectory(base)
add_subdirectory(test)