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

/*! \name Masks for cmdEntry->flags Field
 *
 */
//! @{
#define DATA_INST_MASK	0x01	//!< Write Data Instruction mask
#define WR_OPCODE_MASK	0x02	//!< Write OpCode mask
#define WR_DATFLD_MASK	0x04	//!< Write Data Field mask
#define WR_TIMFLD_MASK	0x08	//!< Write Time-code Field mask
#define DATA_ALGN_MASK	0x30	//!< Data Alignment field mask
#define RSVD_FLD_MASK	0xC0	//!< Reserved Bits mask

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

// C++ extern C fence end
#ifdef __cplusplus
}
#endif
