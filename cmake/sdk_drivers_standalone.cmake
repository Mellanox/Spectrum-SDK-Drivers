
# SDK drivers standalone bootstrap.
#
# Provides the variables and packaging logic for the SDK drivers standalone
# build. Pre-project() settings (cmake_minimum_required, CMP0096,
# in-source-build guard, default compiler) live in
# sxd_kernel/CMakeLists.txt instead, since they must run before project().

# Project build type
set (VALID_BUILD_TYPES "Release|Debug|RelWithDebInfo|asan")
if (NOT CMAKE_BUILD_TYPE OR NOT ${CMAKE_BUILD_TYPE} MATCHES ${VALID_BUILD_TYPES})
    set(default_cmake_build_type "RelWithDebInfo")
    if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "")
        message(STATUS "Invalid CMAKE_BUILD_TYPE specified: \'${CMAKE_BUILD_TYPE}\'. Valid build types: \'${VALID_BUILD_TYPES}\'")
    endif()
    message(STATUS "Setting build type to \'${default_cmake_build_type}\'")
    set(CMAKE_BUILD_TYPE
        "${default_cmake_build_type}"
        CACHE STRING "Build type (default ${default_cmake_build_type})" FORCE)
else()
    message (STATUS "Build type \'${CMAKE_BUILD_TYPE}\'")
endif()

set(CMAKE_C_FLAGS_DEBUG "-ggdb -O0 -D_DEBUG_")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_ASAN "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize-recover=address -g -O0 -D_DEBUG_")

set(CMAKE_INSTALL_PREFIX / CACHE STRING "Install prefix (default /)" FORCE)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE STRING "Output directory of shared object files")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE STRING "Output directory of executable files")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE STRING "Output directory of archived object files")

include(GNUInstallDirs)
set (CMAKE_INSTALL_ETCDIR usr/etc)

# Default component for install() calls that don't pass COMPONENT explicitly
# (e.g. the kernel module installs in drivers/net/*/CMakeLists.txt).
set (CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "main")

# Kernel build options (MEMTRACK, PD_PRESENT, KERNEL_COMPILE_COMMANDS)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/kernel_options.cmake)

# In SDK drivers standalone mode we always build the kernel and only the
# kernel. Any -DUSE_KERNEL=... / -DKERNEL_ONLY=... on the command line is
# silently overridden by these set()s; surface that explicitly so the user
# isn't confused if they check CMakeCache.txt afterward.
if (DEFINED CACHE{USE_KERNEL} AND NOT USE_KERNEL)
    message(STATUS "SDK drivers standalone mode: -DUSE_KERNEL=OFF ignored (forced ON)")
endif()
if (DEFINED CACHE{KERNEL_ONLY} AND NOT KERNEL_ONLY)
    message(STATUS "SDK drivers standalone mode: -DKERNEL_ONLY=OFF ignored (forced ON)")
endif()
set (USE_KERNEL ON)
set (KERNEL_ONLY ON)

# Skip the SDK uninstall step - not applicable in the SDK drivers
# standalone build.
set (SKIP_UNINSTALL_INSTALL_DEPENDENCY ON)

set (CMAKE_SKIP_BUILD_RPATH TRUE)
set (CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

# Kernel info
set (OS_RELEASE ${CMAKE_SYSTEM_VERSION} CACHE STRING "Kernel version")
set (MODULES_PATH /lib/modules/${OS_RELEASE} CACHE STRING "Kernel module path")
set (SX_KERNEL_MODULE_BUILD_PATH ${MODULES_PATH}/build CACHE STRING "Kernel module build path")
set (SX_KERNEL_MODULE_SOURCE_PATH ${MODULES_PATH}/source CACHE STRING "Kernel module source path")
message(STATUS "Kernel release: ${OS_RELEASE}")
message(STATUS "Kernel headers: ${MODULES_PATH}")

message(STATUS "System processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set (OS_ARCH arm64 CACHE STRING "Kernel ARCH")
else()
    set (OS_ARCH x86 CACHE STRING "Kernel ARCH")
endif()

# Install manifest preparation (sudo-friendly write permissions)
set (
    INSTALL_MANIFEST_FILES
    ${CMAKE_BINARY_DIR}/install_manifest.txt
)
foreach (manfile ${INSTALL_MANIFEST_FILES})
    if (NOT EXISTS ${manfile})
        file(TOUCH ${manfile})
        execute_process(COMMAND chmod a+w ${manfile})
    endif()
endforeach()

# CPack configuration is in sdk_drivers_standalone_cpack.cmake and runs
# after all install() commands are registered.
