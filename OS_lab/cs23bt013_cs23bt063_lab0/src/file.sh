#!/bin/bash

INPUT_FILE="../images/$1.ppm"
OUTPUT_FILE="../images/$1_RES.ppm"

if [[ ! -f "$INPUT_FILE" ]]; then
    echo " Input file $INPUT_FILE not found!"
    exit 1
fi

runs=5
sum_read=0
sum_s1=0
sum_s2=0
sum_s3=0
sum_write=0

for ((i=1; i<=runs; i++)); do
    
    output=$(./a.out "$INPUT_FILE" "$OUTPUT_FILE")

    read_ms=$(echo "$output" | grep "Time (read)" | awk '{print $3}')
    s1_ms=$(echo "$output" | grep "Time (S1 smoothen)" | awk '{print $4}')
    s2_ms=$(echo "$output" | grep "Time (S2 find details)" | awk '{print $5}')
    s3_ms=$(echo "$output" | grep "Time (S3 sharpen)" | awk '{print $4}')
    write_ms=$(echo "$output" | grep "Time (write)" | awk '{print $3}')

    sum_read=$((sum_read + read_ms))
    sum_s1=$((sum_s1 + s1_ms))
    sum_s2=$((sum_s2 + s2_ms))
    sum_s3=$((sum_s3 + s3_ms))
    sum_write=$((sum_write + write_ms))
done

avg_read=$(echo "scale=2; $sum_read / $runs" | bc)
avg_s1=$(echo "scale=2; $sum_s1 / $runs" | bc)
avg_s2=$(echo "scale=2; $sum_s2 / $runs" | bc)
avg_s3=$(echo "scale=2; $sum_s3 / $runs" | bc)
avg_write=$(echo "scale=2; $sum_write / $runs" | bc)


echo "Average time for read: $avg_read ms"
echo "Average time for  smoothening: $avg_s1 ms"
echo "Average time for find details: $avg_s2 ms"
echo "Average time for sharpen: $avg_s3 ms"
echo "Average time for write: $avg_write ms"
