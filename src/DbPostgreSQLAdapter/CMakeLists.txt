cmake_minimum_required(VERSION 3.20)

# Find external dependencides
find_package(DbAdapterInterface REQUIRED)
find_package(PostgreSQL REQUIRED)

# Configure preprocessor definitions
add_compile_options(-DBOOST_ALL_DYN_LINK)

# Add project folder into includes
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Configure RapidJSONAdapter static library
set(DB_POSTGRESQL_ADAPTER DbPostgreSQLAdapter)
file(GLOB_RECURSE DB_POSTGRESQL_ADAPTER_SRC "*.cpp")
file(GLOB_RECURSE DB_POSTGRESQL_ADAPTER_HDR "*.h")
add_library(${DB_POSTGRESQL_ADAPTER} STATIC ${DB_POSTGRESQL_ADAPTER_SRC} ${DB_POSTGRESQL_ADAPTER_HDR})

if(DbAdapterInterface_FOUND)
    include_directories(${DbAdapterInterface_INCLUDE_DIRS})
    target_link_libraries(${DB_POSTGRESQL_ADAPTER} DbAdapterInterface::DbAdapterInterface )
endif()

if(PostgreSQL_FOUND)
    include_directories(${PostgreSQL_INCLUDE_DIRS})
    target_link_libraries(${DB_POSTGRESQL_ADAPTER} PostgreSQL::pq)
endif()

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
