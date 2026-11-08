
# Build options used by the SDK kernel drivers.

option(MEMTRACK "Use memtrack in kernel module" OFF)
option(PD_PRESENT "Palladium bring up" OFF)
option(KERNEL_COMPILE_COMMANDS "Use compiledb to add kernel modules to compile_commands.json (requires compiledb)" OFF)
# In CMake 3.21 there is new option to set CPACK_CUSTOM_INSTALL_VARIABLES,
# So skip will set in cpack run time, and not in configuration time.
option(SKIP_DEPMOD_IN_INSTALL_PHASE "Skip run of \"depmod -a\" in install phase, will be run in package post-install" OFF)
