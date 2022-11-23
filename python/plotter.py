import csv
import matplotlib

def extractCSV():
    fileStepsTime = open('csv/steps-time.csv')
    csvreader = csv.reader(fileStepsTime)

    headerStepsTime = []
    headerStepsTime = next(csvreader)    

    rowsStepsTime = []
    for row in csvreader:
        rowsStepsTime.append(row[1])

    fileStepsTime.close()
    #########################
    fileClientsTime = open('csv/clients-time.csv')
    csvreader = csv.reader(fileClientsTime)

    headerClientsTime = []
    headerClientsTime = next(csvreader)

    rowsClientsTime = []
    for row in csvreader:
        row = row[1:]
        rowsClientsTime.append(row)

    fileClientsTime.close()
    #########################

    return headerStepsTime, rowsStepsTime, headerClientsTime, rowsClientsTime


#############################################
#############################################
#############################################
print("start")


# headerImages          = number of images and amount of columns=2
# rowsImages            = classes
# headerEdgeHistogram   = number of images and number of their features
# rowsEdgeHistogram     = all features
headerImages, rowsImages, headerEdgeHistogram, rowsEdgeHistogram = extractCSV()


#fig, ax = plt.subplots(figsize=(10,7))
#ax.plot(t,v,lw=4)
#ax.set_xlabel('Time [s]',fontsize=14)
#ax.set_ylabel('Velocity [$m/s$]',fontsize=14)
#ax.set_title('Experiment 1',fontsize=14)



print("finished")


