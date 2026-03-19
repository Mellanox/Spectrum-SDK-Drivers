
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
# Uses compiledb with make dry-run to generate compile commands at build time
if (KERNEL_COMPILE_COMMANDS)
    find_program(COMPILEDB_EXECUTABLE compiledb)
    if (COMPILEDB_EXECUTABLE)
        message(STATUS "compiledb found: ${COMPILEDB_EXECUTABLE} - will generate kernel compile commands at build time")

        # Get list of kernel module directories dynamically by finding all Makefiles
        file(GLOB_RECURSE KERNEL_MAKEFILES ${CMAKE_CURRENT_SOURCE_DIR}/drivers/net/*/Makefile)
        set(KERNEL_MODULE_DIRS "")
        foreach(MAKEFILE ${KERNEL_MAKEFILES})
            get_filename_component(MODULE_DIR ${MAKEFILE} DIRECTORY)
            # Only include if it's a kernel module (has obj-m) and matches MEMTRACK setting
            file(READ ${MAKEFILE} MAKEFILE_CONTENT)
            if (MAKEFILE_CONTENT MATCHES "obj-m")
                # Check if this is memtrack - only add if MEMTRACK is enabled
                if (MODULE_DIR MATCHES "memtrack")
                    if (MEMTRACK)
                        list(APPEND KERNEL_MODULE_DIRS ${MODULE_DIR})
                    endif()
                else()
                    list(APPEND KERNEL_MODULE_DIRS ${MODULE_DIR})
                endif()
            endif()
        endforeach()

        # Build the compiledb command list for all modules
        set(COMPILEDB_COMMANDS "")
        foreach(MODULE_DIR ${KERNEL_MODULE_DIRS})
            list(APPEND COMPILEDB_COMMANDS
                COMMAND ${COMPILEDB_EXECUTABLE} --no-build -o ${CMAKE_BINARY_DIR}/compile_commands.json
                        make -C ${SX_KERNEL_MODULE_BUILD_PATH}
                        SX_SDK_VERSION=${CMAKE_PROJECT_VERSION}
                        SXD_KERNEL_EXTRA_CFLAGS=${SXD_KERNEL_EXTRA_CFLAGS_VAR}
                        KCFLAGS=${KCFLAGS}
                        ${EXTRA_OPTIONS}
                        M=${MODULE_DIR}
                        modules
            )
        endforeach()

        # Create a custom target that runs before kernel module builds
        set(KERNEL_CC_TARGET_NAME kernel_compile_commands)
        add_custom_target(${KERNEL_CC_TARGET_NAME}
            ${COMPILEDB_COMMANDS}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Generating kernel module compile commands with compiledb"
        )
    else()
        message(WARNING "compiledb not found - install with 'pip install compiledb' for kernel compile_commands.json")
    endif()
endif()

# Macro for kernel modules to add compiledb dependency if enabled
macro(add_kernel_module_compiledb_dependency target_name)
    if (KERNEL_COMPILE_COMMANDS AND COMPILEDB_EXECUTABLE)
        add_dependencies(${target_name} ${KERNEL_CC_TARGET_NAME})
    endif()
endmacro()

# Common module compilation command
set (MODULE_CMD_COMMON $(MAKE) -C ${SX_KERNEL_MODULE_BUILD_PATH} SX_SDK_VERSION=${CMAKE_PROJECT_VERSION} SXD_KERNEL_EXTRA_CFLAGS=${SXD_KERNEL_EXTRA_CFLAGS_VAR} KCFLAGS=${KCFLAGS} ${EXTRA_OPTIONS})

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

add_subdirectory(drivers/net)

# Runs "/sbin/depmod -a" on install stage, post kernel module install in dirvers subdirectory.
# This subdirectory should be last.
add_subdirectory(sx_scripts)
