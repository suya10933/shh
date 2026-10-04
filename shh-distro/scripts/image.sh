#!/usr/bin/env bash
set -euo pipefail

profile=${1:?}
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
out=${OUT:-"$root/out"}
boot="$out/$profile/boot"
rootfs="$out/$profile/rootfs"
image=${2:-"$out/images/shh-$profile.img"}
boot_mib=${BOOT_MIB:-256}

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
bootfs="$work/boot.vfat"

truncate -s "${boot_mib}M" "$bootfs"
mkfs.vfat -F 32 -n BOOT "$bootfs" >/dev/null
mcopy -i "$bootfs" -sp "$boot"/* ::/

boot_sectors=$((boot_mib * 2048))
root_start=$((2048 + boot_sectors))
image_mib=$((boot_mib + 2))

if [[ -d $rootfs ]]; then
	root_mib=${ROOTFS_MIB:-$(( $(du -sm "$rootfs" | cut -f1) + 128 ))}
	((root_mib < 256)) && root_mib=256
	rootfs_image="$work/rootfs.ext4"
	truncate -s "${root_mib}M" "$rootfs_image"
	mkfs.ext4 -F -L rootfs -d "$rootfs" "$rootfs_image" >/dev/null
	root_sectors=$((root_mib * 2048))
	image_mib=$((image_mib + root_mib))
	root_spec="start=$root_start, size=$root_sectors, type=83"
else
	rootfs_image=
	root_spec=
fi

image_tmp="$work/image.img"
truncate -s "${image_mib}M" "$image_tmp"
sfdisk --no-reread --no-tell-kernel "$image_tmp" >/dev/null <<EOF
label: dos
unit: sectors

start=2048, size=$boot_sectors, type=c, bootable
$root_spec
EOF

dd if="$bootfs" of="$image_tmp" bs=1M seek=1 conv=notrunc status=none
if [[ -n $rootfs_image ]]; then
	dd if="$rootfs_image" of="$image_tmp" bs=1M seek=$((boot_mib + 1)) conv=notrunc status=none
fi

sfdisk --verify "$image_tmp" >/dev/null
mkdir -p "$(dirname "$image")"
mv -f "$image_tmp" "$image"
