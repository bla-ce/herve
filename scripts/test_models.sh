#!/bin/bash
set -euo pipefail

URL="http://localhost:1337"

echo "Checking health check endpoint..."

# check that the application is running
status_code=$(curl -s -w "\n%{http_code}" $URL/health | tail -n1)

if [ "$status_code" == "200" ]; then
  echo "PASSED: Application is running"
else
  echo "FAILED: Application is not running"
  exit 1
fi

# Get list of users
response=$(curl -s -w "\n%{http_code}" $URL/user)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
length=$(echo "$body" | jq -r '.data | length')
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$length" -eq "2" ] && [ "$success" == "true" ]; then
  echo "PASSED: GET /user (status: $status_code, length: $length)"
else
  echo "FAILED: GET /user"
  echo "  Expected: status 200, length 2"
  echo "  Got: status $status_code, length $length"
  exit 1
fi

# Get valid user
response=$(curl -s -w "\n%{http_code}" $URL/user/0)
body=$(echo "$response" | head -n -1)
success=$(echo "$body" | jq -r '.success')
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "200" ] && [ "$success" = "true" ]; then
  echo "PASSED: GET /user/0 (status: $status_code)"
else
  echo "FAILED: GET /user/0"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Get invalid user
response=$(curl -s -w "\n%{http_code}" $URL/user/4)
body=$(echo "$response" | head -n -1)
success=$(echo "$body" | jq -r '.success')
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ] && [ "$success" = "false" ]; then
  echo "PASSED: GET /user/4 (status: $status_code)"
else
  echo "FAILED: GET /user/4"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# Create user
response=$(curl -s -w "\n%{http_code}" $URL/user \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "username=user3" \
  -d "password=pass3" \
  -d "age=18" )
body=$(echo "$response" | head -n -1)
success=$(echo "$body" | jq -r '.success')
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "201" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /user (status: $status_code)"
else
  echo "FAILED: POST /user"
  echo "  Expected: status 201"
  echo "  Got: status $status_code"
  exit 1
fi

# Get new user
response=$(curl -s -w "\n%{http_code}" $URL/user/2)
body=$(echo "$response" | head -n -1)
success=$(echo "$body" | jq -r '.success')
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: GET /user/2 (status: $status_code)"
else
  echo "FAILED: GET /user/2"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Get list after post
response=$(curl -s -w "\n%{http_code}" $URL/user)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
length=$(echo "$body" | jq -r '.data | length')
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$length" -eq "3" ] && [ "$success" == "true" ]; then
  echo "PASSED: GET /user (status: $status_code, length: $length)"
else
  echo "FAILED: GET /user"
  echo "  Expected: status 200, length 3"
  echo "  Got: status $status_code, length $length"
  exit 1
fi

# POST invalid user
response=$(curl -s -w "\n%{http_code}" $URL/user \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "password=pass3" \
  -d "age=18" )
body=$(echo "$response" | head -n -1)
success=$(echo "$body" | jq -r '.success')
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "422" ] && [ "$success" == "false" ]; then
  echo "PASSED: POST /user (status: $status_code)"
else
  echo "FAILED: POST /user"
  echo "  Expected: status 422"
  echo "  Got: status $status_code"
  exit 1
fi

# Delete new user
response=$(curl -s -w "\n%{http_code}" -X DELETE $URL/user/2)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: DELETE /user/2 (status: $status_code)"
else
  echo "FAILED: DELETE /user/2"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Delete invalid user
response=$(curl -s -w "\n%{http_code}" -X DELETE $URL/user/6)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "404" ] && [ "$success" == "false" ]; then
  echo "PASSED: DELETE /user/6 (status: $status_code)"
else
  echo "FAILED: DELETE /user/6"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# Get list after delete
response=$(curl -s -w "\n%{http_code}" $URL/user)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
length=$(echo "$body" | jq -r '.data | length')
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$length" -eq "2" ] && [ "$success" == "true" ]; then
  echo "PASSED: GET /user (status: $status_code, length: $length)"
else
  echo "FAILED: GET /user"
  echo "  Expected: status 200, length 2"
  echo "  Got: status $status_code, length $length"
  exit 1
fi

# Try to put deleted user (will create a new one)
response=$(curl -s -w "\n%{http_code}" -X PUT $URL/user/2 \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "username=user3" \
  -d "password=pass3" \
  -d "age=18" )
body=$(echo "$response" | head -n -1)
success=$(echo "$body" | jq -r '.success')
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "201" ] && [ "$success" == "true" ]; then
  echo "PASSED: PUT /user/2 (status: $status_code)"
else
  echo "FAILED: PUT /user/2"
  echo "  Expected: status 201"
  echo "  Got: status $status_code"
  exit 1
fi

# Access the new one - id:2 has been deleted so query id:3
response=$(curl -s -w "\n%{http_code}" $URL/user/3)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: GET /user/3 (status: $status_code)"
else
  echo "FAILED: GET /user/3"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Get the list after put
response=$(curl -s -w "\n%{http_code}" $URL/user)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
length=$(echo "$body" | jq -r '.data | length')
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$length" -eq "3" ] && [ "$success" == "true" ]; then
  echo "PASSED: GET /user (status: $status_code, length: $length)"
else
  echo "FAILED: GET /user"
  echo "  Expected: status 200, length 3"
  echo "  Got: status $status_code, length $length"
  exit 1
fi

# Patch user 0
response=$(curl -s -w "\n%{http_code}" -X PATCH $URL/user/3 \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "age=25" )
body=$(echo "$response" | head -n -1)
success=$(echo "$body" | jq -r '.success')
status_code=$(echo "$response" | tail -n1)
age=$(echo "$body" | jq -r '.data.age')

if [ "$status_code" == "200" ] && [ "$success" == "true" ] &&[ "$age" == "25" ]; then
  echo "PASSED: PATCH /user/3 (status: $status_code)"
else
  echo "FAILED: PATCH /user/3"
  echo "  Expected: status 200, age 25"
  echo "  Got: status $status_code, age $age"
  exit 1
fi

# Put user 1
response=$(curl -s -w "\n%{http_code}" -X PUT $URL/user/1 \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "username=user2" \
  -d "password=pass6" \
  -d "age=18" )
body=$(echo "$response" | head -n -1)
success=$(echo "$body" | jq -r '.success')
status_code=$(echo "$response" | tail -n1)
password=$(echo "$body" | jq -r '.data.password')

if [ "$status_code" == "200" ] && [ "$success" == "true" ] && [ "$password" == "pass6" ]; then
  echo "PASSED: PUT /user/1 (status: $status_code)"
else
  echo "FAILED: PUT /user/1"
  echo "  Expected: status 200, password pass6"
  echo "  Got: status $status_code, password $password"
  exit 1
fi
