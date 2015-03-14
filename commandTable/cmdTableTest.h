/* C code produced by gperf version 3.0.1 */
/* Command-line: 'c:\\Program Files (x86)\\GnuWin32\\bin\\gperf.exe' -m=20 --output-file cmdTableTest.h testDict.gperf  */
/* Computed positions: -k'1,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif


#define TOTAL_KEYWORDS 21
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 43
/* maximum key range = 41, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
getHashTest (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned char asso_values[] =
    {
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 15,
      10,  5,  0, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 10, 20,  0,  5,
      44, 44, 10, 44,  8,  0,  0, 44,  0, 44,
       0,  0, 30,  0, 15, 44, 44, 15,  0, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
      44, 44, 44, 44, 44, 44
    };
  return len + asso_values[(unsigned char)str[len - 1]] + asso_values[(unsigned char)str[0]];
}

#ifdef __GNUC__
__inline
#endif
const struct cmdEntry *
getCmdTest (str, len)
     register const char *str;
     register unsigned int len;
{
  static const struct cmdEntry wordListTest[] =
    {
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
#line 11 "testDict.gperf"
      {"NOP",		0x00,	0,	WR_OPCODE_MASK},
#line 16 "testDict.gperf"
      {"LOAD", 		0x05,	2,	WR_ALL_MASK},
#line 27 "testDict.gperf"
      {"LOOP4",		0x13,	1,	WR_OPCODE_MASK | WR_DATFLD_MASK},
#line 19 "testDict.gperf"
      {"SYNCRQ",		0x08,	1,	WR_OPCODE_MASK | WR_DATFLD_MASK},
#line 17 "testDict.gperf"
      {"NEXTBLK",	0x06,	0,	WR_OPCODE_MASK},
#line 20 "testDict.gperf"
      {"SYNCRQ_X",	0x09,	0,	WR_OPCODE_MASK},
#line 22 "testDict.gperf"
      {"DONE",		0x0B,	0,	WR_OPCODE_MASK},
#line 26 "testDict.gperf"
      {"LOOP3",		0x12,	1,	WR_OPCODE_MASK | WR_DATFLD_MASK},
#line 15 "testDict.gperf"
      {"JMP",		0x04,	1,	WR_OPCODE_MASK | WR_DATFLD_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
#line 31 "testDict.gperf"
      {"ENDLOOP4",	0x17,	1,	WR_OPCODE_MASK | WR_DATFLD_MASK},
#line 18 "testDict.gperf"
      {"HOLD",		0x07,	1,	WR_OPCODE_MASK | WR_TIMFLD_MASK},
#line 25 "testDict.gperf"
      {"LOOP2",		0x11,	1,	WR_OPCODE_MASK | WR_DATFLD_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
#line 13 "testDict.gperf"
      {"BRNCH_X",	0x02,	2,	WR_ALL_MASK},
#line 30 "testDict.gperf"
      {"ENDLOOP3",	0x16,	1,	WR_OPCODE_MASK | WR_DATFLD_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
#line 24 "testDict.gperf"
      {"LOOP1",		0x10,	1,	WR_OPCODE_MASK | WR_DATFLD_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
#line 29 "testDict.gperf"
      {"ENDLOOP2",	0x15,	1,	WR_OPCODE_MASK | WR_DATFLD_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
#line 12 "testDict.gperf"
      {"BRNCH",		0x01,	2,	WR_ALL_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
#line 28 "testDict.gperf"
      {"ENDLOOP1",	0x14,	1,	WR_OPCODE_MASK | WR_DATFLD_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
#line 14 "testDict.gperf"
      {"BRNCHDAT",	0x03,	2,	WR_ALL_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
#line 23 "testDict.gperf"
      {"ERR",		0x0C,	0,	WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
      {"",CMD_NOP_CODE,0,WR_OPCODE_MASK},
#line 21 "testDict.gperf"
      {"WAITSYNC",	0x0A,	0,	WR_OPCODE_MASK}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = getHashTest (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordListTest[key].command;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordListTest[key];
        }
    }
  return 0;
}
