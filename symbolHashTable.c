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

/*!	\brief	Takes a C-style string and return the raw Shift-Add-XOR hash
 *
 *	\param[in]	keyString	C-style string containing key to-be-hashed.
 *	\returns	The raw hash
 */
raw_hash_t getRawHash(const unsigned char *keyString) {
	raw_hash_t hash = 0;
	while (*keyString) {
		hash ^= (hash<<5) + (hash>>2) + *(keyString++);
	}
	return hash;
}

/*!	\brief	Takes a C-style string and return the hash-table index
 *
 *	\param[in]	keyString	C-style string containing key to-be-hashed.
 *	\returns	The index to use in the hash table
 */
red_hash_t getHashIndex(const unsigned char *keyString) {
	red_hash_t outIndex = getRawHash(keyString)%SYMBOL_TABLE_SIZE;
	return outIndex;
}

/*! \brief Creates a new, empty symbol table.
 *
 */
symbol_ptr * newSymbolTable() {
	symbol_ptr * symbolTable = NULL;
	symbolTable = malloc(sizeof(symbol_ptr) * SYMBOL_TABLE_SIZE);
	if ( symbolTable != NULL ) {
		red_hash_t i;
		for(i = 0; i < SYMBOL_TABLE_SIZE; i++) {
			symbolTable[i] = NULL;
		}
	}
	return symbolTable;
}

/*! \brief Frees entire symbol table, including memory claimed by entries
 *
 */
void freeSymbolTable(symbol_ptr * symbolTable) {
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

/*!	\brief Frees a linked list chain
 *	\warning User is responsible for marking head to NULL, since the pointer is passed by value.
 */
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

/*! \brief	Creates a new symbol with provided name and default values
 *	\warning Will return NULL on failure.
 */
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
		symbol->type = 'U';
		symbol->next = NULL;
	} else {
		free(nameStorage);
	}
	
	return symbol;
}

/*! \brief	Searches the table for the given symbol.
 *	\details Searches the symbol table by starting at the root of the bin given by hashing
 *			the name.  Returns NULL if the symbol is not found.
 */
symbol_ptr findSymbol(const unsigned char * symbolName, symbol_ptr * symbolTable) {
	symbol_ptr binEntry = NULL;
	red_hash_t tableBin = getHashIndex(symbolName);
	
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

/*! \brief	Inserts a new symbol at the given location in a chain.
 *
 */
void insertSymbol(symbol_ptr * insertLoc, symbol_ptr toInsert) {
	toInsert->next = *insertLoc;
	*insertLoc = toInsert;
}

/*!  \brief Inline to mark symbol as Undefined */
void setTypeU(symbol_ptr symbol) {
	symbol->type = 'U';
}

/*!  \brief Inline to mark symbol as Defined */
void setTypeD(symbol_ptr symbol) {
	symbol->type = 'D';
}

/*!  \brief Inline to mark symbol as Multiply Defined */
void setTypeM(symbol_ptr symbol) {
	symbol->type = 'M';
}

/*! \brief	Creates a new symbol and adds it to the appropriate table location.
 *
 */
symbol_ptr addSymbol(const unsigned char * symbolName, symbol_ptr * symbolTable) {
	red_hash_t tablePos = 0;
	symbol_ptr toAdd = NULL;
	
	toAdd = newSymbol(symbolName);
	tablePos = getHashIndex(symbolName);
	
	insertSymbol(symbolTable + tablePos, toAdd);
	
	return toAdd;
}
