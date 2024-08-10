set(lib_name "LServer")

add_library(${lib_name} SHARED
    LServer.cpp
    LProcess.cpp
)

target_include_directories(${lib_name}
    PRIVATE ${PROJECT_SOURCE_DIR}/base/threadpool
    PRIVATE ${PROJECT_SOURCE_DIR}/base/util
    PRIVATE ${PROJECT_SOURCE_DIR}/base/log
)

target_link_libraries(${lib_name}
    LUtil
    NanoLog
)