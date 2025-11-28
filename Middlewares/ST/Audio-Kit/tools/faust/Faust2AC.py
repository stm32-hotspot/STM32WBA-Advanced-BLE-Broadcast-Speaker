##############################################################################
# @file    Faust2AC.py
# @author  MCD Application Team
# @brief   Convert a  faust .C to AC implementation 
##############################################################################
# @attention
#
# <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
# All rights reserved.</center></h2>
#
# This software component is licensed by ST under Ultimate Liberty license
# SLA0044, the "License"; You may not use this file except in compliance with
# the License. You may obtain a copy of the License at:
#                             www.st.com/SLA0044
#
##


import sys
import argparse
import os
import shutil
import json

FunctionName = str("Volume")    # AC module name 
GroupName    = str("Sound")     # AC group folder
Skip         = False            # True if we skip cleanup generated
varNameSubstitute = []
tJsonCard =[]
basePath    = "generated/"
addAlgoName = ""

def findVariable(varlist,name):
    for i in range(0,len(varlist)):
        key    = list(varlist)[i]
        if "name" in  varlist[key]:
            varName = varlist[key]["name"]
            if varName == name:
                return i
    return -1

def renameOpt(name):
    global varNameSubstitute
    for i in  range(0,len(varNameSubstitute)) :
        split =varNameSubstitute[i].split(":")
        if split[0] == name :
            return split[1]

    return name

def applyRename(varList):
    for i in range(0,len(varList)):
        var = varList[list(varList)[i]]
        var["name"] = renameOpt(var["name"])


def checkAndFixName(varlist,name):
    match = name.find(":")
    if match != -1:
        name = name[:match]
    name = GetStdCName(name)
    match = findVariable(varlist, name)
    if match == -1:
        return name
    index = 0
    while True:
        newName = name+"_"+str(index)
        match = findVariable(varlist, newName)
        if match == -1:
            break
        index = index+1
    return newName




# Fix a name to be compatible with a C variable 
def GetStdCName(name):
    name = name.lower()
    stdname = ""
    for i in range(0, len(name)):
        if name[i] >= '0' and  name[i] <= '9':
            stdname +=name[i]
            continue
        if name[i] >= 'a' and  name[i] <= 'z':
            stdname +=name[i]
            continue
        if name[i] >= 'A' and  name[i] <= 'Z':
            stdname +=name[i]
            continue
        stdname +='_'
    return stdname


# extract a Faust float in regular float string 
def stripValue(value):
    value = value.lstrip(' ')
    value = value.lstrip('(FAUSTFLOAT)')
    value = value.rstrip(' ')
    value = value.replace('f','')
    value = value.replace('F','')
    value = value.replace(' ','')
    return value

# substitute  a list of keyword 
def substitute(strFile,listStatic):
    for i in range(0,len(listStatic)):
        key    = list(listStatic)[i]
        value = listStatic[key]
        match = strFile.find(key)
        strFile = strFile.replace(key,value)
    return strFile

# Rewind the current char position to the start line 
def rewindStartBlock(block,cur):
    while block[cur] != '\n':
        cur = cur -1
    return cur+1

# move the current char position to the end word
def moveEndWordBlock(block,cur):
    while True :
        if not block[cur].isalnum() and block[cur] != '_':
            break
        cur = cur +1
    return cur



# read the variable name from the current position 
def getVar(block,cur):
    match = moveEndWordBlock(block,cur)
    return block[cur:match]

# build a string with Upper + lower example  atest = Atest
def makeUpperLower(name):
    str = name.upper()
    if len(name) >= 2 :
        str = name[0:1]
        str = str +  name[1:].lower()
    return str

# convert a string name in variable name C
def stdName(name):
    list = name.split(' ')
    if len(list) == 1:
        name = name.lower()
    else:
        name = list[0].lower()
        for i in range(1, len(list)):
            name = name + makeUpperLower(list[i])
    return name

# parse the string block and extract faust UI variable 
def parseBlock(block):
    listLines = block.splitlines()
    dicVar={}
    for line in listLines:
        posVar = line.find("&dsp->")
        if posVar != -1:
            var = getVar(line[posVar + 6:], 0)
            if var != '':
                if var.find('bargraph') == -1:
                    if   var in dicVar:
                        pass
                    else:
                        dicVar[var]={}
    return  dicVar

# parse the string block and extract the tooltip info and fill the dic 
def fillToolTips(block,varList,var):
    listLines = block.splitlines()
    dicVar={}
    for line in listLines:
        if line.find(var) != -1 and line.find("tooltip") != -1:
            posVar = line.find("tooltip")
            if posVar != -1:
                while line[posVar] != "," :
                    posVar = posVar+1
                while line[posVar] != '"':
                    posVar = posVar+1
                posVar = posVar + 1
                posEnd = posVar
                while line[posEnd] != '"':
                    posEnd = posEnd+1
                str = line[posVar:posEnd]
                varList[var]["tooltips"]=str

def fillButton(block,varList,var):
    listLines = block.splitlines()
    dicVar={}
    for line in listLines:
        if line.find(var) != -1 and line.find("addButton") != -1:
            posVar = line.find(",")
            if posVar != -1:
                while line[posVar] != '"':
                    posVar = posVar+1
                posVar = posVar + 1
                posEnd =posVar
                while line[posEnd] != '"':
                    posEnd = posEnd+1
                name = line[posVar:posEnd]
                name = name.rstrip(' ')
                name = name.lstrip(' ')
                name = name.rstrip('"')
                name = name.lstrip('"')
                name = checkAndFixName(varList,name)
                varList[var]["name"] =name
                varList[var]["control"] = "button"
                varList[var]["min"] = "0.0f"
                varList[var]["max"] = "1.0f"
                varList[var]["default"] = "0.0"



# parse the string block and extract the checkbox control info and fill the dic 
def fillCheckbox(block,varList,var):
    listLines = block.splitlines()
    dicVar={}
    for line in listLines:
        if line.find(var) != -1 and line.find("addCheckButton") != -1:
            posVar = line.find(",")
            if posVar != -1:
                while line[posVar] != '"':
                    posVar = posVar+1
                posVar = posVar + 1
                posEnd =posVar
                while line[posEnd] != '"':
                    posEnd = posEnd+1
                name = line[posVar:posEnd]
                name = name.rstrip(' ')
                name = name.lstrip(' ')
                name = name.rstrip('"')
                name = name.lstrip('"')
                name = checkAndFixName(varList,name)

                varList[var]["name"] =name
                varList[var]["control"] = "checkbox"
                varList[var]["min"] = "0.0f"
                varList[var]["max"] = "1.0f"
                varList[var]["default"] = "0.0"

def addStatic(block,name):
    nblock = ""
    listLines = block.splitlines()
    dicVar={}
    for line in listLines:
        match = line.find(name)
        if match != -1 :
            if (line.find("void ") != -1 or line.find("int ") != -1 or line.find("mydsp ") != -1 or line.find("mydsp*") != -1) and  (line.find("static") == -1) :
                line = "static " + line
        nblock = nblock +line + "\n"
    return nblock


# parse the string block and extract the faust slider control info and fill the dic
def fillNum(block,varList,var):
    listLines = block.splitlines()
    for line in listLines:
        if line.find(var) != -1 and line.find("addNumEntry") != -1 :
            wordList  = line.split(",")
            name = wordList[1]
            name = name.rstrip(' ')
            name = name.lstrip(' ')
            name = name.rstrip('"')
            name = name.lstrip('"')
            name = checkAndFixName(varList,name)


            default = stripValue(wordList[3])
            min     = stripValue(wordList[4])
            max     = stripValue(wordList[5])
            varList[var]["name"] = stdName(name)
            varList[var]["control"] = "default"
            varList[var]["min"] = min+'f'
            varList[var]["max"] = max+'f'
            varList[var]["default"] = default





# parse the string block and extract the faust slider control info and fill the dic 
def fillSlider(block,varList,var):
    listLines = block.splitlines()
    dicVar={}
    for line in listLines:
        a = line.find("addHorizontalSlider")
        b = line.find("addVerticalSlider")
        if line.find(var) != -1:
            if line.find("addHorizontalSlider") != -1 or line.find("addVerticalSlider") != -1 :
                wordList  = line.split(",")
                name = wordList[1]
                name = name.rstrip(' ')
                name = name.lstrip(' ')
                name = name.rstrip('"')
                name = name.lstrip('"')
                name = checkAndFixName(varList,name)


                default = stripValue(wordList[3])
                min     = stripValue(wordList[4])
                max     = stripValue(wordList[5])
                varList[var]["name"] = stdName(name)
                if float(min.replace(' ','')) == 0  and float(max.replace(' ','')) == 1:
                    varList[var]["control"] = "percent"
                elif float(max.replace(' ','')) -  float(min.replace(' ','')) <  float(100):
                    varList[var]["control"] = "slidershort"
                else:
                    varList[var]["control"] = "slider"
                varList[var]["min"] = min+'f'
                varList[var]["max"] = max+'f'
                varList[var]["default"] = default




def getGlobalCode():
    strGlobal = ""
    if len(tJsonCard)!= 0:
        for i in tJsonCard:
            strGlobal= strGlobal + i[".pGlobalCode"]+';\n'
    return strGlobal;





# Generate the AC UI Description and return a string 

def generateDescription(varList, name):
    global tJsonCard

    strDesc = str("")
    structName    ="s_"+name+"_dynamicParamsDesc"
    structDynamic = name+'_dynamic_config_t'
    if len(tJsonCard)!= 0:
        for i in tJsonCard:
            strDesc = str(strDesc) + str("  {\n")
            if '.pDescription' in i:
                strDesc= strDesc + '    .pDescription       = '+ i[".pDescription"]+',\n'
            if '.pControl' in i:
                strDesc= strDesc + '    .pControl           = '+ i[".pControl"]+',\n'
            if '.pDefault' in i:
                strDesc= strDesc + '    .pDefault           = '+ i[".pDefault"]+',\n'
            if '.pName' in i:
                strDesc= strDesc + '    .pName              = '+ i[".pName"]+',\n'
            if '.pCustom' in i:
                strDesc= strDesc + '    .pCustom            = '+ i[".pCustom"]+',\n'
            if '.pKeyValue' in i:
                strDesc= strDesc + '    .pKeyValue          = '+ i[".pKeyValue"]+',\n'
            if '.iParamFlag' in i:
                strDesc= strDesc + '    .iParamFlag          = '+ i[".iParamFlag"]+',\n'
            strDesc= strDesc + '  '+ i[".varDesc"]+',\n'
            strDesc= strDesc+ "  },\n"

    for i in range(0,len(varList)):
        var = varList[list(varList)[i]]
        strDesc= str(strDesc) + str("  {\n")
        if "tooltips" in var :
            strDesc= strDesc + '    .pDescription       = AUDIO_ALGO_OPT_STR("' +var["tooltips"]+ '"),\n'
        else:
            strDesc= strDesc + '    .pDescription       = AUDIO_ALGO_OPT_STR(""),\n'
        strDesc= strDesc + '    .pControl           = AUDIO_ALGO_OPT_STR("' +var["control"]+  '"),\n'
        strDesc= strDesc + '    .pDefault           = "'+var["default"]+'",\n'
        strDesc= strDesc + '    .pName              = "'+var["name"]+'",\n'
        strDesc= strDesc + '    AUDIO_DESC_PARAM_F('+structDynamic+ ', '+var["name"]+', '+var["min"]+', '+var["max"]+')\n'
        strDesc= strDesc+ "  },\n"

    return strDesc


# generate the update local attributes 

def generateUpdate(varList,name):
    strUpdate = ""
    for i in range(0,len(varList)):
        varName = list(varList)[i]
        var = varList[varName]
        strUpdate = strUpdate + "  pDsp->"+varName+" = pParams->"+var["name"]+";\n"
    return strUpdate


# generate  the local attributes variable

def generateStruct(varList,name):
    strStruct = ""
    for i in range(0,len(varList)):
        varName = list(varList)[i]
        var = varList[varName]
        strStruct = strStruct + "  float  "+var["name"]+";\n"
    for i in tJsonCard:
        strStruct = strStruct + "  " +i[".varType"]+";\n"
    return strStruct

# Generate and filter an AC template file  and produce a finale AC source code 

def generate_template(fileTemplate,fileDest,substList):
    with open(fileTemplate,"r") as f:
        lines = f.read()
    lines = substitute(lines, substList)
    print("Write   : "+fileDest)        
    with open(fileDest,"w") as w:
        w.write(lines)

# remove some function not compatible with the AC integration  
def removeFn(strFile,fnName):
    match = strFile.find(fnName)
    if match != -1 :
        start = rewindStartBlock(strFile,match)
        defineString = '#if FULL_BUILD\n'
        newStr = strFile[0:start] + defineString + strFile[start:]
        strFile =newStr
        match = strFile.find("}",start)
        if match != -1:
            defineString = '\n#endif\n'
            newStr = strFile[0:match+1] + defineString + strFile[match+1:]
            strFile = newStr
    return strFile


def addAlgoNameFile(AlgoName):
    global addAlgoName
    if addAlgoName != '' :
        if not os.path.exists(addAlgoName) :
            try:
                open(addAlgoName, 'a').close()
            except :
                pass
        with open(addAlgoName, 'a') as file1:
            str = '  {.pAlgoName = "'+AlgoName+'"},'
            file1.write(str+"\n")

def do_generate_livetune(args):
    GroupName    = args.group
    basePath    = args.basePath

    if not os.path.exists(args.addAlgoName):
        print(" no valid algoname file ")
        return;
    with open(args.addAlgoName,"r") as f:
        lines = f.read()

    listStatic={}
    listStatic["$(ALGO_NAME_LIST)"] =  lines
    listStatic["$(ALGO_NAME)"] =  FunctionName
    listStatic["$(ALGO_GROUP)"] =  GroupName
    listStatic["$(ALGO_NAME_PLG)"] =  GetStdCName(FunctionName).replace("_","-").lower()
    listStatic["$(ALGO_NAME_MAJ_STD)"] = GetStdCName(FunctionName).upper()
    listStatic["$(ALGO_NAME_MINUS_STD)"] = GetStdCName(FunctionName).lower()

    os.makedirs(basePath+"Algos/"+GroupName+"/src/livetune", exist_ok=True)
    generate_template("template/"+args.template+"/plugin_processing_template.c",basePath+"Algos/"+GroupName+"/src/livetune/plugin_processing_"+GroupName+".c",listStatic)

def add_json_card(fileJson):
    global tJsonCard
    try:
        with open(fileJson,"r") as f:
            lines = f.read()
    except:
        print("File not found:",fileJson)
        exit(-1)
    try:
        jsonCard = json.loads(lines)
    except:
        print("Cannot parse json:", fileJson)
        exit(-1)
    tJsonCard.append(jsonCard)


# Process a file

def do_generate_ac(args):

    global FunctionName,GroupName,Skip,basePath,varNameSubstitute,addAlgoName
    global tJsonCard

    file = args.infile
    FunctionName =args.name
    GroupName    = args.group
    basePath    = args.basePath
    Skip        = args.skipCleanup
    addAlgoName = args.addAlgoName
    varNameSubstitute =args.rename
    tJsonCard = []
    for i in args.add_card:
        add_json_card("template/card/"+i)

    print("Process : "+ file)
    try:
        with open(file,"r") as f:
            lines = f.read()
        match = lines.find("buildUserInterfacemydsp")
        if match != -1:
            first = lines[match:].find("{")
            end = lines[match:].find("}")
            block = lines[match+first+1:match+end-1]
            varList = parseBlock(block)
            #remove empty entries
            for var in varList:
                fillToolTips(block,varList,var)
                fillCheckbox(block,varList,var)
                fillSlider(block,varList,var)
                fillButton(block, varList, var)
                fillNum(block, varList, var)


            removeEntry = []
            for i in varList:
                d = varList[i]
                if not d:
                    removeEntry.append(i)
            for i in  removeEntry:
                varList.pop(i,None)

            applyRename(varList)
            strDesc   = generateDescription(varList,GetStdCName(FunctionName))
            strUpdate = generateUpdate(varList,GetStdCName(FunctionName))
            strStruct = generateStruct(varList,GetStdCName(FunctionName))
    except:
        print("File not found:",file)
        exit(-1)
    try:
        if Skip == False:
            shutil.rmtree("generated")
    except:
        pass
    os.makedirs(basePath+"Algos/"+GroupName, exist_ok=True)
    os.makedirs(basePath+"Algos/"+GroupName+"/src/wrapper/faust", exist_ok=True)

    listStatic = {}
    if lines.find("FULL_BUILD") == -1:

        lines = addStatic(lines,"newmydsp")
        lines = addStatic(lines,"deletemydsp")
        lines = addStatic(lines,"metadatamydsp")
        lines = addStatic(lines,"getSampleRatemydsp")
        lines = addStatic(lines,"getNumOutputsmydsp")
        lines = addStatic(lines,"classInitmydsp")
        lines = addStatic(lines,"getNumInputsmydsp")
        lines = addStatic(lines,"instanceResetUserInterfacemydsp")
        lines = addStatic(lines,"instanceClearmydsp")
        lines = addStatic(lines,"instanceConstantsmydsp")
        lines = addStatic(lines,"instanceInitmydsp")
        lines = addStatic(lines,"initmydsp")
        lines = addStatic(lines,"buildUserInterfacemydsp")
        lines = addStatic(lines,"computemydsp")


       

        lines = substitute(lines,listStatic)
        lines = removeFn(lines,"buildUserInterfacemydsp")
        lines = removeFn(lines,"metadatamydsp")
    else:
        print("Skip    : update "+file+"base already done")
    print("Write   : "+basePath+"Algos/"+GroupName+"src/wrapper/faust/faust_"+GetStdCName(FunctionName)+".c")
    with open(basePath+"Algos/"+GroupName+"/src/wrapper/faust/"+GetStdCName(FunctionName)+".c","w") as w:
        w.write(lines)

    listStatic={}
    listStatic["$(ALGO_NAME)"] =  FunctionName
    listStatic["$(ALGO_GROUP)"] =  GroupName
    listStatic["$(ALGO_NAME_PLG)"] =  GetStdCName(FunctionName).replace("_","-").lower()
    listStatic["$(ALGO_NAME_MAJ_STD)"] = GetStdCName(FunctionName).upper()
    listStatic["$(ALGO_NAME_MINUS_STD)"] = GetStdCName(FunctionName).lower()
    listStatic["$(ALGO_DESCRIPTION)"] = strDesc
    listStatic["$(ALGO_NAME_UPDATE_PARAM)"] = strUpdate
    listStatic["$(ALGO_PARAM_STRUCT)"] = strStruct
    listStatic["$(ALGO_GLOBAL_CODE)"] = getGlobalCode()


    addAlgoNameFile(listStatic["$(ALGO_NAME_PLG)"])

    generate_template("template/"+args.template+"/audio_chain_faust_template.c",basePath+"Algos/"+GroupName+"/src/wrapper/audio_chain_"+GetStdCName(FunctionName)+".c",listStatic)
    generate_template("template/"+args.template+"/audio_chain_faust_template.h",basePath+"Algos/"+GroupName+"/audio_chain_"+GetStdCName(FunctionName)+".h",listStatic)
    generate_template("template/"+args.template+"/audio_chain_faust_template_factory.c",basePath+"Algos/"+GroupName+"/audio_chain_"+GetStdCName(FunctionName)+"_factory.c",listStatic)
    generate_template("template/"+args.template+"/audio_template.h",basePath+"Algos/"+GroupName+"/audio_"+GetStdCName(FunctionName)+".h",listStatic)

subcmds = {
    'generate_ac': do_generate_ac,
    'generate_livetune': do_generate_livetune
}


# main 
def main():
    global FunctionName,GroupName,Skip,basePath

    parser = argparse.ArgumentParser()
    subs = parser.add_subparsers(help='subcommand help', dest='subcmd')
    generate_ac = subs.add_parser('generate_ac', help='Convert an Faust DSP C source code in AC source code')
    generate_ac.add_argument('infile')
    generate_ac.add_argument("--name",    help="Algo name. Example : --name 'faust-vocoder' ",default="faust")
    generate_ac.add_argument("--skipCleanup",    help="Skip the generated folder cleanup ",action='store_true',default=False)
    generate_ac.add_argument("--group",    help="Set the folder Group",default="faust")
    generate_ac.add_argument("--basePath",    help="Set the base generated folder ",default="generated/")
    generate_ac.add_argument("--rename",  help="Add a variable rename to the dictionary Example:--varRename 'disco:Disconnection'",action='append',default=[])
    generate_ac.add_argument("--addAlgoName",  help="Add algoname in file ( to be used with generate_livetune  Example:--addAlgoName 'algoname.txt'",default='')
    generate_ac.add_argument("--template",  help="Set template type generator/filter",default="filter")
    generate_ac.add_argument("--add-card",action='append',  help="Add a variable card to add ",default=[])



    generate_livetune = subs.add_parser('generate_livetune', help='Generated livetune plugin using fast generated algo')
    generate_livetune.add_argument("--addAlgoName",  help="Add algo name in file ( to be used with generate_livetune  Example:--addAlgoName 'algoname.txt'",default='')
    generate_livetune.add_argument("--group",    help="Set the folder Group",default="faust")
    generate_livetune.add_argument("--basePath",    help="Set the base generated folder ",default="generated/")
    generate_livetune.add_argument("--template",  help="Set template type generator/filter",default="filter")


    args = parser.parse_args()
    if args.subcmd is None:
        print('Must specify a subcommand')
        sys.exit(1)
    subcmds[args.subcmd](args)


if __name__ == "__main__":
    main()