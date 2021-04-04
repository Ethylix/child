# include(${CMAKE_CURRENT_SOURCE_DIR}/../src/CMakeLists.txt)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

find_package(Check REQUIRED)
find_package(Threads REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})

set(TEST_HASHMAP_SOURCES
  check_hashmap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/hashmap.c
)

add_executable(check_hashmap ${TEST_HASHMAP_SOURCES})
target_link_libraries(check_hashmap ${CHECK_LIBRARIES} Threads::Threads)

set(TEST_PARSELINE_SOURCES
  check_parseline.c
)

add_executable(check_parseline ${HEADERS} ${TEST_PARSELINE_SOURCES})
target_link_libraries(check_parseline ${CHECK_LIBRARIES} Threads::Threads childcore)
