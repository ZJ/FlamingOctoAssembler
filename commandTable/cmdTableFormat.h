/*!	\file	cmdTableFormat.h
 *	\brief	Sets up the format to be used for entries in the command table.
 *	\details Also includes macros for extracting values from bit parameters, etc.
 *
 *	\author	Zach Smith <zsmith12@umd.edu>
 *  \version 0.1
 *	\date	2015-02-26
 */

// C++ extern C fence start
#ifdef __cplusplus
extern "C" {
#endif

#define	BLOCK_MASK	0xFE00	//!< The mask of the bits determining which block-equivalent offset to start from
#define BLOCK_SIZE	512		//!< The size of a BRAM block, in 64-bit words
#define CMD_BYTES	8		//!< The number of bytes in a single command word 
#define nextBlock( offset ) (((offset) & BLOCK_MASK) + BLOCK_INC) //!< Macro function to get the next block offset

#define LABEL_END ':'		//!< Used to end a label indication
#define COMMENT_START ';'	//!< Used to mark the start of the comment, also required at end of command
#define ARG_DELIMIT ','		//!< Delimits arguments to commands
#define MAX_CMD_LEN 8		//!< Maximum number of characters for a command

#define CMD_NOP_CODE 0x3E
#define CMD_ERR_CODE 0x3F	

/*! \name Masks for cmdEntry->flags Field
 *
 */
//! @{
#define DATA_INST_MASK	0x01	//!< Write Data Instruction mask
#define WR_OPCODE_MASK	0x02	//!< Write OpCode mask
#define WR_DATFLD_MASK	0x04	//!< Write Data Field mask
#define WR_TIMFLD_MASK	0x08	//!< Write Time-code Field mask
#define DATA_ALGN_MASK	0x30	//!< Data Alignment field mask
#define RSVD_FLD_MASK	0x40	//!< Reserved Bits mask
#define ASSM_DIR_MASK	0x80	//!< Command is actually an assembler directive

//! @}

//! \name	Data Alignment Constants for cmdEntry->flags Data Alignment Field
//! @{
#define DATA_ALIGN_64	0x00	//!< 1x 64-bit word per line
#define DATA_ALIGN_32	0x10	//!< 2x 32-bit words per line
#define DATA_ALIGN_16	0x20	//!< 4x 16-bit words per line
#define DATA_ALIGN_8	0x30	//!< 8x  8-bit words per line

//! @}

/*!	\brief	Structure storing information used in Instruction to OpCode conversion
 *	\details See \ref cmdTableFormat.h for macros defining structure of the ::cmdEntry::flags field
 */
typedef struct cmdEntry {
	unsigned char	command[9];	//!< The C-string expected for command translation
	unsigned char	opcode;		//!< The opcode expected by HW for the command
	unsigned char	numArgs;	//!< The number of arguments to expect for the command
	unsigned char	numLines;	//!< The number of 64-bit memory words the command will occupy on-device 
	unsigned char	flags;		//!< A bitfield giving specific storage requirements for the command
} cmdEntry_t;
typedef cmdEntry_t * cmdEntry_ptr;	//!< Pointer to cmdENtry_t (cmdEntry)

typedef const struct cmdEntry * (*cmdFindFunct_ptr)(register const char *, register unsigned int);

// C++ extern C fence end
#ifdef __cplusplus
}
#endif
