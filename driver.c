#include <stdio.h>

#include "symbolHashTable/symbolHashTable.h"
#include "commandTable/cmdTableFormat.h"

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

/*!
 *	\todo	Define processCmd(const char * cmdStr, char * errMsg, unsigned long * locCount, unsigned long * bramOff);
 *	\returns	NULL on success
 *	\returns	pointer to error description on failure (for printing)
 */
char * processLinePass1(char * lineBuffer, int lineLength, symbolTab_t symbolTable, unsigned long * locCount, unsigned long * bramOff) {
	static char labelStr[256] = "";
	static char cmdStr[9] = "";
	static char errMsg[256] = "";
	static char * buffPos = NULL;
	cmdEntry_t	thisCmd = NULL;
	int i = 0;
	
	while ( isspace(*lineBuffer) ) lineBuffer++; // Skip leading space, if any
	
	// Find closing semicolon, or report error
	buffPos = strchr(lineBuffer, COMMENT_START);
	if ( buffPos == NULL ) {
		errMsg = "No \';\' found";
		return errMsg;
	} else {
		*buffPos = '\0';
	}

	if (*lineBuffer == '\0') return NULL; // Empty line!
	
	// Find label delimiter, if any
	buffPos = strchr(lineBuffer, LABEL_END);
	if ( buffPos != NULL ) {
		symbol_ptr	thisLabel	= NULL;
		
		// First non-whitespace is label separator
		if ( buffPos == lineBuffer ) {
			errMsg = "\':\' Encountered without preceding label";
			return errMsg;
		}
		
		*bufPos = '\0'; // Switch the label sep. to NULL char
		strcpy(labelStr, lineBuffer);
		lineBuffer = buffPos++;
		
		if (defineLabel(labelStr, errMsg, symbolTable, *locCount, *bramOff) != 0) return errMsg; //Problem defining the label
		
		while ( isspace(*lineBuffer) ) lineBuffer++; // Skip whitespace between label and cmd
	}
	
	// Should be at the beginning of the command by here, so go until whitespace
	*cmdStr = '\0';
	for (i=0; i < MAX_CMD_LEN; i++) {
		// Process Line for command Here
		if ( isspace(lineBuffer[i]) || lineBuffer[i] == '\0' ) {
			lineBuffer[i] = '\0';
			strcpy(cmdStr, lineBuffer);
			break;
		}
	}
	if ( cmdStr[0] != '\0' ) {
		thisCmd = processCmd(cmdStr, errMsg, locCount, bramOff);
		if ( thisCmd == NULL) return errMsg; // Problem looking up command. Report the error
	} else {
		errMsg = "Command too long";
		return errMsg;
	}
	
	// If we're here, the command entry is stashed in thisCmd, so we can keep processing
	
	// Check for correct number of arguments
	// (don't bother resolving symbols and literals at this time, that is in pass 2)
	
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
		errMsg = "Labels must start with a letter";
		return -1;
	}
	
	while ( (testChar = labeName[i++]) != '\0' ) {
		if ( !( isalnum(testChar) || (testChar != '_') ) ) {
			errMsg = "Invalid characters found in label";
			return -1;
		}
	}
	
	if ( findCommand(labelName) == NULL ) {
		errMsg = "Command found where expecting a label";
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
	
	validLabel(labelName, errMsg);
		
	thisLabel = findSymbol(labelStr, symbolTable);
	if (thisLabel != NULL) {
		// Found an extant label
		switch(thisLabel->type) {
			case 'U':
				break;
			case 'D':
				setTypeM(thisLabel);
			case 'M':
				errMsg = "Multiply-defined label found";
				return -1;
				break;
			default:
				errMsg = "Internal Error: unknown label type encountered"
				return -1;
				break;
		}
	} else { //thisLabel = NULL
		// New label encountered
		thisLabel = addSymbol(labelName, symbolTable);
		if (thisLabel == NULL) {
			errMsg = "Internal Error: failed to allocate label";
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
	int i=0;
	symbolTable = newSymbolTable();
	
	lfsrChar((unsigned int) symbolTable);
	for (i=0; i<250; i++) {
		char * randWord = lfsrWord((unsigned int) -1);
		printf("%s\r\n", randWord);
		addSymbol(randWord, symbolTable);
	}
	
	printHashTable(symbolTable);
	
	freeSymbolTable(symbolTable);
}
