set(lib_name "MySQLPool")

add_library(${lib_name} SHARED
    MySQLPool.cpp

)

# target_include_directories(${lib_name}
    # PRIVATE ${PROJECT_SOURCE_DIR}/base/log
# )

target_link_libraries(${lib_name}
    mysqlclient
    # NanoLog
)