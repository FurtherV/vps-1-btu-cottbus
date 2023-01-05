import argparse
from typing import List, Dict
import subprocess
import pathlib
import os
import logging
import sys
import socket
import numpy as np


def clear_logs(log_folder: str = "logs/"):
    if not os.path.exists(log_folder):
        return None

    for log_file in [log_folder + x for x in os.listdir(log_folder)]:
        if os.path.exists(log_file):
            os.remove(log_file)
    return None


def launch_process(args: List[str]):
    stdout = subprocess.DEVNULL
    pipe = subprocess.Popen(args, cwd=".", stdout=stdout, stderr=subprocess.STDOUT)
    return pipe


def export(timings: Dict[int, List[int]], file_path: str, header_text: str) -> None:
    benchmark_folder = "benchmarks/"
    os.makedirs(
        os.path.dirname(os.path.join(benchmark_folder, file_path)), exist_ok=True
    )
    handle = open(os.path.join(benchmark_folder, file_path), "w")
    handle.write("# x,y\n")
    handle.write("# steps, time in milliseconds\n")
    handle.write(f"HEADER,{header_text}\n")
    for key, value in timings.items():
        handle.write(",".join([str(key)] + [str(x) for x in value]))
        handle.write("\n")
    handle.close()
    return None


def import_timings_from_file(timings: Dict[int, List[int]], file_path: str):
    data = np.loadtxt(file_path, dtype=int, delimiter=",", ndmin=2)
    for row in data:
        step = row[0] + 1
        time = max(row[1], 1)
        if not step in timings:
            timings[step] = [time]
        else:
            timings[step].append(time)
    return None


def benchmark_taco(
    steps: int, repeat: int, board_file: str, node_count: int
) -> Dict[int, List[int]]:
    executable_path = "./bin/taco"
    benchmark_temp_path = "benchmarks/temp.csv"
    hostfile_path = "~/hostfile"
    timings = {}

    for _ in range(repeat):
        pipe = launch_process(
            [
                "mpirun",
                "-n",
                str(node_count),
                "-default-hostfile",
                hostfile_path,
                executable_path,
                "-i",
                board_file,
                "-r",
                str(steps),
                "--profile",
                benchmark_temp_path,
            ]
        )
        pipe.communicate()
        import_timings_from_file(timings, benchmark_temp_path)
    return timings


def benchmark_mpi(
    steps: int, repeat: int, board_file: str, node_count: int
) -> Dict[int, List[int]]:
    executable_path = "./bin/mpi"
    benchmark_temp_path = "benchmarks/temp.csv"
    hostfile_path = "~/hostfile"
    timings = {}

    for _ in range(repeat):
        pipe = launch_process(
            [
                "mpirun",
                "-n",
                str(node_count),
                "-default-hostfile",
                hostfile_path,
                executable_path,
                "-i",
                board_file,
                "-r",
                str(steps),
                "--profile",
                benchmark_temp_path,
            ]
        )
        pipe.communicate()
        import_timings_from_file(timings, benchmark_temp_path)
    return timings


def benchmark_server(
    steps: int, repeat: int, board_file: str, node_count: int, network_type: int
) -> Dict[int, List[int]]:
    server_executable_path = "./bin/server"
    client_executable_path = "./bin/client"
    benchmark_temp_path = "benchmarks/temp.csv"
    hostfile_path = "~/hostfile"
    server_host_name = socket.gethostname()
    timings = {}

    for _ in range(repeat):
        pipe = launch_process(
            [
                server_executable_path,
                "-i",
                board_file,
                "-r",
                str(steps),
                "--profile",
                benchmark_temp_path,
                "-n",
                str(network_type),
            ]
        )

        launch_process(
            [
                "mpirun",
                "-n",
                str(node_count),
                "-default-hostfile",
                hostfile_path,
                client_executable_path,
                "--host",
                server_host_name,
                "-n",
                str(network_type),
            ]
        )
        pipe.communicate()
        import_timings_from_file(timings, benchmark_temp_path)
    return timings


def benchmark_local(steps: int, repeat: int, board_file: str) -> Dict[int, List[int]]:
    executable_path = "./bin/local"
    benchmark_temp_path = "benchmarks/temp.csv"
    hostfile_path = "~/hostfile"
    timings = {}

    for _ in range(repeat):
        pipe = launch_process(
            [
                "mpirun",
                "-n",
                "1",
                "-default-hostfile",
                hostfile_path,
                executable_path,
                "-i",
                board_file,
                "-r",
                str(steps),
                "--profile",
                benchmark_temp_path,
            ]
        )
        pipe.communicate()
        import_timings_from_file(timings, benchmark_temp_path)
    return timings


if __name__ == "__main__":
    # initialize command line argument parser
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument(
        "--nodes",
        type=int,
        default=8,
        help="Max number of nodes / processors used for computation\nDefault: 8",
    )
    parser.add_argument(
        "--steps",
        type=int,
        default=10,
        help="Amount of steps to be simulated\nDefault: 10",
    )
    parser.add_argument(
        "--repeat",
        type=int,
        default=1,
        help="How many times a step is repeated\nDefault: 1",
    )
    parser.add_argument(
        "--board",
        type=str,
        default="boards/bigun.rle",
        help="Path to a RLE board file\nDefault: boards/bigun.rle",
    )
    args = parser.parse_args()

    # initialize logger
    logFormatter = logging.Formatter(
        "%(asctime)s [%(threadName)s] [%(levelname)s]:  %(message)s"
    )
    rootLogger = logging.getLogger()
    rootLogger.setLevel(logging.DEBUG)

    consoleHandler = logging.StreamHandler(sys.stdout)
    consoleHandler.setFormatter(logFormatter)
    rootLogger.addHandler(consoleHandler)

    # empty logs folder
    clear_logs()
    logging.info("Log Folder cleared.")

    # store cmd args in variables for easier access
    nodes = args.nodes
    steps = args.steps
    repeat = args.repeat
    board_file = args.board
    logging.info(f"Steps: {steps} repeated {repeat} times.")

    # do benchmarks

    timings_local = benchmark_local(steps, repeat, board_file)
    export(timings_local, "steps_over_time/local.csv", "Local,1")
    logging.info("Local Benchmark done.")

    for i in range(1, nodes):
        timings_mpi = benchmark_mpi(steps, repeat, board_file, i)
        export(timings_mpi, f"steps_over_time/mpi_{i}.csv", f"MPI,{i}")
        logging.info(f"MPI Benchmark with {i} clients done.")

    for i in range(1, nodes):
        timings_server = benchmark_server(steps, repeat, board_file, i, 0)
        export(timings_server, f"steps_over_time/server_udp_{i}.csv", f"UDP,{i}")
        logging.info(f"UDP Benchmark with {i} clients done.")

    for i in range(1, nodes):
        timings_server = benchmark_server(steps, repeat, board_file, i, 1)
        export(timings_server, f"steps_over_time/server_tcp_{i}.csv", f"TCP,{i}")
        logging.info(f"TCP Benchmark with {i} clients done.")

    for i in range(1, nodes):
        timings_taco_server = benchmark_taco(steps, repeat, board_file, i)
        export(
            timings_taco_server,
            f"steps_over_time/taco_server_{i}.csv",
            f"TACO-Server,{i}",
        )
        logging.info(f"TACO-Server Benchmark with {i} clients done.")
