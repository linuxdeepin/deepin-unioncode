#!/bin/bash

cd $HOME
NIC=$(ip route | grep default | awk '{print $5}')
BRIDGE="runc0"
CONTAINER_ID="uos-amd64"
MIRROR="https://pools.uniontech.com/deepin"

sudo ip netns add ${CONTAINER_ID}
sudo ip link add ${BRIDGE} type bridge
sudo ip link set ${BRIDGE} up
sudo ip addr add 172.16.0.1/16 dev ${BRIDGE} 
sudo ip link add veth-${CONTAINER_ID} type veth peer name ceth0
sudo ip link set veth-${CONTAINER_ID} master ${BRIDGE} 
sudo ip link set ceth0 netns ${CONTAINER_ID}
sudo ip link set veth-${CONTAINER_ID} up
sudo ip netns exec ${CONTAINER_ID} ip link set lo up
sudo ip netns exec ${CONTAINER_ID} ip link set ceth0 up
sudo ip netns exec ${CONTAINER_ID} ip addr add 172.16.0.101/16 dev ceth0
sudo ip netns exec ${CONTAINER_ID} ip route add default via 172.16.0.1

sudo iptables -t nat -I POSTROUTING 1 --source 172.16.0.1/16 -o ${NIC} -j MASQUERADE
sudo iptables -t filter -A FORWARD -o ${NIC} -i ${BRIDGE} -j ACCEPT
sudo iptables -t filter -A FORWARD -i ${NIC} -o ${BRIDGE} -j ACCEPT

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
			"SHELL=/bin/bash"
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
	"hostname": "uos-amd64",
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
				"path": "/var/run/netns/${CONTAINER_ID}"
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

sudo debootstrap --arch=amd64 apricot ${CONTAINER_ID}/rootfs ${MIRROR}
sudo runc kill ${CONTAINER_ID} KILL > /dev/null 2>&1
sudo runc run --bundle ${CONTAINER_ID} ${CONTAINER_ID}

exit 0
