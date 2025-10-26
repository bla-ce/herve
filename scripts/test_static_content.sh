#!/bin/bash
set -euo pipefail

URL="http://localhost:1337/"
DIR="examples/static_content/public"

# check if the directory exists
if [ ! -d "$DIR" ]; then
  echo "Directory not found: $DIR"
fi

# go over the list of files
for file in $(find "$DIR" -type f); do
  # get relative path
  rel_path="${file#$DIR/}"

  status_code=$(curl -s -w "\n%{http_code}" http://localhost:1337/public/${rel_path} | tail -n1)

  if [ "$status_code" == "200" ]; then
    echo "PASSED: $file"
  else
    echo "FAILED: $file"
    echo "  Got: $status_code"
    exit 1
  fi
done

# create file
FILE=page.html
touch $DIR/$FILE

# try to access it
status_code=$(curl -s -w "\n%{http_code}" http://localhost:1337/public/$FILE | tail -n1)

if [ "$status_code" == "200" ]; then
  echo "PASSED: $FILE"
else
  echo "FAILED: $file"
  echo "  Got: $status_code"
  exit 1
fi
