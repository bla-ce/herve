#!/bin/bash
set -euo pipefail

URL="http://localhost:5000"

echo "Checking health check endpoint..."

# check that the application is running
status_code=$(curl -s -w "\n%{http_code}" $URL/health | tail -n1)

if [ "$status_code" == "200" ]; then
  echo "PASSED: Application is running"
else
  echo "FAILED: Application is not running"
  exit 1
fi

# Try to echo to service 0 before it exists (should fail)
response=$(curl -s -w "\n%{http_code}" -X POST $URL/0/echo -d "yo")
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /0/echo before registration (status: $status_code)"
else
  echo "FAILED: POST /0/echo before registration"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# Register a new service
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/register \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=a service2" \
  -d "type=echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "201" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/register (status: $status_code)"
else
  echo "FAILED: POST /services/register"
  echo "  Expected: status 201"
  echo "  Got: status $status_code"
  exit 1
fi

# Try to echo to service 0 before starting (should fail)
response=$(curl -s -w "\n%{http_code}" -X POST $URL/0/echo -d "yo")
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /0/echo before start (status: $status_code)"
else
  echo "FAILED: POST /0/echo before start"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# Start service 0
response=$(curl -s -w "\n%{http_code}" $URL/services/0/start)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: GET /services/0/start (status: $status_code)"
else
  echo "FAILED: GET /services/0/start"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Echo to service 0 after starting (should succeed)
response=$(curl -s -w "\n%{http_code}" -X POST $URL/0/echo -d "yo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "200" ] && [ "$body" == "yo" ]; then
  echo "PASSED: POST /0/echo after start (status: $status_code)"
else
  echo "FAILED: POST /0/echo after start"
  echo "  Expected: status 200, body 'yo'"
  echo "  Got: status $status_code, body '$body'"
  exit 1
fi

# Stop service 0
response=$(curl -s -w "\n%{http_code}" $URL/services/0/stop)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: GET /services/0/stop (status: $status_code)"
else
  echo "FAILED: GET /services/0/stop"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Try to echo to service 0 after stopping (should fail)
response=$(curl -s -w "\n%{http_code}" -X POST $URL/0/echo -d "yo")
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /0/echo after stop (status: $status_code)"
else
  echo "FAILED: POST /0/echo after stop"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi
