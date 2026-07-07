
# SDK drivers standalone CPack configuration.
#
# Included from sxd_kernel/CMakeLists.txt after all install() commands
# have been registered (CPack must see them - in particular the COMPONENT
# dev entries - before include(CPack) runs, otherwise no -dev package is
# produced).

set(CPACK_GENERATOR "DEB")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_DEBUGINFO_PACKAGE ON)

# Parallelize package compression: CPACK_THREADS=0 uses all cores, and xz multithreads with it. xz
# (not zstd) keeps the .deb installable by any dpkg version on the target machines (dpkg < 1.21.18
# cannot install zstd-compressed .debs). CPACK_DEBIAN_PACKAGE_DEBUG adds verbose generator logs.
set(CPACK_THREADS 0)
set(CPACK_DEBIAN_PACKAGE_DEBUG ON)
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

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
