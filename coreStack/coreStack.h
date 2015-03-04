/*! \file coreStack.h
 *	\brief	Implements the (very simple) core stack memory manager
 *	\details Uses a memory pool defined by the linker via a linker script to process
 *		and store the core stack(s).  Uses a ring buffer to the descriptions of core
 *		stacks that have been allocated from the pool.
 */
#include <inttypes.h>

#define	MAX_SUB_STACKS	10	//!< The maximum number of core stacks we can track

typedef uin64_t memoryWord_t;

typedef struct coreStackDesc {
	memoryWord_t *	start;	//!< Pointer to the first word of the core stack
	unsigned int	size;	//!< Total memory words used for this stack
} coreStackDesc_t;
typedef coreStackDesc_t * coreStackDesc_ptr;

extern memoryWord_t * lowCoreStackBase	= &_Octo_Stack_Start__;
extern memoryWord_t * lowCoreStackEnd	= ((memoryWord_t *) &_Octo_Stack_End__) - 1;