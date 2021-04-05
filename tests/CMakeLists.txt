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

add_executable(check_parseline ${HEADERS} check_parseline.c)
target_link_libraries(check_parseline ${CHECK_LIBRARIES} Threads::Threads childcore)

add_executable(check_nick_api ${HEADERS} check_nick_api.c)
target_link_libraries(check_nick_api ${CHECK_LIBRARIES} Threads::Threads "-Wl,--whole-archive" childcore "-Wl,--no-whole-archive")
