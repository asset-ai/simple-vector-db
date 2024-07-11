for i in $(seq 1 300); do
  vector_size=$(( (RANDOM % 10) + 1 )) 
  vector="["
  for ((j=0; j<$vector_size; j++)); do
    random_number=$(awk -v min=1 -v max=10 'BEGIN{srand(); printf "%.2f", min+rand()*(max-min+1)}')
    vector="${vector}${random_number}"
    if ((j < $vector_size - 1)); then
      vector="${vector}, "
    fi
  done
  vector="${vector}]"
  curl -X POST -H "Content-Type: application/json" -d "${vector}" http://localhost:8888/vector
done