/*!	\file	symbolHashTable.h
 *	\brief	Implements a hash table to store the assembler's symbol table
 *	\details Uses a hash function to determine an index in which to store the
 *          symbols.  Collisions are handled by using this location as the root
 *          of a linked list structure.  (Hash-buckets?)
 *
 *	\author	Zach Smith <zsmith12@umd.edu>
 *  \version 0.1
 *	\date	2015-02-19
 */
 
// Apocryphal wisdom claims this should be a prime, so here is a list of primes 40 < prime < 255
/* 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127,
 * 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
 * 223, 227, 229, 233, 239, 241, 251
 */
 
#define SYMBOL_TABLE_SIZE 41 //!< The number of buckets in the symbol table.
// The smallest prime to fit into an unsigned char is 251

typedef unsigned long raw_hash_t; //!< Type returned by the hashing function
typedef unsigned char red_hash_t; //!< Type returned when reducing the hasing function to a table index

/*! \brief	Struct holding information about symbols encountered during assembly */
typedef struct symbol {
	char *			name;		//!< Pointer to C-string containing the name
	unsigned long	locCount;	//!< Location counter value for this symbol
	char			type;		//!< Type of symbol ('U'ndefined, 'D'efined, or 'M'ultiply defined)
	struct symbol *	next;		//!< Pointer to the next symbol in the linked list
} symbol_t;

typedef symbol_t * symbol_ptr;	//!< Pointer to a symbol object

raw_hash_t getRawHash(const unsigned char *keyString);
red_hash_t getHashIndex(const unsigned char *keyString);
symbol_ptr * newSymbolTable();
void freeSymbolTable(symbol_ptr * symbolTable);
void freeSymbolChain(symbol_ptr head);
symbol_ptr newSymbol(const unsigned char * symbolName);
symbol_ptr findSymbol(const unsigned char * symbolName, symbol_ptr * symbolTable);
void insertSymbol(symbol_ptr * insertLoc, symbol_ptr toInsert);
symbol_ptr addSymbol(const unsigned char * symbolName, symbol_ptr * symbolTable);

/*!  \brief Inline to mark symbol as Undefined */
void setTypeU(symbol_ptr symbol);

/*!  \brief Inline to mark symbol as Defined */
void setTypeD(symbol_ptr symbol);

/*!  \brief Inline to mark symbol as Multiply Defined */
void setTypeM(symbol_ptr symbol);
