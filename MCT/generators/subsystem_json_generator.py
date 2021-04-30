#imports
import glob
import os
import sys
import json
import copy

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

# telemetry file
j = open(telemetryPath, 'r')

# loaded telemetry JSON Data
telemetryData = json.load(j)


def generate():
    '''
    this is the main function that is run to generate the dictoinary file from flow_data.cpp and then convert that into json subsystem/domain object files for every subsystem
    '''
    removeOldJSON()
    createJSON("follower")
    createJSON("leader")

def createFieldList():
    '''
    populates and returns list 'fields' with lists of each telemetry point in file seperated into sections of its state and parents object(s) including subsystem
    
    Example:

    if file contains a.b.c, [a, b, c] wil be added to 'fields'

        Returns:
            fields (list): list of all telemetry points seperated into lists of their state and parents object(s) including subsystem
    '''
    file = open(flowDataPath, 'r')
    fields = []
    lines = file.readlines()
    file.close()
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


# creates a dictionary versin of the list organized in levels (this is the dictionary used by "json_file_generator")

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

            # checks to see if subsystem and then object are alreay in the dictionary
            if telemSubsystem in telem:
                if telemObject in telem[telemSubsystem]:

                    if (telemState in telem[telemSubsystem][telemObject]) == False:
                        telem[telemSubsystem][telemObject][telemState] = "" # finally it checks whether or not the state was added to the dictionary and if it wasn't it adds it using the value of an empty string
                else:
                    # If it doesnt have an object or a state then it first adds an empty object and then 
                    # finally it adds the state to the empty dictionary using the value of an empty string
                    telem[telemSubsystem][telemObject] = {}
                    telem[telemSubsystem][telemObject][telemState] = ""
            else:
                # If it doesnt have a subsystem, or an object, or a state then it first adds an empty subsystem then an empty object and then 
                # finally it adds the state  to the empty dictionary using the value of an empty string
                telem[telemSubsystem] = {}
                telem[telemSubsystem][telemObject] = {}
                telem[telemSubsystem][telemObject][telemState] = ""
        # if the telem point has two elements that means the subsystem of the state is specified but no object.
        elif(len(telemPoint) == 2):
            # initialize the value of the subsystem and state
            telemSubsystem = telemPoint[0]
            telemState = telemPoint[1]

            # checks to see if subsystem and then state are alreay in the dictionary
            if telemSubsystem in telem:
                if (telemState in telem[telemSubsystem]) == False:
                    telem[telemSubsystem][telemState] = ""
            # If it doesnt have a subsystem or a state then it first adds an empty subsystem and then 
                # finally it adds the state to the empty dictionary using the value of an empty string
            else:
                telem[telemSubsystem] = {}
                telem[telemSubsystem][telemState] = ""
        # if the telem point has one element that means neither the subsystem or object is specified.
        elif(len(telemPoint) == 1):
            # initialize of the state
            telemState = telemPoint[0]

            if (telemState in telem) == False: 
                telem[telemState] = "" # finally it adds the state to the empty dictionary using the value of an empty string
    # returns the dictionary
    return telem

def removeOldJSON():
    '''
    removes the old json files before it makes the new versions
    '''
    files = glob.glob(os.path.join(FlightSoftwareDirectory, 'MCT', 'public', 'satellites', '*'))
    for file in files:
        os.remove(file)

def createJSON(satellite):
    '''
    creates all the JSON subsystem/domain object files and puts them in ./public/subsystems
    '''
    # gets the dictionary of fields
    d = createDict(createFieldList())
    sat = {}

    # set key, name, and measurements
    sat['name'] = capFirst(satellite)
    sat['key'] = satellite
    sat['measurements'] = []

    #iterates through each subsystem in the dictionary
    for sub in d:

        #creates subsystem object
        

        # if the subsystem is actually a dictionary
        if isinstance(d[sub], dict):
        
            # iterates through each object in the subsystem dictionary
            for obj in d[sub]:
                #creates container object
                containerObject = {}

                # set key, name, and values;
                containerObject['name'] = satellite + '_' + sub + '.' + obj
                containerObject['key'] = satellite + '_' + sub + '.' + obj
                containerObject['values'] = []

                #add the object to the subsytem
                sat['measurements'].append(containerObject)

                # if the object is actually a dictionary
                if isinstance(d[sub][obj], dict):
                    containerObject['values'] = []
                    # iterates through each state in the object dictionary
                    for state in d[sub][obj]:
                        # get the key value to search the telemetryData json for
                        k = sub + '.' + obj + '.' + state
                        # add this to the container satellite + '_' + object
                        containerObject['values'] = containerObject['values'] + makeValues(k)
                    # creates a timestamp object
                    timestamp = {}
                    timestamp['key'] = 'utc'
                    timestamp['source'] = 'timestamp'
                    timestamp['name'] = 'Timestamp'
                    timestamp['format'] = 'utc'
                    timestamp['hints'] = {}
                    timestamp['hints']['domain'] = 1
                    # adds the timestamp object ot the containerObject with the state Objects
                    containerObject['values'].append(timestamp)
                # if the object is actually a state variable  
                else:
                    # get the key value to search the telemetryData json for
                    k = sub + '.' + obj
                    containerObject['values'] = [] + makeValues(k)
                    timestamp = {}
                    timestamp['key'] = 'utc'
                    timestamp['source'] = 'timestamp'
                    timestamp['name'] = 'Timestamp'
                    timestamp['format'] = 'utc'
                    timestamp['hints'] = {}
                    timestamp['hints']['valuesdomain'] = 1
                    # adds the timestamp object ot the containerObject with the state Objects
                    containerObject['values'].append(timestamp)  
        # if the subsytem is actually a state variable
        else:
            # creates a container Object
            containerObject = {}
            #sets its key, name, and values
            containerObject['name'] = satellite + '_' + sub
            containerObject['key'] = satellite + '_' + sub
            containerObject['values'] = []
            #adds the contianer object to the subsystem object
            
            # get the key value to search the telemetryData json for
            k = sub
            #retrieve the fields from the loaded json
            stateObject = telemetryData['fields'][k]
            # set the key, name, and hints
            stateObject['name'] = capFirst(sub)
            stateObject['key'] = 'value'
            stateObject['hints'] = {}
            stateObject['hints']['range'] = 1
            # add a units option for every type except boolean since boolean doesn't have units
            if stateObject['type'] != 'bool':
                stateObject['units'] = '_____'
            #add the state object to the container object
            containerObject['values'].append(stateObject)
            #creates a timestamp 
            timestamp = {}
            timestamp['key'] = 'utc'
            timestamp['source'] = 'timestamp'
            timestamp['name'] = 'Timestamp'
            timestamp['format'] = 'utc'
            timestamp['hints'] = {}
            timestamp['hints']['domain'] = 1
            #adds the timestamp to the container object
            containerObject['values'].append(timestamp)

            sat['measurements'].append(containerObject)

    # creates a json file called miscellaneous,json in .\MCT\public\subsystems folder and dumps json for subsystem object into it.
    satelliteJSON = open(os.path.join(FlightSoftwareDirectory, 'MCT', 'public', 'satellites', satellite + '.json'), 'w')
    json.dump(sat, satelliteJSON, indent=4)

                
def makeValues(k):
    values = []
    stateData = telemetryData['fields'][k]
    type_k = stateData['type']
    if 'vector' in type_k:
        rawVec = copy.deepcopy(stateData)
        rawVec['name'] = 'rawVec_' + k
        rawVec['key'] = 'rawVec_' + k
        rawVec['hints'] = {}
        rawVec['hints']['range'] = 1
        values.append(rawVec)
        x = copy.deepcopy(stateData)
        x['name'] = 'x_' + k
        x['key'] = 'x_' + k
        x['hints'] = {}
        x['hints']['range'] = 1
        values.append(x)
        y = copy.deepcopy(stateData)
        y['name'] = 'y_' + k
        y['key'] = 'y_' + k
        y['hints'] = {}
        y['hints']['range'] = 1
        values.append(y)
        z = copy.deepcopy(stateData)
        z['name'] = 'z_' + k
        z['key'] = 'z_' + k
        z['hints'] = {}
        z['hints']['range'] = 1
        values.append(z)
    elif 'quaternion' in type_k:
        rawQuat = copy.deepcopy(stateData)
        rawQuat['name'] = 'rawQuat_' + k
        rawQuat['key'] = 'rawQuat_' + k
        rawQuat['hints'] = {}
        rawQuat['hints']['range'] = 1
        values.append(rawQuat)
        a = copy.deepcopy(stateData)
        a['name'] = 'a_' + k
        a['key'] = 'a_' + k
        a['hints'] = {}
        a['hints']['range'] = 1
        values.append(a)
        b = copy.deepcopy(stateData)
        b['name'] = 'b_' + k
        b['key'] = 'b_' + k
        b['hints'] = {}
        b['hints']['range'] = 1
        values.append(b)
        c = copy.deepcopy(stateData)
        c['name'] = 'c_' + k
        c['key'] = 'c_' + k
        c['hints'] = {}
        c['hints']['range'] = 1
        values.append(c)
        d = copy.deepcopy(stateData)
        d['name'] = 'd_' + k
        d['key'] = 'd_' + k
        d['hints'] = {}
        d['hints']['range'] = 1
        values.append(d)
    else:
        state = copy.deepcopy(stateData)
        state['name'] = k
        state['key'] = 'value'
        state['hints'] = {}
        state['hints']['range'] = 1
        # add a units option for every type except boolean since boolean doesn't have units
        if state['type'] != 'bool':
            state['units'] = '_____'
        values.append(state)
    return values

def capFirst(s):
    '''
    makes s's first character uppercase

        Parameters:
            s (str): the string that will be modified
        Returns:
            (str): the modified string
    '''
    return s[0].upper() + s[1:]


# upon running this file with python it will call the main funciton generate()
generate()
print("FlightSoftware/MCT/subsystems/* have been generated")