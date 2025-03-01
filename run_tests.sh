#!/usr/bin/env bash

for i in {1..5}
do
  ./test.sh &
done

# Wait for all background jobs to finish
wait
echo "All 5 runs are complete."
