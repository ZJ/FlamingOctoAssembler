/*!	\file	symbolHashTable.c
 *	\brief	Implements a hash table to store the assembler's symbol table
 *	\details Uses a hash function to determine an index in which to store the
 *          symbols.  Collisions are handled by using this location as the root
 *          of a linked list structure.  (Hash-buckets?)
 *
 *	\author	Zach Smith <zsmith12@umd.edu>
 *  \version 0.1
 *	\date	2015-02-18
 */

#include <string.h>
#include <stdlib.h>

#include "symbolHashTable.h"

raw_hash_t getRawHash(const unsigned char *keyString) {
	raw_hash_t hash = 0;
	while (*keyString) {
		hash ^= (hash<<5) + (hash>>2) + *(keyString++);
	}
	return hash;
}

red_hash_t getSymbolIndex(const unsigned char *keyString) {
	red_hash_t outIndex = getRawHash(keyString)%SYMBOL_TABLE_SIZE;
	return outIndex;
}

red_hash_t getLiteralIndex(const unsigned char *keyString) {
	red_hash_t outIndex = getRawHash(keyString)%LITERAL_TABLE_SIZE;
	return outIndex;
}


symbolTab_t newSymbolTable() {
	symbolTab_t symbolTable = NULL;
	symbolTable = malloc(sizeof(symbol_ptr) * SYMBOL_TABLE_SIZE);
	if ( symbolTable != NULL ) {
		red_hash_t i;
		for(i = 0; i < SYMBOL_TABLE_SIZE; i++) {
			symbolTable[i] = NULL;
		}
	}
	return symbolTable;
}

literalTab_t newLiteralTable() {
	literalTab_t literalTable = NULL;
	literalTable = malloc(sizeof(literal_ptr) * LITERAL_TABLE_SIZE);
	if ( literalTable != NULL ) {
		red_hash_t i;
		for(i = 0; i < LITERAL_TABLE_SIZE; i++) {
			literalTable[i] = NULL;
		}
	}
	return literalTable;
}

void freeSymbolTable(symbolTab_t symbolTable) {
	red_hash_t i;
	for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
		if ( symbolTable[i] != NULL ) {
			freeSymbolChain(symbolTable[i]);
			symbolTable[i] = NULL;
		}
	}
	free(symbolTable);
	symbolTable = NULL;
}

void freeLiteralTable(literalTab_t literalTable) {
	red_hash_t i;
	for (i = 0; i < LITERAL_TABLE_SIZE; i++) {
		if ( literalTable[i] != NULL ) {
			freeLiteralChain(literalTable[i]);
			literalTable[i] = NULL;
		}
	}
	free(literalTable);
	literalTable = NULL;
}

void freeSymbolChain(symbol_ptr head) {
	symbol_ptr nextLink = NULL;
	symbol_ptr thisLink = head;
	while (thisLink != NULL) {
		nextLink = thisLink->next;
		free(thisLink->name); // Need to free the name separately, since it was allocated separately.
		free(thisLink);
		thisLink = nextLink;
	}
	// head = NULL; // No need to set head to NULL, because it is just the local copy of the pointer
}

void freeLiteralChain(literal_ptr head) {
	literal_ptr nextLink = NULL;
	literal_ptr thisLink = head;
	while (thisLink != NULL) {
		nextLink = thisLink->next;
		free(thisLink->name); // Need to free the name separately, since it was allocated separately.
		free(thisLink);
		thisLink = nextLink;
	}
	// head = NULL; // No need to set head to NULL, because it is just the local copy of the pointer
}

symbol_ptr newSymbol(const unsigned char * symbolName) {
	size_t		nameSize = 0;
	char *		nameStorage = NULL;
	symbol_ptr	symbol = NULL;
	
	nameSize = strlen(symbolName) + 1;
	nameStorage = malloc(sizeof(char) * nameSize);
	if ( nameStorage != NULL ) {
		memset(nameStorage, '\0', nameSize);
		strcpy(nameStorage, symbolName);
	} else {
		return NULL;
	}
	
	symbol = malloc(sizeof(symbol_t));
	if ( symbol != NULL ) {
		symbol->name = nameStorage;
		symbol->locCount = 0;
		symbol->bramOffset = 0;
		symbol->type = 'U';
		symbol->next = NULL;
	} else {
		free(nameStorage);
	}
	
	return symbol;
}

literal_ptr newLiteral(const unsigned char * literalName) {
	size_t		nameSize = 0;
	char *		nameStorage = NULL;
	literal_ptr	literal = NULL;
	
	nameSize = strlen(literalName) + 1;
	nameStorage = malloc(sizeof(char) * nameSize);
	if ( nameStorage != NULL ) {
		memset(nameStorage, '\0', nameSize);
		strcpy(nameStorage, literalName);
	} else {
		return NULL;
	}
	
	literal = malloc(sizeof(literal_t));
	if ( literal != NULL ) {
		literal->name = nameStorage;
		literal->value = 0;
		literal->type = 'U';
		literal->next = NULL;
	} else {
		free(nameStorage);
	}
	
	return literal;
}

symbol_ptr findSymbol(const unsigned char * symbolName, symbolTab_t symbolTable) {
	symbol_ptr binEntry = NULL;
	red_hash_t tableBin = getSymbolIndex(symbolName);
	
	binEntry = symbolTable[tableBin];
	if ( binEntry != NULL ) {
		// Chase the linked list, looking for a match.
		while ( !strcmp(binEntry->name, symbolName) ) {
			// If you hit the end of the list, break (will return NULL).
			if ( (binEntry = binEntry->next) == NULL ) break;
		}
	}
	
	return binEntry;  // Either NULL for not found, or the list entry.
}

literal_ptr findLiteral(const unsigned char * literalName, literalTab_t literalTable) {
	literal_ptr binEntry = NULL;
	red_hash_t tableBin = getLiteralIndex(literalName);
	
	binEntry = literalTable[tableBin];
	if ( binEntry != NULL ) {
		// Chase the linked list, looking for a match.
		while ( !strcmp(binEntry->name, literalName) ) {
			// If you hit the end of the list, break (will return NULL).
			if ( (binEntry = binEntry->next) == NULL ) break;
		}
	}
	
	return binEntry;  // Either NULL for not found, or the list entry.
}

void insertSymbol(symbol_ptr * insertLoc, symbol_ptr toInsert) {
	toInsert->next = *insertLoc;
	*insertLoc = toInsert;
}

void insertLiteral(literal_ptr * insertLoc, literal_ptr toInsert) {
	toInsert->next = *insertLoc;
	*insertLoc = toInsert;
}

symbol_ptr addSymbol(const unsigned char * symbolName, symbolTab_t symbolTable) {
	symbol_ptr toAdd = NULL;
	
	toAdd = newSymbol(symbolName);
	if (toAdd != NULL) {
		red_hash_t tablePos = 0;
		tablePos = getSymbolIndex(symbolName);
		insertSymbol(symbolTable + tablePos, toAdd);
	}
	
	return toAdd;
}

literal_ptr addLiteral(const unsigned char * literalName, literalTab_t literalTable) {
	literal_ptr toAdd = NULL;
	
	toAdd = newLiteral(literalName);
	if (toAdd != NULL) {
		red_hash_t tablePos = 0;
		tablePos = getLiteralIndex(literalName);
		insertLiteral(literalTable + tablePos, toAdd);
	}
	
	return toAdd;
}
