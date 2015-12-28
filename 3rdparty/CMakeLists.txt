include(ExternalProject)

# libuv
ExternalProject_Add(UV
  URL "${CMAKE_CURRENT_SOURCE_DIR}/libuv-v1.8.0"
  CONFIGURE_COMMAND "<SOURCE_DIR>/autogen.sh" COMMAND "<SOURCE_DIR>/configure" "--prefix=<INSTALL_DIR>"
)
ExternalProject_Get_Property(UV install_dir)
set(UV_INCLUDE_DIR "${install_dir}/include" PARENT_SCOPE)
set(UV_LIBRARIES "${install_dir}/lib" PARENT_SCOPE)
