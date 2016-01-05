add_definitions(-std=gnu11)

add_executable(mumbleweb
  main.c
)

add_dependencies(mumbleweb
  mumble
)

target_include_directories(mumbleweb
  PUBLIC "../lib"
  PUBLIC ${UV_INCLUDE_DIR}
  PUBLIC ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(mumbleweb
  mumble
  ${UV_LIBRARIES}/libuv.a
  ${PROTOBUF_C_LIBRARIES}/libprotobuf-c.a
  ${PROTOBUF_LIBRARIES}/libprotobuf.a
  ${OPUS_LIBRARIES}/libopus.a
  "pthread"
  "ssl"
  "crypto"
  "m"
)
