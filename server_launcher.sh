mkdir -p logs

pkill -9 -f ./bin/server
pkill -9 -f ./bin/client

while getopts 'i:o:r:c:n:p:' OPTION; do
    case "$OPTION" in
        i)
            INPUT_PATH="$OPTARG"
        ;;
        o)
            OUTPUT_PATH="$OPTARG"
        ;;
        r)
            STEPS="$OPTARG"
        ;;
        c)
            CLIENT_COUNT="$OPTARG"
        ;;
        n)
            NETWORK_TYPE="$OPTARG"
        ;;
        p)
            PROFILE_OUTPUT="$OPTARG"
        ;;
        ?)
            echo "script usage: $(basename \$0) [-i PATH] [-o PATH] [-p PATH] [-r NUMBER] [-c NUMBER] [-n 0 or 1]" >&2
            exit 1
        ;;
    esac
done

echo "-i=${INPUT_PATH}"
echo "-o=${OUTPUT_PATH}"
echo "-r=${STEPS:=1}"
echo "-c=${CLIENT_COUNT:=1}"
echo "-n=${NETWORK_TYPE:=0}"
echo "--profile=${PROFILE_OUTPUT}"

if ! [ -z "$INPUT_PATH" ]; then
    SERVER_LAUNCH_ARGS+="-i ${INPUT_PATH} "
fi

if ! [ -z "$OUTPUT_PATH" ]; then
    SERVER_LAUNCH_ARGS+="-o ${OUTPUT_PATH} "
fi

if ! [ -z "$PROFILE_OUTPUT" ]; then
    SERVER_LAUNCH_ARGS+="--profile ${PROFILE_OUTPUT} "
fi

SERVER_LAUNCH_ARGS+="-r ${STEPS} "
SERVER_LAUNCH_ARGS+="-c ${CLIENT_COUNT} "
SERVER_LAUNCH_ARGS+="-n ${NETWORK_TYPE}"

CLIENT_LAUNCH_ARGS+="-n ${NETWORK_TYPE}"

./bin/server $SERVER_LAUNCH_ARGS >> "./logs/server.log" &
SERVER_JOB_ID="$!"

CLIENT_JOB_IDS=()
for ((i=1; i<=$CLIENT_COUNT; i++)) do
    ./bin/client $CLIENT_LAUNCH_ARGS >> "./logs/client-$i.log" &
    CLIENT_JOB_IDS+=($!)
done

trap 'pkill -P $$' SIGINT SIGTERM

wait

echo "Simulation complete."