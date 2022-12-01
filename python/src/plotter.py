import csv
import glob
import matplotlib.pyplot as plt
import statistics as stats # median function
import numpy as np

def getAllFiles():    
    # All files and directories ending with .txt with depth of 2 folders, ignoring names beginning with a dot:
    return glob.glob("python/src/csv/*/*.csv")


def extractCSV(path):
    file = open(path)
    csvreader = csv.reader(file)   

    header = []
    id = -1
    foundHeader = False

    xValues = []
    yValues = []
    for row in csvreader:
        if not (row[0].startswith("#")):
            if not foundHeader:
                header = row
                foundHeader = True
            else:
                xValues.append(row[0])
                yValues.append(row[1])
    
    if (header[0] == "header"):
        id = int(header[1])
        label = header[2]

    file.close()

    return id, label, xValues, yValues


def makeMedian(xValues, yValues):
    medianX = []
    medianY = [] 

    # append char that marks end of values
    xValues.append('#')
    yValues.append('#')

    currentX = xValues[0]
    currentYValues = []


    for i in range(len(xValues)):

        if currentX == xValues[i]: # needs '#' or last median wont be calculated
            currentYValues.append(int(yValues[i]))
        
        else:
            medianX.append(int(currentX))
            medianY.append(stats.median(currentYValues))

            # update for different xValues
            currentX = xValues[i]
            currentYValues = []
            
    return medianX, medianY



#############################################
#############################################
#############################################

titlesAndAxisLabels = [
    ['Steps over Time', 'steps', 'time in milliseconds'],           # id = 0
    ['Clients over Time', 'clients', 'time in milliseconds']        # id = 1
    # elif id == 2: # TODO erweiterung
]

#############################################
#############################################
#############################################

print("start")


# plots steps over time
SoT = plt.figure('Steps over Time')
plt.title(titlesAndAxisLabels[0][0])
plt.xlabel(titlesAndAxisLabels[0][1])
plt.ylabel(titlesAndAxisLabels[0][2])   
plt.grid(True)



# plots clients over time
CoT = plt.figure('Clients over Time')
plt.title(titlesAndAxisLabels[1][0])
plt.xlabel(titlesAndAxisLabels[1][1])
plt.ylabel(titlesAndAxisLabels[1][2])
plt.grid(True)



# elif id == 2: # TODO erweiterung




for path in getAllFiles():
    id, label, xValues, yValues = extractCSV(path)

    xMedian, yMedian = makeMedian(xValues, yValues)

    if id == 0: # steps over time
        plt.figure(SoT)
        plt.plot(xMedian, yMedian, label=label)
    
    
    
    elif id == 1: # clients over time
        plt.figure(CoT)
        plt.scatter(xMedian, yMedian, label=label)

    # elif id == 2: # TODO erweiterung


# legende setzen
plt.figure(SoT)
plt.legend()


plt.figure(CoT)
plt.legend()

# elif id == 2: # TODO erweiterung



# some important settings so that the graph looks good
axes = plt.figure(CoT).gca() 
# set axis ticks to only have integers
axes.xaxis.get_major_locator().set_params(integer=True) 
#axes.xaxis.get_minor_locator().set_params(integer=True)

# set axis ticks to have more ticks
start, end = axes.get_xlim()
start = 0
axes.xaxis.set_ticks(np.arange(start, end, 1))


# size of plots
figHeight = 7
figWidth = 10
SoT.set_figheight(figHeight)
SoT.set_figwidth(figWidth)
CoT.set_figheight(figHeight)
CoT.set_figwidth(figWidth)


# save plots
SoT.savefig('python/src/fig/steps-over-time.png')
CoT.savefig('python/src/fig/clients-over-time.png')
# elif id == 2: # TODO erweiterung

plt.show()


print("finished")


