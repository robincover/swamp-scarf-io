### NAME
HashToJSON - A C tool for writing JSON-SCARF
### SYNOPSIS
```c
#include <stdio.h>
#include "HashToJSON.c"

int main(int argc, char **argv) {
   HashToJSON *  writer = newHashToJSONForFilename("/path/to/file/");
    if(writer == NULL){
        printf("failed to create writer\n");
        return 1;
    }
    
    SetErrorLevel(writer, 1);
    SetPretty(1);

    AddStartTag(writer, initialInformation);
    AddBug(writer, bugInformation);
    AddMetric(writer, metricInformation);
    AddSummary(writer);
    AddEndTag(writer);

    CloseHashToJSON(writer);
     
    return 0;
}
```
### DESCRIPTION
This module provides the ability to convert C data structures into JSON-SCARF formatted documents. It is dependent on yajl, which is used to handle writing of the JSON-SCARF document.

The writer is controlled primarily by the user through method calls which feed data in to be written. These pieces of data are to be structured as C structures show below.

The user has the ability to set the error level to 0 (none), 1 (warnings), or 2 (exit). Both error levels 1 and 2 will print all error messages found during writing including misformatted elements, required elements not found, wrong value types, and ordering violations. Error level 2 will also exit the program as soon as an error is found to ensure validity of results.

### METHODS
#### HashToJSON * newHashToJSONFilename(char * filename)
Initializes a HashToJSON struct. Opens a file stream from the file represented by the filename to be written to.

#### HashToJSON * newHashToJSONForFile(FILE * file)
Initializes a HashToJSON struct with the given file.

#### HashToJSON * newHashToJSONForString(char * str, int * size)
Initializes a HashToJSON struct to write to a specified address in memory. str is a initial char pointer and size contains the total amount of memory occupied by the string.

#### int SetPretty ( HashToJSON * writerInfo, int pretty_en ) 
Either enable or disable pretty printing with a true or false value (default true). Returns 0 or 1 if error changing settings.

#### int SetErrorLevel(HashToJSON * writerInfo, int errorLevel)
Allows changing the errorlevel of the writer to 0 (none), 1 (print warnings), or 2 (print warnings and exit). If ERRORLEVEL does not equal one of these value returns 1 and does nothing.

#### int GetErrorLevel(HashToJSON * writerInfo)
Returns the current error level.

#### char * CheckStart (Initial * INITIALDATA)
Checks a Initial to see if there are any errors. Returns a string containing all errors found seperated by newlines, otherwise returns an empty string.
#### char * CheckBug (BugInstance * BUGDATA)
Checks a BugInstance to see if there are any errors. Returns a string containing all errors found seperated by newlines, otherwise returns an empty string.
#### char * CheckMetric (Metric * METRICDATA)
Checks a Metric to see if there are any errors. Returns a string containing all errors found seperated by newlines, otherwise returns an empty string.

#### AddStartTag(HashToJSON * writerInfo, Initial * INITIALDATA)
Writes a start tag to the file based on INITIALDATA. For details on valid data structures see below. Must be called exactly once before other 'Add' methods below.

#### AddBugInstance(HashToJSON * writerInfo, BugInstance * BUGDATA)
Writes a bug to the file based on BUGDATA. For details on valid data structures see below. May be called 0 or more time and inter-layered with 'AddMetric' calls. Not allowed after 'AddSummary' and 'AddEndTag'.

#### AddMetric(HashToJSON * writerInfo, Metric * METRICDATA)
Writes a metric to the file based on METRICDATA. For details on valid data structures see below. May be called 0 or more time and inter-layered with 'AddBug' calls. Not allowed after 'AddSummary' and 'AddEndTag'.

#### AddSummary(HashToJSON * writerInfo)
Writes a summary to the file based on all bugs and metrics already written with this writer. May be called 0 or more times before 'AddEndTag'.

#### AddEndTag(HashToJSON * writerInfo)
Writes an end tag to the file. Must be called exactly once after which no other 'Add' methods may be called.

#### void closeHashToJSON(HashToJSON * writerInfo)
Closes writer and any opened files opened by the writer.


### DATA STRUCTURES

The following are the data structures used in the methods listed above. Elements listed as required must be included to produce a valid JSON-SCARF file. All other elements are optional, but should they be included shall be written to JSON-SCARF in the correct format.

#### InitialData
```
{
    char * tool_name       #REQUIRED
    char * tool_version    #REQUIRED 
    char * uuid            #REQUIRED
} 
```
#### BugInstance
```
{
    char * className
    char * bugSeverity
    char * bugRank
    char * resolutionSuggestion
    char * bugMessage                #REQUIRED
    char * bugCode
    char * bugGroup 
    char * assessmentReportFile      #REQUIRED
    char * buildId                   #REQURIED
    CweIds * cweIds
    InstanceLocation * instanceLocation
    Method * methods
    Location * bugLocations          #REQUIRED
}
```
##### CweIds
```
{
    int cweid        #REQUIRED
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
    int start      #REQUIRED
    int end        #REQUIRED
}
```
##### Method
```
{
    int primary    #REQUIRED
    char * name    #REQUIRED
    Method * next
}
```
##### Location
```
{
    int primary        #REQUIRED
    int startLine
    int endLine
    int startColumn
    int endColumn
    char * explanation
    char * sourceFile  #REQUIRED
    Location * next
}
```
#### Metric
```
{
    char * value        #REQUIRED
    char * clas
    char * method
    char * sourceFile   #REQUIRED
    char * type         #REQUIRED
}
```
