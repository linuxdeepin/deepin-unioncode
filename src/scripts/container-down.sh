#!/bin/bash

CONTAINER_ID=$1
CID=${CONTAINER_ID:0:7}
NIC="$(ip route | grep default | awk '{print $5}')"
NETNS_ID="netns${CID}"
BR_ID="br${CID}"
BR_IP="172.16.0.1/16"

sudo iptables -t nat -D POSTROUTING  --source ${BR_IP} -o ${NIC} -j MASQUERADE
sudo iptables -t filter -D FORWARD -o ${NIC} -i ${BR_ID} -j ACCEPT
sudo iptables -t filter -D FORWARD -i ${NIC} -o ${BR_ID} -j ACCEPT

sudo ip netns delete ${NETNS_ID}
sudo ip link delete ${BR_ID}

set +x
