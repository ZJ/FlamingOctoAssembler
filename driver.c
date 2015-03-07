#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symbolHashTable/symbolHashTable.h"
#include "commandTable/cmdTableFormat.h"
#include "commandTable/cmdTableTest.h"
#include "errs/parserErrs.h"

#include "testFile.h"

#define	NUM_CORES 1

// This doesn't frigging work
#define TEST_CORE_CMD_LOOKUP getCmdTest
// Define up to the right number of cores

cmdFindFunct_ptr cmdLookupHandles[NUM_CORES] = {&TEST_CORE_CMD_LOOKUP};


void printSymbol(symbol_ptr toPrint) {
	printf("\t @ 0x%08x %s \t%c \t%d \t0x%08x\n\r", toPrint, toPrint->name, toPrint->type, toPrint->locCount, toPrint->next);
}

void printChain(symbol_ptr head) {
	printSymbol(head);
	while ( (head = head->next) != NULL ) printSymbol(head);
}

void printHashTable(symbol_ptr * symbolTable) {
	red_hash_t i;
	puts("Printing table:\n\r");
	for (i=0; i<SYMBOL_TABLE_SIZE; i++) {
		printf("%d:\t0x%08x\n\r", i, symbolTable[i]);
		if (symbolTable[i] != NULL) printChain(symbolTable[i]);
	}
}

#define BIT(n) ((lfsrState >> (n)) & 0x01)
char lfsrChar (unsigned int seed) {
	static unsigned int lfsrState;
	unsigned int fbBit = 0;
	if (seed != ((unsigned int) -1) ) lfsrState = seed;
	fbBit = BIT(0) ^ BIT(31) ^ BIT(19) ^ BIT(11) ^ BIT(3);
	lfsrState = (lfsrState << 1) & 0xFFFFFFFE | fbBit;
	
	return lfsrState % 93 + 33;
}

static char * lfsrWord (unsigned int seed) {
	static char word[9] = "\0\0\0\0\0\0\0\0\0";
	static char i;
	for (i=0; i<8; i++) word[i] = lfsrChar(seed);
	return word;
}

cmdEntry_ptr findCommand(const char * labelName) {
	cmdEntry_ptr theCmd = NULL;
	unsigned int length = strlen(labelName);
	theCmd = (*cmdLookupHandles)(labelName, length);
	if ( theCmd == (cmdEntry_ptr) 0 ) return NULL;
	return theCmd;
}

cmdEntry_ptr processCmd(const char * cmdStr, char * errMsg, unsigned long * locCount, unsigned long * bramOff) {
	cmdEntry_ptr theCmd = findCommand(cmdStr);
	if ( theCmd == NULL ) {
		strcpy(errMsg, ERR_PARSE_CMD_NOT_FOUND);
		return NULL;
	}
	return theCmd;
}

/*!	\brief	Handles pass 1 tasks for a passed line of the assembly file.
 *	\details - Searches for label definitions to build the symbol table, and checks that none a multiply defined.
 *			 - Also checks for literal definitions and creates those
 *			 - Handles other assembler directives
 *			 - Looks up commands strings and keeps track of total memory size so it can be allocated for the second pass.
 *			 - Checks total argument count to match with the command
 *	\todo	Define processCmd(const char * cmdStr, char * errMsg, unsigned long * locCount, unsigned long * bramOff);
 *	\returns	NULL on success
 *	\returns	pointer to error description on failure (for printing)
 */
char * processLinePass1(char * lineBuffer, int lineLength, symbolTab_t symbolTable, unsigned long * locCount, unsigned long * bramOff) {
	static char labelStr[256] = "";
	static char cmdStr[9] = "";
	static char errMsg[256] = "";
	static char * buffPos = NULL;
	static cmdEntry_ptr	thisCmd = NULL;
	static int i = 0;
	static int argCnt = 0;
	
	while ( isspace(*lineBuffer) ) lineBuffer++; // Skip leading space, if any
	if (*lineBuffer == '\0') return NULL; // Empty line!
	
	// Find closing semicolon, or report error
	buffPos = strchr(lineBuffer, COMMENT_START);
	if ( buffPos == NULL ) {
		strcpy(errMsg, ERR_PARSE_NO_END);
		return errMsg;
	} else {
		*buffPos = '\0';
	}
	
	// Find label delimiter, if any
	buffPos = strchr(lineBuffer, LABEL_END);
	if ( buffPos != NULL ) {
		symbol_ptr	thisLabel	= NULL;
		
		// First non-whitespace is label separator
		if ( buffPos == lineBuffer ) {
			strcpy(errMsg, ERR_PARSE_LBL_EMPTY);
			return errMsg;
		}
		
		*buffPos = '\0'; // Switch the label sep. to NULL char
		strcpy(labelStr, lineBuffer);
		lineBuffer = ++buffPos;
		
		if (defineLabel(labelStr, errMsg, symbolTable, *locCount, *bramOff) != 0) return errMsg; //Problem defining the label
		
		while ( isspace(*lineBuffer) ) lineBuffer++; // Skip whitespace between label and cmd
	}
	
	// Should be at the beginning of the command by here, so go until whitespace
	*cmdStr = '\0';
	buffPos = lineBuffer;
	for (i=0; i < MAX_CMD_LEN; i++) {
		// Process Line for command Here
		if ( isspace(*lineBuffer) || ((*lineBuffer) == '\0') ) {
			*lineBuffer = '\0';
			strcpy(cmdStr, buffPos);
			lineBuffer++;// Increment past the null we just inserted
			break;
		}
		lineBuffer++;
	}

	if ( cmdStr[0] != '\0' ) {
		thisCmd = processCmd(cmdStr, errMsg, locCount, bramOff);
		if ( thisCmd == NULL ) return errMsg; // Problem looking up command. Report the error
	} else {
		strcpy(errMsg, ERR_PARSE_CMD_LONG);
		return errMsg;
	}

	// If we're here, the command entry is stashed in thisCmd, so we can keep processing
	*locCount   += thisCmd->numLines;
	*bramOff    += (thisCmd->numLines * CMD_BYTES);
	

	// Check for correct number of arguments
	// (don't bother resolving symbols and literals at this time, that is in pass 2)
	// In other words, don't even look at the content of the stuff, just count.
	while ( isspace(*lineBuffer) ) lineBuffer++; // Skip any whitespace before the first argument

	argCnt = 0;
	if (*lineBuffer != '\0') {
		argCnt = 1;
		buffPos = strchr(lineBuffer, ARG_DELIMIT);
		while ( buffPos != NULL ) {
			argCnt++;
			buffPos = strchr(++buffPos, ARG_DELIMIT);
		}
	}
	
	if ( argCnt != thisCmd->numArgs) {
		strcpy(errMsg, ERR_PARSE_ARG_CNT);
		return errMsg;
	}
	
	// If we're done, it is success, return NULL
	return NULL;
}

/*	\brief	Takes a label name and checks if it has a valid format.
 *
 *	\todo	Needs a check against known commands via findCommand()
 *
 *	\returns  0 on success
 *	\returns -1 on failure
 */
int validLabel(const char * labelName, char * errMsg) {
	int i = 1;
	char testChar = '\0';
	
	if ( !isalpha(labelName[0]) ) {
		strcpy(errMsg, ERR_PARSE_LBL_LETTR);
		return -1;
	}
	
	while ( (testChar = labelName[i++]) != '\0' ) {
		if ( !( isalnum(testChar) || (testChar == '_') ) ) {
			strcpy(errMsg, ERR_PARSE_LBL_BAD_CHAR);
			errMsg[strlen(errMsg)-2] = testChar;
			return -1;
		}
	}
	
	if ( findCommand(labelName) != NULL ) {
		strcpy(errMsg, ERR_PARSE_LBL_CMD_COLL);
		return -1;
	}
	
	return 0;
}

/*	\brief Takes a label name and attempts to update the symbol table
 *
 *	\returns  0 on success
 *	\returns -1 on failure
 */
int defineLabel(const char * labelName, char * errMsg, symbolTab_t symbolTable, unsigned long locCount, unsigned long bramOff) {
	symbol_ptr thisLabel = NULL;
	
	if ( validLabel(labelName, errMsg) != 0 ) return -1; // errMsg populated by validLabel, just return unsuccessfully
		
	thisLabel = findSymbol(labelName, symbolTable);
	if (thisLabel != NULL) {
		// Found an extant label
		switch(thisLabel->type) {
			case 'U':
				break;
			case 'D':
				setTypeM(thisLabel);
			case 'M':
				strcpy(errMsg, ERR_LBL_MULT_DEF);
				return -1;
				break;
			default:
				strcpy(errMsg, ERR_LBL_UNKWN_TYPE);
				return -1;
				break;
		}
	} else { //thisLabel = NULL
		// New label encountered
		thisLabel = addSymbol(labelName, symbolTable);
		if (thisLabel == NULL) {
			strcpy(errMsg, ERR_LBL_ALLOC_FAIL);
			return -1;
		}
	}
	
	// Either type 'U' or the first we've seen of it, so set the pointers appropriately
	setTypeD(thisLabel);
	thisLabel->locCount   = locCount;
	thisLabel->bramOffset = bramOff;
	return 0;
}

int main(int argc, const char* argv[]) {
	symbol_ptr * symbolTable = NULL;
	char * 		 lineBuffer = NULL;
	int			 bufferLength = 256;
	unsigned long locCount = 0;
	unsigned long bramOff  = 0;
	char *	errPtr = NULL;
	int i = 0;
	unsigned int lineCount = 0;
	char * strPtr = gTestString;
	
	lineBuffer = malloc(bufferLength + 1);
	if (lineBuffer == NULL) return -1;
	
	symbolTable = newSymbolTable();
	
	/*lfsrChar((unsigned int) symbolTable);
	for (i=0; i<250; i++) {
		char * randWord = lfsrWord((unsigned int) -1);
		printf("%s\r\n", randWord);
		addSymbol(randWord, symbolTable);
	}
	*/
	//while ( getLine(&lineBuffer, &bufferLength, stream) != -1 )
	while( *strPtr != '\0' ) {
		char * endOfLine = strPtr;
		lineCount++;
		// Building a line buffer formation unit here
		endOfLine = strchr(strPtr,'\n');
		if (endOfLine != NULL) {
			size_t len = (endOfLine-strPtr);
			strncpy(lineBuffer, strPtr, len);
			* (lineBuffer + (len)) = '\0';
			strPtr = ++endOfLine;
		} else {
			strcpy(lineBuffer, strPtr);
			strPtr = strchr(strPtr, '\0');
		}
		//printf("%4u:\t%s\n",lineCount,lineBuffer);
		errPtr = processLinePass1(lineBuffer, strlen(lineBuffer), symbolTable, &locCount, &bramOff);
		if (errPtr != NULL) printf("On line %u:\n\t%s\r\n\n", lineCount, errPtr);
		//break;		
	}
	printf("\nOrig. Buffer:\n%s\n",gTestString);

	printHashTable(symbolTable);
	
	freeSymbolTable(symbolTable);
	
	free(lineBuffer);
	lineBuffer = NULL;
}
