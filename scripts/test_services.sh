#!/bin/bash
set -euo pipefail

URL="http://localhost:5000"

echo "=== Service API Tests ==="
echo ""

# =============================================================================
# Health Check
# =============================================================================
echo "--- Health Check ---"

status_code=$(curl -s -w "\n%{http_code}" $URL/health | tail -n1)

if [ "$status_code" == "200" ]; then
  echo "PASSED: Application is running"
else
  echo "FAILED: Application is not running"
  exit 1
fi

# =============================================================================
# Initial State - No Services
# =============================================================================
echo ""
echo "--- Initial State ---"

# Get services list (should be empty)
response=$(curl -s -w "\n%{http_code}" $URL/services)
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

# =============================================================================
# Register Multiple Services
# =============================================================================
echo ""
echo "--- Register Multiple Services ---"

# Register first service
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/register \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=service-alpha" \
  -d "type=echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "201" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/register service-alpha (status: $status_code)"
else
  echo "FAILED: POST /services/register service-alpha"
  echo "  Expected: status 201"
  echo "  Got: status $status_code"
  exit 1
fi

# Register second service
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/register \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=service-beta" \
  -d "type=echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "201" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/register service-beta (status: $status_code)"
else
  echo "FAILED: POST /services/register service-beta"
  echo "  Expected: status 201"
  echo "  Got: status $status_code"
  exit 1
fi

# Register third service
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/register \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=service-gamma" \
  -d "type=echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "201" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/register service-gamma (status: $status_code)"
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

response=$(curl -s -w "\n%{http_code}" $URL/services)
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

# Check service status
status_0=$(echo "$body" | jq -r '.data[0].status')
status_1=$(echo "$body" | jq -r '.data[1].status')
status_2=$(echo "$body" | jq -r '.data[2].status')

if [ "$status_0" == "registered" ] && [ "$status_1" == "registered" ] && [ "$status_2" == "registered" ]; then
  echo "PASSED: All services have status 'registered'"
else
  echo "FAILED: Initial service status check"
  echo "  Expected: all 'registered'"
  echo "  Got: service 0='$status_0', service 1='$status_1', service 2='$status_2'"
  exit 1
fi

# =============================================================================
# Service Before Start (should fail)
# =============================================================================
echo ""
echo "--- Service Before Start ---"

response=$(curl -s -w "\n%{http_code}" -X POST $URL/0/echo -d "test")
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /0/echo before start returns 404"
else
  echo "FAILED: POST /0/echo before start"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Start Multiple Services
# =============================================================================
echo ""
echo "--- Start Multiple Services ---"

# Start service 0
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/0/start)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/0/start (status: $status_code)"
else
  echo "FAILED: POST /services/0/start"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Start service 1
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/1/start)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/1/start (status: $status_code)"
else
  echo "FAILED: POST /services/1/start"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Verify Status After Start
# =============================================================================
echo ""
echo "--- Verify Status After Start ---"

response=$(curl -s -w "\n%{http_code}" $URL/services)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
status_0=$(echo "$body" | jq -r '.data[0].status')
status_1=$(echo "$body" | jq -r '.data[1].status')
status_2=$(echo "$body" | jq -r '.data[2].status')

if [ "$status_0" == "running" ] && [ "$status_1" == "running" ] && [ "$status_2" == "registered" ]; then
  echo "PASSED: Service 0 and 1 are 'running', service 2 is 'registered'"
else
  echo "FAILED: Status check after starting services 0 and 1"
  echo "  Expected: service 0='running', service 1='running', service 2='registered'"
  echo "  Got: service 0='$status_0', service 1='$status_1', service 2='$status_2'"
  exit 1
fi

# =============================================================================
# Echo to Running Services
# =============================================================================
echo ""
echo "--- Echo to Running Services ---"

# Echo to service 0
response=$(curl -s -w "\n%{http_code}" -X POST $URL/0/echo -d "hello from 0")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "200" ] && [ "$body" == "hello from 0" ]; then
  echo "PASSED: POST /0/echo returns correct echo"
else
  echo "FAILED: POST /0/echo"
  echo "  Expected: status 200, body 'hello from 0'"
  echo "  Got: status $status_code, body '$body'"
  exit 1
fi

# Echo to service 1
response=$(curl -s -w "\n%{http_code}" -X POST $URL/1/echo -d "hello from 1")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "200" ] && [ "$body" == "hello from 1" ]; then
  echo "PASSED: POST /1/echo returns correct echo"
else
  echo "FAILED: POST /1/echo"
  echo "  Expected: status 200, body 'hello from 1'"
  echo "  Got: status $status_code, body '$body'"
  exit 1
fi

# Echo to service 2 (not started, should fail)
response=$(curl -s -w "\n%{http_code}" -X POST $URL/2/echo -d "hello from 2")
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /2/echo returns 404 (service not started)"
else
  echo "FAILED: POST /2/echo (service not started)"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Stop Services
# =============================================================================
echo ""
echo "--- Stop Services ---"

# Stop service 0
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/0/stop)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/0/stop (status: $status_code)"
else
  echo "FAILED: POST /services/0/stop"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Verify service 0 is stopped
response=$(curl -s -w "\n%{http_code}" $URL/services)
body=$(echo "$response" | head -n -1)
status_0=$(echo "$body" | jq -r '.data[0].status')

if [ "$status_0" == "stopped" ]; then
  echo "PASSED: Service 0 status is 'stopped' after stop"
else
  echo "FAILED: Service 0 status after stop"
  echo "  Expected: 'stopped'"
  echo "  Got: '$status_0'"
  exit 1
fi

# Echo to stopped service 0 (should fail)
response=$(curl -s -w "\n%{http_code}" -X POST $URL/0/echo -d "test")
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /0/echo after stop returns 404"
else
  echo "FAILED: POST /0/echo after stop"
  echo "  Expected: status 404"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Unregister Services
# =============================================================================
echo ""
echo "--- Unregister Services ---"

# Unregister service 2 (the one that was never started)
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/2/unregister)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/2/unregister (status: $status_code)"
else
  echo "FAILED: POST /services/2/unregister"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Verify we now have 2 services
response=$(curl -s -w "\n%{http_code}" $URL/services)
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

# Unregister service 1 (was running)
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/1/unregister)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/1/unregister (running service)"
else
  echo "FAILED: POST /services/1/unregister"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Verify we now have 1 service
response=$(curl -s -w "\n%{http_code}" $URL/services)
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

# Unregister service 0 (was stopped)
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/0/unregister)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/0/unregister (stopped service)"
else
  echo "FAILED: POST /services/0/unregister"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# =============================================================================
# Verify Empty State After Unregister All
# =============================================================================
echo ""
echo "--- Verify Empty State ---"

response=$(curl -s -w "\n%{http_code}" $URL/services)
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
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/0/unregister)
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "404" ]; then
  echo "PASSED: POST /services/0/unregister on non-existent service returns 404"
else
  echo "FAILED: POST /services/0/unregister on non-existent service"
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
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/register \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=fresh-service" \
  -d "type=echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "201" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/register fresh-service after unregister all"
else
  echo "FAILED: POST /services/register after unregister all"
  echo "  Expected: status 201"
  echo "  Got: status $status_code"
  exit 1
fi

# Start and test the fresh service
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/3/start)
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)
success=$(echo "$body" | jq -r '.success')

if [ "$status_code" == "200" ] && [ "$success" == "true" ]; then
  echo "PASSED: POST /services/3/start fresh service"
else
  echo "FAILED: POST /services/3/start fresh service"
  echo "  Expected: status 200"
  echo "  Got: status $status_code"
  exit 1
fi

# Echo to fresh service
response=$(curl -s -w "\n%{http_code}" -X POST $URL/3/echo -d "fresh echo")
body=$(echo "$response" | head -n -1)
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "200" ] && [ "$body" == "fresh echo" ]; then
  echo "PASSED: POST /3/echo to fresh service"
else
  echo "FAILED: POST /3/echo to fresh service"
  echo "  Expected: status 200, body 'fresh echo'"
  echo "  Got: status $status_code, body '$body'"
  exit 1
fi

# Cleanup: unregister the fresh service
response=$(curl -s -w "\n%{http_code}" -X POST $URL/services/3/unregister)
status_code=$(echo "$response" | tail -n1)

if [ "$status_code" == "200" ]; then
  echo "PASSED: Cleanup - unregistered fresh service"
else
  echo "WARNING: Cleanup failed - could not unregister fresh service"
fi

# =============================================================================
# Summary
# =============================================================================
echo ""
echo "=== All Tests Passed ==="
