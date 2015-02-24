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

/*!	\brief	Takes a C-style string and return the raw Shift-Add-XOR hash
 *
 *	\param[in]	keyString	C-style string containing key to-be-hashed.
 *	\returns	The raw hash
 */
raw_hash_t getRawHash(const unsigned char *keyString);

/*!	\brief	Takes a C-style string and return the hash-table index
 *
 *	\param[in]	keyString	C-style string containing key to-be-hashed.
 *	\returns	The index to use in the hash table
 */
red_hash_t getHashIndex(const unsigned char *keyString);

/*! \brief Creates a new, empty symbol table.
 *
 */
symbol_ptr * newSymbolTable();

/*! \brief Frees entire symbol table, including memory claimed by entries
 *
 */
void freeSymbolTable(symbol_ptr * symbolTable);

/*!	\brief Frees a linked list chain
 *	\warning User is responsible for marking head to NULL, since the pointer is passed by value.
 */
void freeSymbolChain(symbol_ptr head);

/*! \brief	Creates a new symbol with provided name and default values
 *	\warning Will return NULL on failure.
 */
symbol_ptr newSymbol(const unsigned char * symbolName);

/*! \brief	Searches the table for the given symbol.
 *	\details Searches the symbol table by starting at the root of the bin given by hashing
 *			the name.  Returns NULL if the symbol is not found.
 */
symbol_ptr findSymbol(const unsigned char * symbolName, symbol_ptr * symbolTable);

/*! \brief	Inserts a new symbol at the given location in a chain.
 *
 */
void insertSymbol(symbol_ptr * insertLoc, symbol_ptr toInsert);

/*! \brief	Creates a new symbol and adds it to the appropriate table location.
 *
 */
symbol_ptr addSymbol(const unsigned char * symbolName, symbol_ptr * symbolTable);

/*!	\brief Mark symbol as Undefined
 *	\param _symbol_ptr_ symbol pointer to a symbol struct.
 */
#define setTypeU(symbol) (symbol)->type = 'U'

/*!	\brief Mark symbol as Defined
 *	\param _symbol_ptr_ symbol pointer to a symbol struct.
 */
#define setTypeD(symbol) (symbol)->type = 'D'

/*!	\brief Mark symbol as Multiply Defined
 *	\param _symbol_ptr_ symbol pointer to a symbol struct.
 */
#define setTypeM(symbol) (symbol)->type = 'M'
