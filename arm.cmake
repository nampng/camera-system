set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR ARM)

add_compile_definitions(ON_TARGET)

set(TARGET_PREFIX "PUT_PREFIX_HERE")

set(CMAKE_C_COMPILER ${TARGET_PREFIX}/bin/arm-linux-gcc)
set(CMAKE_CXX_COMPILER ${TARGET_PREFIX}/bin/arm-linux-g++)
set(CMAKE_SYSROOT ${TARGET_PREFIX}/arm-buildroot-linux-gnueabihf/sysroot) 
set(CMAKE_FIND_ROOT_PATH ${TARGET_PREFIX}/arm-buildroot-linux-gnueabihf/sysroot)

set(PKG_CONFIG_EXECUTABLE "${TARGET_PREFIX}/bin/pkg-config")
set(ENV{PKG_CONFIG} "${PKG_CONFIG_EXECUTABLE}")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}")
set(ENV{PKG_CONFIG_PATH} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig")

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
