import csv
import glob
import matplotlib
import matplotlib.pyplot as plt
import numpy as np


def getAllFiles():
    # All files and directories ending with .txt with depth of 2 folders, ignoring names beginning with a dot:
    return glob.glob("python/src/csv/*/*.csv")


def extractCSV(path):
    handle = open(path)
    lines = [x for x in handle.read().splitlines() if not x.startswith("#")]

    header_line = lines[0]
    header_entries = header_line.split(",")
    id = int(header_entries[1])
    label = str(header_entries[2])

    data = np.loadtxt(lines, delimiter=",", dtype=int, skiprows=1)
    xValues = []
    yValues = []
    for row in data:
        if len(row) > 1:
            for column in row[1:]:
                xValues.append(row[0])
                yValues.append(column)
    return id, label, xValues, yValues


def makeMedian(xValues, yValues):
    value_map = {}
    for i in range(min(len(xValues), len(yValues))):
        x_value = float(xValues[i])
        y_value = float(yValues[i])
        if not x_value in value_map:
            value_map[x_value] = []
        value_map[x_value].append(y_value)

    for x in value_map.keys():
        values = list(value_map[x])
        values.sort()
        median = np.median(values)
        value_map[x] = median

    return list(value_map.keys()), list(value_map.values())


#############################################
#############################################
#############################################

titlesAndAxisLabels = [
    ["Steps over Time", "steps", "time in milliseconds"],  # id = 0
    ["Clients over Time", "clients", "time in milliseconds"]  # id = 1
    # elif id == 2: # TODO erweiterung
]

#############################################
#############################################
#############################################

print("start")
print(matplotlib.get_backend())

# plots steps over time
SoT = plt.figure("Steps over Time")
plt.title(titlesAndAxisLabels[0][0])
plt.xlabel(titlesAndAxisLabels[0][1])
plt.ylabel(titlesAndAxisLabels[0][2])
plt.grid(True)

# plots clients over time
CoT = plt.figure("Clients over Time")
plt.title(titlesAndAxisLabels[1][0])
plt.xlabel(titlesAndAxisLabels[1][1])
plt.ylabel(titlesAndAxisLabels[1][2])
plt.grid(True)

# elif id == 2: # TODO erweiterung

SoT_xTicks = []

for path in getAllFiles():
    print(f"plotting file {path}")
    id, label, xValues, yValues = extractCSV(path)
    xMedian, yMedian = makeMedian(xValues, yValues)

    if id == 0:  # steps over time
        plt.figure(SoT)
        plt.plot(xMedian, yMedian, marker="o", label=label)
        SoT_xTicks.extend(xMedian)

    elif id == 1:  # clients over time
        plt.figure(CoT)
        plt.plot(xMedian, yMedian, marker="o", label=label)

    # elif id == 2: # TODO erweiterung

print("plotting done.")

# SoT xTicks
SoT_xTicks = list(set(SoT_xTicks))
SoT_xTicks.sort()
plt.xticks(SoT_xTicks)

# legende setzen
plt.figure(SoT)
handles, labels = plt.gca().get_legend_handles_labels()
labels_to_handles = dict(zip(labels, handles))
labels.sort()
print(labels)
plt.legend([labels_to_handles[x] for x in labels], labels)

plt.figure(CoT)
plt.legend()

# elif id == 2: # TODO erweiterung


# some important settings so that the graph looks good
axes = plt.figure(CoT).gca()
# set axis ticks to only have integers
axes.xaxis.get_major_locator().set_params(integer=True)
# axes.xaxis.get_minor_locator().set_params(integer=True)

# set axis ticks to have more ticks
start, end = axes.get_xlim()
start = 0
axes.xaxis.set_ticks(np.arange(start, end, 1))


# size of plots
figHeight = 9
figWidth = 16
SoT.set_figheight(figHeight)
SoT.set_figwidth(figWidth)
CoT.set_figheight(figHeight)
CoT.set_figwidth(figWidth)

# save plots
SoT.savefig("python/src/fig/steps-over-time.pdf")
CoT.savefig("python/src/fig/clients-over-time.pdf")
# elif id == 2: # TODO erweiterung

plt.show()


print("finished")
