set(HEADERS
  ${CONFIG_HEADERS}
  botserv.h
  channel.h
  child.h
  commands.h
  core.h
  core_api.h
  db.h
  hashmap.h
  llist.h
  logging.h
  mem.h
  modules.h
  net.h
  nick.h
  server.h
  string_utils.h
  trust.h
  undef.h
  user.h
)

set(LIB_SOURCES
  botserv.c
  base64.c
  channel.c
  commands.c
  core.c
  core_api.c
  db.c
  hashmap.c
  loadconf.c
  logging.c
  md5.c
  mem.c
  modules.c
  net.c
  nick.c
  parseline.c
  server.c
  string_utils.c
  trust.c
  user.c
)

set(BIN_SOURCES
  child.c
)

add_library(childcore STATIC ${HEADERS} ${LIB_SOURCES})
target_link_libraries(childcore OpenSSL::Crypto ${MYSQL_LIBRARIES} sodium ${DL_LIBRARY})
target_include_directories(childcore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

add_executable(child ${HEADERS} ${BIN_SOURCES})
target_link_libraries(child "-Wl,--whole-archive" childcore "-Wl,--no-whole-archive")
target_include_directories(child PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
set_target_properties(child PROPERTIES ENABLE_EXPORTS 1)

add_subdirectory(modules)
