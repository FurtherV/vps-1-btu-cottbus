import argparse
import logging
import sys
from typing import List, Dict, Tuple
import numpy as np
import pathlib
import matplotlib
import matplotlib.pyplot as plt

HEADER_PREFIX = "HEADER"


def parse_header(header_text: str) -> Tuple[str, int]:
    elements = header_text.split(",")
    type = elements[0]
    nodes = 1
    if len(elements) > 1:
        nodes = elements[1]
    return type, int(nodes)


def import_data(csv_file: str) -> Tuple[str, Dict[int, List[int]]]:
    file_descriptor = open(csv_file, "r")
    lines = file_descriptor.read().splitlines()
    file_descriptor.close()

    # remove comment lines
    lines = [x for x in lines if not x.startswith("#")]

    if len(lines) <= 1:
        return "EMPTY", {}

    # read header line
    header = [x for x in lines if x.lower().startswith(HEADER_PREFIX.lower())][0]
    header = header[(len(HEADER_PREFIX) + 1) :]

    # parse data
    data = {}
    csv_rows = np.loadtxt(lines[1:], dtype=int, delimiter=",", ndmin=2)
    for row in csv_rows:
        if len(row) <= 1:
            continue

        index = row[0]
        if index not in data:
            data[index] = []
        data[index].extend(row[1:])

    return header, data


def plot_steps_time_seperate(data: Dict, output_folder: str):
    for type in data.keys():
        fig, ax = plt.subplots()
        for nodes in data[type].keys():
            predicate = lambda x: x % 5 == 0
            x_values = [float(x) for x in data[type][nodes].keys() if predicate(x)]
            y_values = [
                float(np.median(data[type][nodes][x])) for x in x_values if predicate(x)
            ]
            ax.plot(x_values, y_values, marker="o", label=f"Nodes: {nodes}")
            ax.set_xticks(np.linspace(0, max(x_values), len(x_values) + 1))
            ax.set_xlabel("Steps")
            ax.set_ylabel("Time in milliseconds")
            ax.grid()
        fig.legend()

        fig.suptitle(f"Steps x Time with {type}")

        # increase figure size
        fig_width, fig_height = fig.get_size_inches()
        fig.set_size_inches(fig_width * 2, fig_height * 2)

        file_name = "steps_over_time-" + str(type).lower().replace(" ", "_") + ".pdf"
        fig.savefig(output_folder + "/" + file_name)
    return None


def plot_steps_time_combined(data: Dict, output_folder: str):
    ncols = 2
    nrows = 2
    fig, axs = plt.subplots(ncols=ncols, nrows=nrows, sharex=True, sharey=True)
    types = list(data.keys())
    for row in range(0, nrows):
        for col in range(0, ncols):
            type = types[row * ncols + col]
            ax = axs[row][col]
            ax.set_title(type, loc="left")
            for nodes in data[type].keys():
                predicate = lambda x: x % 5 == 0
                x_values = [float(x) for x in data[type][nodes].keys() if predicate(x)]
                y_values = [
                    float(np.median(data[type][nodes][x]))
                    for x in x_values
                    if predicate(x)
                ]
                ax.plot(x_values, y_values, marker="o", label=f"Nodes: {nodes}")
                ax.set_xticks(np.linspace(0, max(x_values), len(x_values) + 1))
                ax.set_xlabel("Steps")
                ax.set_ylabel("Time in milliseconds")
                ax.grid()
                ax.legend()

    fig.suptitle("Steps x Time (Combined)")

    # increase figure size
    fig_width, fig_height = fig.get_size_inches()
    fig.set_size_inches(fig_width * 2, fig_height * 2)

    file_name = "steps_over_time-" + "combined".lower().replace(" ", "_") + ".pdf"
    fig.savefig(output_folder + "/" + file_name)
    return None


def plot_clients_time_combined(data: Dict, output_folder: str):
    fig, ax = plt.subplots()
    for type in data.keys():
        x_values = list(data[type].keys())
        max_step_count = max(list(data[type][x_values[0]].keys()))
        y_values = [
            np.median(data[type][node_count][max_step_count]) for node_count in x_values
        ]
        ax.plot(x_values, y_values, marker="o", label=f"{type}")
        ax.set_xticks(np.linspace(0, max(x_values), len(x_values) + 1))
        ax.set_xlabel("Steps")
        ax.set_ylabel("Time in milliseconds")
        ax.grid(True)
        ax.legend()
        fig.suptitle(f"Clients x Time with {max_step_count} steps")

    # increase figure size
    fig_width, fig_height = fig.get_size_inches()
    fig.set_size_inches(fig_width * 2, fig_height * 2)

    file_name = "clients_over_time-" + "combined".lower().replace(" ", "_") + ".pdf"
    fig.savefig(output_folder + "/" + file_name)
    return None


if __name__ == "__main__":
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument(
        "--input", nargs="+", type=str, help="Input files and folders", required=True
    )
    parser.add_argument("--output", type=str, help="Output folder", required=True)
    parser.add_argument("--show", action="store_true", help="Output folder")
    args = parser.parse_args()

    logFormatter = logging.Formatter(
        "%(asctime)s [%(threadName)s] [%(levelname)s]:  %(message)s"
    )
    rootLogger = logging.getLogger()
    rootLogger.setLevel(logging.DEBUG)

    consoleHandler = logging.StreamHandler(sys.stdout)
    consoleHandler.setFormatter(logFormatter)
    rootLogger.addHandler(consoleHandler)

    input_files = []
    for arg in args.input:
        input_files.extend(str(x) for x in pathlib.Path(".").rglob(arg))

    output_folder = args.output

    logging.info(f"Plotting files: {input_files}")
    logging.info(f"Output will be placed in: {output_folder}")

    data_storage: Dict[str, Dict[int, Dict[int, List[int]]]] = {}
    for input_file in input_files:
        header, data = import_data(input_file)
        type, nodes = parse_header(header)
        if type not in data_storage:
            data_storage[type] = {}
        if nodes not in data_storage[type]:
            data_storage[type][nodes] = {}
        data_storage[type][nodes] = data

    plot_steps_time_seperate(data_storage, output_folder)
    plot_steps_time_combined(data_storage, output_folder)
    plot_clients_time_combined(data_storage, output_folder)

    if args.show:
        plt.show()
