import csv
import matplotlib.pyplot as plt
import statistics as stats # median function

def extractCSV(path):
    file = open(path)
    csvreader = csv.reader(file)   

    xValues = []
    yValues = []
    for row in csvreader:
        if not (row[0].startswith("#")):
            xValues.append(row[0])
            yValues.append(row[1])

    file.close()
    

    return xValues, yValues


def makeMedian(xValues, yValues):
    medianX = []
    medianY = []

    # append char that marks end of values
    xValues.append('#')
    yValues.append('#')

    currentX = '0'
    currentYValues = [int(0)]


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

stepsOverTimePath = [
    'csv/steps-over-time/steps_over_time_local-100-steps.csv',
    'csv/steps-over-time/steps_over_time_server-100-steps-01-clients-tcp.csv',
    'csv/steps-over-time/steps_over_time_server-100-steps-01-clients-udp.csv',
    'csv/steps-over-time/steps_over_time_server-100-steps-10-clients-tcp.csv',
    'csv/steps-over-time/steps_over_time_server-100-steps-10-clients-udp.csv',
    'csv/steps-over-time/steps_over_time_server-100-steps-20-clients-tcp.csv',
    'csv/steps-over-time/steps_over_time_server-100-steps-20-clients-udp.csv',
]

stepsOverTimeLabel = [
    'Local',
    '1 Client TCP',
    '1 Client UDP',
    '10 Client TCP',
    '10 Client UDP',
    '20 Client TCP',
    '20 Client UDP',
]


clientsOverTimePath = [ 
    'csv/clients-over-time/clients_over_time_server-20-steps-1to20-clients-tcp.csv',
    'csv/clients-over-time/clients_over_time_server-20-steps-1to20-clients-udp.csv'
]

clientsOverTimeLabel = [
    '20 Steps TCP',
    '20 Steps UDP'
]


graphLabels = [['steps'  , 'time in milliseconds'],
               ['clients', 'time in milliseconds']]


#############################################
#############################################
#############################################

print("start")


# plots steps over time
SoT = plt.figure('Steps over Time')
plt.figure(SoT)

for i in range(len(stepsOverTimePath)):
    xValues, yValues = extractCSV(stepsOverTimePath[i])

    xMedian, yMedian = makeMedian(xValues, yValues)

    plt.plot(xMedian, yMedian, label=stepsOverTimeLabel[i])
    #plt.scatter(xMedian,yMedian)

plt.xlabel(graphLabels[0][0])
plt.ylabel(graphLabels[0][1])    
plt.title('Steps over Time')
plt.legend()
SoT.savefig('fig/steps-over-time.png')


# plots clients over time
CoT = plt.figure('Clients over Time')
plt.figure(CoT)

for i in range(len(clientsOverTimePath)):
    xValues, yValues = extractCSV(clientsOverTimePath[i])

    xMedian, yMedian = makeMedian(xValues, yValues)

    plt.plot(xMedian, yMedian, label=clientsOverTimeLabel[i])
    #plt.scatter(xMedian,yMedian)

plt.xlabel(graphLabels[1][0])
plt.ylabel(graphLabels[1][1])    
plt.title('Clients over Time')
plt.legend()
plt.show()
CoT.savefig('fig/clients-over-time.png')

print("finished")


