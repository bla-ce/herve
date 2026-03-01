#!/bin/bash
set -euo pipefail

URL="http://localhost:5000"
AUTH="admin:password"

echo "=== Service API Tests ==="
echo ""

# =============================================================================
# Health Check
# =============================================================================
echo "--- Health Check ---"

status_code=$(curl -s -u $AUTH -w "\n%{http_code}" $URL/health | tail -n1)

if [ "$status_code" == "200" ]; then
  echo "PASSED: Application is running"
else
  echo "FAILED: Application is not running"
  exit 1
fi

# =============================================================================
# Authentication - 401 Unauthorized
# =============================================================================
echo ""
echo "--- Authentication ---"

# Request without credentials should return 401
status_code=$(curl -s -w "\n%{http_code}" $URL/services | tail -n1)

if [ "$status_code" == "401" ]; then
  echo "PASSED: Request without auth returns 401"
else
  echo "FAILED: Request without auth"
  echo "  Expected: status 401"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Initial State - No Services
# =============================================================================
echo ""
echo "--- Initial State ---"

# Get services list (should be empty)
response=$(curl -s -u $AUTH -w "\n%{http_code}" $URL/services)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
length=$(echo "$body" | jq -r '.data | length')

if [ "$status_code" == "200" ] && [ "$length" == "0" ]; then
  echo "PASSED: GET /services returns empty list (length: $length)"
else
  echo "FAILED: GET /services initial state"
  echo "  Expected: status 200, length 0"
  echo "  Got: status $status_code, length $length"
  exit 1
fi

# Try to echo to a fake uuid before any service exists (should fail)
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/00000000-0000-0000-0000-000000000000/echo -d "yo")
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /fake-uuid/echo before registration (status: $status_code)"
else
  echo "FAILED: POST /fake-uuid/echo before registration"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Register Multiple Services
# =============================================================================
echo ""
echo "--- Register Multiple Services ---"

# Register first service
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/register \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=service-alpha" \
  -d "type=echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')
UUID_ALPHA=$(echo "$body" | jq -r '.data.uuid')

if [ "$status_code" == "201" ] && [ "$success" == "true" ] && [ "$UUID_ALPHA" != "null" ]; then
  echo "PASSED: POST /services/register service-alpha (uuid: $UUID_ALPHA)"
else
  echo "FAILED: POST /services/register service-alpha"
  echo "  Expected: status 201"
  echo "  Got: status $status_code"
  exit 1
fi

# Register second service
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/register \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=service-beta" \
  -d "type=echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')
UUID_BETA=$(echo "$body" | jq -r '.data.uuid')

if [ "$status_code" == "201" ] && [ "$success" == "true" ] && [ "$UUID_BETA" != "null" ]; then
  echo "PASSED: POST /services/register service-beta (uuid: $UUID_BETA)"
else
  echo "FAILED: POST /services/register service-beta"
  echo "  Expected: status 201"
  echo "  Got: status $status_code"
  exit 1
fi

# Register third service
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/register \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=service-gamma" \
  -d "type=echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')
UUID_GAMMA=$(echo "$body" | jq -r '.data.uuid')

if [ "$status_code" == "201" ] && [ "$success" == "true" ] && [ "$UUID_GAMMA" != "null" ]; then
  echo "PASSED: POST /services/register service-gamma (uuid: $UUID_GAMMA)"
else
  echo "FAILED: POST /services/register service-gamma"
  echo "  Expected: status 201"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Verify Services List Length
# =============================================================================
echo ""
echo "--- Verify Services List ---"

response=$(curl -s -u $AUTH -w "\n%{http_code}" $URL/services)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
length=$(echo "$body" | jq -r '.data | length')

if [ "$status_code" == "200" ] && [ "$length" == "3" ]; then
  echo "PASSED: GET /services returns 3 services (length: $length)"
else
  echo "FAILED: GET /services after registering 3 services"
  echo "  Expected: status 200, length 3"
  echo "  Got: status $status_code, length $length"
  exit 1
fi

# Check service status by uuid
status_alpha=$(echo "$body" | jq -r --arg uuid "$UUID_ALPHA" '.data[] | select(.uuid == $uuid) | .status')
status_beta=$(echo "$body" | jq -r --arg uuid "$UUID_BETA" '.data[] | select(.uuid == $uuid) | .status')
status_gamma=$(echo "$body" | jq -r --arg uuid "$UUID_GAMMA" '.data[] | select(.uuid == $uuid) | .status')

if [ "$status_alpha" == "registered" ] && [ "$status_beta" == "registered" ] && [ "$status_gamma" == "registered" ]; then
  echo "PASSED: All services have status 'registered'"
else
  echo "FAILED: Initial service status check"
  echo "  Expected: all 'registered'"
  echo "  Got: alpha='$status_alpha', beta='$status_beta', gamma='$status_gamma'"
  exit 1
fi

# =============================================================================
# Service Before Start (should fail)
# =============================================================================
echo ""
echo "--- Service Before Start ---"

response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/$UUID_ALPHA/echo -d "test")
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /$UUID_ALPHA/echo before start returns 404"
else
  echo "FAILED: POST /$UUID_ALPHA/echo before start"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Start Multiple Services
# =============================================================================
echo ""
echo "--- Start Multiple Services ---"

# Start service alpha
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/$UUID_ALPHA/start)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/$UUID_ALPHA/start (status: $status_code)"
else
  echo "FAILED: POST /services/$UUID_ALPHA/start"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Start service beta
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/$UUID_BETA/start)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/$UUID_BETA/start (status: $status_code)"
else
  echo "FAILED: POST /services/$UUID_BETA/start"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Verify Status After Start
# =============================================================================
echo ""
echo "--- Verify Status After Start ---"

response=$(curl -s -u $AUTH -w "\n%{http_code}" $URL/services)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
status_alpha=$(echo "$body" | jq -r --arg uuid "$UUID_ALPHA" '.data[] | select(.uuid == $uuid) | .status')
status_beta=$(echo "$body" | jq -r --arg uuid "$UUID_BETA" '.data[] | select(.uuid == $uuid) | .status')
status_gamma=$(echo "$body" | jq -r --arg uuid "$UUID_GAMMA" '.data[] | select(.uuid == $uuid) | .status')

if [ "$status_alpha" == "running" ] && [ "$status_beta" == "running" ] && [ "$status_gamma" == "registered" ]; then
  echo "PASSED: Service alpha and beta are 'running', gamma is 'registered'"
else
  echo "FAILED: Status check after starting services alpha and beta"
  echo "  Expected: alpha='running', beta='running', gamma='registered'"
  echo "  Got: alpha='$status_alpha', beta='$status_beta', gamma='$status_gamma'"
  exit 1
fi

# =============================================================================
# Echo to Running Services
# =============================================================================
echo ""
echo "--- Echo to Running Services ---"

# Echo to service alpha
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/$UUID_ALPHA/echo -d "hello from alpha")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "200" ] && [ "$body" == "hello from alpha" ]; then
  echo "PASSED: POST /$UUID_ALPHA/echo returns correct echo"
else
  echo "FAILED: POST /$UUID_ALPHA/echo"
  echo "  Expected: status 200, body 'hello from alpha'"
  echo "  Got: status $status_code, body '$body'"
  exit 1
fi

# Echo to service beta
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/$UUID_BETA/echo -d "hello from beta")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "200" ] && [ "$body" == "hello from beta" ]; then
  echo "PASSED: POST /$UUID_BETA/echo returns correct echo"
else
  echo "FAILED: POST /$UUID_BETA/echo"
  echo "  Expected: status 200, body 'hello from beta'"
  echo "  Got: status $status_code, body '$body'"
  exit 1
fi

# Echo to service gamma (not started, should fail)
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/$UUID_GAMMA/echo -d "hello from gamma")
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /$UUID_GAMMA/echo returns 404 (service not started)"
else
  echo "FAILED: POST /$UUID_GAMMA/echo (service not started)"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Stop Services
# =============================================================================
echo ""
echo "--- Stop Services ---"

# Stop service alpha
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/$UUID_ALPHA/stop)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/$UUID_ALPHA/stop (status: $status_code)"
else
  echo "FAILED: POST /services/$UUID_ALPHA/stop"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Verify service alpha is stopped
response=$(curl -s -u $AUTH -w "\n%{http_code}" $URL/services)
body=$(echo "$response" | head -n -1)
status_alpha=$(echo "$body" | jq -r --arg uuid "$UUID_ALPHA" '.data[] | select(.uuid == $uuid) | .status')

if [ "$status_alpha" == "stopped" ]; then
  echo "PASSED: Service alpha status is 'stopped' after stop"
else
  echo "FAILED: Service alpha status after stop"
  echo "  Expected: 'stopped'"
  echo "  Got: '$status_alpha'"
  exit 1
fi

# Echo to stopped service alpha (should fail)
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/$UUID_ALPHA/echo -d "test")
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /$UUID_ALPHA/echo after stop returns 404"
else
  echo "FAILED: POST /$UUID_ALPHA/echo after stop"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Unregister Services
# =============================================================================
echo ""
echo "--- Unregister Services ---"

# Unregister service gamma (the one that was never started)
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/$UUID_GAMMA/unregister)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/$UUID_GAMMA/unregister (status: $status_code)"
else
  echo "FAILED: POST /services/$UUID_GAMMA/unregister"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Verify we now have 2 services
response=$(curl -s -u $AUTH -w "\n%{http_code}" $URL/services)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
length=$(echo "$body" | jq -r '.data | length')

if [ "$status_code" == "200" ] && [ "$length" == "2" ]; then
  echo "PASSED: GET /services returns 2 services after unregister"
else
  echo "FAILED: GET /services after unregister"
  echo "  Expected: status 200, length 2"
  echo "  Got: status $status_code, length $length"
  exit 1
fi

# Stop service beta
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/$UUID_BETA/stop)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/$UUID_BETA/stop (running service)"
else
  echo "FAILED: POST /services/$UUID_BETA/stop"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Unregister service beta (was stopped)
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/$UUID_BETA/unregister)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/$UUID_BETA/unregister (stopped service)"
else
  echo "FAILED: POST /services/$UUID_BETA/unregister"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Verify we now have 1 service
response=$(curl -s -u $AUTH -w "\n%{http_code}" $URL/services)
body=$(echo "$response" | head -n -1)
length=$(echo "$body" | jq -r '.data | length')

if [ "$length" == "1" ]; then
  echo "PASSED: GET /services returns 1 service after second unregister"
else
  echo "FAILED: Service count after second unregister"
  echo "  Expected: length 1"
  echo "  Got: length $length"
  exit 1
fi

# Unregister service alpha (was stopped)
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/$UUID_ALPHA/unregister)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/$UUID_ALPHA/unregister (stopped service)"
else
  echo "FAILED: POST /services/$UUID_ALPHA/unregister"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Verify Empty State After Unregister All
# =============================================================================
echo ""
echo "--- Verify Empty State ---"

response=$(curl -s -u $AUTH -w "\n%{http_code}" $URL/services)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
length=$(echo "$body" | jq -r '.data | length')

if [ "$status_code" == "200" ] && [ "$length" == "0" ]; then
  echo "PASSED: GET /services returns empty list after unregistering all"
else
  echo "FAILED: GET /services after unregistering all"
  echo "  Expected: status 200, length 0"
  echo "  Got: status $status_code, length $length"
  exit 1
fi

# Try to unregister non-existent service
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/$UUID_ALPHA/unregister)
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /services/$UUID_ALPHA/unregister on non-existent service returns 404"
else
  echo "FAILED: POST /services/$UUID_ALPHA/unregister on non-existent service"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Re-register and Test Fresh State
# =============================================================================
echo ""
echo "--- Re-register and Test Fresh State ---"

# Register a new service after clearing all
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/register \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=fresh-service" \
  -d "type=echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')
UUID_FRESH=$(echo "$body" | jq -r '.data.uuid')

if [ "$status_code" == "201" ] && [ "$success" == "true" ] && [ "$UUID_FRESH" != "null" ]; then
  echo "PASSED: POST /services/register fresh-service after unregister all (uuid: $UUID_FRESH)"
else
  echo "FAILED: POST /services/register after unregister all"
  echo "  Expected: status 201"
  echo "  Got: status $status_code"
  exit 1
fi

# Start and test the fresh service
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/services/$UUID_FRESH/start)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/$UUID_FRESH/start fresh service"
else
  echo "FAILED: POST /services/$UUID_FRESH/start fresh service"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Echo to fresh service
response=$(curl -s -u $AUTH -w "\n%{http_code}" -X POST $URL/$UUID_FRESH/echo -d "fresh echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "200" ] && [ "$body" == "fresh echo" ]; then
  echo "PASSED: POST /$UUID_FRESH/echo to fresh service"
else
  echo "FAILED: POST /$UUID_FRESH/echo to fresh service"
  echo "  Expected: status 200, body 'fresh echo'"
  echo "  Got: status $status_code, body '$body'"
  exit 1
fi

# =============================================================================
# Summary
# =============================================================================
echo ""
echo "=== All Tests Passed ==="
