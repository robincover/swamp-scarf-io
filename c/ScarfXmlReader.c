//  Copyright 2016 Brandon G. Klein
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libxml/xmlreader.h>
#include "filestructure.h"

/////////////////////////TypeDef for Clarity///////////////////////////////////////


typedef void *(*BugCallback)(BugInstance *bug, void *reference);
typedef void *(*BugSummaryCallback)(BugSummary *bugSum, void *reference);
typedef void *(*MetricCallback)(Metric *metr, void *reference);
typedef void *(*MetricSummaryCallback)(MetricSummary *metrSum, void *reference);
typedef void *(*InitialCallback)(Initial *initial, void *reference);
typedef void *(*FinalCallback)(void *killValue, void *reference);

typedef struct Callback {
    BugCallback bugCall;
    MetricCallback  metricCall;
    InitialCallback initialCall;
    BugSummaryCallback bugSumCall;
    MetricSummaryCallback metricSumCall;
    FinalCallback finishCallback;
    void *CallbackData;
} Callback;


typedef struct ScarfXmlReader{
    xmlTextReaderPtr reader;
    Callback *callback;
} ScarfXmlReader;

///////////////Initiailize a Metric//////////////////////////////////////////////
Metric *initializeMetric()
{
    Metric *metric = calloc(1,sizeof(Metric));
    return metric;
}


///////////////Initiailize a BugInstance//////////////////////////////////////////////
BugInstance *initializeBug()
{
    BugInstance *bug = calloc(1,sizeof(BugInstance));
    return bug;
}


///////////////////////////////free summaries///////////////////////////////////
int DeleteBugSummary(BugSummary *bugSummary){
    BugSummary *cur = bugSummary;
    BugSummary *prev = NULL;
    while (cur != NULL) {
	prev = cur;
	cur = cur->next;
	xmlFree((xmlChar *) prev->code);
	xmlFree((xmlChar *) prev->group);
	free(prev);
    }
    return 0;
}

int DeleteMetricSummary(MetricSummary *metricSummary){
    MetricSummary *cur = metricSummary;
    MetricSummary *prev = NULL;
    while (cur != NULL) {
	prev = cur;
	cur = cur->next;
	xmlFree((xmlChar *) prev->type);
	free(prev);
    }
    return 0;
}


///////////////////////////////Free initial struct//////////////////////////////////
int DeleteInitial(Initial *initial){
    xmlFree((xmlChar *) initial->tool_name);   
    xmlFree((xmlChar *) initial->tool_version);   
    xmlFree((xmlChar *) initial->uuid);   
    free(initial);
    return 0;
}


///////////////////////////////Free a Metric///////////////////////////////////
int DeleteMetric(Metric *metric) 
{
    free(metric->type);
    free(metric->className);
    free(metric->methodName);
    free(metric->sourceFile);
    free(metric->value);
    free(metric);
    return 0;
}


///////////////////////////////Free a BugInstance///////////////////////////////////
int DeleteBug(BugInstance *bug) 
{
    free(bug->assessmentReportFile);
    free(bug->buildId);
    free(bug->bugCode);
    free(bug->bugRank);
    free(bug->className);
    free(bug->bugSeverity);
    free(bug->bugGroup);
    free(bug->bugMessage);
    free(bug->resolutionSuggestion);
    free(bug->instanceLocation.xPath);

//    CweIds *cwe = bug->cweIds;
//    CweIds *prevCwe;
//    while (cwe != NULL) {
//	prevCwe = cwe;
//	cwe = cwe->next;
//	free(prevCwe);
//    }
    if (bug->cweIds != NULL){
	free(bug->cweIds->cweids);
	free(bug->cweIds);
    }
    
    if (bug->cweIds != NULL){
	int i;
	for ( i = 0; i < bug->methods->count; i++ ) {
	    free(bug->methods->methods[i].name);
	}
	free(bug->methods->methods);
	free(bug->methods);
    }

    if ( bug->bugLocations != NULL) {
	int i;
	for ( i = 0; i < bug->bugLocations->count; i++ ) {
	    free(bug->bugLocations->locations[i].explanation);
	    free(bug->bugLocations->locations[i].sourceFile);
	}
	free(bug->bugLocations->locations);
	free(bug->bugLocations);
    }
//    Method *method = bug->methods;
//    Method *prevMethod;
//    while (method != NULL) {
//	prevMethod = method;
//	method = method->next;
//	free(prevMethod->name);
//	free(prevMethod);
//    }

//    Location *loc = bug->bugLocations;
//    Location *prevLoc;
//    while (loc != NULL) {
//	prevLoc = loc;
//	loc = loc->next;	
//	free(prevLoc->sourceFile);
//	free(prevLoc->explanation);
//	free(prevLoc);
//    }

    free(bug);
    return 0;
}


////////////////////////COPY DATA//////////////////////////////////////////////
BugInstance *CopyBug(BugInstance *bug) {
    BugInstance *ret = calloc(1, sizeof(BugInstance));
    ret->bugId = bug->bugId;
    if (bug->cweIds != NULL) {
	ret->cweIds = malloc(sizeof(CweIds));
	ret->cweIds->size = bug->cweIds->size;
	ret->cweIds->count = bug->cweIds->count;
	ret->cweIds->cweids = malloc(ret->cweIds->size * sizeof(int));
	memcpy(ret->cweIds->cweids, bug->cweIds->cweids, bug->cweIds->size * sizeof(int));
    }

    ret->instanceLocation = bug->instanceLocation;
    if (bug->className != NULL) {
	ret->className =  malloc(strlen(bug->className) + 1);
	strcpy(ret->className, bug->className);
    }
    if (bug->cweIds != NULL) {
	ret->bugSeverity = malloc(strlen(bug->bugSeverity) + 1);
	strcpy(ret->bugSeverity, bug->bugSeverity);
    }
    if (bug->bugRank != NULL) {
	ret->bugRank = malloc(strlen(bug->bugRank) + 1);
	strcpy(ret->bugRank, bug->bugRank);
    }
    if (bug->resolutionSuggestion != NULL) {
	ret->resolutionSuggestion = malloc(strlen(bug->resolutionSuggestion) + 1);
	strcpy(ret->resolutionSuggestion, bug->resolutionSuggestion);
    }
    if (bug->cweIds != NULL) {
	ret->bugMessage = malloc(strlen(bug->bugMessage) + 1);
	strcpy(ret->bugMessage, bug->bugMessage);
    }
    if (bug->cweIds != NULL) {
	ret->bugCode = malloc(strlen(bug->bugCode) + 1);
	strcpy(ret->bugCode, bug->bugCode);
    }
    if (bug->cweIds != NULL) {
	ret->bugGroup =  malloc(strlen(bug->bugGroup) + 1 );
	strcpy(ret->bugGroup, bug->bugGroup);
    }
    if (bug->cweIds != NULL) {
	ret->assessmentReportFile = malloc(strlen(bug->assessmentReportFile) + 1);
	strcpy(ret->assessmentReportFile, bug->assessmentReportFile);
    }
    if (bug->cweIds != NULL) {
	ret->buildId = malloc(strlen(bug->buildId) + 1);
	strcpy(ret->buildId, bug->buildId);
    }
    if (bug->cweIds != NULL) {
	ret->methods = malloc(sizeof(Methods));
	ret->methods->size = bug->methods->size;
	ret->methods->count = bug->methods->count;
	ret->methods->methods =  malloc(ret->methods->size * sizeof(Method));
	int i;
	for ( i = 0; i < ret->methods->count; i++ ) {
	    ret->methods->methods[i].methodId = bug->methods->methods[i].methodId;
	    ret->methods->methods[i].primary = bug->methods->methods[i].primary;
	    ret->methods->methods[i].name =  malloc(strlen(bug->methods->methods[i].name) + 1);
	    strcpy(ret->methods->methods[i].name, bug->methods->methods[i].name);
	}
    }
    if (bug->cweIds != NULL) {
	ret->bugLocations =  malloc(sizeof(BugLocations));
	ret->bugLocations->size = bug->bugLocations->size;
	ret->bugLocations->count = bug->bugLocations->count;
	ret->bugLocations->locations =  malloc(ret->bugLocations->size * sizeof(Location));
	for ( i = 0; i < ret->bugLocations->count; i++ ) {
	    Location * retloc = &ret->bugLocations->locations[i];
	    Location * bugloc = &bug->bugLocations->locations[i];
	    retloc->locationId = bugloc->locationId;
	    retloc->primary =  bugloc->primary;
	    retloc->startLine = bugloc->startLine;
	    retloc->endLine = bugloc->endLine;
	    retloc->startColumn = bugloc->startColumn;
	    retloc->endColumn = bugloc->endColumn;
	    retloc->explanation =  malloc(strlen(bugloc->explanation) + 1);
	    strcpy(retloc->explanation, bugloc->explanation);
	    retloc->sourceFile =  malloc(strlen(bugloc->sourceFile) + 1);
	    strcpy(retloc->sourceFile, bugloc->sourceFile);
	}
    }
    return ret;
}


Metric *CopyMetric(Metric *metr) {
    Metric *ret = calloc(1, sizeof(Metric));
    ret->id = metr->id;
    if ( metr->value != NULL) {
	ret->value = malloc(strlen(metr->value) + 1);
	strcpy(ret->value, metr->value);
    }
    if ( metr->className != NULL) {
	ret->className = malloc(strlen(metr->className) + 1);
	strcpy(ret->className, metr->className);
    }
    if (metr->methodName != NULL) {
	ret->methodName = malloc(strlen(metr->methodName) + 1);
	strcpy(ret->methodName, metr->methodName);
    }
    if ( metr->sourceFile != NULL) {
    ret->sourceFile = malloc(strlen(metr->sourceFile) + 1);
    strcpy(ret->sourceFile, metr->sourceFile);
    }
    if (metr->type != NULL){
	ret->type = malloc(strlen(metr->type) + 1);
	strcpy(ret->type, metr->type);
    }
    return ret;   
}

Initial *CopyInitial(Initial *init) {
    Initial *ret = calloc(1, sizeof(Initial)); 
    if (init->tool_version != NULL) {
	ret->tool_version =  malloc(strlen(init->tool_version) + 1);
	strcpy(ret->tool_version, init->tool_version);
    }
    if ( init->tool_name != NULL ) {
	ret->tool_name =  malloc(strlen(init->tool_name) + 1);
	strcpy(ret->tool_name, init->tool_name);
    }
    if (init->uuid != NULL) {
	ret->uuid = malloc(strlen(init->uuid) + 1);
	strcpy(ret->uuid, init->uuid);
    }
    return ret;
}


//////////////////Generic clear leading/trailing whitespace method//////////////////////
char *trim(char *str)
{
    char *end;
    while ( isspace(*str) ) {
	str++;
    }
    if ( *str == 0 ) {
	return str;
    }
    end = str + strlen(str) - 1;
    while ( end > str && isspace(*end) ) {
	end--;
    }
    *(end+1) = '\0';
    char *newStr = malloc(strlen(str) + 1);
    strcpy(newStr, str);
    return newStr;
}


////////////////////parse singular line of metric  file////////////////////////////////
int processMetric(xmlTextReaderPtr reader, Metric *metric)
{ 
    char *name = (char *) xmlTextReaderName(reader);
    int type = xmlTextReaderNodeType(reader);
    
    //type 1 == start tag
    if (type == 1) { 	
	if (strcmp(name, "Metric") == 0) {
	    char *temp = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "id");
	    metric->id = strtol(temp, NULL, 10);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "Value") == 0) { 
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    metric->value = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "Type") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    metric->type = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "Class") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    metric->className = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "Method") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    metric->methodName = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "SourceFile") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    metric->sourceFile = trim(temp);
	    xmlFree((xmlChar *) temp);
	}

    //type 15 == end tag
    } else if (type == 15 && strcmp(name, "Metric") == 0) {
	xmlFree((xmlChar *) name);
	return 1;
    }

    xmlFree((xmlChar *) name);
    return 0;
}


////////////////////parse singular line of bug file////////////////////////////////////
int processBug(xmlTextReaderPtr reader, BugInstance *bug) 
{
    char *name = (char *) xmlTextReaderName(reader);
    int type = xmlTextReaderNodeType(reader);
    
    //start tags    
    if (type == 1) { 

	if (strcmp(name, "BugInstance") == 0) {
	    char *temp = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "id");
	    bug->bugId = strtol(temp, NULL, 10);
	    xmlFree((xmlChar *) temp);

//	} else if (strcmp(name, "InstanceLocation") == 0) {
//	    InstanceLocation *inst = calloc(1, sizeof(InstanceLocation));
//	    bug->instanceLocation = inst;

	} else if (strcmp(name, "Xpath") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    bug->instanceLocation.xPath = trim(temp);
	    xmlFree((xmlChar *) temp);

//	} else if (strcmp(name, "LineNum") == 0) {
//	    LineNum lineNum = {0};
//	    bug->instanceLocation->lineNum = lineNum;

	} else if (strcmp(name, "Start") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader);
	    bug->instanceLocation.lineNum.start = strtol(temp, NULL, 10);
	    xmlFree((xmlChar *) temp);

	} else if (strcmp(name, "End") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader);
	    bug->instanceLocation.lineNum.end = strtol(temp, NULL, 10);
	    xmlFree((xmlChar *) temp);

	}  else if (strcmp(name, "CweId") == 0) {

	    if ( bug->cweIds == NULL ) {
		bug->cweIds = malloc(sizeof(CweIds));
		bug->cweIds->size = 5;
		bug->cweIds->count = 0;
		bug->cweIds->cweids = malloc(bug->cweIds->size * sizeof(int));
	    }
	    if ( bug->cweIds->count >= bug->cweIds->size ) {
		bug->cweIds->size = bug->cweIds->size * 2;
		int *tempArray = realloc(bug->cweIds->cweids, bug->cweIds->size * sizeof(int));
		if (tempArray) {
		     bug->cweIds->cweids = tempArray;
		} else {
		    printf("Could not expand CweID array. Exiting parsing");
		    exit(1);
		}	
	    } 
//	    CweIds *cweid = calloc(1, sizeof(CweIds));
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader);
	    bug->cweIds->cweids[bug->cweIds->count] = strtol(temp, NULL, 10);
	    bug->cweIds->count++;
	    xmlFree((xmlChar *) temp);
	    
//	    if (bug->cweIds == NULL) {
//		bug->cweIds = cweid;
///	    } else {
//		CweIds * cur = bug->cweIds;
//		while (cur->next != NULL) {
//		    cur = cur->next;
//		}
//		cur->next = cweid;
//	    }

    	} else if (strcmp(name, "Method") == 0){
	    if ( bug->methods == NULL ) {
		bug->methods = malloc(sizeof(Methods));
		bug->methods->size = 5;
		bug->methods->count = 0;
		bug->methods->methods = calloc(1, bug->methods->size * sizeof(Method));
	    }
	    if ( bug->methods->count >= bug->methods->size ) {
		bug->methods->size = bug->methods->size * 2;
		int *tempArray = realloc(bug->methods->methods, bug->methods->size * sizeof(Methods));
		if (tempArray) {
		     bug->methods->methods = (Method *)tempArray;
		     memset(&bug->methods->methods[bug->methods->count], 0, (bug->methods->size/2) * sizeof(Method));
		} else {
		    printf("Could not expand Methods array. Exiting parsing");
		    exit(1);
		}	
	    } 
	    Method *method = &bug->methods->methods[bug->methods->count];
	    char *temp = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "id");
	    method->methodId = strtol(temp, NULL, 10);  
	    xmlFree((xmlChar *) temp);
	    temp = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "primary");
	    if (strcmp(temp, "true") == 0) {
		method->primary = 1;
	    } else {
		method->primary = 0;
	    }
	    xmlFree((xmlChar *) temp);
	    temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    method->name = trim(temp);
	    xmlFree((xmlChar *) temp);
	    bug->methods->count++;

	} else if (strcmp(name, "Location") == 0) {
	    if ( bug->bugLocations == NULL ) {
		bug->bugLocations = malloc(sizeof(BugLocations));
		bug->bugLocations->size = 5;
		bug->bugLocations->count = 0;
		bug->bugLocations->locations = calloc(1, bug->bugLocations->size * sizeof(Location));
	    }
	    if ( bug->bugLocations->count >= bug->bugLocations->size ) {
		bug->bugLocations->size = bug->bugLocations->size * 2;
		int *tempArray = realloc(bug->bugLocations->locations, bug->bugLocations->size * sizeof(Location));
		if (tempArray) {
		     bug->bugLocations->locations = (Location *)tempArray;
		     memset(&bug->bugLocations->locations[bug->bugLocations->count], 0, (bug->bugLocations->size/2) * sizeof(Location));
		} else {
		    printf("Could not expand Locations  array. Exiting parsing");
		    exit(1);
		}	
	    } 
	    //Location *loc = calloc(1, sizeof(Location));
	    Location *loc = &bug->bugLocations->locations[bug->bugLocations->count];
	    char *temp = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "id");
	    loc->locationId = strtol(temp, NULL, 10);
	    xmlFree((xmlChar *) temp);
	    temp = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "primary");
	    printf("%s\n", temp);
	    if (strcmp(temp, "true") == 0) {
		loc->primary = 1;
    	    } else {
		loc->primary = 0;
	    }
	    xmlFree((xmlChar *) temp);
	    bug->bugLocations->count++;

	} else if (strcmp(name,"StartLine") == 0) {   
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader);
	    Location * cur = &bug->bugLocations->locations[bug->bugLocations->count-1];
	    cur->startLine = strtol(temp, NULL, 10);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name,"EndLine") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader);
	    Location * cur = &bug->bugLocations->locations[bug->bugLocations->count-1];
	    cur->endLine = strtol(temp, NULL, 10);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name,"StartColumn") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader);
	    Location * cur = &bug->bugLocations->locations[bug->bugLocations->count-1];
	    cur->startColumn = strtol(temp, NULL, 10);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name,"EndColumn") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader);
	    Location * cur = &bug->bugLocations->locations[bug->bugLocations->count-1];
	    cur->endColumn = strtol(temp, NULL, 10);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name,"Explanation") == 0) {
	    Location * cur = &bug->bugLocations->locations[bug->bugLocations->count-1];
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    cur->explanation = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name,"SourceFile") == 0) {
	    Location * cur = &bug->bugLocations->locations[bug->bugLocations->count-1];
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    cur->sourceFile = trim(temp);
	    xmlFree((xmlChar *) temp);

	} else if (strcmp(name, "AssessmentReportFile") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    bug->assessmentReportFile = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "BuildId") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    bug->buildId = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "BugCode") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    bug->bugCode = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "BugRank") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    bug->bugRank = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "ClassName") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    bug->className = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "BugSeverity") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    bug->bugSeverity = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "BugGroup") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    bug->bugGroup = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "BugMessage") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    bug->bugMessage = trim(temp);
	    xmlFree((xmlChar *) temp);
	} else if (strcmp(name, "ResolutionSuggestion") == 0) {
	    char *temp = (char *) xmlTextReaderReadInnerXml(reader); 
	    bug->resolutionSuggestion = trim(temp);
	    xmlFree((xmlChar *) temp);
	}
    
    //end tags
    } else if (type == 15) {	
	if (strcmp(name, "BugInstance") == 0) {
	    xmlFree((xmlChar *) name);
	    return 1;
	} 
    }

    xmlFree((xmlChar *) name); 
    return 0;
}


//////////////////////change filename/reset parser////////////////////////////////////////

ScarfXmlReader *NewScarfXmlReaderFromFile(char *filename, char *encoding)
{
    struct Callback *calls= calloc(1, sizeof(struct Callback));
    ScarfXmlReader *reader = malloc(sizeof(ScarfXmlReader));
    reader->reader = xmlReaderForFile(filename, encoding, 0);
    reader->callback = calls;
    return reader;
}
ScarfXmlReader *NewScarfXmlReaderFromString(char *str, char *encoding)
{
    struct Callback *calls= calloc(1, sizeof(struct Callback));
    ScarfXmlReader *reader = malloc(sizeof(ScarfXmlReader));
    reader->reader = xmlReaderForDoc((xmlChar *)str, NULL, encoding, 0);
    reader->callback = calls;
    return reader;
}
ScarfXmlReader *NewScarfXmlReaderFromFd(int fd, char *encoding)
{
    struct Callback *calls= calloc(1, sizeof(struct Callback));
    ScarfXmlReader *reader = malloc(sizeof(ScarfXmlReader));
    reader->reader = xmlReaderForFd(fd, NULL, encoding, 0);
    reader->callback = calls;
    return reader;
}
ScarfXmlReader *NewScarfXmlReaderFromMemory(char *loc, int size, char *encoding)
{
    struct Callback *calls= calloc(1, sizeof(struct Callback));
    ScarfXmlReader *reader = malloc(sizeof(ScarfXmlReader));
    reader->reader = xmlReaderForMemory(loc, size, NULL, encoding, 0);
    reader->callback = calls;
    return reader;
}



void SetBugCallback(ScarfXmlReader *reader, BugCallback callback) {
    reader->callback->bugCall = callback;
}
void SetMetricCallback(ScarfXmlReader *reader, MetricCallback callback) {
    reader->callback->metricCall = callback;
}
void SetBugSummaryCallback(ScarfXmlReader *reader, BugSummaryCallback callback) {
    reader->callback->bugSumCall = callback;
}
void SetMetricSummaryCallback(ScarfXmlReader *reader, MetricSummaryCallback callback) {
    reader->callback->metricSumCall = callback;
}
void SetFinalCallback(ScarfXmlReader *reader, FinalCallback callback) {
    reader->callback->finishCallback = callback;
}
void SetInitialCallback(ScarfXmlReader *reader, InitialCallback callback) {
    reader->callback->initialCall = callback;
}
void SetCallbackData(ScarfXmlReader *reader, void *callbackData) {
    reader->callback->CallbackData = callbackData;
}


BugCallback GetBugCallback(ScarfXmlReader *reader, BugCallback callback) {
    return reader->callback->bugCall;
}
MetricCallback GetMetricCallback(ScarfXmlReader *reader, MetricCallback callback) {
    return reader->callback->metricCall;
}
BugSummaryCallback GetBugSummaryCallback(ScarfXmlReader *reader, BugSummaryCallback callback) {
    return reader->callback->bugSumCall;
}
MetricSummaryCallback GetMetricSummaryCallback(ScarfXmlReader *reader, MetricSummaryCallback callback) {
    return reader->callback->metricSumCall;
}
FinalCallback GetFinalCallback(ScarfXmlReader *reader, FinalCallback callback) {
    return reader->callback->finishCallback;
}
InitialCallback GetInitialCallback(ScarfXmlReader *reader, InitialCallback callback) {
    return reader->callback->initialCall;
}
void *GetCallbackData(ScarfXmlReader *reader, void *callbackData) {
    return reader->callback->CallbackData;
}

int _clearBug(BugInstance * bug) {
    free(bug->assessmentReportFile);
    free(bug->buildId);
    free(bug->bugCode);
    free(bug->bugRank);
    free(bug->className);
    free(bug->bugSeverity);
    free(bug->bugGroup);
    free(bug->bugMessage);
    free(bug->resolutionSuggestion);
    free(bug->instanceLocation.xPath);
    if (bug->cweIds != NULL){
	free(bug->cweIds->cweids);
    }
    free(bug->cweIds);
    

    int i;
    if ( bug->methods != NULL ) {
	for ( i = 0; i < bug->methods->count; i++ ) {
	    free(bug->methods->methods[i].name);
	}
    
	free(bug->methods->methods);
	bug->methods->count = 0;
	free(bug->methods);
    }
    for ( i = 0; i < bug->bugLocations->count; i++ ) {
        free(bug->bugLocations->locations[i].explanation);
        free(bug->bugLocations->locations[i].sourceFile);

    }
    bug->bugLocations->count = 0;

    free(bug->bugLocations->locations);
    free(bug->bugLocations);

    memset(bug, 0, sizeof(BugInstance));
    return 0;
}

int  _clearMetric(Metric * metric) {
    free(metric->type);
    free(metric->className);
    free(metric->methodName);
    free(metric->sourceFile);
    free(metric->value);
    memset(metric, 0, sizeof(Metric));
    return 0;
}

void * Parse(ScarfXmlReader *hand)
{
    void *kill = NULL;
    xmlTextReaderPtr reader = hand->reader;
    if (reader != NULL) {
	char *name;
	int finished = 0;
	int type;
	int ret = 1;
	BugInstance *bug = initializeBug();
	Metric *metric = initializeMetric();
	Callback *callback = hand->callback;
        while (ret == 1 && kill == NULL) {
	    ret = xmlTextReaderRead(reader);
	    name = (char *) xmlTextReaderName(reader);
	    type = xmlTextReaderNodeType(reader); 
	    if ( type == 1 ) {
	        if ( strcmp ( name, "AnalyzerReport" ) == 0 && callback->initialCall != NULL ) {
		    Initial *initial = calloc(1, sizeof(Initial));
		    initial->tool_name = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "tool_name");
		    initial->tool_version = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "tool_version");
		    initial->uuid = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "uuid");
		    kill = callback->initialCall(initial, callback->CallbackData);
		    DeleteInitial(initial);

	        } else if ( strcmp ( name, "BugInstance" ) == 0 && callback->bugCall != NULL ) {
		    int foundBug = 0;
		    //iBugInstance *bug = initializeBug();
		    while (ret == 1 && foundBug == 0) {
			foundBug = processBug(reader, bug);
			if (foundBug == 0) {
			    ret = xmlTextReaderRead(reader);
			}
		    }
		    if (foundBug == 1) {
			kill = callback->bugCall(bug, callback->CallbackData);
		    }
		    _clearBug(bug);

	        } else if ( strcmp ( name, "Metric" ) == 0 && callback->metricCall != NULL ) {
		    //Metric *metric = initializeMetric();
		    int foundMetric = 0;
		    while (ret == 1 && foundMetric == 0) {
			foundMetric = processMetric(reader, metric);
			if (foundMetric == 0) {
			    ret = xmlTextReaderRead(reader);
			}	
		    }
		    if (foundMetric == 1) {
			kill = callback->metricCall(metric, callback->CallbackData);
		    }
		    _clearMetric(metric);

	        } else if ( strcmp ( name, "BugSummary" ) == 0 && callback->bugSumCall !=NULL ) {
		    BugSummary *bugsum = NULL;
		    int finSummary = 0;
		    while (ret == 1 && !finSummary) {
			ret = xmlTextReaderRead(reader);
			name = (char *) xmlTextReaderName(reader);
			type = xmlTextReaderNodeType(reader); 
			if (type == 1) {
			    if (strcmp(name, "BugCategory") == 0) {
				BugSummary *temp = calloc(1, sizeof(BugSummary));
				char *att = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "count");
				temp->count = strtol(att, NULL, 10);
				xmlFree((xmlChar *) att);
				att = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "bytes");
				temp->byteCount = strtol(att, NULL, 10);
				xmlFree((xmlChar *) att);
				temp->code = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "code");
				temp->group = (char *) xmlTextReaderGetAttribute(reader, (xmlChar *) "group");
				if ( bugsum == NULL ) {
				    bugsum = temp;
				} else {
				    BugSummary *curr = bugsum;
				    while (curr->next != NULL) {
					curr = curr->next;
				    }
				    curr->next = temp;
				}
			    }
			} else if (type == 15 && strcmp(name, "BugSummary") == 0) {
			    finSummary = 1;
			}
		    }
		    if ( finSummary == 1 ) {
			kill = callback->bugSumCall(bugsum, callback->CallbackData);
			DeleteBugSummary(bugsum);
		    }

	        } else if ( strcmp ( name, "MetricSummaries") == 0 && callback->metricSumCall != NULL ) {
		    MetricSummary *metricsum = NULL;
		    MetricSummary *temp = NULL;
		    int finSummary = 0;
		    while (ret == 1 && !finSummary) {
			ret = xmlTextReaderRead(reader);
			name = (char *) xmlTextReaderName(reader);
			type = xmlTextReaderNodeType(reader); 
			if (type == 1) {
			    if ( strcmp(name, "MetricSummary") == 0 ) {
				temp = calloc(1, sizeof(MetricSummary));
			    } else if ( strcmp(name, "Type") == 0 ) {
				temp->type = (char *) xmlTextReaderReadInnerXml(reader);
			    } else if ( strcmp(name, "Count") == 0 ) {
				char *text = (char *) xmlTextReaderReadInnerXml(reader);
				temp->count = strtod(text, NULL);
				xmlFree((xmlChar *) text);
			    } else if ( strcmp(name, "Minimum") == 0 ) {
				char *text = (char *) xmlTextReaderReadInnerXml(reader);
				temp->min = strtod(text, NULL);
				xmlFree((xmlChar *) text);
			    } else if ( strcmp(name, "Maximum") == 0 ) {
				char *text = (char *) xmlTextReaderReadInnerXml(reader);
				temp->max = strtod(text, NULL);
				xmlFree((xmlChar *) text);
			    } else if ( strcmp(name, "Average") == 0 ) {
				char *text = (char *) xmlTextReaderReadInnerXml(reader);
				temp->average = strtod(text, NULL);
				xmlFree((xmlChar *) text);
			    } else if ( strcmp(name, "StandardDeviation") == 0 ) {
				char *text = (char *) xmlTextReaderReadInnerXml(reader);
				temp->stdDeviation = strtod(text, NULL);
				xmlFree((xmlChar *) text);
			    } else if ( strcmp(name, "Sum") == 0 ) {
				char *text = (char *) xmlTextReaderReadInnerXml(reader);
				temp->sum = strtod(text, NULL);
				xmlFree((xmlChar *) text);
			    } else if ( strcmp(name, "SumOfSquares") == 0 ) {
				char *text = (char *) xmlTextReaderReadInnerXml(reader);
				temp->sumOfSquares = strtod(text, NULL);
				xmlFree((xmlChar *) text);
			    }
			} else if ( type == 15 ) {
			    if ( strcmp(name, "MetricSummary") == 0 ) {
				if ( metricsum == NULL ) {
                                    metricsum = temp;
                                } else {
                                    MetricSummary *curr = metricsum;
                                    while (curr->next != NULL) {
                                        curr = curr->next;
                                    }
                                    curr->next = temp;
				}
			    } else if ( strcmp(name, "MetricSummaries") == 0 ) {
				kill = callback->metricSumCall(metricsum, callback->CallbackData);
				DeleteMetricSummary(metricsum);
				finSummary = 1;
			    } else if ( strcmp(name, "AnalyzerReport") == 0 ) {
				if ( callback->finishCallback != NULL ) {
				    kill = callback->finishCallback(kill, callback->CallbackData);
				    finished = 1;
				}

			    }
			}
		    }
	        }
	    }
	}
	if (ret != 0) {
	    printf("Failed to parse set file\n");
	    return NULL;
	} else if ( kill != NULL && !finished ) {
	    if ( callback->finishCallback != NULL ) {
		kill = callback->finishCallback(kill, callback->CallbackData);
	    }
	}
	
    } else {
	printf("ScarfXmlReader set to invalid file\n");
	return NULL;
    } 
    return kill;
}



//////////////////Close parser////////////////////////////////////////////
int DeleteScarfXmlReader(ScarfXmlReader *reader)
{
    return xmlTextReaderClose(reader->reader);
}



