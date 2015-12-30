add_definitions(-std=gnu11)

include(FindOpenSSL)

add_library(mumble
  STATIC
  mumble.c
  proto/Mumble.pb-c.c
  uv_ssl.c
)

add_custom_command(
  TARGET mumble
  PRE_BUILD
  COMMAND "${PROTOBUF_C_BIN}/protoc-c" "--c_out=." "Mumble.proto"
  WORKING_DIRECTORY "proto"
  COMMENT "Generating protobuf source for Mumble protocol"
  VERBATIM
)

target_include_directories(mumble
  PUBLIC ${UV_INCLUDE_DIR}
  PUBLIC ${OPENSSL_INCLUDE_DIR}
  PUBLIC ${PROTOBUF_C_INCLUDE_DIR}
  PUBLIC ${PROTOBUF_INCLUDE_DIR}
)
