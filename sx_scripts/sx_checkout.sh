#!/bin/bash
#
# SPDX-FileCopyrightText: Copyright (c) 2008-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
# property and proprietary rights in and to this material, related
# documentation and any modifications thereto. Any use, reproduction,
# disclosure or distribution of this material and related documentation
# without an express license agreement from NVIDIA CORPORATION or
# its affiliates is strictly prohibited.
#

# get ref to "real" stdout
exec 3>&1

# Execute command w/ echo and exit if it fails
ex()
{
        echo "$@" >&3
        eval "$@"
        if [ $? -ne 0 ]; then
                printf "\nFailed executing $*\n" >&3
                exit 1
        fi
}

# Like ex above, but command is self echoing on stderr
xex()
{
        eval "$@" 2>&3
        if [ $? -ne 0 ]; then
                printf "\nFailed executing $*\n" >&3
                exit 1
        fi
}

# branch defaults to master
branch=${1:-master}

# make sure we have the list of all the other files and dirs
if [ ! -e sx_scripts/checkout_files ]; then
	ex git checkout ${branch} sx_scripts/checkout_files
fi

xex xargs -rt git ls-tree -r --name-only ${branch} <sx_scripts/checkout_files |
	xex xargs -rt git checkout ${branch}

rm -f sx_scripts/checkout_files

ex git update-ref HEAD ${branch}

# shellcheck disable=SC2226    # the destination is the current directory, spelled by omission
ln -snf sx_scripts/configure
# shellcheck disable=SC2226    # the destination is the current directory, spelled by omission
ln -snf sx_scripts/Makefile
# shellcheck disable=SC2226    # the destination is the current directory, spelled by omission
ln -snf sx_scripts/makefile