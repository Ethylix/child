set(HEADERS
  ${CONFIG_HEADERS}
  botserv.h
  channel.h
  child.h
  commands.h
  core.h
  db.h
  filter.h
  hashmap.h
  llist.h
  mem.h
  modules.h
  net.h
  server.h
  string_utils.h
  trust.h
  undef.h
  user.h
)

set(BIN_SOURCES
  botserv.c
  base64.c
  channel.c
  child.c
  commands.c
  core.c
  db.c
  filter.c
  hashmap.c
  loadconf.c
  md5.c
  mem.c
  modules.c
  net.c
  parseline.c
  server.c
  string_utils.c
  trust.c
  user.c
)

add_executable(child ${HEADERS} ${BIN_SOURCES})
target_link_libraries(child OpenSSL::Crypto ${MYSQL_LIBRARIES} ${DL_LIBRARY})
target_include_directories(child PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
set_target_properties(child PROPERTIES ENABLE_EXPORTS 1)

add_subdirectory(modules)
