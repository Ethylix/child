include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

find_package(Check REQUIRED)
find_package(Threads REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})

set(TEST_SOURCES
  check_hashmap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/hashmap.c
)

add_executable(check_hashmap ${TEST_SOURCES})
target_link_libraries(check_hashmap ${CHECK_LIBRARIES} Threads::Threads)
