mkdir -p benchmarks

pkill -9 -f ./bin/main

STEPS=(1 5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)
REPEAT=5
OUTPUT_PATH="benchmarks/steps_over_time_local.csv"

for i in "${STEPS[@]}"
do
    # declare file paths
    BENCHMARK_TEMP_PATH="benchmarks/temp.csv"
    printf -v BENCHMARK_ID "%03d" $i
    BENCHMARK_PATH="benchmarks/steps_over_time-local-$BENCHMARK_ID.csv"
    
    # delete old files
    rm -f "$BENCHMARK_PATH"
    rm -f "$BENCHMARK_TEMP_PATH"
    
    # start benchmark and repeat REPEAT times
    echo "Starting Benchmark for $i steps"
    for ((j=0; j<$REPEAT; j++)) do
        ./bin/main -i boards/bigun.rle --profile $BENCHMARK_TEMP_PATH -r $i
        wait
        echo -n "$i," >> "$BENCHMARK_PATH"
        cat "$BENCHMARK_TEMP_PATH" >> "$BENCHMARK_PATH"
        echo "" >> "$BENCHMARK_PATH"
    done
    echo "Done."
    echo ""
done

rm -f "$OUTPUT_PATH"
echo "# x,y" >> "$OUTPUT_PATH"
echo "# steps,time in milliseconds" >> "$OUTPUT_PATH"
find "benchmarks/" -name "steps_over_time-local-*.csv" -print0 | sort -z | xargs -0 cat >> "$OUTPUT_PATH"

echo "Benchmark complete."
exit 0