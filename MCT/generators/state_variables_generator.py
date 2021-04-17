#imports
import os
import sys

# path to the FlightSoftware folder 
FlightSoftwareDirectory = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]

# path to the state-variable.js file
stateVariablesPath = os.path.join(FlightSoftwareDirectory, 'MCT', 'server-files', 'state-variables.js')

# path to the flow_data.cpp file
flowDataPath = os.path.join(FlightSoftwareDirectory, 'src', 'flow_data.cpp')

# path to the telemetry file
telemetryPath = os.path.join(FlightSoftwareDirectory, 'telemetry')

#the initial stdout before changing it to a file
original_stdout = sys.stdout

# flow_data.cpp file
f = open(flowDataPath, 'r')


def generate():
    '''
    this is the main function that is run to generate the state-variables.js file from flow_data.cpp
    '''
    writeStateVariables(createDict(createFieldList(f)))

def createFieldList(file):
    '''
    populates and returns list 'fields' with lists of each telemetry point in file seperated into sections of its state and parents object(s) including subsystem
    
    Example:

    if file contains a.b.c, [a, b, c] wil be added to 'fields'

        Parameters: 
            file (File): The flow_data.cpp file that will be scanned

        Returns:
            fields (list): list of all telemetry points seperated into lists of their state and parents object(s) including subsystem
    '''
    
    fields = []
    lines = file.readlines()

    # ignores all the lines before the line the variable is declared in when searching for field matches
    if lines[0].startswith("const"):
        canStartSearching = True
    else:
        canStartSearching = False
    
    # iterates through every line - first checking to see if the line with "const" has been reached yet
    # if const has it begins to check for fields surrounded by quatation marks which are then added to 'fields'
    for line in lines:

        # once it is able to start its search it enters this if statement
        if canStartSearching == True:

            # it finds all the parenthesis pairs in the line
            quotationMarkPresent = False
            index = line.find("\"")
            if index != -1:
                quotationMarkPresent = True
            while quotationMarkPresent == True:
                line = line[index+1:]
                index = line.find("\"")
                if index != -1:
                    telemetryPoint = line[:index]
                    # it stores the telemetry point split around the 2 initial periods (to indicate a maximum of a subsystem, object, and state value) then adds this to fields
                    fields.append(telemetryPoint.split(".", 2))
                    line = line[index+1:]

                # checks before exiting each iteration of loop to see if there is still a quotation mark or if the loop can terminate it
                index = line.find("\"")
                if index == -1:
                    quotationMarkPresent = False

        else:# goes in here when it cant start counting yet

            if line.startswith("const"): # if "const" is sean it may start looking for fields
                canStartSearching = True


    # returns fields
    return fields


def createDict(l):
    '''
    populates and returns dictionary 'telem' organized by the object stucture outlined in list l

        Parameters:
            l (list): List of telemetry keys seperated around first 2 periods
        Returns:
            telem (dict): dictionary interpretation of list l organized in levels designated in each point of list l
    '''
    
    # the dictionary object that will be returned
    telem = {}

    # iterate through every telemetry point laid out in the list
    for telemPoint in l:

        # if the telem point has three elements that means the object and subsystem of the state is specified.
        if(len(telemPoint) == 3):

            # initialize the value of the subsystem, object, and state
            telemSubsystem = telemPoint[0]
            telemObject = telemPoint[1]
            telemState = telemPoint[2]
            # retrieve the value of the telem point from initialTelemValue()
            telemValue = initialTelemValue(telemSubsystem + '.' + telemObject + '.' + telemState)
            
            # checks to see if subsystem and then object are alreay in the dictionary
            if telemSubsystem in telem:
                if telemObject in telem[telemSubsystem]:
                    if (telemState in telem[telemSubsystem][telemObject]) == False: # finally it checks whether or not the state was added to the dictionary and if it wasn't it adds it using the value initialized earlier
                        telem[telemSubsystem][telemObject][telemState] = telemValue
                else:
                    # If it doesnt have an object or a state then it first adds an empty object and then 
                    # finally it adds the state to the empty dictionary using the value initialized earlier
                    telem[telemSubsystem][telemObject] = {}
                    telem[telemSubsystem][telemObject][telemState] = telemValue
            else:
                # If it doesnt have a subsystem, or an object, or a state then it first adds an empty subsystem then an empty object and then 
                # finally it adds the state  to the empty dictionary using the value initialized earlier
                telem[telemSubsystem] = {}
                telem[telemSubsystem][telemObject] = {}
                telem[telemSubsystem][telemObject][telemState] = telemValue
        # if the telem point has two elements that means the subsystem of the state is specified but no object.
        elif(len(telemPoint) == 2):
            # initialize the value of the subsystem and state
            telemSubsystem = telemPoint[0]
            telemState = telemPoint[1]
            # retrieve the value of the telem point from initialTelemValue()
            telemValue = initialTelemValue(telemSubsystem + '.' + telemState)
            
            # checks to see if subsystem and then state are alreay in the dictoinary
            if telemSubsystem in telem:
                if (telemState in telem[telemSubsystem]) == False: # finally it checks whether or not the state was added to the dictionary and if it wasn't it adds it using the value initialized earlier
                    telem[telemSubsystem][telemState] = telemValue
            else:
                # If it doesnt have a subsystem or a state then it first adds an empty subsystem and then 
                # finally it adds the state to the empty dictionary using the value initialized earlier
                telem[telemSubsystem] = {}
                telem[telemSubsystem][telemState] = telemValue
        # if the telem point has one element that means neither the subsystem or object is specified.
        elif(len(telemPoint) == 1):
            # initialize of the state
            telemState = telemPoint[0]
            # retrieve the value of the telem point from initialTelemValue()
            telemValue = initialTelemValue(telemState)
            

            if (telemState in telem) == False: # finally it adds the state to the empty dictionary using the value initialized earlier
                telem[telemState] = telemValue
    # returns the dictionary
    return telem

def initialTelemValue(state):
    '''
    returns the initial value for a specific piece of state telemetry based on its type

        Parameters: 
            state (str): the key for a telemetry state
        Returns: the initial value of that telemetry based on its type
    '''

    # opens telemetry
    t = open(telemetryPath, 'r')

    # a data structure holding all the coorisponding initial values for each type
    initialValues = {
        'unsigned char': 0, 
        'bool': 'false', 
        'float': 0.0, 
        'std float vector': 0.0, 
        'unsigned int': 0, 
        'lin float vector': 0.0, 
        'lin float quaternion': '', 
        'double': 0.0, 
        'signed int': 0, 
        'lin double vector': 0.0, 
        'std double vector': 0.0, 
        'gps_time_t': ''
    }
    # if the type is not in this list return an empty string
    defaultValue = ''

    lines = t.readlines()
    t.close()

    # first look for the state in all the lines
    stateFound = False
    for line in lines:
        if stateFound  == False:
            if line.find(state) != -1 and line.find('{') != -1:
                stateFound = True

        # once found, look for type field
        else:
            if line.find('type') !=-1:
                index = line.rfind("\"") # extract the type from the line
                type = line[:index]
                index = type.rfind("\"")
                type = type[index+1:]
                for key in initialValues:
                    if type == key:
                        return initialValues[key] # if the type from telemetry matches a type in initialValues it will return that cooresponding initial value
    
    # returns the initial values for the state
    return defaultValue


def writeStateVariables(d):
    '''
    generates the state-variables.js file
        Parameters:
            d (dict): The dictionary containing the fields and their initial values
    '''

    #removes the old state-variables file and creates a new one
    os.remove(stateVariablesPath)
    sv = open(stateVariablesPath, "x")

    #sets the stdout to the state-variables file, prints out the dictionary, and then makes the stdout the original again
    sys.stdout = sv
    print(d)
    sys.stdout = original_stdout
    sv.close()

    #reads the lines
    sv = open(stateVariablesPath, "r")
    line = sv.readlines()[0]
    sv.close()

    #for every character in the line, it removes any single quotes to make it into a js object literal form
    for i in range(len(line)):
        # it double checks that it isnt deleting any bordering single quotes which are used for actual values in this program. It also makes sure that it doesn't have any index out of bounds exceptions
        if(i==0):
            if(line[i] == '\'' ) and (line[i+1] != '\''):
                line = actionAtIndex(line, '\'', i, 'rm')
        elif(i>0 and i<len(line)-1):
            if(line[i] == '\'' ) and (line[i-1] != '\'' and line[i+1] != '\''):
                line = actionAtIndex(line, '\'', i, 'rm')
        elif(i==len(line)-1):
            if(line[i] == '\'' ) and (line[i-1] != '\''):
                line = actionAtIndex(line, '\'', i, 'rm')

    # rewrites the new revised line
    sv = open(stateVariablesPath, 'w')
    sv.write(line)
    sv.close()

    # starts to read the line again
    sv = open(stateVariablesPath, 'r')
    line = sv.readlines()[0]
    processedLine = ""

    # adds a new line after every '{' to make it more readable
    formattingIncomplete = False
    if(line.find('{') != -1):
        formattingIncomplete = True
    while formattingIncomplete: # the while loop iterates until their is no open braces left
        index = line.find('{')
        if(index != -1):
            processedLine = processedLine + line[:index+1] + '\n' #transfers line over to processedLine piece by piece until line is an empty string and processedLine is processed with the formatting
            line = line[index+1:]
        else:
            processedLine = processedLine + line
            line = ""
            formattingIncomplete = False

    # adds a new line after every ',' to make it more readable and 2 new lines if there is a open parenthesis after it
    formattingIncomplete = False
    if(processedLine.find(',') != -1):
        formattingIncomplete = True
    while formattingIncomplete: # the while loop iterates until their is no open braces left
        index = processedLine.find(',')
        if(index != -1):
            #checks to see if there is a open brace and then if there is an open brace sooner than a comma 
            if(processedLine[index+1:].find('{') != -1 and processedLine[index+1:].find('{') <  processedLine[index+1:].find(',')): 
                line = line + processedLine[:index+1].lstrip() + '\n\n' # adds two new lines if the brace is a lower index than the next comma
            else:
                line = line + processedLine[:index+1].lstrip() + '\n' # adds one new line if the brace is not a lower index than the next comma
            # they build back from processedLine onto line until processed line is an empty string and line is processed with the correct formatting
            
            processedLine = processedLine[index+1:].lstrip()
        else:
            line = line + processedLine.lstrip()
            processedLine = ""
            formattingIncomplete = False
    

    # writing the final results
    sv = open(stateVariablesPath, 'w')
    sv.write("//This file was AUTOGENERATED from the script '/state_variables_generator.py' and represents all the initial values of states in OpenMCT\n")
    sv.close()
    sv = open(stateVariablesPath, 'a')
    sv.write('module.exports = ')
    sv.write(line)


def actionAtIndex(s1, s2, i, action):
    '''
    does an action to a string (s1) at a specific index (i) with the string (s2). 
    
    The two actions allowed are:
        'add' - adds the string s2 to s1 at index i
        'rm' - removes the string s2 from s1 at index i if it exists in this position
    
        Parameters:
            s1 (string): string that is losing or gainging
            s2 (string): string that is being added or removed
            i (int): the integer of the index of action
            action (string): a limited option string which can either be 'add' or 'rm indicating whether the function needs to add or remove to the string
    '''
    if action == 'add': # if action is 'add' then it will add s2 to s1 at position i
        return s1[:i] + s2 + s1[i:]
    if action == 'rm': # if action is 'rm' then it will remove s2 from s1 at position i
        if s1[i:i+len(s2)] == s2: # only if it exists at that index however
            return s1[:i]+s1[i+len(s2):]
        else:
            return s1
    
# upon running this file with python it will call the main funciton generate()
generate()
print("FlightSoftware/MCT/server-files/state-variables.js has been generated")
