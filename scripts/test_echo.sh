#!/bin/bash
set -euo pipefail

URL="http://localhost:1337/"
WORDLIST="examples/echo/medium.txt"

# check if the file exists
if [ ! -f "$WORDLIST" ]; then
  echo "File not found: $WORDLIST"
fi

# go over the list of words
while IFS= read -r word;
do
  out=$(curl -s http://localhost:8080 --data-raw "${word}")

  if [ "$out" == "$word" ]; then
    echo "PASSED: $word"
  else
    echo "FAILED:"
    echo "  Got: $out"
    echo "  Expected: $word"
    exit 1
  fi
done < "$WORDLIST"
