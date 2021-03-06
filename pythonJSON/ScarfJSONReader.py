import sys
#from yajl import *
from yajl import YajlContentHandler
from yajl import YajlParser

class KillParse(Exception):
    def __init__(self, ret):
	self.ret = ret

##############################Callbacks#################################
class ParseContentHandler(YajlContentHandler):
    def __init__(self, callbacks):
        self.callbacks = callbacks
        self.data = {}
        self.initialInfo = {}

        self.curr = ""
        self.hashType = ""
        self.validStart = 0
        self.requiredStart = 0
        self.validBody = 0
        self.depth = 0
	self.ret = None

        self.isArray = 0
        self.arrayLoc = 0
        self.arrayType = ""
    
        self.sumGroup = ""
        self.sumCode = "" 


    def yajl_null(self, ctx):
        pass


    def yajl_boolean(self, ctx, boolVal):
        if self.hashType == "bug":
            if self.isArray:
                if self.arrayType == "Methods" or  self.arrayType == "BugLocations":
                    if boolVal:
                        self.data[self.arrayType][self.arrayLoc]["primary"] = 1
                    else:
                        self.data[self.arrayType][self.arrayLoc]["primary"] = 0


    def yajl_number(self, ctx, stringNum):
        if self.hashType == "bug":
            if self.curr in ["BugId"]:
                self.data[self.curr] = stringNum
            if self.curr in ["Start", "End"]:
                self.data["InstanceLocation"]["LineNum"][self.curr] = stringNum
            elif self.isArray:
                if self.arrayType == "BugLocations":
                    if self.curr in ["primary", "LocationId", "EndLine", "StartLine", "EndColumn", "StartColumn"]:
                        self.data["BugLocations"][self.arrayLoc][self.curr] = stringNum
                elif self.arrayType == "Methods":
                    if self.curr in ["primary", "MethodId"]:
                        self.data["Methods"][self.arrayLoc][self.curr] = stringNum
                elif self.arrayType == "CweIds":
                    self.data[self.arrayType].append({})
                    self.data["CweIds"][self.arrayLoc] = stringNum
                    self.arrayLoc = self.arrayLoc + 1
        
	if self.hashType == "metric":
            if self.curr in ["MetricId", "Value"]:
                self.data[self.curr] = stringNum;
        
	if self.hashType == "bugsum" :
            if self.curr in ["bytes", "count"]:
                if  (self.sumCode) not in self.data:
                    self.data[self.sumCode] = {}
                if (self.sumGroup) not in self.data[self.sumCode]:
                    self.data[self.sumCode][self.sumGroup] = {} 
                    self.data[self.sumCode][self.sumGroup][self.curr] = stringNum

        if self.hashType == "metrsum" :
            if self.curr in ["Sum", "Maximum", "Minimum", "Average", "Count", "SumOfSquares", "StandardDeviation"]:
                if (self.sumGroup) not in self.data:
                    self.data[self.sumGroup] = {} 
                    self.data[self.sumGroup][self.curr] = stringNum

    def yajl_string(self, ctx, stringVal):
        if not self.requiredStart and self.curr in ["uuid", "tool_name", "tool_version"]:
            self.initialInfo[self.curr] = stringVal
            if "uuid" in self.initialInfo and "tool_name" in self.initialInfo and "tool_version" in self.initialInfo:
                self.requiredStart = 1
                if "InitialCallback" in self.callbacks:
                    if "CallbackInfo" in self.callbacks:
                        ret = self.callbacks["InitialCallback"](self.initialInfo, self.callbacks["CallbackInfo"])
                        if ret is not None:
			    raise KillParse(ret)
                    else:
                        ret = self.callbacks["InitialCallback"](self.initialInfo)
                        if ret is not None:
			    raise KillParse(ret)

        elif self.hashType == "bug":
            if self.curr in ["AssessmentReportFile", "BuildId", "ClassName", "BugGroup", "BugCode", "BugRank", "BugSeverity", "BugMessage", "ResolutionSuggestion", "BugId"]:
                self.data[self.curr] = stringVal
            elif self.curr in ["Xpath"]:
                self.data["InstanceLocation"][self.curr] = stringVal
            elif self.curr in ["Start", "End"]:
                self.data["InstanceLocation"]["LineNum"][self.curr] = stringVal

            elif self.isArray:
                if self.arrayType == "BugLocations":
                    if self.curr in ["LocationId", "EndLine", "StartLine", "EndColumn", "StartColumn", "SourceFile", "Explanation"]:
                        self.data["BugLocations"][self.arrayLoc][self.curr] = stringVal
                    elif self.curr == "primary":
                        if stringVal == "true" or stringVal == "1":
                            self.data["BugLocations"][self.arrayLoc]["primary"] = 1
                        else:
                            self.data["BugLocations"][self.arrayLoc]["primary"] = 0
                
		elif self.arrayType == "Methods":
                    if self.curr in ["MethodId", "name"]:
                        self.data["Methods"][self.arrayLoc][self.curr] = stringVal
                    elif self.curr == "primary":
                        if stringVal == "true" or stringVal == "1":
                            self.data["Methods"][self.arrayLoc]["primary"] = 1
                        else:
                            self.data["Methods"][self.arrayLoc]["primary"] = 0
                
		elif self.arrayType == "CweIds":
                    self.data[self.arrayType].append({})
                    self.data["CweIds"][self.arrayLoc] = stringVal
                    self.arrayLoc = self.arrayLoc + 1;
        
        elif self.hashType == "metric":
            if self.curr in ["Value", "MetricId", "Class", "Method", "Type", "SourceFile"]:
                self.data[self.curr] = stringVal

        elif self.hashType == "bugsum" :
            if self.curr in ["bytes", "count"]:
                if  (self.sumCode) not in self.data:
                    self.data[self.sumCode] = {}
                if (self.sumGroup) not in self.data[self.sumCode]:
                    self.data[self.sumCode][self.sumGroup] = {} 
                    self.data[self.sumCode][self.sumGroup][self.curr] = stringVal
        
        elif self.hashType == "metrsum" :
            if self.curr in ["Sum", "Maximum", "Minimum", "Average", "Count", "SumOfSquares", "StandardDeviation"]:
                if (self.sumGroup) not in self.data:
                    self.data[self.sumGroup] = {} 
                    self.data[self.sumGroup][self.curr] = stringVal

    def yajl_start_map(self, ctx):
        self.depth = self.depth + 1
        if self.isArray:
            self.data[self.arrayType].append({})

    def yajl_map_key(self, ctx, stringVal):
        if self.depth == 1 and stringVal == "AnalyzerReport":
            self.validStart = 1
        
	elif stringVal == "BugInstances" and self.depth == 2:
            if  not self.validStart :
                print("Misformed SCARF File: No AnalyzerReport Tag before first element")
            self.hashType = "bug"
            self.data = {}
            self.validBody = 1
        
	elif stringVal == "Metrics" and self.depth == 2:
            if  not self.validStart :
                print("Misformed SCARF File: No AnalyzerReport Tag before first element")
            self.hashType = "metric"
            self.data = {}
            self.validBody = 1
        
	elif stringVal == "BugSummaries" and self.depth == 2:
            if  not self.validStart :
                print("Misformed SCARF File: No AnalyzerReport Tag before first element")
            if not self.validBody:
                print("No BugInstances or Metrics present")
            self.hashType = "bugsum"
            self.data = {}
        
	elif stringVal == "MetricSummaries" and self.depth == 2:
            if  not self.validStart :
                print("Misformed SCARF File: No AnalyzerReport Tag before first element")
            if not self.validBody:
                print("No BugInstances or Metrics present")
            self.hashType = "metrsum"
            self.data = {}

        
	elif stringVal == "BugLocations" and self.depth == 3:
            self.data["BugLocations"] = []
        elif stringVal == "Methods" and self.depth == 3:
            self.data["Methods"] = []
        elif stringVal == "CweIds" and self.depth == 3:
            self.data["CweIds"] = []
        elif stringVal == "InstanceLocation" and self.depth == 3:
            self.data["InstanceLocation"] = {}
        elif stringVal == "LineNum" and self.depth == 4:
            self.data["InstanceLocation"]["LineNum"] = {}
        
	elif self.hashType == "bugsum" and self.depth == 3:
            self.sumCode = stringVal
        elif self.hashType == "bugsum" and self.depth == 4:
            self.sumGroup = stringVal
        elif self.hashType == "metrsum" and self.depth == 3:
            self.sumGroup = stringVal
        self.curr = stringVal
        return self

    def yajl_end_map(self, ctx):
        self.depth = self.depth - 1
        if self.depth == 2:
            if self.hashType == "bug" and "BugCallback" in self.callbacks:
                if "CallbackData" in self.callbacks:
                    ret = self.callbacks["BugCallback"](self.data, self.callbacks["CallbackData"])
		    if ret is not None:
                        raise KillParse(ret)
                else:
                    ret self.callbacks["BugCallback"](self.data)
		    if ret is not None:
                        raise KillParse(ret) 
            
	    elif self.hashType == "metric" and "MetricCallback" in self.callbacks:
                if "CallbackData" in self.callbacks:
                    ret self.callbacks["MetricCallback"](self.data, self.callbacks["CallbackData"])
		    if ret is not None:
                        raise KillParse(ret)
                else:
                    ret self.callbacks["MetricCallback"](self.data)
		    if ret is not None:
                        raise KillParse(ret)
            
	    elif self.hashType == "bugsum" and "BugSummaryCallback" in self.callbacks:
                if "CallbackData" in self.callbacks:
                    ret = self.callbacks["BugSummaryCallback"](self.data, self.callbacks["CallbackData"])
		    if ret is not None:
                        raise KillParse(ret)
                else:
                    ret = self.callbacks["BugSummaryCallback"](self.data)
		    if ret is not None:
                        raise KillParse(ret)
            
	    elif self.hashType == "metrsum" and "MetricSummaryCallback" in self.callbacks:
                if "CallbackData" in self.callbacks:
                    ret = self.callbacks["MetricSummaryCallback"](self.data, self.callbacks["CallbackData"])
		    if ret is not None:
                        raise KillParse(ret)
                else:
                    ret = self.callbacks["MetricSummaryCallback"](self.data)
		    if ret is not None:
                        raise KillParse(ret)
        
	elif self.depth == 0:
            if "InitialCallback" in self.callbacks and not self.requiredStart:
                if "CallbackData" in self.callbacks:
                    ret = self.callbacks["InitialCallback"](self.initialInfo, self.callbacks)
		    if ret is not None:
                        raise KillParse(ret)
                else:
                    ret = self.callbacks["InitialCallback"](self.initialInfo)
		    if ret is not None:
                        raise KillParse(ret)
        if self.isArray:
            self.arrayLoc = self.arrayLoc + 1
        

    def yajl_start_array(self, ctx):
        if self.depth > 2 :
            self.isArray = 1
            self.arrayLoc = 0
            self.arrayType = self.curr
            
    
    def yajl_end_array(self, ctx):
            self.isArray = 0



####################parser##############################
class ScarfJSONReader:
    def __init__(self, inputFile):
        self.callbacks = {}
        self.handler = None
        self.parser = None
        self.filename = inputFile
	self.utf8 = 1;

    #################Callback Accessors/Mutators##############
    def SetInitialCallback(self, callback):
        self.callback["InitialCallback"] = callback

    def SetBugCallback(self, callback):
        self.callback["BugCallback"] = callback

    def SetMetricCallback(self, callback):
        self.callback["MetricCallback"] = callback

    def SetBugSummaryCallback(self, callback):
        self.callback["BugSummaryCallback"] = callback

    def SetMetricSummaryCallback(self, callback):
        self.callback["MetricSummaryCallback"] = callback

    def SetFinalCallback(self, callback):
        self.callback["FinalCallback"] = callback

    def SetCallbackData(self, callbackData):
        self.callback["CallbackData"] = callback

    def SetUTF8(self, utf8):
	self.utf8 = utf8


    def GetUTF8(self):
	return self.utf8

    def GetInitialCallback(self):
        return self.callback["InitialCallback"]

    def GetBugCallback(self):
        return self.callback["BugCallback"]

    def GetMetricCallback(self):
        return self.callback["MetricCallback"]

    def GetBugSummaryCallback(self):
        return self.callback["BugSummaryCallback"]

    def GetMetricSummaryCallback(self):
        return self.callback["MetricSummaryCallback"]

    def GetFinalCallback(self):
        return self.callback["FinalCallback"]

    def GetCallbackData(self):
        return self.callback["CallbackData"]



    def parse(self):
	ret = None
        self.handler = ParseContentHandler(callbacks)
        self.parser = YajlParser(content_handler = self.handler)
	self.parser.dont_validate_strings = self.utf8
        fh = self.filename
	try:
            fh.read()
        except AttributeError:
            fh =  open(self.filename, 'r')

	try:
	    self.parser.parse(fh)
	except KillParse as e:
	    ret = e.ret
        if "FinalCallback" in self.callbacks:
            if "CallbackData" in self.callbacks:
                ret = self.callbacks["FinalCallback"](ret, self.callbacks)
            else:
                ret = self.callbacks["FinalCallback"](ret)
        fh.close()
	return ret


1
