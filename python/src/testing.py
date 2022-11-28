import glob

def getAllFiles():    
    # All files and directories ending with .txt with depth of 2 folders, ignoring names beginning with a dot:
    return glob.glob("python/src/csv/*/*.csv")    