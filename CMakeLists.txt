
# SDK drivers standalone bootstrap: when sxd_kernel/ is configured as the
# top-level project, set up the project here. Pre-project() settings
# (policy, in-source-build guard, default compiler) must run before
# project() and so live in this block; everything else is delegated to
# cmake/sdk_drivers_standalone.cmake.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.20.2)

    # CMP0096: preserve leading zeros in project() VERSION components.
    # Must be set before project().
    if (POLICY CMP0096)
        cmake_policy(SET CMP0096 NEW)
    endif()

    # In-source build guard. Must run before project() to prevent
    # project() from polluting the source tree with CMakeCache.txt /
    # CMakeFiles/.
    if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
        message(FATAL_ERROR
            "Build in source directory is not allowed.\n"
            "Generate build tree in a separate directory, e.g.:\n"
            "    cmake -S . -B build"
        )
    endif()

    # Default C compiler. Must be set before project(... LANGUAGES C)
    # to influence compiler selection.
    set (CMAKE_C_COMPILER "/usr/bin/gcc" CACHE STRING "Default compiler")

    # SDK_BUILD_PRIVATE_VERSION is patched by set_sdk_version.sh
    set(SDK_BUILD_PRIVATE_VERSION 3.0.0000)
    project(sx_kernel VERSION ${SDK_BUILD_PRIVATE_VERSION} LANGUAGES C)
    set(CMAKE_PROJECT_VERSION "4.10.1086")

    set (SDK_DRIVERS_STANDALONE_BUILD ON)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdk_drivers_standalone.cmake)
endif()

# Global path location
set (SXD_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Extra CFLAGS for kernel modules
if (MEMTRACK)
    set (MEMTRACK_EXTRA_CFLAGS_VAR "-include ${CMAKE_CURRENT_SOURCE_DIR}/drivers/net/mlx_sx/debug/memtrack/mtrack.h")
    set (MEMTRACK_EXTRA_SYMVERS_VAR ${CMAKE_CURRENT_SOURCE_DIR}/drivers/net/mlx_sx/debug/memtrack/Module.symvers)
endif()

set (SXD_KERNEL_EXTRA_CFLAGS_VAR
    "${MEMTRACK_EXTRA_CFLAGS_VAR} -I${CMAKE_CURRENT_SOURCE_DIR}/include -I${CMAKE_CURRENT_SOURCE_DIR}/include/stub"
)
set (EXTRA_SYMVERS_VAR
    "${MEMTRACK_EXTRA_SYMVERS_VAR} ${CMAKE_CURRENT_SOURCE_DIR}/drivers/net/mlx_sx/Module.symvers"
)

if (PD_PRESENT)
    set (SXD_KERNEL_EXTRA_CFLAGS_VAR "${SXD_KERNEL_EXTRA_CFLAGS_VAR} -DPD_BU")
endif()

# Compilation flags and defenitions for kernel modules
set (KCFLAGS "-D__KERNEL__ $ENV{KCFLAGS} -Werror -Wall -Wswitch -Wunused -Wno-format-truncation -Wframe-larger-than=4096 -Wno-date-time")

if ((CMAKE_C_COMPILER_LOADED) AND (CMAKE_COMPILER_IS_GNUCC))
    string(APPEND KCFLAGS " -Wno-stringop-truncation")
endif()

if (OS_RELEASE MATCHES "4.18.0-372.*")
    string(APPEND KCFLAGS " -DCONFIG_SX_RHEL_8_6")
elseif (OS_RELEASE MATCHES "5.14.0-427.*")
    string(APPEND KCFLAGS " -DCONFIG_SX_RHEL_9_4")
elseif (OS_RELEASE MATCHES "4.18.0-408.*")
    string(APPEND KCFLAGS " -DCONFIG_SX_CENTOS_8_4")
endif()

# Function to detect kernel compiler
function(detect_kernel_compiler)
    # Read /proc/version
    file(READ "/proc/version" PROC_VERSION_CONTENT)

    # Check for Clang in /proc/version
    string(FIND "${PROC_VERSION_CONTENT}" "clang version" KERNEL_CC_IS_CLANG)
    if (KERNEL_CC_IS_CLANG GREATER -1)
        message(STATUS "Kernel was compiled with Clang")
        set(KERNEL_COMPILER "Clang" PARENT_SCOPE)
        return()
    endif()

    # Fallback: Check kernel build configuration (.config)
    set(KERNEL_CONFIG "/lib/modules/${CMAKE_SYSTEM_VERSION}/build/.config")
    if (EXISTS "${KERNEL_CONFIG}")
        file(READ "${KERNEL_CONFIG}" CONFIG_CONTENT)
        string(FIND "${CONFIG_CONTENT}" "CONFIG_CC_IS_CLANG=y" KERNEL_CC_CONFIG_IS_CLANG)
        if (KERNEL_CC_CONFIG_IS_CLANG GREATER -1)
            message(STATUS "Kernel build configuration indicates Clang")
            set(KERNEL_COMPILER "Clang" PARENT_SCOPE)
            return()
        endif()
    endif()

    # Default to GCC if no Clang evidence is found
    message(STATUS "Kernel was compiled with GNU GCC")
    set(KERNEL_COMPILER "GNU" PARENT_SCOPE)
endfunction()

# Call function to detect kernel compiler
detect_kernel_compiler()

# Ensure the compilers match
if (NOT CMAKE_C_COMPILER_ID MATCHES "${KERNEL_COMPILER}")
    message(FATAL_ERROR "Kernel was compiled with ${KERNEL_COMPILER}, but the current compiler is ${CMAKE_C_COMPILER_ID}. Please use the same compiler.")
endif()

if (KERNEL_COMPILER STREQUAL "Clang")
    set (EXTRA_OPTIONS CC=clang LD=ld.lld NM=llvm-nm)
else()
    set (EXTRA_OPTIONS "")
endif()

if (DEFINED BULLSEYE_DIR_PATH)
    # currently memtrack is not supported with Bullseye.
    # it means that code coverage build should have -DMEMTRACK=OFF
    if (MEMTRACK)
        message(FATAL_ERROR "currently, code coverage build does not support memtrack")
    endif()

    set (ENV{PATH} ${BULLSEYE_DIR_PATH}:$ENV{PATH})
    set (EXTRA_OPTIONS CC=gcc)
    set (BULLSEYE_EXTRA_SYMVERS_VAR /tmp/sx_evb_libcov_lkm/Module.symvers)
    set (MEMTRACK_EXTRA_SYMVERS_VAR ${MEMTRACK_EXTRA_SYMVERS_VAR} ${BULLSEYE_EXTRA_SYMVERS_VAR})
    set (EXTRA_SYMVERS_VAR ${EXTRA_SYMVERS_VAR} ${BULLSEYE_EXTRA_SYMVERS_VAR})
endif()

# Generate compile_commands.json for kernel modules (for clangd/IDE support)
# Option defined in cmake/options.cmake, enabled via sdktool or -DKERNEL_COMPILE_COMMANDS=ON
# Parses kbuild .cmd files after kernel modules are built.
# Define script path unconditionally (avoids "EXISTS" errors when KERNEL_COMPILE_COMMANDS=OFF)
set(GEN_KERNEL_CC_SCRIPT "${SXD_KERNEL_PATH}/cmake/gen_kernel_compile_commands.py")

if (KERNEL_COMPILE_COMMANDS)
    # Find Python3 interpreter (required for running the generator script)
    # Minimum version 3.6 required for f-string syntax used in gen_kernel_compile_commands.py
    find_package(Python3 3.6 COMPONENTS Interpreter)

    if (NOT Python3_FOUND)
        message(FATAL_ERROR "KERNEL_COMPILE_COMMANDS requires Python 3.6+ interpreter but it was not found. "
                            "Please install Python 3.6 or later, or set -DKERNEL_COMPILE_COMMANDS=OFF")
    endif()

    if (EXISTS "${GEN_KERNEL_CC_SCRIPT}")
        message(STATUS "Kernel compile_commands: will parse .cmd files after kernel module builds")
        message(STATUS "  Using Python3: ${Python3_EXECUTABLE}")
    else()
        message(WARNING "gen_kernel_compile_commands.py not found at ${GEN_KERNEL_CC_SCRIPT}")
    endif()
endif()

# Common module compilation command.
# Pass ARCH explicitly (OS_ARCH is x86 or arm64) so the kernel Kbuild does not
# inherit a stray, wrongly-cased ARCH (e.g. "X86") from the build environment.
# A make command-line assignment overrides the environment, keeping the build
# deterministic; otherwise Kbuild looks for a non-existent arch/<ARCH>/Makefile.
set (MODULE_CMD_COMMON
    $(MAKE) -C ${SX_KERNEL_MODULE_BUILD_PATH}
    ARCH=${OS_ARCH}
    SX_SDK_VERSION=${CMAKE_PROJECT_VERSION}
    SXD_KERNEL_EXTRA_CFLAGS=${SXD_KERNEL_EXTRA_CFLAGS_VAR}
    KCFLAGS=${KCFLAGS}
    ${EXTRA_OPTIONS})

# build the directory tree (without files) before building the modules
execute_process(
    COMMAND bash -c "pushd ${CMAKE_CURRENT_SOURCE_DIR}/drivers; find . -type d -exec mkdir -p '${CMAKE_CURRENT_BINARY_DIR}/drivers/{}' \;; popd"
    OUTPUT_QUIET
)

# Install empty sx-kernel directory for dev package
install(
    DIRECTORY
    DESTINATION usr/src/sx-kernel
    COMPONENT dev
)

# Process all kernel module subdirectories
# Each module will register itself via set_property(GLOBAL APPEND PROPERTY KERNEL_MODULE_TARGETS ...)
add_subdirectory(drivers/net)

# Generate kernel compile_commands.json AFTER all kernel modules are built
# This target parses the .cmd files created by kbuild during compilation
if (KERNEL_COMPILE_COMMANDS AND EXISTS "${GEN_KERNEL_CC_SCRIPT}")
    # Retrieve the list of all kernel module targets (populated by subdirectories via GLOBAL property)
    get_property(KERNEL_MODULE_TARGETS GLOBAL PROPERTY KERNEL_MODULE_TARGETS)

    add_custom_target(kernel_compile_commands ALL
        COMMAND "${Python3_EXECUTABLE}" "${GEN_KERNEL_CC_SCRIPT}"
                "${CMAKE_CURRENT_SOURCE_DIR}"
                "${CMAKE_BINARY_DIR}"
                "${SX_KERNEL_MODULE_BUILD_PATH}"
                "${CMAKE_SOURCE_DIR}"
        WORKING_DIRECTORY "${SXD_KERNEL_PATH}"
        COMMENT "Generating kernel compile_commands.json from .cmd files"
    )
    # Run AFTER all kernel modules are built (they create .cmd files)
    # Dependencies ensure .cmd files exist before script runs
    if (KERNEL_MODULE_TARGETS)
        add_dependencies(kernel_compile_commands ${KERNEL_MODULE_TARGETS})
    else()
        message(WARNING "No kernel module targets found for compile_commands generation")
    endif()
endif()

# Runs "/sbin/depmod -a" on install stage, post kernel module install in dirvers subdirectory.
# This subdirectory should be last.
add_subdirectory(sx_scripts)

# SDK drivers standalone CPack configuration. Must run AFTER all install()
# commands above so CPack sees the dev component and the kernel module
# installs.
if (SDK_DRIVERS_STANDALONE_BUILD)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdk_drivers_standalone_cpack.cmake)
endif()
