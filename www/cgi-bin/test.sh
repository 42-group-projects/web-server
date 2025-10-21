#!bin/bash

body="$(env | sort)"
length=$(printf '%s\n' "$body" | wc -c | awk '{print $1}')

printf 'HTTP/1.1 200 OK\r\n'
printf 'Content-Type: text/plain\r\n'
printf 'Connection: close\r\n'
printf '\r\n'
printf '%s\n' "$body"