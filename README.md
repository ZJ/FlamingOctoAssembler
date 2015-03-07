# FOR-ALL
(**F**laming **O**cto **R**obot-**A**ssembly **L**anguage **L**oader) 

An assembler project for custom hardware (See [flaming-octo-robot](https://github.com/JQIamo/flaming-octo-robot))

Trying mostly to get the basic structure of things laid out in advance.

## Flaming Octo Assembly Language Format
Valid line formats:
  - `[<LABEL>:] <COMMAND> [<ARG1>[, <ARG2>[, ...<ARGN>]]]; [<COMMENT>]`
  - `;[<COMMENT>]`
  - `LIT <NAME> <VALUE>;[<COMMENT>]`
  - `SETLC <VALUE>;[<COMMENT>]` or other reserved assembler directives 
  - A blank line, or a line filled with only whitespace

Noting that:
  - `LABEL`, and/or `NAME`
    - Must be unique
    - Must not be a word reserved for a `COMMAND`
    - Must start with a letter and contain only alphanumeric ASCII plus underscore (`_`)
  - `COMMAND` must be from the predefined list of commands, 
  - `ARG1`...`ARGN` may be either a defined literal or a numeric value
    - Each `COMMAND`'s arguments have predefined sizes, and values provided as `ARG1`...`ARGN` will be masked to fit those sizes, keeping the LSBs
  - Numeric values will be given as one of:
    - Hex `0x<value>` or `0X<value>`, upper-, lower-, or mixed-case letters acceptable
    - Octal `0<value>`
    - Binary `0b<value>` or `0B<value>`
    - Decimal `<value>`
    - Invalid digit values (e.g. `0b3`, `1A`, or `099`) for any of the above will cause an error.
 
## Machine Code Format
  - 64-bit lines
  - First line, MSB to LSB:
    - OpCode (8 bits)
    - Data (24 bits)
    - Timecode (32 bits)
  - ~~Subsequent lines (part of same instruction) are data aligned on 8-, 16-, 32-, or 64-bit boundaries as defined by that instruction~~ _Every opcode is now a single line_

## Command Translation Table Format
Assuming for the time being a const C-struct array, with the struct formatted as:
  - `unsigned char[9] command` for the command string (Max 8 characters plus the null-terminator)
  - `unsigned char opcode` for the opcode value
  - `unsigned char numArgs` for the total number of arguments to expect
  - `unsigned char flags` to hold a bunch of fields describing how to map arguments into memory:
    - 0: "Write OpCode" if set writes the OpCode to the high byte of the first line.  Will also mask that byte from any attempted writes to the Data or Timecode fields.  If not set, will be available for the Data field to write in, or if the Data field is also unused, the Timecode field. _Ignored if bit 0 is set_
    - 1: "Write Data" if set writes the first argument to the Data field, and masks Data field bytes from attempted writes to the Timecode field. If not set, all bytes will be available to the Timecode field for writing. _Ignored if bit 0 is set_
    - 2: "Write Timecode" if set writes the next argument to the Timecode field.  Will be able to write into either the low 32-, 56, or 64- bytes depenending on the setting of bits 1 and 2. _Ignored if bit 0 is set_
	- [3:7] _Reserved_ (Currently no effect, but set to 0 or face weird behavior in the future) 

### Example Commands
	- "BRANCH"  , 0x??, 2, 0x07; Write in branch's opcode, then the first two branch locations
	- "BRNCHDAT", 0x??, 2, 0x07; Write in branch data's opcode, then the next two locations
	- "JUMP"    , 0x??, 1, 0x03; Write in jumps's opcode, the the jump location in the timecode field (Or use flags = 0x05 and write it to the data field)
	- "NEXTBLK" , 0x??, 0, 0x01; Write just NEXTBLK's opcode and nothing else

## Special Considerations

`LOAD` commands will necessitate a symbol table with two entries.  One for the eventual location in BRAM to be used in the `LOAD` command, and one with the Location Counter entry appropriate to the block it is in.  This also means the assembler  ~~will have to be~~ may wish to be aware of the base address in BRAM it is placing the assembled code. Alternatively, BRAM addresses might be saved as offsets relative to the first instruction.

Because a `LOAD` or similar command will exist, we'll need a special assembler directive `SETLC` or similar.  This would indicate to the assembler that the location counter has a new value so that the symbols used to set up `JUMP`s &c. will point to appropriate locations.  This directive would not emit any machine code to memory, rather it would only change the internal state of the assembler.

## Assembler Directives
Assembler directives alter the internal state of the assembler without causing any machine code to be output.
  - `LIT` Sets up a mapping from a string to a value
  - `SETLC` Sets the location counter to a specified value. Used for the results of jump and/or move commands and their effects on the stack.
  