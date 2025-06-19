#!/bin/bash

if [ -z "$1" ]; then
  echo "Usage: $0 <number_of_blocks>"
  exit 1
fi

COUNT=$1
OUTDIR="data"
mkdir -p "$OUTDIR"
rm -f "$OUTDIR"/*.txt

current_hash=$(curl -s https://api.blockcypher.com/v1/btc/main | grep '"hash"' | head -n 1 | sed 's/.*"hash":[[:space:]]*"\([^"]*\)".*/\1/')

for (( i=0; i<$COUNT; i++ ))
do
  echo "Fetching block $i with hash: $current_hash"

  block_data=$(curl -s https://api.blockcypher.com/v1/btc/main/blocks/$current_hash)

  hash=$(echo "$block_data" | grep '"hash"' | head -n 1 | sed 's/.*"hash":[[:space:]]*"\([^"]*\)".*/\1/')
  height=$(echo "$block_data" | grep '"height"' | sed 's/.*"height":[[:space:]]*\([0-9]*\).*/\1/')
  total=$(echo "$block_data" | grep '"total"' | sed 's/.*"total":[[:space:]]*\([0-9]*\).*/\1/')
  time=$(echo "$block_data" | grep '"time"' | sed 's/.*"time":[[:space:]]*"\([^"]*\)".*/\1/')
  relayed_by=$(echo "$block_data" | grep '"relayed_by"' | sed 's/.*"relayed_by":[[:space:]]*"\([^"]*\)".*/\1/')
  prev_block=$(echo "$block_data" | grep '"prev_block"' | sed 's/.*"prev_block":[[:space:]]*"\([^"]*\)".*/\1/')

  {
    echo "hash: $hash"
    echo "height: $height"
    echo "total: $total"
    echo "time: $time"
    echo "relayed_by: $relayed_by"
    echo "prev_block: $prev_block"
  } > "$OUTDIR/block_$i.txt"

  current_hash=$prev_block
done

