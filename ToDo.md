# ToDo.md

## Codebase changes
  - [x] Move documentation from symbolHashTable.c to symbolHashTable.h
  - [x] Flesh out documentation to do parameters & return values
  - [x] Change one-line functions to Macros
  - [x] Add BRAM offset to symbol table
  - [x] Add literal table
  - [x] Check initialization in newSymbol()
  - [x] gperf/gnuperf working
  - [x] Generate a dummy command table
  - [x] Implement processCmd()
  - [x] Implement findCommand()
  - [ ] Shift some functions out of driver.c
  - [ ] Test 1st pass on a dummy file

## Toolchain/Environment Management
  - [ ] ~~Setup Lint for syntax checking~~
    - [ ] ~~Find/Download lint~~
	- [ ] ~~Compile lint~~
	- [ ] ~~Invoke lint in pre-commit hook~~
  - [x] Setup doxygen for command-line invocation
  - [ ] Setup makefiles
  - [ ] Add doc generation to makefile
