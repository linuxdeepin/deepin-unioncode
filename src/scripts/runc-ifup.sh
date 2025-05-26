#!/bin/sh
# SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
NIC=$1
BRIDGE="runc0"
PROXY="172.16.0.1/16"

if [ `id -u` = 0 ]; then
	SUDO=""
else
	SUDO="sudo"
fi

# set runc bridge 
$SUDO ip link add $BRIDGE type bridge
$SUDO ip link set $BRIDGE up
$SUDO ip addr add $PROXY dev $BRIDGE


# set veth peer device
VETH="veth0"
CETH="ceth0"
# add veth pair devices
$SUDO ip link add name $VETH type veth peer name $CETH
$SUDO ip link set $VETH up
$SUDO ip link set dev $VETH master $BRIDGE

NETNS="runc0"
NAME="eth0"
ADDR="172.16.0.101/16"
GATEWAY="172.16.0.1"
$SUDO ip netns add $NETNS
$SUDO ip link set $CETH netns $NETNS
$SUDO ip netns exec $NETNS ip link set $CETH name $NAME
$SUDO ip netns exec $NETNS ip addr add $ADDR dev $NAME
$SUDO ip netns exec $NETNS ip link set $NAME up
$SUDO ip netns exec $NETNS ip route add default via $GATEWAY


# set iptables 
# snat 
$SUDO iptables -t nat -I POSTROUTING 1 --source $PROXY -o $NIC -j MASQUERADE
# filer eth0 <--> runc0
$SUDO iptables -t filter -A FORWARD -o $NIC -i $BRIDGE -j ACCEPT
$SUDO iptables -t filter -A FORWARD -i $NIC -o $BRIDGE -j ACCEPT

exit 0

