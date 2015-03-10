/*!	\file	parserErrs.h
 *	\brief	Holds #defined string constants for parser errors
 */
 
#define	ERR_PARSE_NO_END		"No \';\' found"
#define	ERR_PARSE_LBL_EMPTY		"\':\' Encountered without preceding label"
#define ERR_PARSE_CMD_LONG		"Command too long"
#define	ERR_PARSE_LBL_LETTR		"Labels must start with a letter"
#define ERR_PARSE_LBL_BAD_CHAR	"Invalid characters found in label: \" \""
#define ERR_PARSE_LBL_CMD_COLL	"Command found where expecting a label"
#define ERR_PARSE_ARG_CNT		"Wrong number of arguments found"
#define ERR_PARSE_CMD_NOT_FOUND	"Command does not exist"

#define ERR_LBL_MULT_DEF		"Multiply-defined label found"
#define ERR_LBL_UNKWN_TYPE		"Internal Error: unknown label type encountered"
#define ERR_LBL_ALLOC_FAIL		"Internal Error: failed to allocate label"

#define ERR_LIT_FORMAT			"LIT directive improperly formatted"
#define ERR_LIT_LETTER			"LIT names must start with a letter"
#define ERR_LIT_MULT_DEF		"Multiply-defined literal found"
#define ERR_LIT_ALLOC_FAIL		"Internal Error: failed to allocate label"

#define ERR_BAD_CMD_TABLE		"Encountered an imporperly formatted command table entry"

#define ERR_BAD_ARG_FMT			"Found an improperly formatted numeric argument"
#define ERR_BAD_ARG_UNDEF		"Found an undefined argument"
#define ERR_LIT_DDR_LOOKUP		"Tried to access DDR offset of a literal"

#define ERR_UNDEF_SYMB_FMT		"\tUndefined symbol \"%s\" (first seen on line %u)\n"

typedef enum {ERR_NONE, ERR_WARNING, ERR_ERROR, ERR_FATAL} errorLevel;
typedef struct error {
	unsigned int	errLine;
	errorLevel		errLvl;
	char 			errMsg[256];
} error_type;