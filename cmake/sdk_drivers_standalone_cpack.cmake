
# SDK drivers standalone CPack configuration.
#
# Included from sxd_kernel/CMakeLists.txt after all install() commands
# have been registered (CPack must see them - in particular the COMPONENT
# dev entries - before include(CPack) runs, otherwise no -dev package is
# produced).

set(CPACK_GENERATOR "DEB")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_DEBUGINFO_PACKAGE ON)

# Parallelize package compression: CPACK_THREADS=0 uses all cores. Compressor:
# default xz, which keeps the .deb installable by any dpkg version (a zstd .deb
# needs dpkg >= 1.21.18). USE_ZSTD_DEB_COMPRESSION opts into zstd (much faster) for
# build==target environments; it is passed through to this standalone-drivers
# configure by install.sh / the build wrapper. Being opt-in it is honored strictly:
# if enabled where this CMake cannot create a zstd archive or the local dpkg cannot
# install one, configuration fails hard (see below) rather than downgrading to xz.
# CPACK_DEBIAN_PACKAGE_DEBUG adds verbose logs.
set(CPACK_THREADS 0)
set(CPACK_DEBIAN_PACKAGE_DEBUG ON)
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
if (USE_ZSTD_DEB_COMPRESSION)
    # Strict opt-in: require BOTH this CMake/libarchive to create a zstd archive AND
    # the local dpkg (build==target, must be >= 1.21.18) to install one. Fail hard on
    # either rather than silently producing xz.
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar cf ${CMAKE_BINARY_DIR}/.zstd_probe.tar.zst --zstd ${CMAKE_CURRENT_LIST_FILE}
        RESULT_VARIABLE _zstd_cmake_rc OUTPUT_QUIET ERROR_QUIET)
    file(REMOVE ${CMAKE_BINARY_DIR}/.zstd_probe.tar.zst)
    execute_process(COMMAND dpkg-deb --help RESULT_VARIABLE _dpkg_rc OUTPUT_VARIABLE _dpkg_help ERROR_QUIET)
    string(FIND "${_dpkg_help}" "zstd" _dpkg_zstd_pos)
    if (NOT _zstd_cmake_rc EQUAL 0)
        message(FATAL_ERROR "USE_ZSTD_DEB_COMPRESSION=ON but this CMake/libarchive cannot create zstd "
                            "archives. Use a zstd-capable CMake (>= 3.22 built with libzstd) or drop the option.")
    elseif (NOT _dpkg_rc EQUAL 0 OR _dpkg_zstd_pos EQUAL -1)
        message(FATAL_ERROR "USE_ZSTD_DEB_COMPRESSION=ON but the local dpkg cannot install zstd .debs "
                            "(needs dpkg >= 1.21.18). Drop USE_ZSTD_DEB_COMPRESSION or upgrade dpkg.")
    else()
        set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd")
        message(STATUS "DEB payload compression: zstd (USE_ZSTD_DEB_COMPRESSION=ON)")
    endif()
endif()

set(CPACK_PACKAGE_NAME sx_kernel)
set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "Tamar Warshai")
set(CPACK_PACKAGE_VENDOR "Nvidia")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

# Debian
string(REPLACE "_" "-" _cpack_deb_name ${CPACK_PACKAGE_NAME})
set(CPACK_DEBIAN_PACKAGE_NAME ${_cpack_deb_name})
string(REGEX REPLACE "_|-" "." _cpack_deb_version ${CPACK_PACKAGE_VERSION})
set(CPACK_DEBIAN_PACKAGE_VERSION "1.mlnx.${_cpack_deb_version}")
execute_process (
    COMMAND dpkg --print-architecture
    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
)
string(REGEX REPLACE "\n$" "" CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${SXD_KERNEL_PATH}/cmake/postinst)
set(CPACK_DEBIAN_DEV_PACKAGE_CONTROL_EXTRA "${SXD_KERNEL_PATH}/cmake/postinst_dev")
set(CPACK_DEBIAN_FILE_NAME ${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb)
set(CPACK_DEBIAN_DEV_FILE_NAME ${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}-dev.deb)

# RPM
set(CPACK_RPM_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
string(REPLACE "-" "_" _cpack_rpm_version ${CPACK_PACKAGE_VERSION})
set(CPACK_RPM_PACKAGE_VERSION ${_cpack_rpm_version})
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${SXD_KERNEL_PATH}/cmake/postinst)
list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/lib")
set(CPACK_RPM_SPEC_MORE_DEFINE "
%undefine __brp_mangle_shebangs
%define __strip /bin/true
%global _binary_payload w7.zstdio
")

include(CPack)

# Development package
cpack_add_component(dev
                    DISPLAY_NAME "Development"
                    DESCRIPTION  "Development Package")
