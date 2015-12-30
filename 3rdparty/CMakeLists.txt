include(ExternalProject)

# libuv
ExternalProject_Add(UV
  URL "${CMAKE_CURRENT_SOURCE_DIR}/libuv-v1.8.0"
  CONFIGURE_COMMAND "<SOURCE_DIR>/autogen.sh" COMMAND "<SOURCE_DIR>/configure" "--prefix=<INSTALL_DIR>"
)
ExternalProject_Get_Property(UV install_dir)
set(UV_INCLUDE_DIR "${install_dir}/include" PARENT_SCOPE)
set(UV_LIBRARIES "${install_dir}/lib" PARENT_SCOPE)

# protobuf
ExternalProject_Add(PROTOBUF
  URL "${CMAKE_CURRENT_SOURCE_DIR}/protobuf-2.6.1"
  CONFIGURE_COMMAND "<SOURCE_DIR>/autogen.sh" COMMAND "<SOURCE_DIR>/configure" "--prefix=<INSTALL_DIR>"
  BUILD_IN_SOURCE 1
)
ExternalProject_Get_Property(PROTOBUF install_dir)
set(PROTOBUF_INCLUDE_DIR "${install_dir}/include" PARENT_SCOPE)
set(PROTOBUF_LIBRARIES "${install_dir}/lib")
set(PROTOBUF_LIBRARIES "${install_dir}/lib" PARENT_SCOPE)

# protobuf-c
ExternalProject_Add(PROTOBUF_C
  URL "${CMAKE_CURRENT_SOURCE_DIR}/protobuf-c-1.1.1"
  CONFIGURE_COMMAND "<SOURCE_DIR>/autogen.sh" COMMAND "<SOURCE_DIR>/configure" "--prefix=<INSTALL_DIR>" "PKG_CONFIG_PATH=${PROTOBUF_LIBRARIES}/pkgconfig"
  BUILD_IN_SOURCE 1
)
ExternalProject_Get_Property(PROTOBUF_C install_dir)
set(PROTOBUF_C_INCLUDE_DIR "${install_dir}/include" PARENT_SCOPE)
set(PROTOBUF_C_LIBRARIES "${install_dir}/lib" PARENT_SCOPE)
set(PROTOBUF_C_BIN "${install_dir}/bin" PARENT_SCOPE)
