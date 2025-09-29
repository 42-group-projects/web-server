#!/bin/bash

# Check for at least one argument
if [ $# -lt 1 ]; then
    echo "Usage: $0 <file1.cpp|file1.hpp> [file2.cpp|file2.hpp ...]"
    exit 1
fi

# Use the local AStyle binary in the same folder as the script
ASTYLE="$(dirname "$0")/astyle"
if [ ! -x "$ASTYLE" ]; then
    echo "AStyle binary not found or not executable. Aborting."
    exit 1
fi

# Backup directory for .orig files
BACKUP_DIR="$(dirname "$0")/astyle_orig"
mkdir -p "$BACKUP_DIR"

# Loop through all provided files
for FILE in "$@"; do
    if [ ! -f "$FILE" ]; then
        echo "File not found: $FILE"
        continue
    fi

    # Apply AStyle formatting with backup
    "$ASTYLE" --style=allman --indent=tab \
        --break-blocks \
        --pad-oper \
        --keep-one-line-statements \
        --keep-one-line-blocks \
        --pad-header \
        --unpad-paren \
        --delete-empty-lines \
        --break-closing-braces \
        --break-one-line-headers \
        --remove-braces "$FILE"

    # Move the .orig to the backup folder if it exists
    ORIG_FILE="${FILE}.orig"
    if [ -f "$ORIG_FILE" ]; then
        mv "$ORIG_FILE" "$BACKUP_DIR/"
    fi

    echo "Formatted '$FILE'. Backup (if any) is in '$BACKUP_DIR'."
done

echo "AStyle formatting completed."
