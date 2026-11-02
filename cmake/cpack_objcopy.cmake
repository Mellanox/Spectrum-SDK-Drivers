# CPack project config file: route CPack's debuginfo strip step through
# objcopy_strip_debug.sh, handing it the objcopy CPack would have used.
# CMake >= 3.25 reads CPACK_OBJCOPY_EXECUTABLE (defaults to the toolchain's
# objcopy); older versions read OBJCOPY_EXECUTABLE and search PATH.
if (NOT SX_CPACK_REAL_OBJCOPY)
    if (CPACK_OBJCOPY_EXECUTABLE)
        set(SX_CPACK_REAL_OBJCOPY ${CPACK_OBJCOPY_EXECUTABLE})
    elseif (OBJCOPY_EXECUTABLE)
        set(SX_CPACK_REAL_OBJCOPY ${OBJCOPY_EXECUTABLE})
    else()
        find_program(SX_CPACK_REAL_OBJCOPY NAMES objcopy)
    endif()
endif()
set(CPACK_OBJCOPY_EXECUTABLE ${CMAKE_CURRENT_LIST_DIR}/objcopy_strip_debug.sh ${SX_CPACK_REAL_OBJCOPY})
set(OBJCOPY_EXECUTABLE ${CPACK_OBJCOPY_EXECUTABLE})
