#!/bin/sh
# SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
# import docker image as runc rootfs
IMAGE=$1
ROOTFS="rootfs"

docker export $(docker create $IMAGE) > $IMAGE.tar
tar -C $ROOTFS -xvf $IMAGE.tar
