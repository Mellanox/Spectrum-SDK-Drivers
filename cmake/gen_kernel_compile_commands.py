#!/usr/bin/env python3
"""
Generate kernel module compile_commands.json entries from kbuild .cmd files.

This script parses the .cmd files created by the kernel build system (kbuild)
after compilation. These files contain the exact compilation commands used,
which is more reliable than parsing make dry-run output (compiledb approach).

Usage:
    python3 gen_kernel_compile_commands.py <kernel_source_dir> <cmake_binary_dir> <kernel_build_dir> [workspace_root]

Example:
    python3 gen_kernel_compile_commands.py /path/to/sxd_kernel /path/to/builds/cmake-xxx /lib/modules/$(uname -r)/build /path/to/workspace
"""

import argparse
import glob
import json
import os
import re
import shlex
import sys


def find_cmd_files(kernel_source_dir):
    """Find all .o.cmd files in kernel module directories (recursively)."""
    # Use glob with recursive pattern to find all hidden .o.cmd files
    # Pattern: drivers/net/**/.*.o.cmd (recursive, hidden files ending in .o.cmd)
    pattern = os.path.join(kernel_source_dir, "drivers/net/**/.*.o.cmd")
    return glob.glob(pattern, recursive=True)


def parse_cmd_file(cmd_file, kernel_build_dir):
    """
    Parse a .cmd file and extract the compilation command.

    .cmd file format:
        cmd_/path/to/file.o := gcc-<version> ... -c -o file.o file.c ; objtool ...

    Args:
        cmd_file: Path to the .cmd file
        kernel_build_dir: Kernel build directory (working directory for compilation)

    Returns:
        dict with 'directory', 'arguments', 'file' keys, or None if not a valid compilation
    """
    # Skip .mod.o.cmd files (auto-generated module linkage, not actual source)
    if ".mod.o.cmd" in cmd_file:
        return None

    try:
        with open(cmd_file, "r") as f:
            content = f.read()
    except (IOError, OSError) as e:
        print(f"Warning: Could not read {cmd_file}: {e}", file=sys.stderr)
        return None

    # Extract the cmd_... := line
    # Format: cmd_/path/to/file.o := <compilation command> ; <objtool command>
    match = re.search(r"^(?:saved)?cmd_[^\s]+ := (.+?)(?:\s*;|$)", content, re.MULTILINE | re.DOTALL)
    if not match:
        return None

    cmd_line = match.group(1).strip()

    # Extract the source file from the command
    # Find ALL .c files in the command, then take the last one
    # Pattern matches: /absolute/path.c, ./relative/path.c, ../path.c, or just file.c
    # This handles cases where .c file is not at the very end (e.g., has trailing arguments)
    all_c_files = re.findall(r"(\S+\.c)", cmd_line)
    if not all_c_files:
        return None

    # Use the last .c file found (typically the source being compiled)
    source_file_original = all_c_files[-1]

    # If relative path, convert to absolute
    if not os.path.isabs(source_file_original):
        # Relative paths in .cmd files are typically relative to where make was invoked
        # The .cmd file itself is in the build tree at the same relative location
        # So we can use the .cmd file's directory as a reference
        cmd_dir = os.path.dirname(os.path.abspath(cmd_file))
        source_file_original = os.path.abspath(os.path.join(cmd_dir, source_file_original))

    # Normalize path: resolve symlinks to canonical absolute path
    # This ensures the path matches however the IDE/clangd resolves it
    source_file = os.path.realpath(source_file_original)

    # Clean up the command - remove everything after objtool or other post-processing
    cmd_clean = re.sub(r"\s*;\s*\./tools/objtool.*$", "", cmd_line)

    # Parse arguments (split on whitespace, but this is simplified)
    args = cmd_clean.split()

    # CRITICAL: Replace the original source path with the normalized one in arguments
    for i in range(len(args)):
        if args[i] == source_file_original:
            args[i] = source_file
            break

    # Verify this is a compilation command (has -c flag)
    if "-c" not in args:
        return None

    # Verify this is NOT a pre-processing command
    if "-E" in args and "-D__GENKSYMS__" in args:
        return None

    # Convert arguments array to command string to match CMake's format
    # This ensures consistent format with applibs entries (avoids mixed format issues)
    command = " ".join(shlex.quote(arg) for arg in args)

    # Use the provided kernel build directory as the working directory
    # This is where relative includes like ./arch/x86/include/generated resolve
    return {"directory": kernel_build_dir, "command": command, "file": source_file}


def load_compile_commands(compile_commands_path):
    """Load existing compile_commands.json or return empty list."""
    if not os.path.exists(compile_commands_path):
        return []

    try:
        with open(compile_commands_path, "r") as f:
            return json.load(f)
    except (IOError, json.JSONDecodeError) as e:
        print(f"Warning: Could not load {compile_commands_path}: {e}", file=sys.stderr)
        return []


def normalize_paths(entries, workspace_root):
    """
    Normalize all paths to match the workspace root format used by IDE.

    Issue: NFS mounts/symlinks can appear with different prefixes:
      - /mtrsysgwork/...
      - /auto/mtrsysgwork/...
      - /.autodirect/mtrsysgwork/...

    Build systems may use one format while IDEs use another, causing clangd path mismatches
    that break cross-file "Find All References".

    Solution: Use os.path.realpath() on workspace_root to get the canonical path format,
    then normalize all entry paths to match that format.

    Args:
        entries: List of compile_commands.json entries
        workspace_root: Workspace root directory (used to detect canonical path format)

    Returns:
        entries with normalized paths
    """
    # Get canonical workspace path (resolves symlinks, relative paths, etc.)
    canonical_workspace = os.path.realpath(workspace_root)

    # Detect all possible path variants by checking common NFS mount prefixes
    # We'll normalize from any of these to the canonical format
    path_variants = []

    # Extract the distinctive part after common prefixes
    for prefix in ["/.autodirect/", "/auto/", "/"]:
        if canonical_workspace.startswith(prefix):
            # Get the part after the prefix
            suffix = canonical_workspace[len(prefix):]
            # Add all possible variants
            path_variants.append(("/.autodirect/" + suffix, canonical_workspace))
            path_variants.append(("/auto/" + suffix, canonical_workspace))
            # Handle the bare path (no .autodirect or auto prefix)
            if "/" + suffix != canonical_workspace:
                path_variants.append(("/" + suffix, canonical_workspace))
            break

    # If no prefix matched, just use realpath normalization
    if not path_variants:
        for entry in entries:
            if "file" in entry:
                entry["file"] = os.path.realpath(entry["file"])
            if "directory" in entry:
                entry["directory"] = os.path.realpath(entry["directory"])
        return entries

    # Normalize paths in all entries
    for entry in entries:
        # Normalize file path
        if "file" in entry:
            for old_path, new_path in path_variants:
                if entry["file"].startswith(old_path):
                    entry["file"] = entry["file"].replace(old_path, new_path, 1)
                    break

        # Normalize directory path
        if "directory" in entry:
            for old_path, new_path in path_variants:
                if entry["directory"].startswith(old_path):
                    entry["directory"] = entry["directory"].replace(old_path, new_path, 1)
                    break

        # Normalize command paths
        if "command" in entry:
            for old_path, new_path in path_variants:
                entry["command"] = entry["command"].replace(old_path, new_path)

    return entries


def save_compile_commands(compile_commands_path, entries):
    """Save compile_commands.json."""
    try:
        with open(compile_commands_path, "w") as f:
            json.dump(entries, f, indent=2)
    except IOError as e:
        print(f"Error: Could not write {compile_commands_path}: {e}", file=sys.stderr)
        return False
    return True


def main():
    parser = argparse.ArgumentParser(
        description="Generate kernel compile_commands.json from kbuild .cmd files"
    )
    parser.add_argument("kernel_source_dir", help="Path to sxd_kernel source directory")
    parser.add_argument("cmake_binary_dir", help="Path to cmake binary/build directory")
    parser.add_argument("kernel_build_dir", help="Kernel build directory (e.g., /lib/modules/.../build)")
    parser.add_argument("workspace_root", nargs="?", default=None,
                        help="Workspace root for the merged compile_commands.json copy "
                             "(defaults to kernel_source_dir if omitted)")
    args = parser.parse_args()

    kernel_source_dir = os.path.abspath(args.kernel_source_dir)
    cmake_binary_dir = os.path.abspath(args.cmake_binary_dir)
    compile_commands_path = os.path.join(cmake_binary_dir, "compile_commands.json")

    # Normalize and validate kernel build directory
    kernel_build_dir = os.path.realpath(args.kernel_build_dir)
    if not os.path.isdir(kernel_build_dir):
        print(f"Warning: Kernel build directory does not exist: {kernel_build_dir}", file=sys.stderr)
        print(f"Using it anyway (may cause clangd issues with relative includes)", file=sys.stderr)

    if args.workspace_root:
        workspace_root = os.path.abspath(args.workspace_root)
    else:
        workspace_root = kernel_source_dir

    # Find all .cmd files
    cmd_files = find_cmd_files(kernel_source_dir)
    if not cmd_files:
        print(f"Warning: No .cmd files found in {kernel_source_dir}", file=sys.stderr)
        print("Kernel modules may not have been built yet.", file=sys.stderr)
        return 0

    # Parse .cmd files
    kernel_entries = []
    for cmd_file in cmd_files:
        entry = parse_cmd_file(cmd_file, kernel_build_dir)
        if entry:
            kernel_entries.append(entry)

    print(f"Parsed {len(kernel_entries)} kernel compilation entries from {len(cmd_files)} .cmd files")

    # Load existing compile_commands.json
    existing_entries = load_compile_commands(compile_commands_path)
    original_count = len(existing_entries)

    # Remove old kernel entries: any entry whose file resolves under the
    # kernel source dir. Path-based rather than substring-based so a
    # checkout directory not literally named "sxd_kernel" still works.
    canonical_kernel_source = os.path.realpath(kernel_source_dir)

    def _is_kernel_entry(entry):
        file_path = entry.get("file", "")
        if not file_path:
            return False
        try:
            return os.path.commonpath([os.path.realpath(file_path),
                                       canonical_kernel_source]) == canonical_kernel_source
        except ValueError:
            return False
    non_kernel_entries = [e for e in existing_entries if not _is_kernel_entry(e)]
    removed_count = original_count - len(non_kernel_entries)

    # Merge: non-kernel entries + new kernel entries
    merged_entries = non_kernel_entries + kernel_entries

    # Normalize all paths to match workspace root format (for IDE compatibility)
    canonical_workspace = os.path.realpath(workspace_root)
    print(f"Normalizing paths to match workspace format: {canonical_workspace}")
    merged_entries = normalize_paths(merged_entries, workspace_root)

    # Save
    if save_compile_commands(compile_commands_path, merged_entries):
        print(f"Updated {compile_commands_path}")
        print(f"  Removed {removed_count} old kernel entries")
        print(f"  Added {len(kernel_entries)} new kernel entries")
        print(f"  Total entries: {len(merged_entries)}")

        # Copy to workspace root for clangd. Skip if it's already the same
        # file as the source (e.g. when workspace_root happens to be the
        # cmake binary dir).
        workspace_compile_commands = os.path.join(workspace_root, "compile_commands.json")
        if os.path.realpath(workspace_compile_commands) == os.path.realpath(compile_commands_path):
            print(f"Workspace root copy skipped (same as {compile_commands_path})")
        else:
            try:
                import shutil
                shutil.copy2(compile_commands_path, workspace_compile_commands)
                print(f"Copied to {workspace_compile_commands}")
            except IOError as e:
                print(f"Warning: Could not copy to workspace root: {e}", file=sys.stderr)
    else:
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
