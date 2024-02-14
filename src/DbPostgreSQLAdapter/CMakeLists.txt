cmake_minimum_required(VERSION 3.20)

# Find external dependencides
find_package(DbAdapterInterface REQUIRED)
find_package(PostgreSQL REQUIRED)


# Configure RapidJSONAdapter static library
set(DB_POSTGRESQL_ADAPTER DbPostgreSQLAdapter)
file(GLOB_RECURSE DB_POSTGRESQL_ADAPTER_SRC "*.cpp")
file(GLOB_RECURSE DB_POSTGRESQL_ADAPTER_HDR "*.h")
add_library(${DB_POSTGRESQL_ADAPTER} STATIC ${DB_POSTGRESQL_ADAPTER_SRC} ${DB_POSTGRESQL_ADAPTER_HDR})

target_include_directories(${DB_POSTGRESQL_ADAPTER} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
													PUBLIC ${DbAdapterInterface_INCLUDE_DIRS}
													PRIVATE ${PostgreSQL_INCLUDE_DIRS})

target_link_libraries(${DB_POSTGRESQL_ADAPTER} DbAdapterInterface::DbAdapterInterface 
											   PostgreSQL::pq)

#Configure source groups
foreach(FILE ${DB_POSTGRESQL_ADAPTER_SRC} ${DB_POSTGRESQL_ADAPTER_HDR}) 
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
    string(REPLACE "/" "\\" GROUP "${GROUP}")
       
    if ("${FILE}" MATCHES ".*\\.cpp")
       set(GROUP "Source Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.h")
       set(GROUP "Header Files${GROUP}")
    endif()

    source_group("${GROUP}" FILES "${FILE}")
endforeach()
