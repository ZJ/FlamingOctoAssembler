#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "symbolHashTable/symbolHashTable.h"
#include "commandTable/cmdTableFormat.h"
#include "commandTable/cmdTableTest.h"
#include "errs/parserErrs.h"

#include "testFile.h"

#define	NUM_CORES 1

#define TRUE	1
#define FALSE	0

#define DEBUG_PRINT	TRUE

#define START_LINE_BUFF_SIZE 128

// This doesn't frigging work
#define TEST_CORE_CMD_LOOKUP getCmdTest
// Define up to the right number of cores

cmdFindFunct_ptr cmdLookupHandles[NUM_CORES] = {&TEST_CORE_CMD_LOOKUP};

typedef enum {FIRST, SECOND} passNum_type;

typedef struct progCounters {
	unsigned long	locCount;
	unsigned long	ddrOffset;
	unsigned long	lineCount;
	passNum_type	whichPass;
} progCnt_t;

inline char * enlargeBuffer(char * buffer, unsigned int numChars) {
	if (DEBUG_PRINT) printf("Enlarging line buffer to %u\n", numChars);
	return realloc(buffer, (numChars+1)*sizeof(char));
}

void printSymbol(symbol_ptr toPrint) {
	printf("\t @ 0x%08x %s \t%c \t%d \t%d \t0x%08x\n\r", toPrint, toPrint->name, toPrint->type, toPrint->locCount, toPrint->ddrOffset, toPrint->next);
}

void printChain(symbol_ptr head) {
	printSymbol(head);
	while ( (head = head->next) != NULL ) printSymbol(head);
}

void printHashTable(symbol_ptr * symbolTable) {
	red_hash_t i;
	printf("Printing table:\n\r");
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

cmdEntry_ptr processCmd(const char * cmdStr, char * errMsg, unsigned long * locCount, unsigned long * ddrOff) {
	cmdEntry_ptr theCmd = findCommand(cmdStr);
	if ( theCmd == NULL ) {
		strcpy(errMsg, ERR_PARSE_CMD_NOT_FOUND);
		return NULL;
	}
	return theCmd;
}

char * resolveArg(char * argStr, char * errMsg, uint64_t * argVal, symbolTab_t symbolTable, progCnt_t * progCnt) {
	uint64_t tempVal = 0x0ULL;
	char * endConv = NULL;
	symbol_ptr symbolicArg = NULL;
	int useDDR = FALSE;
	
	// Kill trailing whitespace
	endConv = strchr(argStr, '\0') - 1;
	while( isspace(*endConv) ) endConv--; // Skip trailing whitespace
	*(++endConv) = '\0';
	endConv = NULL;
	
	if( isdigit(*argStr) ) { //Starts with a #, should be a constant
		tempVal = strtoull(argStr, &endConv, 0);
		if ( endConv != strchr(argStr, '\0') ) {
			// Not everything went to be a number
			strcpy(errMsg, ERR_BAD_ARG_FMT);
			return errMsg;
		} else {
			*argVal = tempVal;
			return NULL;
		}
	}
	
	// check for leading @, means we should replace w/ DDR offset
	if ( useDDR = (*argStr == '@') ) argStr++;
	
	symbolicArg = findSymbol(argStr, symbolTable);
	if (symbolicArg != NULL) { // Found the symbol
		if ( useDDR && (symbolicArg->type == 'L') ) { // Address of DDR is not allowed
			errMsg = strcpy(errMsg, ERR_LIT_DDR_LOOKUP);
			return errMsg;
		}
		// Set argVal to retrieved value.  Costs nothing, so do it always
		*argVal = useDDR ? symbolicArg->ddrOffset : symbolicArg->locCount;
		return NULL;
	} else if ( progCnt->whichPass == FIRST ) { // No symbol yet, but first pass
		symbolicArg = addSymbol(argStr, symbolTable);
		if (symbolicArg == NULL) {
			strcpy(errMsg, ERR_LBL_ALLOC_FAIL);
			return errMsg;
		}
		symbolicArg->locCount = progCnt->lineCount; // Mark down first sighting line #
		return NULL;
	}
	
	strcpy(errMsg, ERR_BAD_ARG_UNDEF);
	return errMsg;
}

directiveStatus processLitDec(char * restLine, char * errMsg, symbolTab_t litTab) {
	symbol_ptr	litPtr = NULL;
	unsigned long litVal = 0;
	char * scanPos = restLine;
	char litName[MAX_LIT_NAME_LEN+1];
	int i = 0;
	
	while( isspace(*scanPos) ) scanPos++; //Skip whitespace
	
	if( !isalpha(*scanPos) ) {
		strcpy(errMsg, ERR_LIT_LETTER);
		return ERROR;
	}
	
	for (i=0; i < MAX_LIT_NAME_LEN; i++) {
		// Process Line for command Here
		if ( isspace(*scanPos) || ((*scanPos) == '\0') ) {
			litName[i] = '\0';
			break;
		}
		litName[i] = *scanPos;
		scanPos++;
	}
	
	if ( memchr(litName, '\0', MAX_LIT_NAME_LEN+1) == NULL ) {
		strcpy(errMsg, ERR_PARSE_CMD_LONG);
		return ERROR;
	}
	
	if ( validLabel(litName, errMsg) != 0) return ERROR;
	
	litPtr = findSymbol(litName, litTab);
	if (litPtr != NULL) {
		if (litPtr->type != 'U') {
			setTypeM(litPtr);
			strcpy(errMsg, ERR_LIT_MULT_DEF);
			return ERROR;
		}
	} else {
		litPtr = addSymbol(litName, litTab);
		if (litPtr == NULL) {
			strcpy(errMsg, ERR_LBL_ALLOC_FAIL);
			return ERROR;
		}
	}
	
	litVal = strtoull(scanPos, &scanPos, 0);
	while( isspace(*scanPos) ) scanPos++; //Skip whitespace
	// If we hit anything at all besides a null char (formerly the closing semicolon
	// this is improperly formatted SETLC directive
	if ( *scanPos != '\0' ) {
		printf("Where we're at: %s", scanPos);
		strcpy(errMsg, ERR_LIT_FORMAT);
		return ERROR;
	}
	
	// If we're here, we can safely put in the value
	litPtr->locCount = litVal;
	setTypeL(litPtr);
	
	if (DEBUG_PRINT) printf("LIT \"%s\" = %u.\n", litPtr->name, litPtr->locCount);
	return DIRECTIVE;
}

directiveStatus processSetLC(const char * restLine, char * errMsg, progCnt_t * progCnt) {
	unsigned long newLC = progCnt->locCount;
	char * checkEnding = NULL;
	newLC = strtoull(restLine, &checkEnding, 0);
	
	while( isspace(*checkEnding) ) checkEnding++; //Skip whitespace
	
	// If we hit anything at all besides a null char (formerly the closing semicolon
	// this is improperly formatted SETLC directive
	if ( *checkEnding != '\0' ) {
		strcpy(errMsg, ERR_LIT_FORMAT);
		return ERROR;
	}
	
	if (DEBUG_PRINT) printf("New LC via SETLC:\t%u\n", newLC);
	progCnt->locCount = newLC;
	
	return DIRECTIVE;
}

directiveStatus processDirective(const char * cmdStr, char * restLine, char * errMsg, progCnt_t * progCnt, symbolTab_t symbolTable, int firstPass) {
	switch (*(cmdStr++)) {
		case 'L':
			if ( strcmp(cmdStr, /*L*/"IT") == 0 ) return firstPass ? processLitDec(restLine, errMsg, symbolTable) : DIRECTIVE;
			// Do more ifs for other 'L' directives
			break;
		case 'S':
			if ( strcmp(cmdStr, /*S*/"ETLC") == 0 ) return processSetLC(restLine, errMsg, progCnt);
			// Do more ifs for other 'S'
			break;
		default:
			break;
	}
	return NOTDIRECTIVE; // We've not returns from any special code, so NOTDIRECTIVE.
}

/*!	\brief	Handles pass 1 tasks for a passed line of the assembly file.
 *	\details - Searches for label definitions to build the symbol table, and checks that none a multiply defined.
 *			 - Also checks for literal definitions and creates those
 *			 - Handles other assembler directives
 *			 - Looks up commands strings and keeps track of total memory size so it can be allocated for the second pass.
 *			 - Checks total argument count to match with the command
 *	\todo	Define processCmd(const char * cmdStr, char * errMsg, unsigned long * locCount, unsigned long * ddrOff);
 *	\returns	NULL on success
 *	\returns	pointer to error description on failure (for printing)
 */
char * processLinePass1(char * lineBuffer, symbolTab_t symbolTable, progCnt_t * progCnt, uint64_t * memline, passNum_type whichPass) {
	static char labelStr[256] = "";
	static char cmdStr[9] = "";
	static char errMsg[256] = "";
	static char * buffPos = NULL;
	static cmdEntry_ptr	thisCmd = NULL;
	static int i = 0;
	static int argCnt = 0;
	static directiveStatus checkDir = NOTDIRECTIVE;
	
	while ( isspace(*lineBuffer) ) lineBuffer++; // Skip leading space, if any

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
		
		if (defineLabel(labelStr, errMsg, symbolTable, progCnt->locCount, progCnt->ddrOffset) != 0) return errMsg; //Problem defining the label
		
		while ( isspace(*lineBuffer) ) lineBuffer++; // Skip whitespace between label and cmd
	}

	if (*lineBuffer == '\0') return NULL; // Empty line!
	
	// Find closing semicolon, or report error
	buffPos = strchr(lineBuffer, COMMENT_START);
	if ( buffPos == NULL ) {
		strcpy(errMsg, ERR_PARSE_NO_END);
		return errMsg;
	} else {
		*buffPos = '\0';
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

	if ( cmdStr[0] == '\0' ) {
		strcpy(errMsg, ERR_PARSE_CMD_LONG);
		return errMsg;
	}
	
	checkDir = processDirective(cmdStr, lineBuffer, errMsg, progCnt, symbolTable, TRUE);
	if ( checkDir == DIRECTIVE ) return NULL;
	if ( checkDir == ERROR ) return errMsg;
	
	// This far means we're expecting an actual command
	thisCmd = processCmd(cmdStr, errMsg, &(progCnt->locCount), &(progCnt->ddrOffset));
	if ( thisCmd == NULL ) return errMsg; // Problem looking up command. Report the error
	

	// If we're here, the command entry is stashed in thisCmd, so we can keep processing
	progCnt->locCount	+= thisCmd->numLines;
	progCnt->ddrOffset	+= (thisCmd->numLines * CMD_BYTES);
	
	// Check for correct number of arguments
	// Also check format of arguments
	while ( isspace(*lineBuffer) ) lineBuffer++; // Skip any whitespace before the first argument

	argCnt = 0;
	while (*lineBuffer != '\0') {
		uint64_t dummyVal;
		argCnt++;
		buffPos = strchr(lineBuffer, ARG_DELIMIT);
		if (buffPos != NULL) {
			*buffPos = '\0';
			if ( resolveArg(lineBuffer, errMsg, &dummyVal, symbolTable, progCnt) != NULL) return errMsg;
			lineBuffer = ++buffPos;
			while ( isspace(*lineBuffer) ) lineBuffer++; // Skip any whitespace before the next argument
		} else {
			if ( resolveArg(lineBuffer, errMsg, &dummyVal, symbolTable, progCnt) != NULL) return errMsg;
			lineBuffer = strchr(lineBuffer, '\0');
		}
	}
	
	if ( argCnt != thisCmd->numArgs) {
		printf("%d found, %d expected.\n", argCnt, thisCmd->numArgs);
		strcpy(errMsg, ERR_PARSE_ARG_CNT);
		return errMsg;
	}
	
	// If we're done, it is success, return NULL
	return NULL;
}

/*!	\brief	Handles pass 2 tasks for a passed line of the assembly file.
 *	\details - Writes assembly into memory, if possible.
 *	\todo	Actually document this function
 *	\returns	NULL on success
 *	\returns	pointer to error description on failure (for printing)
 */
char * processLinePass2(char * lineBuffer, symbolTab_t symbolTable, progCnt_t * progCnt, uint64_t * memline, passNum_type whichPass) {
	static char labelStr[256] = "";
	static char cmdStr[9] = "";
	static char errMsg[256] = "";
	static char * buffPos = NULL;
	static cmdEntry_ptr	thisCmd = NULL;
	static int i = 0;
	static int argCnt = 0;
	static directiveStatus checkDir = NOTDIRECTIVE;
	uint64_t thisLine = 0x0UL;
	uint64_t thisMask = 0xFFFFFFFFFFFFFFFFUL;
	uint64_t argArray[MAX_ARG_COUNT] ={0};
	char * lineCopy = NULL;
	
	if (DEBUG_PRINT) {
		lineCopy = malloc((strlen(lineBuffer)+1)*sizeof(char));
		if (lineCopy != NULL) strcpy(lineCopy, lineBuffer);
		//printf("%s\n:",lineCopy);
	}
	
	while ( isspace(*lineBuffer) ) lineBuffer++; // Skip leading space, if any
	
	// Find label delimiter, if any, and skip label definition (we already read it)
	buffPos = strchr(lineBuffer, LABEL_END);
	if ( buffPos != NULL ) {
		lineBuffer = ++buffPos;
		while ( isspace(*lineBuffer) ) lineBuffer++; // Skip whitespace between label and cmd
	}

	if (*lineBuffer == '\0') return NULL; // Empty line!
	
	// Find closing semicolon, or report error
	buffPos = strchr(lineBuffer, COMMENT_START);
	if ( buffPos == NULL ) {
		strcpy(errMsg, ERR_PARSE_NO_END);
		return errMsg;
	} else {
		*buffPos = '\0';
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

	if ( cmdStr[0] == '\0' ) {
		strcpy(errMsg, ERR_PARSE_CMD_LONG);
		return errMsg;
	}
	
	checkDir = processDirective(cmdStr, lineBuffer, errMsg, progCnt, symbolTable, FALSE);
	if ( checkDir == DIRECTIVE ) return NULL;
	if ( checkDir == ERROR ) return errMsg;
	
	// This far means we're expecting an actual command
	thisCmd = processCmd(cmdStr, errMsg, &(progCnt->locCount), &(progCnt->ddrOffset));
	if ( thisCmd == NULL ) return errMsg; // Problem looking up command. Report the error
	
	// Start building the command word:
	if ( (thisCmd->flags & WR_OPCODE_MASK) != 0 ) {
		thisLine |= ( thisMask & ( ((uint64_t)thisCmd->opcode) << INST_OPCD_OFFSET ));
		thisMask &= ~INST_OPCD_MASK;
	}
	
	// If we're here, the command entry is stashed in thisCmd, so we can keep processing
	progCnt->locCount	+= thisCmd->numLines;
	progCnt->ddrOffset	+= (thisCmd->numLines * CMD_BYTES);
	
	// Resolve individual arguments.  We've already vetted them for format
	argCnt = thisCmd->numArgs;
	for(i=0; i<argCnt; i++){
		char * startArg;
		while ( isspace(*lineBuffer) ) lineBuffer++; // Skip any whitespace before the argument
		startArg = lineBuffer;
		buffPos = strchr(lineBuffer, ARG_DELIMIT);
		if (buffPos != NULL) {
			*buffPos = '\0';
			lineBuffer = buffPos + 1;
		} else {
			buffPos = strchr(lineBuffer, '\0');
		};
		buffPos--;
		while ( isspace(*buffPos) ) buffPos--; // Rewind over trailing whitespace
		*(++buffPos) = '\0';
		strcpy(labelStr, startArg);
		if( resolveArg( labelStr, errMsg, (argArray + i), symbolTable, progCnt) != NULL) return errMsg;
	}
		
	// Now put the arguments where they should go.
	// Order is Data, then Time.
	// This method is clear, but not extensible
	switch (argCnt) {
		case 0:
			// Can put in other default data, but right now it is 0's
			break;
		case 1: // Do we need to check for cases that won't happen if the cmd table is formatted properly?
			if ( (thisCmd->flags & WR_DATFLD_MASK) != 0 ) {
				thisLine |= ( thisMask & ( argArray[0] << INST_DATA_OFFSET ) );
			} else if ( (thisCmd->flags & WR_TIMFLD_MASK) != 0 ) {
				thisLine |= ( thisMask & ( argArray[0] << INST_TIME_OFFSET ) );					
			}
			break;
		case 2:
			thisLine |= ( thisMask & ( argArray[0] << INST_DATA_OFFSET ) );
			thisLine |= ( (thisMask & INST_TIME_MASK) & ( argArray[1] << INST_TIME_OFFSET ) );
			break;
		default:
			strcpy(errMsg, ERR_BAD_CMD_TABLE);
			return errMsg;
			break;
	}
	
	// We've got a command, now write the opcode to memory
	*memline = thisLine;
	if (DEBUG_PRINT) {
		if (lineCopy != NULL) {
			printf("@0x%08x:\t0x%016" PRIx64 ";\t%s\n", memline, *memline, lineCopy);
			free(lineCopy);
			lineCopy = NULL;
		} else {
			printf("@0x%08x:\t0x%016" PRIx64 ";\t<Insufficient memory for line echo>\n", memline, *memline);
		}
	}
	memline++; // This will need to be different, and memline will need to be uint64_t **
	
	// If we're done, it is success, return NULL
	return NULL;
}

/*!	\brief	Takes a label name and checks if it has a valid format.
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

/*!	\brief Takes a label name and attempts to update the symbol table
 *
 *	\returns  0 on success
 *	\returns -1 on failure
 */
int defineLabel(const char * labelName, char * errMsg, symbolTab_t symbolTable, unsigned long locCount, unsigned long ddrOff) {
	symbol_ptr thisLabel = NULL;
	
	if ( validLabel(labelName, errMsg) != 0 ) return -1; // errMsg populated by validLabel, just return unsuccessfully
		
	thisLabel = findSymbol(labelName, symbolTable);
	if (thisLabel != NULL) {
		// Found an extant label
		switch(thisLabel->type) {
			case 'U':
				break;
			case 'D':
			case 'L':
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
	thisLabel->ddrOffset = ddrOff;
	return 0;
}

char * undefinedSymbolCheck(symbolTab_t symbolTable) {
	static int i;
	static symbol_ptr thisSymbol = NULL;
	static char undefStr[MAX_LIT_NAME_LEN + 64 +1] = "";
	
	// Pickup from where we left off
	while ( i < SYMBOL_TABLE_SIZE ) {
		while ( thisSymbol != NULL ) {
			if( thisSymbol->type == 'U' ) {
				sprintf(undefStr, ERR_UNDEF_SYMB_FMT, thisSymbol->name, thisSymbol->locCount);
				thisSymbol = thisSymbol->next;
				return undefStr;
			}
			thisSymbol = thisSymbol->next;
		}
		i++;
		thisSymbol = symbolTable[i];
	}
	i = -1; // Start from the beginning on next invocation.
	thisSymbol = NULL;
	return NULL;
}

int main(int argc, const char* argv[]) {
	symbolTab_t  symbolTable  = NULL;
	char * 		 lineBuffer = NULL;
	int			 bufferLength = START_LINE_BUFF_SIZE;
	progCnt_t	progCnt = {0, 0, 0};
	unsigned long locCount = 0;
	unsigned long ddrOff  = 0;
	char *	errPtr = NULL;
	int i = 0;
	unsigned int lineCount = 0;
	char * strPtr = gTestString;
	uint64_t dummyLine = 0;
	
	if (DEBUG_PRINT) printf("++++====++++\nIn main()\n");
	
	lineBuffer = malloc((bufferLength + 1) * sizeof(char));
	if (lineBuffer == NULL) return -1;
	
	symbolTable = newSymbolTable();
	if (symbolTable == NULL) {
		printf("Problem allocating symbol table\n");
		return -1;
	}
		
	while( *strPtr != '\0' ) {
		char * endOfLine = strPtr;
		progCnt.lineCount++;
		// Building a line buffer formation unit here
		endOfLine = strchr(strPtr,'\n');
		if (endOfLine != NULL) {
			size_t len = (endOfLine-strPtr);
			if (len > bufferLength) {
				if (DEBUG_PRINT) printf("len: %u, buffLen: %u\n", len, bufferLength);
				lineBuffer = enlargeBuffer(lineBuffer, len);
				if (lineBuffer == NULL) {
					printf("Ran out of memory resizing line buffer\n");
					return -1;
				}
				bufferLength = len;
			}
			strncpy(lineBuffer, strPtr, len);
			* (lineBuffer + (len)) = '\0';
			strPtr = ++endOfLine;
		} else {
			size_t len = strlen(strPtr);
			if (len > bufferLength) {
				if (DEBUG_PRINT) printf("len: %u, buffLen: %u\n", len, bufferLength);
				lineBuffer = enlargeBuffer(lineBuffer, len);
				if (lineBuffer == NULL) {
					printf("Ran out of memory resizing line buffer\n");
					return -1;
				}
				bufferLength = len;
			}
			strcpy(lineBuffer, strPtr);
			strPtr = strchr(strPtr, '\0');
		}
		//printf("%4u:\t%s\n",progCnt.lineCount,lineBuffer);
		errPtr = processLinePass1(lineBuffer, symbolTable, &progCnt, NULL, FIRST);
		if (errPtr != NULL) printf("On line %u:\n\t%s\r\n\n", progCnt.lineCount, errPtr);
		//break;		
	}
	if (DEBUG_PRINT) printf("\nOrig. Buffer:\n%s\n",gTestString);

	if (DEBUG_PRINT) printHashTable(symbolTable);
	// Check for undefined symbols here
	errPtr = undefinedSymbolCheck(symbolTable);
	if (errPtr != NULL) {
		printf("Undefined symbols found:\n%s", errPtr);
		while( (errPtr = undefinedSymbolCheck(symbolTable)) != NULL ) printf("%s", errPtr);
		return -1;
	}
	
	strPtr = gTestString;
	progCnt.lineCount = 0;
	while( *strPtr != '\0' ) {
		char * endOfLine = strPtr;
		progCnt.lineCount++;
		// Building a line buffer formation unit here
		endOfLine = strchr(strPtr,'\n');
		if (endOfLine != NULL) {
			size_t len = (endOfLine-strPtr);
			strncpy(lineBuffer, strPtr, len);
			* (lineBuffer + (len)) = '\0';
			strPtr = ++endOfLine;
		} else {
			size_t len = strlen(strPtr);
			strcpy(lineBuffer, strPtr);
			strPtr = strchr(strPtr, '\0');
		}
		//printf("%4u:\t%s\n",progCnt.lineCount,lineBuffer);
		errPtr = processLinePass2(lineBuffer, symbolTable, &progCnt, &dummyLine, SECOND);
		if (errPtr != NULL) printf("On line %u:\n\t%s\r\n\n", progCnt.lineCount, errPtr);
		//break;		
	}

	freeSymbolTable(symbolTable);
	symbolTable = NULL;
	
	free(lineBuffer);
	lineBuffer = NULL;
}
