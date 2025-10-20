#!/bin/bash

if [ -z "$1" ]; then
    echo "run.sh: No path provided, using default: ./configFiles/webserv.conf"
    file_path="./configFiles/webserv.conf"
else
    file_path="$1"
fi

if [ ! -e "$file_path" ]; then
    echo "run.sh: Error: File does not exist at '$file_path'."
    exit 1
fi


username=$(whoami)
tmp_file=$(mktemp)

awk -v user="$username" '
{
    if ($0 ~ /root/) {
        buffer[++n] = $0
        if ($0 ~ ("\\<" user "\\>")) special = n
    } else {
        if (n) {
            if (special) {
                tmp = buffer[special]
                for (i = special; i < n; i++) buffer[i] = buffer[i+1]
                buffer[n] = tmp
            }
            for (i = 1; i <= n; i++) print buffer[i]
            n = 0
            special = 0
        }
        print
    }
}
END {
    if (n) {
        if (special) {
            tmp = buffer[special]
            for (i = special; i < n; i++) buffer[i] = buffer[i+1]
            buffer[n] = tmp
        }
        for (i = 1; i <= n; i++) print buffer[i]
    }
}
' "$file_path" > "$tmp_file"

mv "$tmp_file" "$file_path"

if [ -x "./webserv" ]; then
    ./webserv
else
    make && ./webserv
fi
