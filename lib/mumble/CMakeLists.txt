add_definitions(-std=gnu11)

include(FindOpenSSL)

add_library(mumble
  STATIC
  mumble.c
  uv_ssl.c
)

target_include_directories(mumble
  PUBLIC ${UV_INCLUDE_DIR}
  PUBLIC ${OPENSSL_INCLUDE_DIR}
)

link_directories(mumble
  # ${UV_LIBRARIES}
  # ${OPENSSL_LIBRARIES}
)
