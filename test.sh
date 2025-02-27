#!/bin/bash

# Define colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No color

routes=0

# Define the tests - format: url method status_code (response)
tests=(
  "http://192.168.122.129:1337/ GET 400" # missing parameter
  "http://192.168.122.129:1337/?name=jul&coucou=cocou GET 204"
  "http://192.168.122.129:1337/ POST 404"
  "http://192.168.122.129:1337/ HEHE 404"
  "http://192.168.122.129:1337/ HEHEHEHEHEHE 500"
  "http://192.168.122.129:1337/health GET 200"
  "http://192.168.122.129:1337/redirect GET 302"
  "http://192.168.122.129:1337/template GET 200"
  "http://192.168.122.129:1337/index GET 200"
  "http://192.168.122.129:1337/index/path GET 404"
  "http://192.168.122.129:1337/examples/views/index.html GET 200"
  "http://192.168.122.129:1337/examples/views/index.js GET 200"
  "http://192.168.122.129:1337/examples/views/style.css GET 200"
  "http://192.168.122.129:1337/in GET 404"
  "http://192.168.122.129:1337/index?query=53094 GET 200"
  "http://192.168.122.129:1337/index#anchor GET 200"
  "http://192.168.122.129:1337/not-found GET 404"
  "http://192.168.122.129:1337/nonot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundnot-foundt-found GET 500"
)

sum_time_us=0  # Accumulate total processing time in microseconds
sum_tests=0

# Loop for repeated test runs
for i in {1..100}
do
  total_tests=${#tests[@]}
  passed_tests=0
  routes=0

  # Function to perform the test
  perform_test() {
    local url=$1
    local method=$2
    local expected_status=$3
    local test_number=$4
    local total_tests=$5

    # Perform the request.
    # The -w option outputs:
    # 1) HTTP status code on the second-to-last line
    # 2) Total time (in seconds) on the last line
    response=$(curl -s -w "\n%{http_code}\n%{time_total}" -X "$method" "$url")
    status_code=$(echo "$response" | tail -n2 | head -n1)
    request_time=$(echo "$response" | tail -n1)

    # Convert the request time to microseconds
    request_time_us=$(echo "$request_time * 1000000" | bc)
    sum_time_us=$(echo "$sum_time_us + $request_time_us" | bc)

    if [ "$status_code" -eq 000 ]; then
      echo "Received 000"
      echo "$method $url"
      echo "Routes: $routes"
    fi

    # Check if the actual status code matches the expected one
    if [ "$status_code" -eq "$expected_status" ]; then
      echo -e "[${YELLOW}TEST $test_number/$total_tests${NC}] : $method $url -> ${GREEN}PASSED${NC} (expected: $expected_status, time: ${request_time_us} µs)"
      passed_tests=$((passed_tests + 1))
    else
      echo -e "[${YELLOW}TEST $test_number/$total_tests${NC}] : $method $url -> ${RED}FAILED${NC} (expected: $expected_status, got: $status_code, time: ${request_time_us} µs)"
      read -p "FAILED TEST"
    fi

    ((routes++))
  }

  echo -e "\n---------- HTTP Server Tests (Run #$i) ----------\n"

  # Loop through each test
  test_number=1
  for test in "${tests[@]}"; do
    # Split the test into URL, method, expected status code
    IFS=' ' read -r -a parts <<< "$test"
    url="${parts[0]}"
    method="${parts[1]}"
    expected_status="${parts[2]}"

    # Perform the test
    perform_test "$url" "$method" "$expected_status" "$test_number" "$total_tests"

    # Increment the test number
    test_number=$((test_number + 1))
    sum_tests=$((sum_tests + 1))
  done

  # Calculate the average processing time in microseconds for the run

  echo -e "\n--------------- Results for Run #$i ---------------\n"
  echo -e "Total Tests: $total_tests, ${GREEN}PASSED${NC}: $passed_tests, ${RED}FAILED${NC}: $((total_tests - passed_tests))"
done

avg_time_us=$(echo "scale=0; $sum_time_us / $sum_tests" | bc)
echo -e "Average Processing Time: ${avg_time_us} µs\n"
