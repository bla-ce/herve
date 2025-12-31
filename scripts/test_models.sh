#!/bin/bash
set -euo pipefail

URL="http://localhost:1337"

declare -A EXPECT_USER_0=( ["id"]=0 ["username"]="user1" ["password"]="pass1" )
declare -A EXPECT_USER_1=( ["id"]=1 ["username"]="user2" ["password"]="pass2" )
declare -A EXPECT_POST_USER=( ["id"]=2 ["username"]="user3" ["password"]="pass3" ["age"]="18")
declare -A EXPECT_PATCH_USER=( ["id"]=0 ["username"]="user1" ["password"]="pass4" )

check_success() {
    local json="$1"
    local expected="$2"
    actual=$(jq -r '.success' <<< "$json")
    if [[ "$actual" != "$expected" ]]; then
        echo "success mismatch (expected=$expected, actual=$actual)"
        exit 1
    fi
}

validate_user_fields() {
    local json="$1"
    declare -n expected="$2"
    for field in id username password; do
        local expected_val="${expected[$field]}"
        local actual_val
        actual_val=$(jq -r --arg f "$field" '.data[$f]' <<< "$json")
        if [[ "$actual_val" != "$expected_val" ]]; then
            echo "Field mismatch: $field (expected=$expected_val, actual=$actual_val)"
            exit 1
        fi
    done
}

validate_user_list() {
    local json="$1"
    shift
    local index=0
    for expected_name in "$@"; do
        declare -n expected="$expected_name"
        for field in id username password; do
            local expected_val="${expected[$field]}"
            local actual_val
            actual_val=$(jq -r ".data[$index].$field" <<< "$json")
            if [[ "$actual_val" != "$expected_val" ]]; then
                echo "List mismatch at index $index: $field"
                echo "expected=$expected_val"
                echo "actual=$actual_val"
                exit 1
            fi
        done
        index=$((index + 1))
    done
}

request() {
    local method="$1"
    local path="$2"
    curl -sS -X "$method" "$URL$path" -H "Content-Type: application/json"
}

echo "Get user list"
resp=$(request GET /user)
check_success "$resp" true
validate_user_list "$resp" EXPECT_USER_0 EXPECT_USER_1
echo "User list OK"

echo "Get valid user (id=1)"
resp=$(request GET /user/1)
check_success "$resp" true
validate_user_fields "$resp" EXPECT_USER_1
echo "Valid user OK"

echo "Get invalid user"
resp=$(request GET /user/999)
check_success "$resp" false
echo "Invalid user OK"

echo "POST new user (user3, pass3, 18)"
resp=$(curl -sS -X POST "$URL/user" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "username=user3" \
  -d "password=pass3" \
  -d "age=18" )

check_success "$resp" true
validate_user_fields "$resp" EXPECT_POST_USER
echo "POST user OK"

echo "Get new user (id=2)"
resp=$(request GET /user/2)
check_success "$resp" true
validate_user_fields "$resp" EXPECT_POST_USER
echo "New user OK"

echo "Get list after POST"
resp=$(request GET /user)
check_success "$resp" true
validate_user_list "$resp" EXPECT_USER_0 EXPECT_USER_1 EXPECT_POST_USER
echo "User list OK after POST"

echo "Delete user id=2"
curl -sS -X DELETE "$URL/user/2" >/dev/null
echo "User deleted"

echo "Get list after delete"
resp=$(request GET /user)
check_success "$resp" true
validate_user_list "$resp" EXPECT_USER_0 EXPECT_USER_1
echo "Delete list OK"

echo "PATCH user 0"
resp=$(curl -sS -X PATCH "$URL/user/0" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "password=pass4")

check_success "$resp" true
validate_user_fields "$resp" EXPECT_PATCH_USER

resp=$(request GET /user)
check_success "$resp" true
validate_user_list "$resp" EXPECT_PATCH_USER EXPECT_USER_1
echo "PATCH user OK"

echo "ALL TESTS PASSED"
