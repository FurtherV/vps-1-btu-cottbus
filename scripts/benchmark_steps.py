import argparse
from typing import List, Dict
import subprocess
import pathlib
import os
import logging
import sys


def clear_logs(log_folder: str = "logs/"):
    if not os.path.exists(log_folder):
        return None

    for log_file in [log_folder + x for x in os.listdir(log_folder)]:
        if os.path.exists(log_file):
            os.remove(log_file)
    return None


def launch_process(args: List[str], log_file: str, log_folder: str = "logs/"):
    stdout = subprocess.DEVNULL
    if log_file != "/dev/null" and log_file != "" and log_file != None:
        pathlib.Path(log_folder).mkdir(parents=True, exist_ok=True)
        stdout = open(log_folder + log_file, "a")
    pipe = subprocess.Popen(args, cwd=".", stdout=stdout, stderr=subprocess.STDOUT)
    return pipe


def export(timings: Dict[int, List[int]], name: str) -> None:
    benchmark_folder = "benchmarks/"
    benchmark_file = benchmark_folder + name
    benchmark_file_name = benchmark_file.split("/")[-1]
    pathlib.Path(benchmark_file).parent.mkdir(parents=True, exist_ok=True)

    handle = open(benchmark_file, "w")
    handle.write("# x,y\n")
    handle.write("# steps, time in milliseconds\n")
    handle.write(f"header,0,{benchmark_file_name}\n")
    for key, value in timings.items():
        handle.write(",".join([str(key)] + [str(x) for x in value]))
        handle.write("\n")
    handle.close()
    return None


def benchmark_mpi(
    steps: List[int], repeat: int, client_count: int
) -> Dict[int, List[int]]:
    executable_path = "bin/mpi"
    board_path = "boards/bigun.rle"
    benchmark_temp_path = "benchmarks/temp.csv"
    timings = {}

    for step in steps:
        if step not in timings:
            timings[step] = []

        for _ in range(repeat):
            pipe = launch_process(
                [
                    "mpirun",
                    "-np",
                    str(client_count),
                    executable_path,
                    "-i",
                    board_path,
                    "-r",
                    str(step),
                    "--profile",
                    benchmark_temp_path,
                ],
                "mpi.log",
            )
            pipe.communicate()
            handle = open(benchmark_temp_path, "r")
            timing = max(1, int(handle.read()))
            handle.close()
            timings[step].append(timing)
    return timings


def benchmark_server(
    steps: List[int], repeat: int, client_count: int, network_type: int
) -> Dict[int, List[int]]:
    executable_server_path = "bin/server"
    executable_client_path = "bin/client"
    board_path = "boards/bigun.rle"
    benchmark_temp_path = "benchmarks/temp.csv"
    timings = {}
    for step in steps:
        if step not in timings:
            timings[step] = []
        for _ in range(repeat):
            for i in range(client_count):
                launch_process(
                    [executable_client_path, "-n", str(network_type)], f"client_{i}.log"
                )

            pipe = launch_process(
                [
                    executable_server_path,
                    "-i",
                    board_path,
                    "-r",
                    str(step),
                    "--profile",
                    benchmark_temp_path,
                    "-c",
                    str(client_count),
                    "-n",
                    str(network_type),
                ],
                "server.log",
            )
            pipe.communicate()
            handle = open(benchmark_temp_path, "r")
            timing = max(1, int(handle.read()))
            handle.close()
            timings[step].append(timing)
    return timings


def benchmark_local(steps: List[int], repeat: int) -> Dict[int, List[int]]:
    executable_path = "bin/local"
    board_path = "boards/bigun.rle"
    benchmark_temp_path = "benchmarks/temp.csv"
    timings = {}

    for step in steps:
        if step not in timings:
            timings[step] = []

        for _ in range(repeat):
            pipe = launch_process(
                [
                    executable_path,
                    "-i",
                    board_path,
                    "-r",
                    str(step),
                    "--profile",
                    benchmark_temp_path,
                ],
                "local.log",
            )
            pipe.communicate()
            handle = open(benchmark_temp_path, "r")
            timing = max(1, int(handle.read()))
            handle.close()
            timings[step].append(timing)
    return timings


if __name__ == "__main__":
    logFormatter = logging.Formatter(
        "%(asctime)s [%(threadName)s] [%(levelname)s]:  %(message)s"
    )
    rootLogger = logging.getLogger()
    rootLogger.setLevel(logging.DEBUG)

    consoleHandler = logging.StreamHandler(sys.stdout)
    consoleHandler.setFormatter(logFormatter)
    rootLogger.addHandler(consoleHandler)

    clear_logs()
    logging.info("Log Folder cleared.")

    steps = [1, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
    repeat = 1
    logging.info(f"Steps: {steps} repeated {repeat} times.")

    timings_local = benchmark_local(steps, repeat)
    export(timings_local, "steps_over_time/local.csv")
    logging.info("Local Benchmark done.")

    for i in range(2, 9):
        timings_mpi = benchmark_mpi(steps, repeat, i)
        export(timings_mpi, f"steps_over_time/mpi_{i}.csv")
        logging.info(f"MPI Benchmark with {i} nodes done.")

    for i in range(1, 8):
        timings_server = benchmark_server(steps, repeat, i, 0)
        export(timings_server, f"steps_over_time/server_udp_{i}.csv")
        logging.info(f"UDP Benchmark with {i} clients done.")

    for i in range(1, 8):
        timings_server = benchmark_server(steps, repeat, i, 0)
        export(timings_server, f"steps_over_time/server_tcp_{i}.csv")
        logging.info(f"TCP Benchmark with {i} clients done.")
