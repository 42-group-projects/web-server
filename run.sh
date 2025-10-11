#!/usr/bin/env bash

username="${SUDO_USER:-$USER}"
if [ -z "$username" ]; then
	username="$(id -un 2>/dev/null || whoami 2>/dev/null || getent passwd "$(id -u)" | cut -d: -f1)"
fi

for file in ./configFiles/*.conf; do
	tmp="$(mktemp)"

	awk -v user="$username" '
	function trim(s) {
		sub(/^[ \t]+/, "", s);
		sub(/[ \t]+$/, "", s);
		return s;
	}
	{
		if ($1 == "root") {
			root_block[root_count++] = $0
			next
		} else {
			if (root_count > 0) {
				exact_line = ""
				for (i = 0; i < root_count; i++) {
					comment = ""
					n = split(root_block[i], parts, "#")
					if (n > 1) comment = trim(parts[2])
					if (comment == user) {
						exact_line = root_block[i]
					} else {
						print root_block[i]
					}
				}
				if (exact_line != "") print exact_line
				root_count = 0
			}
			print
		}
	}
	END {
		if (root_count > 0) {
			exact_line = ""
			for (i = 0; i < root_count; i++) {
				comment = ""
				n = split(root_block[i], parts, "#")
				if (n > 1) comment = trim(parts[2])
				if (comment == user) {
					exact_line = root_block[i]
				} else {
					print root_block[i]
				}
			}
			if (exact_line != "") print exact_line
		}
	}
	' "$file" > "$tmp"

	mv "$tmp" "$file"
done

./webserv