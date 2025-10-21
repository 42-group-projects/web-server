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
echo "run.sh: Current user is '$username'."

tmp_file=$(mktemp)
echo "run.sh: Temporary file created: $tmp_file"

echo "run.sh: Rearranging 'root' blocks in config..."

awk -v user="$username" '
{
    if ($0 ~ /root|cgi/) {
        buffer[++n] = $0

        # extract comment after #
        c = index($0, "#")
        if (c) {
            comment = substr($0, c+1)
            gsub(/^[ \t]+|[ \t]+$/, "", comment)  # trim spaces

            # stricter: exact username match as a whole word in comment
            split(comment, words, /[ \t()]+/)
            for (i in words) {
                if (words[i] == user) {
                    special = n
                    print "awk: Found exact username in comment:", $0 > "/dev/stderr"
                    break
                }
            }
        }
    } else {
        if (n) {
            if (special) {
                tmp = buffer[special]
                for (i = special; i < n; i++) buffer[i] = buffer[i+1]
                buffer[n] = tmp
                print "awk: Moved special line to end of block:", tmp > "/dev/stderr"
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
            print "awk: Moved special line to end of final block:", tmp > "/dev/stderr"
        }
        for (i = 1; i <= n; i++) print buffer[i]
    }
}
' "$file_path" > "$tmp_file"

echo "run.sh: Updating original config file..."
mv "$tmp_file" "$file_path"

if [ -x "./webserv" ]; then
    echo "run.sh: Found executable './webserv'. Running it..."
    ./webserv
else
    echo "run.sh: './webserv' not found. Running 'make' first..."
    make && echo "run.sh: Build finished. Running './webserv'..." && ./webserv
fi
