include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

find_package(Check REQUIRED)
find_package(Threads REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})

macro(CHILD_ADD_TEST testName testFile)
  add_executable(${testName} ${testFile})
  target_link_libraries(${testName} ${CHECK_LIBRARIES} Threads::Threads "-Wl,--whole-archive" childcore "-Wl,--no-whole-archive")
  add_test(NAME ${testName} COMMAND ${testName})
endmacro()

CHILD_ADD_TEST(check_hashmap check_hashmap.c)
CHILD_ADD_TEST(check_parseline check_parseline.c)
CHILD_ADD_TEST(check_nick_api check_nick_api.c)
