### NAME
ScarfToHash - A C tool for parsing SCARF
### SYNOPSIS
```c
#include <stdio.h>
#include "ScarfToHash.c"

int main(int argc, char **argv) {
    ScarfToHash * reader = newScarfToHashFilename("/path/to/file");
    if(reader == NULL){
	printf("reader failed\n");
	return 1;
    }

    SetInitialCallback(reader, initialFunction);
    SetBugCallback(reader, bugFunction);
    SetMetricCallback(reader, metricFunction);
    SetBugSummaryCallback(reader, bugSummaryFunction);
    SetMetricSummaryCallback(reader, metricSummaryFunction);
    SetFinishCallback(reader, finishFunction);
    
    void * context = DATA
    SetCallbackData(reader, context);

    Parse(reader);
    CloseScarfToHash(reader);
    return 0;
}
```
### DESCRIPTION
This module provides the ability to convert SCARF files into C data structures. It is dependent on libxml, which is used to handle parsing of the XML document.

The parser is controlled primarily by the callbacks the user sets prior to calling the parse method. A callback will be called once the parser is finished parsing a section of the document. These sections are the beginning AnalyzerReport tag, an entire BugInstance or Metric, a complete BugSummary or MetricSummary, and the end AnalyzerReport tag.

All Callbacks except the FinishCallback receive as parameters a reference to a struct containing information on their section of parsed data and the context reference provided as CallbackData. Upon reaching the end of an Analyzer Report the FinishCallback  will be called with the error value returned by the previous call and the context reference.

While the parser does do minor checks to ensure the inputted file is a SCARF file, if an invalid SCARF file is passed in, most behavior is undefined.
### METHODS
#### ScarfToHash * newScarfToHashFilename(char * filename)
Initializes a ScarfToHash struct. Opens a file stream from the file represented by the filename to be parsed.

#### ScarfToHash * newScarfToHashForString(char * str)
Initializes a ScarfToHash struct with the string representing the entire file to be parsed from memory.

#### ScarfToHash * newScarfToHashForFd(int fd)
Initializes a ScarfToHash struct with the given file descriptor.

#### ScarfToHash * newScarfToHashForMemory(char * str, int size)
Initializes a ScarfToHash struct to parse from memory. str is a initial char pointer and size represents the size of the document in memory.

#### void setInitialCallback(ScarfToHash * reader, InitialCallback startFunction)
Sets the InitialCallback to be called after each AnalyzerReport start tag is parsed. For details on callbacks see below.

#### void setBugCallback(ScarfToHash * reader, BugCallback callback)
Sets the BugCallback to be called after each full BugInstance is parsed. For details on callbacks see below.    
 
#### void setMetricCallback(ScarfToHash * reader, MetricCallback callback)
Sets the MetricCallback to be called after each full Metric is parsed. For details on callbacks see below.

#### void setBugSummaryCallback(ScarfToHash * reader, BugSummaryCallback callback)
Sets the BugSummaryCallback to be called after all metric summaries are parsed. For details on callbacks see below.

#### void setMetricSummaryCallback(ScarfToHash * reader, MetricSummaryCallback callback)
Sets the MetricSummaryCallback to be called after all metric summaries are parsed. For details on callbacks see below. 

#### void setFinalCallback(ScarfToHash * reader, FinalCallback callback) 
Sets the FinalCallback to be called after each AnalyzerReport end tag is parsed. For details on callbacks see below.

#### void setCallbackData(ScarfToHash * reader, void * callbackData) 
Sets the data to be used as an additional parameter to callbacks. CALLBACKDATA can be data stored in a reference.  This data is passed to all callbacks and can be used to avoid global variables.

#### InitialCallback getInitialCallback(ScarfToHash * reader, InitialCallback callback) 
Access the current value of InitialCallback. For details on callbacks see below.

#### BugCallback getBugCallback(ScarfToHash * reader, BugCallback callback) 
Access the current value of BugCallback. For details on callbacks see below.

#### MetricCallback getMetricCallback(ScarfToHash * reader, MetricCallback callback) 
Access the current value of MetricCallback. For details on callbacks see below.

#### BugSummaryCallback getBugSummaryCallback(ScarfToHash * reader, BugSummaryCallback callback) 
Access the current value of BugSummaryCallback. For details on callbacks see below.

#### MetricSummaryCallback getMetricSummaryCallback(ScarfToHash * reader, MetricSummaryCallback callback) 
Access the current value of MetricCallback. For details on callbacks see below.

#### FinalCallback getFinalCallback(ScarfToHash * reader, FinalCallback callback) 
Access the current value of FinalCallback. For details on callbacks see below.

#### void * getCallbackData(ScarfToHash * reader, void * callbackData) 
Access the current value of CallbackData.

#### void * parse()
This method initiates the parsing of the set file. If parsing fails an exit(1) call will be thrown. The return value of parse will be the return value of FinalCallback if it is defined. Otherwise the reutrn value will be the same as the last callback executed or NULL if there are no errors.

#### int closeScarfToHash(ScarfToHash * reader)
Closes parser. Returns 0 or -1 in case of error.


### CALLBACKS
The main purpose of this module is to interpret data from libxml and assemble them into usable C data structures. When parsing, the module will call the pre-defined callbacks upon completion of parsing an object of their respective type. All callbacks receive CallbackData as an additional parameter as a context variable. For details on the structure of each individual C struct see below.

#### void * InitialCallback(Initial * intitial-data, void * context)
This is called just after the AnalyzerReport start tag is parsed. Any non-NULL return value will terminate parsing and skip to FinishCallback.

#### void * MetricCallback(Metric * metric-data, void * context)
This is called every time a single Metric completes parsing. Any non-NULL return value will terminate parsing and skip to FinishCallback.

#### void * BugCallback(BugInstance * bug-data, void * context)
This is called every time a single BugInstance completes parsing. Any non-NULL return value will terminate parsing and skip to FinishCallback.

#### void * BugSummaryCallback(BugSummary * bug-summary-data, void * context)
This is called after all BugSummaries have been parsed. Any non-NULL return value will terminate parsing and skip to FinishCallback.

#### void * MetricSummaryCallback(MetricSummary * metric-summary-data, void * context)
This is called once all MetricSummaries have been parsed. Any non-NULL return value will terminate parsing and skip to FinishCallback.

#### void * FinishCallback(void * ret, void * context)
This is called after reaching an AnalayzerReport end tag. If one of the above callbacks terminates parsing with a defined return value, ret will equal that value, otherwise ret will be NULL.


### DATA STRUCTURES

The following are the data structures used in the callbacks listed above. Elements that are either not defined or do not exist in the Scarf file will be NULL.

#### InitialData
```
{
    char * tool_name
    char * tool_version
    char * uuid
} 
```
#### BUGDATA
```
{
    int bugId
    char * className
    char * bugSeverity
    char * bugRank
    char * resolutionSuggestion
    char * bugMessage
    char * bugCode
    char * bugGroup
    char * assessmentReportFile
    char * buildId
    CweIds * cweIds
    InstanceLocation * instanceLocation
    Method * methods
    Location * bugLocations
}
```
##### CweIds
```
{
    int cweid
    CweIds * next
}
```
##### InstanceLocation
```
{
    LineNum lineNum
    char * xPath
}
```
###### LineNum
```
{
    int start
    int end
}
```
##### Method
```
{
    int methodId
    int primary
    char * name
    Method * next
}
```
##### Location
```
{
    int primary
    int startLine
    int endLine
    int startColumn
    int endColumn
    int locationId
    char * explanation
    char * sourceFile
    Location * next
}
```
#### Metric
```
{
    int id
    char * value
    char * clas
    char * method
    char * sourceFile
    char * type
}
```

#### BugSummary
```
{
    int count
    int byteCount
    char * code
    char * group
    BugSummary * next
}
```

#### MetricSummary
```
{
    double count
    double sum
    double sumOfSquares
    double max
    double min
    double stdDeviation
    double average
    int valid
    char * type
    MetricSummary * next
}
```
