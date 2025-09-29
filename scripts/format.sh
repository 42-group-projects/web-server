#!/bin/bash

# Use the local AStyle binary in the same folder as the script
ASTYLE="$(dirname "$0")/astyle"
if [ ! -x "$ASTYLE" ]; then
    echo "AStyle binary not found or not executable. Aborting."
    exit 1
fi

# Base directory is one level up from the script
BASE_DIR=$(realpath "$(dirname "$0")/..")

# Backup directory for .orig files
BACKUP_DIR="$BASE_DIR/astyle_orig"
mkdir -p "$BACKUP_DIR"

# Find all .cpp and .hpp files recursively
FILES=$(find "$BASE_DIR" -type f \( -iname "*.cpp" -o -iname "*.hpp" \))

# Apply AStyle formatting with backups
echo "Running AStyle..."
for f in $FILES; do
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
        --remove-braces "$f"

    # Move the .orig to the backup folder if it exists
    ORIG_FILE="${f}.orig"
    if [ -f "$ORIG_FILE" ]; then
        mv "$ORIG_FILE" "$BACKUP_DIR/"
    fi
done

echo "AStyle formatting completed. Backups are in '$BACKUP_DIR'."

