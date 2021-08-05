include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

find_package(Check REQUIRED)
find_package(Threads REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})

set(TEST_SOURCES
  check_check.c
  check_check.h
  check_hashmap.c
  check_parseline.c
  check_nick_api.c
)

add_executable(check_child ${TEST_SOURCES})
target_link_libraries(check_child ${CHECK_LIBRARIES} Threads::Threads "-Wl,--whole-archive" childcore "-Wl,--no-whole-archive")
add_test(NAME check_child COMMAND check_child WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
