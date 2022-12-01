BIN_FOLDER="./bin"
LOCAL_EXECUTABLE="$BIN_FOLDER/local"
SERVER_EXECUTABLE="$BIN_FOLDER/server"
CLIENT_EXECUTABLE="$BIN_FOLDER/client"
MPI_EXECUTABLE="$BIN_FOLDER/mpi"
MPI_RUNNER="mpirun"

# check if executables exist
if [ ! -f "$LOCAL_EXECUTABLE" ]; then
    echo "$LOCAL_EXECUTABLE does not exist."
    exit 1
fi

if [ ! -f "$SERVER_EXECUTABLE" ]; then
    echo "$SERVER_EXECUTABLE does not exist."
    exit 1
fi

if [ ! -f "$CLIENT_EXECUTABLE" ]; then
    echo "$CLIENT_EXECUTABLE does not exist."
    exit 1
fi

if [ ! -f "$MPI_EXECUTABLE" ]; then
    echo "$MPI_EXECUTABLE does not exist."
    exit 1
fi

# store command line arguments
while getopts 'm:i:r:c:' OPTION; do
    case "$OPTION" in
        m)
            MODE="$OPTARG"
        ;;
        i)
            INPUT_PATH="$OPTARG"
        ;;
        r)
            STEPS="$OPTARG"
        ;;
        c)
            CLIENT_COUNT="$OPTARG"
        ;;
        ?)
            echo "script usage: $(basename \$0) [-m 0,1,2,3,4] [-i PATH] [-r NUMBER] [-c NUMBER]" >&2
            exit 1
        ;;
    esac
done

if [ -z ${MODE+x} ]; then
    echo "mode not specified, use -m to specify mode"
    exit 1
fi

if [ -z ${INPUT_PATH+x} ]; then
    echo "input path not specified, use -i to set input path"
    exit 1
fi

if [ -z ${STEPS+x} ]; then
    echo "steps not specified, use -r to set steps"
    exit 1
fi

if [ -z ${CLIENT_COUNT+x} ]; then
    echo "client count not specified, use -c to specify clients, use 0 if mode is 0"
    exit 1
fi

case "$MODE" in
    0)
        "$LOCAL_EXECUTABLE" -i "$INPUT_PATH" -r "$STEPS" --profile "benchmarks/temp.csv"
    ;;
    1)
        echo "Not yet implemented"
    ;;
    2)
        echo "Not yet implemented"
    ;;
    3)
        NODE_COUNT=$(($CLIENT_COUNT+1))
        "$MPI_RUNNER" -n "$NODE_COUNT" "$MPI_EXECUTABLE" -i "$INPUT_PATH" -r "$STEPS" --profile "benchmarks/temp.csv" --mode "simple"
    ;;
    4)
        NODE_COUNT=$(($CLIENT_COUNT+1))
        "$MPI_RUNNER" -n "$NODE_COUNT" "$MPI_EXECUTABLE" -i "$INPUT_PATH" -r "$STEPS" --profile "benchmarks/temp.csv" --mode "advanced"
    ;;
    ?)
        echo "script usage: $(basename \$0) [-t 0,1,2,3,4] [-i PATH] [-r NUMBER] [-c NUMBER]" >&2
        exit 1
    ;;
esac

exit 0