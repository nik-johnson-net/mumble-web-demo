add_definitions(-std=gnu11)

include(FindOpenSSL)

add_library(mumble
  STATIC
  audio.c
  audio_decoder.c
  audio_encoder.c
  buffer.c
  codec/opus.c
  mumble.c
  proto/Mumble.pb-c.c
  util.c
  uv_tcp_ssl.c
  uv_udp_ssl.c
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
  PUBLIC ${OPUS_INCLUDE_DIR}
)
