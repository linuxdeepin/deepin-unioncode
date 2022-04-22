#!/bin/bash

PLATFORM="$HOME/.config/unioncode/configures/platform"
cd $PLATFORM
set -x
ARCH=$1
NIC="$(ip route | grep default | awk '{print $5}')"
CONTAINER_ID=$2
PROJECT_PATH=$3
CID=${CONTAINER_ID:0:7}
CID_NETMASK="16"
CID_IP="172.16.0.101/16"
VETH_ID="veth${CID}"
NETNS_ID="netns${CID}"
BR_ID="br${CID}"
BR_IP="172.16.0.1/16"
BR_GW="172.16.0.1"
MIRROR="https://pools.uniontech.com/deepin"

sudo ip netns add ${NETNS_ID}
sudo ip link add ${BR_ID} type bridge
sudo ip link set ${BR_ID} up
sudo ip addr add ${BR_IP} dev ${BR_ID} 
sudo ip link add ${VETH_ID} type veth peer name ceth0
sudo ip link set ${VETH_ID} master ${BR_ID} 
sudo ip link set ceth0 netns ${NETNS_ID}
sudo ip link set ${VETH_ID} up
sudo ip netns exec ${NETNS_ID} ip link set lo up
sudo ip netns exec ${NETNS_ID} ip link set ceth0 up
sudo ip netns exec ${NETNS_ID} ip addr add ${CID_IP} dev ceth0
sudo ip netns exec ${NETNS_ID} ip route add default via ${BR_GW}

sudo iptables -t nat -I POSTROUTING 1 --source ${BR_IP} -o ${NIC} -j MASQUERADE
sudo iptables -t filter -A FORWARD -o ${NIC} -i ${BR_ID} -j ACCEPT
sudo iptables -t filter -A FORWARD -i ${NIC} -o ${BR_ID} -j ACCEPT
set +x
cd $ARCH 
if [ ! -e ${CONTAINER_ID} ]; then
	mkdir -pv ${CONTAINER_ID}

cat>>${CONTAINER_ID}/config.json<<EOF
{
	"ociVersion": "1.0.1",
	"process": {
		"terminal": true,
		"user": {
			"uid": 0,
			"gid": 0
		},
		"args": [
			"bash"
		],
		"env": [
			"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
			"TERM=xterm-256color",
			"SHELL=/bin/bash",
			"DISPLAY=:0"
		],
		"cwd": "/",
		"capabilities": {
			"bounding": [
				"CAP_AUDIT_WRITE",
				"CAP_KILL",
				"CAP_NET_BIND_SERVICE",
				"CAP_NET_RAW",
				"CAP_NET_ADMIN",
				"CAP_CHOWN",
				"CAP_DAC_OVERRIDE",
				"CAP_FSETID",
				"CAP_FOWNER",
				"CAP_MKNOD",
				"CAP_SETGID",
				"CAP_SETUID",
				"CAP_SETFCAP",
				"CAP_SETPCAP",
				"CAP_SYS_CHROOT"
			],
			"effective": [
				"CAP_AUDIT_WRITE",
				"CAP_KILL",
				"CAP_NET_BIND_SERVICE",
				"CAP_NET_RAW",
				"CAP_NET_ADMIN",
				"CAP_CHOWN",
				"CAP_DAC_OVERRIDE",
				"CAP_FSETID",
				"CAP_FOWNER",
				"CAP_MKNOD",
				"CAP_SETGID",
				"CAP_SETUID",
				"CAP_SETFCAP",
				"CAP_SETPCAP",
				"CAP_SYS_CHROOT"
			],
			"inheritable": [
				"CAP_AUDIT_WRITE",
				"CAP_KILL",
				"CAP_NET_BIND_SERVICE",
				"CAP_NET_RAW",
				"CAP_NET_ADMIN",
				"CAP_CHOWN",
				"CAP_DAC_OVERRIDE",
				"CAP_FSETID",
				"CAP_FOWNER",
				"CAP_MKNOD",
				"CAP_SETGID",
				"CAP_SETUID",
				"CAP_SETFCAP",
				"CAP_SETPCAP",
				"CAP_SYS_CHROOT"
			],
			"permitted": [
				"CAP_AUDIT_WRITE",
				"CAP_KILL",
				"CAP_NET_BIND_SERVICE",
				"CAP_NET_RAW",
				"CAP_NET_ADMIN",
				"CAP_CHOWN",
				"CAP_DAC_OVERRIDE",
				"CAP_FSETID",
				"CAP_FOWNER",
				"CAP_MKNOD",
				"CAP_SETGID",
				"CAP_SETUID",
				"CAP_SETFCAP",
				"CAP_SETPCAP",
				"CAP_SYS_CHROOT"
			],
			"ambient": [
				"CAP_AUDIT_WRITE",
				"CAP_KILL",
				"CAP_NET_BIND_SERVICE",
				"CAP_NET_RAW",
				"CAP_NET_ADMIN",
				"CAP_CHOWN",
				"CAP_DAC_OVERRIDE",
				"CAP_FSETID",
				"CAP_FOWNER",
				"CAP_MKNOD",
				"CAP_SETGID",
				"CAP_SETUID",
				"CAP_SETFCAP",
				"CAP_SETPCAP",
				"CAP_SYS_CHROOT"
			]
		},
		"rlimits": [
			{
				"type": "RLIMIT_NOFILE",
				"hard": 1024,
				"soft": 1024
			}
		],
		"noNewPrivileges": true
	},
	"root": {
		"path": "rootfs",
		"readonly": false
	},
	"hostname": "${CONTAINER_ID}",
	"mounts": [
		{
			"destination": "/proc",
			"type": "proc",
			"source": "proc"
		},
		{
			"destination": "/dev",
			"type": "tmpfs",
			"source": "tmpfs",
			"options": [
				"nosuid",
				"strictatime",
				"mode=755",
				"size=65536k"
			]
		},
		{
			"destination": "/dev/pts",
			"type": "devpts",
			"source": "devpts",
			"options": [
				"nosuid",
				"noexec",
				"newinstance",
				"ptmxmode=0666",
				"mode=0620",
				"gid=5"
			]
		},
		{
			"destination": "/dev/shm",
			"type": "tmpfs",
			"source": "shm",
			"options": [
				"nosuid",
				"noexec",
				"nodev",
				"mode=1777",
				"size=65536k"
			]
		},
		{
			"destination": "/dev/mqueue",
			"type": "mqueue",
			"source": "mqueue",
			"options": [
				"nosuid",
				"noexec",
				"nodev"
			]
		},
		{
			"destination": "/sys",
			"type": "sysfs",
			"source": "sysfs",
			"options": [
				"nosuid",
				"noexec",
				"nodev",
				"ro"
			]
		},
		{
			"destination": "/sys/fs/cgroup",
			"type": "cgroup",
			"source": "cgroup",
			"options": [
				"nosuid",
				"noexec",
				"nodev",
				"relatime",
				"ro"
			]
		},
		{
			"destination": "${PROJECT_PATH}",
			"type": "bind",
			"source": "${PROJECT_PATH}",
			"options": [
				"rbind",
				"rw"
			]
		}
	],
	"linux": {
		"resources": {
			"devices": [
				{
					"allow": false,
					"access": "rwm"
				}
			]
		},
		"namespaces": [
			{
				"type": "pid"
			},
			{
				"type": "network",
				"path": "/var/run/netns/${NETNS_ID}"
			},
			{
				"type": "ipc"
			},
			{
				"type": "uts"
			},
			{
				"type": "mount"
			}
		],
		"maskedPaths": [
			"/proc/kcore",
			"/proc/latency_stats",
			"/proc/timer_list",
			"/proc/timer_stats",
			"/proc/sched_debug",
			"/sys/firmware",
			"/proc/scsi"
		],
		"readonlyPaths": [
			"/proc/asound",
			"/proc/bus",
			"/proc/fs",
			"/proc/irq",
			"/proc/sys",
			"/proc/sysrq-trigger"
		]
	}
}
EOF

fi

# import rootfs from docker image 
#mkdir rootfs
#docker export $(docker image uos-amd64) > uos-amd64.tar
#tar -C rootfs -xvf uos-amd64.tar

BUILD_INCLUDE_DEBS1="git,subversion,build-essential,crossbuild-essential-amd64,crossbuild-essential-arm64,crossbuild-essential-mips64el"
BUILD_INCLUDE_DEBS2="cmake,ninja-build,llvm,clang,libqt5dbus5"
INCLUDE_DEBS="$BUILD_INCLUDE_DEBS1,$BUILD_INCLUDE_DEBS2"
if [ ! -d rootfs ]; then
	sudo debootstrap --arch=$ARCH --include=$INCLUDE_DEBS apricot ${CONTAINER_ID}/rootfs ${MIRROR}
fi
sudo runc kill ${CONTAINER_ID} KILL > /dev/null 2>&1
# mount bind workspaces to runc container
# mount -o bind $HOME/workspaces $RUNC_CID_ROOTFS
sudo runc run --bundle ${CONTAINER_ID} ${CONTAINER_ID}

exit 0
