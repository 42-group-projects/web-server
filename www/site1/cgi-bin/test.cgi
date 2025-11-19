#!/bin/sh

# Send headers
echo "Content-Type: text/plain"
echo ""

# Send main content
echo "CGI Test Script Output"
echo "-----------------------"
echo ""

# Send environment variables
echo "CGI Environment Variables:"
echo "REQUEST_METHOD: $REQUEST_METHOD"
echo "QUERY_STRING: $QUERY_STRING"
echo "REMOTE_ADDR: $REMOTE_ADDR"
echo "HTTP_USER_AGENT: $HTTP_USER_AGENT"
echo "SERVER_PROTOCOL: $SERVER_PROTOCOL"
echo "SCRIPT_NAME: $SCRIPT_NAME"
echo "PATH_INFO: $PATH_INFO"
echo "SERVER_NAME: $SERVER_NAME"
echo "SERVER_PORT: $SERVER_PORT"
echo "GATEWAY_INTERFACE: $GATEWAY_INTERFACE"
echo "SERVER_SOFTWARE: $SERVER_SOFTWARE"
echo ""

# Print ALL HTTP_* environment variables
echo "HTTP Headers passed as environment variables:"
env | grep "^HTTP_" | sort | while IFS='=' read -r name value; do
    echo "$name: $value"
done