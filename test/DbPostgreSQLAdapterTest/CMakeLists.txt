enable_testing()

# Find external dependencides
find_package(GTest REQUIRED)

# Configure test project
set(DB_POSTGRESQL_ADAPTER_TEST_PROJECT DbPostgreSQLAdapterTest)
file(GLOB_RECURSE DB_POSTGRESQL_ADAPTER_TEST_PROJECT_SRC "*.cpp")
file(GLOB_RECURSE DB_POSTGRESQL_ADAPTER_TEST_PROJECT_HDR "*.h")
add_executable(${DB_POSTGRESQL_ADAPTER_TEST_PROJECT} ${DB_POSTGRESQL_ADAPTER_TEST_PROJECT_SRC} ${DB_POSTGRESQL_ADAPTER_TEST_PROJECT_HDR})
target_include_directories(${DB_POSTGRESQL_ADAPTER_TEST_PROJECT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${DB_POSTGRESQL_ADAPTER_TEST_PROJECT} DbPostgreSQLAdapter
															GTest::gtest)

#Configure source groups
foreach(FILE ${DB_POSTGRESQL_ADAPTER_TEST_PROJECT_SRC} ${DB_POSTGRESQL_ADAPTER_TEST_PROJECT_HDR}) 
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

# Register tests
add_test(NAME DbPostgreSQLAdapterTest COMMAND ${DB_POSTGRESQL_ADAPTER_TEST_PROJECT})
