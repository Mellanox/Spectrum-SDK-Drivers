#!/bin/sh
#
# SPDX-FileCopyrightText: Copyright (c) 2008-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
# property and proprietary rights in and to this material, related
# documentation and any modifications thereto. Any use, reproduction,
# disclosure or distribution of this material and related documentation
# without an express license agreement from NVIDIA CORPORATION or
# its affiliates is strictly prohibited.
#


GIT_URL=$1
GIT_BRANCH=$2

if [[ -z "$GIT_URL" ]]; then
  echo need GIT URL
  exit
fi

if [[ -z "$GIT_BRANCH" ]]; then
  echo need GIT branch
  exit
fi

mkdir sxd_kernel
cd sxd_kernel

echo === cloning bare
git clone -s --bare $GIT_URL .git
perl -ni -e "s/(bare = ).*/\${1}false/;print" .git/config
echo === checkout the checkout script
git checkout $GIT_BRANCH -- `git ls-tree -r --name-only $GIT_BRANCH sx_scripts/sx_checkout.sh`
echo === running the checkout script
./sx_scripts/sx_checkout.sh $GIT_BRANCH
git update-ref HEAD $GIT_BRANCH

echo === Done. Now exec:
echo "    cd sxd_kernel; ./configure "
echo "or:"
echo "    sx_prepare_rpm.sh"