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
 
#define SYMBOL_TABLE_SIZE 251 //!< The number of buckets in the symbol table.
// The smallest prime to fit into an unsigned char is 251

typedef unsigned long raw_hash_t; //!< Type returned by the hashing function
typedef unsigned char red_hash_t; //!< Type returned when reducing the hashing function to a table index

/*! \brief	Struct holding information about symbols encountered during assembly
 *	\details Ordering is from largest type to smallest, which minimizes the amount of
 *			padding the compiler will insert into the object.
 */
typedef struct symbol {
	unsigned long	locCount;	//!< Location counter value for this symbol
	unsigned long	bramOffset;	//!< BRAM Offset value for this symbol
	char *			name;		//!< Pointer to C-string containing the name
	struct symbol *	next;		//!< Pointer to the next symbol in the linked list
	char			type;		//!< Type of symbol ('U'ndefined, 'D'efined, or 'M'ultiply defined)
} symbol_t;

typedef symbol_t * symbol_ptr;	//!< Pointer to a ::symbol (::symbol_t) object

/*!	\brief	Takes a C-style string and return the raw Shift-Add-XOR hash
 *
 *	\param[in]	keyString	C-style string containing key to-be-hashed.
 *	\returns	The raw hash as ::raw_hash_t
 */
raw_hash_t getRawHash(const unsigned char *keyString);

/*!	\brief	Takes a C-style string and return the hash-table index
 *
 *	\param[in]	keyString	C-style string containing key to-be-hashed.
 *	\returns	The index to use in the hash table as ::red_hash_t
 */
red_hash_t getHashIndex(const unsigned char *keyString);

/*! \brief Creates a new, empty symbol table.
 *
 *	\returns	A pointer to the new symbol table as ::symbol_ptr *
 */
symbol_ptr * newSymbolTable();

/*! \brief Frees entire symbol table, including memory claimed by entries
 *	\details Iterates through the entire table, freeing the linked list (via 
 * 			::freeSymbolChain) rooted at each entry before freeing the symbol
 *			table itself.
 *
 *	\param[in] symbolTable	the pointer to the root of the symbolTable
 */
void freeSymbolTable(symbol_ptr * symbolTable);

/*!	\brief Frees a linked list chain
 *	\warning User is responsible for marking head to NULL
 *
 *	\param[in] head	a pointer to the head of a linked list.
 */
void freeSymbolChain(symbol_ptr head);

/*! \brief	Creates a new symbol with provided name and default values
 *	\warning Will return NULL on failure.
 *
 *	\param[in]	symbolName	a C-string giving the name for the new symbol
 *	\returns	pointer to the newly-created symbol on success
 *	\returns	NULL on failure
 */
symbol_ptr newSymbol(const unsigned char * symbolName);

/*! \brief	Searches the table for the given symbol.
 *	\details Searches the symbol table by starting at the root of the bin given by
 *			hashing the name.
 *
 *	\param[in]	symbolName	a C-string giving the name to search for
 *	\param[in]	symbolTable	a pointer to the root of the symbolTable to search
 *	\returns	pointer to the symbol if found
 *	\returns	NULL if the symbol is not found
 */
symbol_ptr findSymbol(const unsigned char * symbolName, symbol_ptr * symbolTable);

/*! \brief	Inserts a new symbol at the given location in a chain.
 *
 *	\param[in,out]	insertLoc	Pointer to the location at which to insert the
 *								symbol. Will be changed to point at the inserted
 *								symbol.
 *	\param[in,out]	toInsert	Pointer to the symbol to insert.  The "next"
 *								field will be update to point to the rest of the
 *								list.
 */
void insertSymbol(symbol_ptr * insertLoc, symbol_ptr toInsert);

/*! \brief	Creates a new symbol and adds it to the appropriate table location.
 *
 *	\warning Does not check for duplicate entries.
 *
 *	\param[in]	symbolName	a C-string giving the name to address
 *	\param[in]	symbolTable	a pointer to the root of the symbol table adding to
 *	\returns	a pointer to the newly-created ::symbol
 */
symbol_ptr addSymbol(const unsigned char * symbolName, symbol_ptr * symbolTable);

/*!	\brief Mark symbol as Undefined
 *	\param symbolToSet (::symbol_ptr) pointer to a symbol struct.
 */
#define setTypeU(symbolToSet) (symbolToSet)->type = 'U'

/*!	\brief Mark symbol as Defined
 *	\param symbolToSet (::symbol_ptr) pointer to a symbol struct.
 */
#define setTypeD(symbolToSet) (symbolToSet)->type = 'D'

/*!	\brief Mark symbol as Multiply Defined
 *	\param symbolToSet (::symbol_ptr) pointer to a symbol struct.
 */
#define setTypeM(symbolToSet) (symbolToSet)->type = 'M'
