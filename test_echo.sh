#!/bin/bash

URL="http://localhost:1337/echo/"

WORDLIST="/usr/share/wfuzz/wordlist/general/medium.txt"

# check if the file exists
if [ ! -f "$WORDLIST" ]; then
    echo "Error: File '$WORDLIST' not found!"
    exit 1
fi

echo "Testing echo server at $URL using payloads from wfuzz list: $WORDLIST"

# go over each line
while IFS= read -r payload || [ -n "$payload" ]; do
    if [ -z "$payload" ]; then
        continue
    fi

    # curl the echo server with POST data
    response=$(curl -s -X POST -d "${payload}" "$URL")
    
    # trim payload and response
    trimmed_payload=$(echo "$payload" | sed 's/^[ \t]*//;s/[ \t]*$//')
    trimmed_response=$(echo "$response" | sed 's/^[ \t]*//;s/[ \t]*$//')
    
    # compare payload and response
    if [ "$trimmed_response" = "$trimmed_payload" ]; then
        echo "PASS: Payload '$trimmed_payload'"
    else
        echo "FAIL: Payload '$trimmed_payload'"
        echo "  Expected: '$trimmed_payload'"
        echo "  Got:      '$trimmed_response'"
        exit 1
    fi

done < "$WORDLIST"

