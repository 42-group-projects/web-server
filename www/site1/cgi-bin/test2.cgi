#!/bin/sh

body="$(env | sort)"

echo "Content-Type: text/plain"
echo ""
echo "$body"