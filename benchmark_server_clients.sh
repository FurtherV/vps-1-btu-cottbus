mkdir -p benchmarks

pkill -9 -f ./bin/server
pkill -9 -f ./bin/client

CLIENTS=(1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20)
STEPS=20
REPEAT=5
NETWORK_TYPE=1
OUTPUT_PATH="benchmarks/clients_over_time_server-$STEPS-steps-${CLIENTS[-1]}-clients-tcp.csv"

for i in "${CLIENTS[@]}"
do
    # declare file paths
    BENCHMARK_TEMP_PATH="benchmarks/temp.csv"
    printf -v BENCHMARK_ID "%03d" $i
    BENCHMARK_PATH="benchmarks/clients_over_time-$STEPS-$BENCHMARK_ID.csv"
    
    # delete old files
    rm -f "$BENCHMARK_PATH"
    rm -f "$BENCHMARK_TEMP_PATH"
    
    # start benchmark and repeat REPEAT times
    echo "Starting Benchmark for $STEPS steps with $i clients"
    for ((j=0; j<$REPEAT; j++)) do
        ./server_launcher.sh -i boards/bigun.rle -p $BENCHMARK_TEMP_PATH -c $i -r $STEPS -n $NETWORK_TYPE
        wait
        echo -n "$i," >> "$BENCHMARK_PATH"
        cat "$BENCHMARK_TEMP_PATH" >> "$BENCHMARK_PATH"
        echo "" >> "$BENCHMARK_PATH"
    done
    echo "Done."
    echo ""
done

echo "# x,y" >> "$OUTPUT_PATH"
echo "# clients,time in milliseconds" >> "$OUTPUT_PATH"
find "benchmarks/" -name "clients_over_time-$STEPS-*.csv" -print0 | sort -z | xargs -0 cat >> "$OUTPUT_PATH"

echo "Benchmark complete."
exit 0