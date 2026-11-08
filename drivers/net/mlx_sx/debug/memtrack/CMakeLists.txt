
set (TARGET_NAME memtrack)
set (MODULE_NAME ${TARGET_NAME}.ko)
set (TARGET_NAME_CLEAN ${TARGET_NAME}_clean)
file(RELATIVE_PATH SXD_MODULE_PATH ${SXD_KERNEL_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

set (MODULE_CMD ${MODULE_CMD_COMMON} M=${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target (
    ${TARGET_NAME} ALL
    COMMAND ${MODULE_CMD} modules
    COMMENT "Building ${MODULE_NAME}"
    VERBATIM
)

# Add compiledb dependency if enabled
add_kernel_module_compiledb_dependency(${TARGET_NAME})

add_custom_target (${TARGET_NAME_CLEAN}
    COMMAND ${MODULE_CMD} clean
    COMMENT "Cleaning ${MODULE_NAME}"
)

install (
    PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}
    DESTINATION ${MODULES_PATH}/update/kernel/${SXD_MODULE_PATH}
)

