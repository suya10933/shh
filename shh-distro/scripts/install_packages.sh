#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

. /etc/os-release

if [[ $ID == void ]]; then
	mapfile -t packages < "$script_dir/packages-void.list"
	sudo xbps-install -Sy "${packages[@]}"
else
	mapfile -t packages < "$script_dir/packages.list"
	sudo apt-get -y --ignore-missing install "${packages[@]}"
fi
