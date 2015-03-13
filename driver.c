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

#define CORE_ERR_STR "TOOMANY"

cmdFindFunct_ptr cmdLookupHandles[NUM_CORES] = {&TEST_CORE_CMD_LOOKUP};

typedef enum {FIRST, SECOND} passNum_type;

typedef struct progCounters {
	unsigned long	locCount;
	unsigned long	ddrOffset;
	unsigned long	lineCount;
	passNum_type	whichPass;
} progCnt_t;

inline void startSecondPass(progCnt_t * progCount) {
	progCount->locCount		= 0;
	progCount->ddrOffset	= 0;
	progCount->lineCount	= 0;
	progCount->whichPass	= SECOND;
}

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

char * grabCmd(char * cmdStr, char ** buffer) {
	char * buffPos = *buffer;
	int i;
	
	// Should be at the beginning of the command by here, so go until whitespace
	*cmdStr = '\0';
	for (i=0; i < MAX_CMD_LEN; i++) {
		// Process Line for command Here
		if ( isspace(*(*buffer)) || ((*(*buffer)) == '\0') ) {
			(*(*buffer)) = '\0';
			strcpy(cmdStr, buffPos);
			(*buffer)++;// Increment past the null we just inserted
			break;
		}
		(*buffer)++;
	}
	
	return cmdStr;
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

// Should add enough err codes to pad to a block edge.  Maybe block size needs to be an argument?
directiveStatus processPadStack(char * cmdStr, char * restLine, char * errMsg, progCnt_t * progCnt, unsigned int * rptCnt) {
	unsigned long padBlockSize;
	char * endConv = NULL;
	unsigned int cmdCount = ((progCnt->ddrOffset)/CMD_BYTES);
	while ( isspace(*restLine) ) restLine++;
	
	padBlockSize = strtoul(restLine, &endConv, 0);
	if (padBlockSize == 0) padBlockSize = BLOCK_SIZE;
	while ( isspace(*endConv) ) endConv++;
	if (*endConv != '\0') {
		strcpy(errMsg, "ERR_PARSE_ARG_CNT");
		return ERROR;
	}
		
	strcpy(cmdStr, CORE_ERR_STR);
	*restLine = '\0';
	*rptCnt = padBlockSize - (cmdCount%padBlockSize);
	
	printf("Pad invocation: \"%s\", \"%s\", %u, %u\n", cmdStr, restLine, *rptCnt, padBlockSize);
	
	return PART_DIRECTIVE;
}	

directiveStatus processRepeat(char * cmdStr, char * restLine, char * errMsg, unsigned int * rptCnt) {
	char * newStart = restLine;
	
	// strtou will skip whitespace
	*rptCnt = strtoul(restLine, &newStart, 0);
	
	// skip whitespace, grab command, skip whitespace
	while ( isspace(*newStart) ) newStart++;
	printf("%s\n",newStart);
	if ( *grabCmd(cmdStr, &newStart) == '\0' ) {
		strcpy(errMsg, ERR_PARSE_CMD_LONG);
		return ERROR;
	}
	while ( isspace(*newStart) ) newStart++;
	
	memmove(restLine, newStart, (strlen(newStart)+1)*sizeof(char));

	return PART_DIRECTIVE;
}

directiveStatus processDirective(char * cmdStr, char * restLine, char * errMsg, progCnt_t * progCnt, symbolTab_t symbolTable, unsigned int * rptCnt) {
	switch (*(cmdStr++)) {
		case 'L':
			if ( strcmp(cmdStr, /*L*/"IT") == 0 ) return (progCnt->whichPass == FIRST) ? processLitDec(restLine, errMsg, symbolTable) : DIRECTIVE;
			// Do more ifs for other 'L' directives
			break;
		case 'S':
			if ( strcmp(cmdStr, /*S*/"ETLC") == 0 ) return processSetLC(restLine, errMsg, progCnt);
			// Do more ifs for other 'S'
			break;
		case 'P':
			if (strcmp(cmdStr, /*P*/"AD") == 0 ) return processPadStack(cmdStr - 1, restLine, errMsg, progCnt, rptCnt);
			// Do more ifs for other 'P'
			break;
		case 'R':
			if (strcmp(cmdStr, /*R*/"PT") == 0 ) return processRepeat(cmdStr - 1, restLine, errMsg, rptCnt);
			// Do more ifs for other 'R'
			break;
		default:
			break;
	}
	return NOTDIRECTIVE; // We've not returns from any special code, so NOTDIRECTIVE.
}

/*!	\brief	Handles processing of a single line of a file.
 *	\details - Searches for label definitions to build the symbol table, and checks that none a multiply defined.
 *			 - Also checks for literal definitions and creates those
 *			 - Handles other assembler directives
 *			 - Looks up commands strings and keeps track of total memory size so it can be allocated for the second pass.
 *			 - Checks total argument count to match with the command
 *	\returns	NULL on success
 *	\returns	pointer to error description on failure (for printing)
 */
char * processLine(char * lineBuffer, symbolTab_t symbolTable, progCnt_t * progCnt, uint64_t * memline) {
	static char labelStr[256] = "";
	static char cmdStr[9] = "";
	static char errMsg[256] = "";
	static char * buffPos = NULL;
	static cmdEntry_ptr	thisCmd = NULL;
	static int i = 0;
	static int argCnt = 0;
	static directiveStatus checkDir = NOTDIRECTIVE;
	uint64_t argArray[MAX_ARG_COUNT] ={0};
	char * lineCopy = NULL;
	unsigned int rptCount = 1;
	
	if (DEBUG_PRINT) {
		lineCopy = malloc((strlen(lineBuffer)+1)*sizeof(char));
		if (lineCopy != NULL) strcpy(lineCopy, lineBuffer);
		//printf("%s\n:",lineCopy);
	}
	
	while ( isspace(*lineBuffer) ) lineBuffer++; // Skip leading space, if any

	// Find label delimiter, if any
	buffPos = strchr(lineBuffer, LABEL_END);
	if ( buffPos != NULL ) {
		symbol_ptr	thisLabel	= NULL;
		
		// On first pass, we'd define the label.
		if (progCnt->whichPass == FIRST) {
			// First non-whitespace is label separator
			if ( buffPos == lineBuffer ) {
				strcpy(errMsg, ERR_PARSE_LBL_EMPTY);
				return errMsg;
			}
			
			*buffPos = '\0'; // Switch the label sep. to NULL char
			strcpy(labelStr, lineBuffer);
			if (defineLabel(labelStr, errMsg, symbolTable, &progCnt) != 0) return errMsg; //Problem defining the label
		}
		// Start after the label separator and skip whitespace
		lineBuffer = ++buffPos;
		while ( isspace(*lineBuffer) ) lineBuffer++;
	}
	
	// If we're to the null character, this line is empty
	if (*lineBuffer == '\0') return NULL;
	
	// Find closing semicolon, or report error
	buffPos = strchr(lineBuffer, COMMENT_START);
	if ( buffPos == NULL ) {
		strcpy(errMsg, ERR_PARSE_NO_END);
		return errMsg;
	} else {
		*buffPos = '\0';
	}
	
	// If we didn't hit whitespace before the max length,
	// it never got copied, so cmdStr[0] is '\0'
	// If we find that, the command was thus too long.
	if ( *grabCmd(cmdStr, &lineBuffer) == '\0' ) {
		strcpy(errMsg, ERR_PARSE_CMD_LONG);
		return errMsg;
	}
	
	// Check if it is a assembler directive, and take appropriate action.
	checkDir = processDirective(cmdStr, lineBuffer, errMsg, progCnt, symbolTable, &rptCount);
	if ( checkDir == DIRECTIVE ) return NULL;
	if ( checkDir == ERROR ) return errMsg;
	
	// This far means we're expecting an actual command
	thisCmd = processCmd(cmdStr, errMsg, &(progCnt->locCount), &(progCnt->ddrOffset));
	if ( thisCmd == NULL ) return errMsg; // Problem looking up command. Report the error

	// If we're here, the command entry is stashed in thisCmd, so we can keep processing
	progCnt->locCount	+= (thisCmd->numLines * rptCount);
	progCnt->ddrOffset	+= (thisCmd->numLines * CMD_BYTES * rptCount);
	
	// Check for correct number of arguments
	// Also check format of arguments
	while ( isspace(*lineBuffer) ) lineBuffer++; // Skip any whitespace before the first argument
	
	argCnt = 0;
	while (*lineBuffer != '\0') {
		argCnt++;
		buffPos = strchr(lineBuffer, ARG_DELIMIT);
		if ( argCnt > MAX_ARG_COUNT ) { // Don't bother checking extra args
			if (buffPos != NULL) {
				lineBuffer = ++buffPos;
			} else {
				lineBuffer = strchr(lineBuffer, '\0');
			}
			continue;
		}
		if (buffPos != NULL) {
			*buffPos = '\0';
			if (resolveArg(lineBuffer, errMsg, (argArray + (argCnt - 1)), symbolTable, progCnt) != NULL) return errMsg;
			lineBuffer = ++buffPos;
			while ( isspace(*lineBuffer) ) lineBuffer++; // Skip any whitespace before the next argument
		} else {
			if (resolveArg(lineBuffer, errMsg, (argArray + (argCnt - 1)), symbolTable, progCnt) != NULL) return errMsg;
			lineBuffer = strchr(lineBuffer, '\0');
		}
	}
	
	// Check we found the right number of arguments
	if ( argCnt != thisCmd->numArgs) {
		printf("%d found, %d expected.\n", argCnt, thisCmd->numArgs);
		strcpy(errMsg, ERR_PARSE_ARG_CNT);
		return errMsg;
	}
	
	// All the output stuff only happens on the second pass
	if ( progCnt->whichPass == SECOND) {
		uint64_t thisLine	= 0x0ULL;
		uint64_t thisMask	= 0xFFFFFFFFFFFFFFFFULL;
		uint64_t * storLoc	= (memline + (progCnt->ddrOffset/CMD_BYTES) - rptCount);
		
		// Start building the command word:
		if ( (thisCmd->flags & WR_OPCODE_MASK) != 0 ) {
			thisLine |= ( thisMask & ( ((uint64_t)thisCmd->opcode) << INST_OPCD_OFFSET ));
			thisMask &= ~INST_OPCD_MASK;
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
		for( i=0; i<rptCount; i++) {
			*(storLoc + i) = thisLine;
			if (DEBUG_PRINT) {
				if ( i == 0 ) {
					if (lineCopy != NULL) {
						printf("@0x%08x:\t0x%016" PRIx64 ";\t%s\n", storLoc, *(storLoc), lineCopy);
						free(lineCopy);
						lineCopy = NULL;
					} else {
						printf("@0x%08x:\t0x%016" PRIx64 ";\t<Insufficient memory for line echo>\n", storLoc, *storLoc);
					}
				} else printf("@0x%08x:\t0x%016" PRIx64 ";\t  (RPT #%u)\n", storLoc + i, *(storLoc + i), i+1);
			}
		}
	}
	
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
int defineLabel(const char * labelName, char * errMsg, symbolTab_t symbolTable, progCnt_t * prgCnt) {
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
	thisLabel->locCount   = prgCnt->locCount;
	thisLabel->ddrOffset  = prgCnt->ddrOffset;
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

inline unsigned long numBlocks(unsigned long offsetCnt) {
	return (offsetCnt == 0) ? 1 : (offsetCnt-1)/BLOCK_SIZE + 1;
}

int main(int argc, const char* argv[]) {
	symbolTab_t  symbolTable  = NULL;
	char * 		 lineBuffer = NULL;
	int			 bufferLength = START_LINE_BUFF_SIZE;
	progCnt_t	progCnt = {0, 0, 0, FIRST};
	char *	errPtr = NULL;
	uint64_t dummyLine = 0;
	uint64_t * progStorage = NULL;

	char * strPtr = gTestString;
	char lastChar = *strPtr;
	
	if (DEBUG_PRINT) printf("++++====++++\nIn main()\n");
	
	lineBuffer = malloc((bufferLength + 1) * sizeof(char));
	if (lineBuffer == NULL) return -1;
	
	symbolTable = newSymbolTable();
	if (symbolTable == NULL) {
		printf("Problem allocating symbol table\n");
		return -1;
	}
	
	while( lastChar != '\0' ) {
		char * endOfLine = strPtr;
		size_t len = 0;
		progCnt.lineCount++;
		// Building a line buffer formation unit here
		endOfLine = strchr(strPtr,'\n');
		if (endOfLine == NULL) endOfLine = strchr(strPtr, '\0');
		lastChar = *endOfLine;
		
		len = (endOfLine-strPtr);
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
		strPtr = endOfLine + 1;
		
		//printf("%4u:\t%s\n",progCnt.lineCount,lineBuffer);
		errPtr = processLine(lineBuffer, symbolTable, &progCnt, &dummyLine);
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
	
	progStorage = malloc( numBlocks(progCnt.ddrOffset) * BLOCK_SIZE * sizeof(uint64_t));
	if (progStorage == NULL) {
		printf("Could not allocate program storage\n");
		return -1;
	}
	
	strPtr = gTestString;
	startSecondPass(&progCnt);
	lastChar = *strPtr;
	
	while( lastChar != '\0' ) {
		char * endOfLine = strPtr;
		size_t len = 0;
		progCnt.lineCount++;
		// Building a line buffer formation unit here
		endOfLine = strchr(strPtr,'\n');
		if (endOfLine == NULL) endOfLine = strchr(strPtr, '\0');
		lastChar = *endOfLine;

		len = (endOfLine-strPtr);
		strncpy(lineBuffer, strPtr, len);
		* (lineBuffer + (len)) = '\0';
		strPtr = ++endOfLine;

		//printf("%4u:\t%s\n",progCnt.lineCount,lineBuffer);
		errPtr = processLine(lineBuffer, symbolTable, &progCnt, progStorage);
		if (errPtr != NULL) printf("On line %u:\n\t%s\r\n\n", progCnt.lineCount, errPtr);
		//break;		
	}
	
	{
		int i;
		printf("Code code code:\n");
		for ( i=0; i<20; i++) {
			printf("\t%p:\t0x%016" PRIx64 "\n", progStorage+i, *(progStorage+i));
		}
	}
	
	freeSymbolTable(symbolTable);
	symbolTable = NULL;
	
	free(lineBuffer);
	lineBuffer = NULL;
}
