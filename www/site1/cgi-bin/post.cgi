#!/bin/sh

# Send headers
echo "Content-Type: text/plain"
echo ""

# Send request info
echo "CGI POST Test"
echo "-----------------------"
echo "REQUEST_METHOD: $REQUEST_METHOD"
echo "CONTENT_TYPE: $CONTENT_TYPE"
echo "CONTENT_LENGTH: $CONTENT_LENGTH"
echo ""

# Read and output POST data
echo "POST Data:"
if [ "$REQUEST_METHOD" = "POST" ]; then
  if [ ! -z "$CONTENT_LENGTH" ]; then
    # Read stdin up to CONTENT_LENGTH
    dd bs=1 count=$CONTENT_LENGTH 2>/dev/null
  fi
fi