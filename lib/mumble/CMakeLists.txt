add_definitions(-std=gnu11)
add_definitions(-fstack-protector)

include(FindOpenSSL)

add_library(mumble
  STATIC
  ocb_aes.c
  audio.c
  audio_decoder.c
  audio_encoder.c
  buffer.c
  codec/opus.c
  mumble_client.c
  proto/Mumble.pb-c.c
  util.c
  udp_ping.c
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

add_executable(ocb_aes_test
  ocb_aes_test.c
)

target_include_directories(ocb_aes_test
  PUBLIC ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(ocb_aes_test
  crypto
)

add_test(ocb_aes ocb_aes_test)
