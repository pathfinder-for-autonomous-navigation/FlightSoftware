from state_variables_generator import createList
from state_variables_generator import list
import os
import glob

# generates the needed subsystem .json files by processing the dictionary created by state_variables_generator
def generate():
    files = glob.glob('./states/*');
    for f in files:
        if(f.find("tempState") == -1 and f.find("timestampState") == -1):
            os.remove(f);
    files = glob.glob('./objects/*');
    for f in files:
        os.remove(f);
    files = glob.glob('./subsystems/*');
    for f in files:
        os.remove(f);
    createList();
    subsystems = list;

    processDictionary(subsystems, 0, "");

# processes the dictionary d creating necessary state, object, and subsystem files to generate the .json
def processDictionary(d, num, start):
    for k,v in d.items():
        print("NUMBER IS: ", num, "\n");
        print("KEY IS: ", k, "\n");
        print("VALUE IS: ", v, "\n");
        if isinstance(v, dict):
            if(num == 0):
                subsystemMaker(start + k);
            else:
                objectMaker(start+k,num)
            processDictionary(v, num +1,start+k+'.');
            parentSubsystem = start[:len(start)-1];
            if(num != 0):
                f = open(".\\objects\\" + start + k + ".json", "r");
                fLines = f.readlines();
                f.close();
                insertButAddOn(".\\states\\timestampState.json", ".\\objects\\" + start + k + ".json", len(fLines) - 2)
                f = open(".\\subsystems\\" + parentSubsystem + ".json", "r");
                fLines = f.readlines();
                f.close();
                if(fLines[len(fLines)-3].find("[") != -1):
                    insertFile(".\\objects\\" + start+k + ".json", ".\\subsystems\\" + parentSubsystem + ".json", len(fLines) - 2);
                else:
                    insertButAddOn(".\\objects\\" + start+k + ".json", ".\\subsystems\\" + parentSubsystem + ".json", len(fLines) - 2);
        else:

            parentObject = start[:len(start)-1];
            level = "";
            if(num == 1):
                level = "objects";
                objectMaker(start+k,num)
                stateMaker(start+k, num+1);
                parentObject = start+k;
                parentSubsystem = start[:len(start)-1];
                f = open(".\\" + level + "\\" + parentObject + ".json", "r");
                fLines = f.readlines();
                f.close();
                if(fLines[len(fLines)-3].find("[") != -1):
                    insertFile(".\\states\\" + start+k + ".json", ".\\" + level + "\\" + parentObject + ".json", len(fLines) - 2);
                else:
                    insertButAddOn(".\\states\\" + start+k + ".json", ".\\" + level + "\\" + parentObject + ".json", len(fLines) - 2);
                f = open(".\\" + level + "\\" + parentObject + ".json", "r");
                fLines = f.readlines();
                f.close();
                insertButAddOn(".\\states\\timestampState.json", ".\\" + level + "\\" + parentObject + ".json", len(fLines) - 2)
                level = "subsystems";
                f = open(".\\" + level + "\\" + parentSubsystem + ".json", "r");
                fLines = f.readlines();
                f.close();
                if(fLines[len(fLines)-3].find("[") != -1):
                    insertFile(".\\objects\\" + start+k + ".json", ".\\" + level + "\\" + parentSubsystem + ".json", len(fLines) - 2);
                else:
                    insertButAddOn(".\\objects\\" + start+k + ".json", ".\\" + level + "\\" + parentSubsystem + ".json", len(fLines) - 2);
            else:
                level = "objects";
                stateMaker(start+k, num);
                parentObject = start[:len(start)-1];
                f = open(".\\" + level + "\\" + parentObject + ".json", "r");
                fLines = f.readlines();
                f.close();
                if(fLines[len(fLines)-3].find("[") != -1):
                    insertFile(".\\states\\" + start+k + ".json", ".\\" + level + "\\" + parentObject + ".json", len(fLines) - 2);
                else:
                    insertButAddOn(".\\states\\" + start+k + ".json", ".\\" + level + "\\" + parentObject + ".json", len(fLines) - 2);
            
# creates a state file for the given state k at the depth level of num
def stateMaker(k, num):
    telemetry = open("telemetry.txt", "r");
    indentation = "";
    for i in range(2* num):
        indentation = indentation + "\t";
    f = open(".\\states\\" + k + ".json", "x");
    f = open(".\\states\\" + k + ".json", "a");
    f.write(indentation + "{\n");
    findState(k, indentation);
    state = open(".\\states\\tempState.json", "r");
    stateLines =state.readlines();
    stateName = k[k.rfind(".")+1:];
    f.write(indentation + "\t\"name\": \"" + capFirst(stateName) + "\",\n");
    f.write(indentation + "\t\"key\": \"" + stateName + "\",\n");
    f.write(indentation + "\t\"hints\": {\n");
    f.write(indentation + "\t\t\"range\": \"1\"\n");
    f.write(indentation + "\t}");
    if(stateLines[-2].find("bool") == -1):
        f.write(",\n");
        f.write(indentation + "\t\"units\": \"_________\"\n");
    else:
        f.write("\n");
    f.write(indentation + "}\n");
    f.close();
    insertFile(".\\states\\tempState.json", ".\\states\\" + k + ".json", 1);

# creates a object file for the given state k at the depth level of num
def objectMaker(k, num):
    indentation = "";
    for i in range(2* num):
        indentation = indentation + "\t";
    f = open(".\\objects\\" + k+ ".json","x");
    f = open(".\\objects\\" + k+ ".json","a");
    f.write(indentation + "{\n");
    f.write(indentation + "\t\"name\": \"" + periodRemovedCapWords(k) + "\",\n");
    f.write(indentation + "\t\"key\": \"" + k + "\",\n");
    f.write(indentation + "\t\"values\": [\n");
    f.write(indentation + "\t]\n");
    f.write(indentation + "}\n")

# creates a subsystem file for the given subsystem k - These are the desired jsonfile and will be stored in './subsystems/'
def subsystemMaker(k):
    f = open(".\\subsystems\\" + k + ".json", "x");
    f = open (".\\subsystems\\" + k + ".json", "a");
    f.write("{\n");
    f.write("\t\"name\": \"" + capFirst(k) + "\",\n");
    f.write("\t\"key\": \"" + k + "\",\n");
    f.write("\t\"measurements\": [\n");
    f.write("\t]\n");
    f.write("}")


# Inserts all lines of file fL1 in file fL2 at line position p
def insertFile(fL1, fL2, p):
    f1 = open(fL1, "r");
    f2 = open(fL2, "r");
    f2Lines = f2.readlines();
    firstSection = f2Lines[0:p];
    secondSection = f2Lines[p:];
    f2 = open(fL2, "w");
    f2.write(firstSection[0]);
    f2 = open(fL2, "a");
    for line in firstSection[1:]:
        f2.write(line);
    f1Lines = f1.readlines();
    for line in f1Lines:
        f2.write(line);
    for line in secondSection:
        f2.write(line);

# Inserts all lines of file fL1 in file fL2 at line position p but also adds it as a new argument in the json by adding a comma
def insertButAddOn(fL1, fL2,num):
    f = open(fL2, "r");
    fLines = f.readlines();
    firstSection = fLines[:num-1];
    changedLine = fLines[num-1].strip("\n") + ",\n";
    secondSection = fLines[num:];
    f = open(fL2, "w");
    f.write(firstSection[0]);
    f = open(fL2, "a");
    for line in firstSection[1:]:
        f.write(line);
    f.write(changedLine);
    for line in secondSection:
        f.write(line);
    f.close();
    insertFile(fL1,fL2, num);

# Finds the state k's data in "telemetry.txt" and store this in "tempState.json"
def findState(k, indent):
    f = open("telemetry.txt", "r");
    fLines = f.readlines();
    i = 0;
    iFound = 0;
    for x in fLines:

        if(x.find(k) != -1 and iFound == 0):
            iFound = 1;
        if(iFound == 0):
            i = i+1;
    if(iFound == 1):
        remaining = fLines[i+1:];
        iFound = 0;
        i = 0
        for lines in remaining:
            if(lines.find("}") != -1 and iFound == 0):
                iFound = 1;
            if(iFound == 0):
                i = i+1;

        remaining = remaining[:i];
        f = open(".\\states\\tempState.json", "w");
        f.write(indent + "\t" + remaining[0].lstrip());
        f = open(".\\states\\tempState.json", "a");
        for xline in remaining[1:len(remaining)-1]:
            f.write(indent + "\t" + xline.lstrip());
        finalLine = indent + "\t" + remaining[len(remaining)-1].lstrip().strip("\n") + ",\n";
        f.write(finalLine);

# makes the periods of str s spaces and capitalizes the first char of s to make the "Name" variable in the json
def periodRemovedCapWords(s):
    i = s.find(".");
    newS = capFirst(s[0:i]) + " " + capFirst(s[i+1:]);
    return newS;

# capitalizes the first char of the str s
def capFirst(s):
    first = s[0];
    first = first.upper();
    return first + s[1:];





generate();

