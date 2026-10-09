#!/usr/bin/env bash
set -euo pipefail

# Keep these in sync with the debug USB network in env.mk.
server_ip=192.168.7.1
server_cidr=$server_ip/24
client_ip=192.168.7.2
host_mac=02:00:00:00:00:01
unit=shh-debug-nfs.service
runtime=/run/shh-debug-nfs

die() {
	printf 'nfs_server: %s\n' "$*" >&2
	exit 1
}

usage() {
	cat <<'EOF'
Usage: sudo ./scripts/nfs_server.sh [--interface IFACE] [--nfs-dir DIRECTORY]

Serve out/debug/nfs to the CM4 at 192.168.7.2 over NFSv3 (UDP and TCP).
OUT, when set, replaces the default out directory. The exported absolute
paths must match the paths built into U-Boot and the kernel boot arguments.

By default, wait for the USB Ethernet device with MAC 02:00:00:00:00:01.
--interface selects a device explicitly. USB disconnect/reconnect during
the transition from U-Boot to Linux is handled while the script runs.

Requires Linux with systemd, nfs-utils (Debian/Ubuntu: nfs-kernel-server),
rpcbind, iproute2, and util-linux. Packages must already be installed.
No packages are installed and no service is enabled by this script.
If a host firewall is active, it must allow the target to reach TCP/UDP
ports 111 (rpcbind), 2049 (NFS), and 20048 (mountd).

Ctrl-C, TERM, HUP, launcher death, and host shutdown stop the temporary
service. NFS configuration/state live in /run and private mounts; only
the IP address and link state changed by this script are restored.
Changes the target makes to the exported rootfs remain in that rootfs.
EOF
}

process_start() {
	local stat
	local -a fields
	IFS= read -r stat < "/proc/$1/stat" || return 1
	# comm may contain spaces or parentheses; starttime is field 22.
	read -r -a fields <<< "${stat##*) }"
	[[ ${fields[0]} != Z ]] || return 1
	printf '%s\n' "${fields[19]}"
}

launcher_alive() {
	[[ $(process_start "$launcher_pid" 2>/dev/null) == "$launcher_start" ]]
}

find_interface() {
	local device mac
	if [[ -n $requested_interface ]]; then
		[[ -e /sys/class/net/$requested_interface/ifindex ]] || return 1
		printf '%s\n' "$requested_interface"
		return
	fi
	for device in /sys/class/net/*; do
		IFS= read -r mac < "$device/address" || continue
		if [[ $mac == "$host_mac" ]]; then
			printf '%s\n' "${device##*/}"
			return
		fi
	done
	return 1
}

configure_interface() {
	local iface=$1 index mac flags record addresses
	IFS= read -r index < "/sys/class/net/$iface/ifindex" || return 1
	IFS= read -r mac < "/sys/class/net/$iface/address" || return 1
	IFS= read -r flags < "/sys/class/net/$iface/flags" || return 1
	record=$runtime/interfaces/$index
	if [[ ! -d $record ]]; then
		mkdir "$record" || return 1
		printf '%s\n' "$mac" > "$record/mac" || return 1
		if (( !(flags & 1) )); then
			# Record intent before changing the host so partial failure is reversible.
			: > "$record/raised-link" || return 1
		fi
	fi
	printf '%s\n' "$iface" > "$record/name" || return 1
	addresses=$(ip -o -4 address show dev "$iface") || return 1
	if ! awk -v cidr="$server_cidr" '$4 == cidr { found=1 } END { exit !found }' <<< "$addresses"; then
		if awk -v addr="$server_ip" 'split($4, a, "/") && a[1] == addr { found=1 } END { exit !found }' <<< "$addresses"; then
			die "$iface already has $server_ip with a different prefix"
		fi
		: > "$record/added-address" || return 1
		ip address add "$server_cidr" dev "$iface" || return 1
	fi
	if (( !(flags & 1) )); then
		ip link set dev "$iface" up || return 1
	fi
	return 0
}

prepare_mounts() {
	# Both ExecStart and ExecStopPost get independent private mount namespaces.
	# Bind the same /run state into each; never write the host's etab or rmtab.
	mount --bind "$runtime/state" /var/lib/nfs || return 1
	mount --bind "$runtime/nfs.conf" /etc/nfs.conf || return 1
	local path
	for path in /etc/nfs.conf.d /usr/etc/nfs.conf.d; do
		if [[ -d $path ]]; then
			mount -t tmpfs -o mode=0755 tmpfs "$path" || return 1
		fi
	done
	if [[ -f /usr/etc/nfs.conf ]]; then
		mount --bind "$runtime/nfs.conf" /usr/etc/nfs.conf || return 1
	fi
	if ! mountpoint -q /proc/fs/nfsd; then
		mount -t nfsd nfsd /proc/fs/nfsd || return 1
	fi
	return 0
}

cleanup_service() {
	local failed=0 record iface index mac version
	# A failed preflight must never shut down somebody else's NFS server.
	if [[ -f $runtime/owns-nfsd ]]; then
		if prepare_mounts; then
			rpc.nfsd 0 || failed=1
			exportfs -ua || failed=1
			if [[ -f $runtime/versions ]]; then
				cat "$runtime/versions" > /proc/fs/nfsd/versions || failed=1
			fi
		else
			printf 'nfs_server: could not mount NFS controls for cleanup\n' >&2
			failed=1
		fi
	fi
	# mountd may have been killed before it could unregister with a shared rpcbind.
	if [[ -f $runtime/owns-mountd && -f $runtime/shared-rpcbind ]]; then
		for version in 1 2 3; do
			rpcinfo -d 100005 "$version" 2>/dev/null || true
		done
	fi
	for record in "$runtime"/interfaces/*; do
		[[ -d $record ]] || continue
		IFS= read -r iface < "$record/name" || continue
		[[ -e /sys/class/net/$iface/ifindex ]] || continue
		IFS= read -r index < "/sys/class/net/$iface/ifindex" || continue
		IFS= read -r mac < "/sys/class/net/$iface/address" || continue
		# A re-enumerated or renamed device must not inherit an old rollback.
		[[ $index == "${record##*/}" && $mac == "$(< "$record/mac")" ]] || continue
		if [[ -f $record/added-address ]]; then
			if ip -o -4 address show dev "$iface" | awk -v cidr="$server_cidr" '$4 == cidr { found=1 } END { exit !found }'; then
				ip address del "$server_cidr" dev "$iface" || failed=1
			fi
		fi
		if [[ -f $record/raised-link ]]; then
			ip link set dev "$iface" down || failed=1
		fi
	done
	return "$failed"
}

serve() {
	launcher_pid=$1
	launcher_start=$2
	requested_interface=$3
	local nfs_dir=$4 iface rpcbind_pid= mountd_pid= previous_iface= started=0
	trap 'exit 0' INT TERM HUP
	prepare_mounts
	[[ $(< /proc/fs/nfsd/threads) == 0 ]] || die 'an NFS server is already running'
	[[ -z $(cat /proc/fs/nfsd/portlist) ]] || die 'NFS listening sockets already exist'
	if pgrep -x rpc.mountd >/dev/null; then
		die 'rpc.mountd is already running'
	fi
	if [[ -f $runtime/shared-rpcbind ]]; then
		local registrations
		registrations=$(rpcinfo -p 127.0.0.1) || die 'cannot contact the existing rpcbind'
		if awk '$1 == 100003 || $1 == 100005 || $1 == 100227 { found=1 } END { exit !found }' <<< "$registrations"; then
			die 'rpcbind already has NFS or mountd registrations'
		fi
	fi
	cat /proc/fs/nfsd/versions > "$runtime/versions"
	: > "$runtime/owns-nfsd"
	exportfs -i -o ro,sync,no_subtree_check,insecure "$client_ip:$nfs_dir"
	exportfs -i -o rw,sync,no_subtree_check,no_root_squash,insecure "$client_ip:$nfs_dir/rootfs"
	printf 'Waiting for USB Ethernet (server %s, target %s). Ctrl-C stops the server.\n' "$server_ip" "$client_ip"
	while launcher_alive; do
		iface=$(find_interface) || iface=
		if [[ -n $iface ]]; then
			if ! configure_interface "$iface"; then
				[[ ! -e /sys/class/net/$iface/ifindex ]] || die "could not configure $iface"
				# USB can disappear between discovery and assigning the address.
				previous_iface=
				sleep 1 &
				wait "$!" || true
				continue
			fi
			if [[ $iface != "$previous_iface" ]]; then
				printf 'USB Ethernet: %s (%s)\n' "$iface" "$server_cidr"
			fi
			if (( !started )); then
				if [[ ! -f $runtime/shared-rpcbind ]]; then
					rpcbind -f -h "$server_ip" &
					rpcbind_pid=$!
					local attempt
					for ((attempt=0; attempt<50; attempt++)); do
						rpcinfo -p 127.0.0.1 >/dev/null 2>&1 && break
						kill -0 "$rpcbind_pid" 2>/dev/null || die 'rpcbind exited during startup'
						sleep 0.1
					done
					(( attempt < 50 )) || die 'rpcbind did not become ready'
				fi
				: > "$runtime/owns-mountd"
				rpc.mountd --foreground --port 20048 --no-nfs-version 4 &
				mountd_pid=$!
				rpc.nfsd --host "$server_ip" --udp --tcp --no-nfs-version 4 8
				started=1
				printf 'Serving %s and writable rootfs to %s over NFSv3 (UDP/TCP).\n' "$nfs_dir" "$client_ip"
			fi
		fi
		previous_iface=$iface
		if (( started )); then
			kill -0 "$mountd_pid" 2>/dev/null || die 'rpc.mountd exited'
			[[ -z $rpcbind_pid ]] || kill -0 "$rpcbind_pid" 2>/dev/null || die 'rpcbind exited'
			[[ $(< /proc/fs/nfsd/threads) != 0 ]] || die 'NFS kernel threads stopped'
		fi
		sleep 1 &
		wait "$!" || true
	done
	printf 'Launcher exited; stopping temporary NFS service.\n'
}

launch() {
	local project_root out nfs_dir requested_interface= arg
	project_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)
	out=${OUT:-$project_root/out}
	nfs_dir=$out/debug/nfs
	while (( $# )); do
		arg=$1
		case $arg in
			-h|--help) usage; return ;;
			--interface|--nfs-dir)
				(( $# >= 2 )) && [[ -n $2 ]] || die "$arg requires a value"
				if [[ $arg == --interface ]]; then requested_interface=$2; else nfs_dir=$2; fi
				shift 2 ;;
			*) die "unknown argument: $arg (see --help)" ;;
		esac
	done
	[[ $EUID == 0 ]] || die 'run this script with sudo'
	[[ -d /run/systemd/system ]] || die 'a running systemd system manager is required'
	[[ -z $requested_interface || $requested_interface =~ ^[a-zA-Z0-9_.:-]{1,15}$ ]] || die 'invalid interface name'
	for arg in systemd-run systemctl mount mountpoint exportfs rpc.nfsd rpc.mountd rpcbind rpcinfo ip pgrep awk; do
		command -v "$arg" >/dev/null || die "missing command: $arg (see --help for packages)"
	done
	if ! pgrep -x rpcbind >/dev/null && systemctl is-active --quiet rpcbind.socket; then
		die 'rpcbind.socket is active without rpcbind; start rpcbind separately before running this script'
	fi
	[[ -f /etc/nfs.conf && -d /var/lib/nfs ]] || die 'install nfs-utils/nfs-kernel-server first'
	[[ -d $nfs_dir ]] || die "missing $nfs_dir; run make PROFILE=debug install first"
	nfs_dir=$(cd "$nfs_dir" && pwd -P)
	[[ $nfs_dir != *[[:space:]:]* ]] || die 'NFS boot paths cannot contain whitespace or colons'
	for arg in Image bcm2711-shh.dtb overlays/uart1.dtbo overlays/dwc2.dtbo rootfs/sbin/shh-init; do
		[[ -s $nfs_dir/$arg ]] || die "missing debug boot artifact: $nfs_dir/$arg"
	done
	[[ -x $nfs_dir/rootfs/sbin/shh-init ]] || die 'rootfs/sbin/shh-init is not executable'
	umask 077
	mkdir "$runtime" || die 'another instance is running or /run/shh-debug-nfs already exists'
	local service_requested=0 runner_pid=
	finish() {
		local status=$? stopped=1
		trap - EXIT INT TERM HUP
		if (( service_requested )); then
			if ! systemctl stop "$unit"; then
				printf 'nfs_server: failed to stop %s; inspect journalctl -u %s\n' "$unit" "$unit" >&2
				status=1
				stopped=0
			fi
		fi
		if (( stopped )) && [[ -n $runner_pid ]]; then
			wait "$runner_pid" 2>/dev/null || true
		fi
		# systemd normally removes RuntimeDirectory after ExecStopPost.
		# Also cover a failure to create the transient service in the first place.
		if (( stopped )) && [[ -d $runtime ]]; then rm -rf -- "$runtime"; fi
		exit "$status"
	}
	trap finish EXIT
	trap 'exit 130' INT
	trap 'exit 143' TERM
	trap 'exit 129' HUP
	mkdir "$runtime/state" "$runtime/interfaces"
	touch "$runtime/state/etab"
	cp -- "${BASH_SOURCE[0]}" "$runtime/worker.sh"
	cat > "$runtime/nfs.conf" <<'EOF'
[nfsd]
vers2=n
vers3=y
vers4=n
udp=y
tcp=y
rdma=n
[mountd]
state-directory-path=/var/lib/nfs
EOF
	local -a properties=(
		--property=PrivateMounts=yes
		--property=TemporaryFileSystem=/run
		--property="BindPaths=$runtime"
		--property=RuntimeDirectory=shh-debug-nfs
		--property=RuntimeDirectoryMode=0700
		--property=KillMode=control-group
		--property=TimeoutStopSec=15s
		--property="ExecStopPost=/bin/bash $runtime/worker.sh --cleanup"
	)
	if pgrep -x rpcbind >/dev/null; then
		: > "$runtime/shared-rpcbind"
		# libtirpc uses the local Unix socket to unregister RPC programs.
		properties[2]="--property=BindPaths=$runtime -/run/rpcbind.sock"
	fi
	service_requested=1
	systemd-run --unit="$unit" --collect --wait --pipe --service-type=exec --expand-environment=no \
		"${properties[@]}" /bin/bash "$runtime/worker.sh" \
		--serve "$$" "$(process_start "$$")" "$requested_interface" "$nfs_dir" &
	runner_pid=$!
	local run_status=0
	wait "$runner_pid" || run_status=$?
	runner_pid=
	service_requested=0
	exit "$run_status"
}

main() {
	case ${1:-} in
		--serve|--cleanup)
			[[ $EUID == 0 && ${RUNTIME_DIRECTORY:-} == "$runtime" && -n ${INVOCATION_ID:-} ]] || die 'internal mode requires the transient service'
			if [[ $1 == --serve ]]; then shift; serve "$@"; else cleanup_service; fi ;;
		*) launch "$@" ;;
	esac
}

if [[ ${BASH_SOURCE[0]} == "$0" ]]; then main "$@"; fi
