/*!	\file	parserErrs.h
 *	\brief	Holds #defined string constants for parser errors
 */
 
#define	ERR_PARSE_NO_END		"No \';\' found"
#define	ERR_PARSE_LBL_EMPTY		"\':\' Encountered without preceding label"
#define ERR_PARSE_CMD_LONG		"Command too long"
#define	ERR_PARSE_LBL_LETTR		"Labels must start with a letter"
#define ERR_PARSE_LBL_BAD_CHAR	"Invalid characters found in label"
#define ERR_PARSE_LBL_CMD_COLL	"Command found where expecting a label"
#define ERR_PARSE_ARG_CNT		"Wrong number or arguments found"

#define ERR_LBL_MULT_DEF		"Multiply-defined label found"
#define ERR_LBL_UNKWN_TYPE		"Internal Error: unknown label type encountered"
#define ERR_LBL_ALLOC_FAIL		"Internal Error: failed to allocate label"