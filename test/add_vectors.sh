#!/bin/bash

# Seed the random number generator only once
RANDOM=$$$(date +%s)

for i in $(seq 1 100000); do
  # Generate a UUID for each vector
  uuid=$(uuidgen)

  # Vector size (set to 128 for all vectors)
  vector_size=128
  vector="["
  for ((j=0; j<$vector_size; j++)); do
    # Generate a new integer part for each element
    integer_part=$(awk -v min=1 -v max=9 -v seed=$RANDOM 'BEGIN{srand(seed); printf "%d", int(min+rand()*(max-min+1))}')
    decimal_places=$((RANDOM % 7))
    decimal_part=""
    for ((k=0; k<$decimal_places; k++)); do
      decimal_part="${decimal_part}$((RANDOM % 10))"
    done

    # Add decimal point only if there are decimal places
    if (( decimal_places > 0 )); then
      random_number="${integer_part}.${decimal_part}"
    else
      random_number="${integer_part}"
    fi

    vector="${vector}${random_number}"
    if ((j < $vector_size - 1)); then
      vector="${vector}, "
    fi
  done
  vector="${vector}]"

  # Construct JSON payload with UUID and vector
  json_payload=$(printf '{"uuid": "%s", "vector": %s}' "$uuid" "$vector")

  # Send POST request with JSON payload
  curl -X POST -H "Content-Type: application/json" -d "${json_payload}" http://localhost:8888/vector
done
