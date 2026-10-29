#!/bin/sh
#
# SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
# property and proprietary rights in and to this material, related
# documentation and any modifications thereto. Any use, reproduction,
# disclosure or distribution of this material and related documentation
# without an express license agreement from NVIDIA CORPORATION or
# its affiliates is strictly prohibited.
#
# objcopy wrapper that CPack calls instead of objcopy (see cpack_objcopy.cmake).
#
# CPack moves debug info to the dbgsym package and then strips every binary
# with --strip-unneeded; that flag is hard-coded in CMake's CPackDeb module.
# On a kernel module it also drops the names of static functions. The kernel
# names oops frames from the module's own symbol table, so a frame inside a
# static function is printed under the exported function before it.
# For .ko files keep the names of static functions (and nothing else the
# strip would drop): every extra symbol slows the kernel's module load.
# Everything else is passed through unchanged. The first argument is the
# objcopy CPack would have run itself.

objcopy=$1
shift
case "$*" in
    "--strip-unneeded "*.ko)
        syms=$(mktemp) || exit 1
        readelf -sW "$2" > "$syms" || { rm -f "$syms"; exit 1; }
        awk '$4 == "FUNC" && $5 == "LOCAL" { print $8 }' "$syms" > "$syms.keep"
        "$objcopy" --strip-unneeded --keep-symbols="$syms.keep" "$2"
        rc=$?
        rm -f "$syms" "$syms.keep"
        exit $rc
        ;;
esac
exec "$objcopy" "$@"
