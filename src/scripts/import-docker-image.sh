#!/bin/sh

# import docker image as runc rootfs
IMAGE=$1
ROOTFS="rootfs"

docker export $(docker create $IMAGE) > $IMAGE.tar
tar -C $ROOTFS -xvf $IMAGE.tar
