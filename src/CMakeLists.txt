add_definitions(-std=gnu11)

add_executable(mumbleweb
  main.c
)

add_dependencies(mumbleweb
  mumble
)

target_include_directories(mumbleweb
  PUBLIC "../lib/mumble"
  PUBLIC ${UV_INCLUDE_DIR}
  PUBLIC ${OPENSSL_INCLUDE_DIR}
)

link_directories(mumbleweb
  ${UV_LIBRARIES}
)

target_link_libraries(mumbleweb
  mumble
  ${UV_LIBRARIES}/libuv.a
  "pthread"
  "ssl"
  "crypto"
)
