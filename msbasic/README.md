# Microsoft BASIC for 6502

This is a single integrated assembly source tree that can generate nine different versions of Microsoft BASIC for 6502.

By running ./make.sh, this will generate .bin files for ROM. The CC65 compiler suite is need to build this project.

The following symbols can be defined in addition:

| Configuration Symbol              | Description
| --------------------------------- | --------------------------------------------------------------------------------
| CONFIG_CBM1_PATCHES               | jump out into CBM1's binary patches instead of doing the right thing inline
| CONFIG_CBM_ALL                    | add all Commodore-specific additions except file I/O
| CONFIG_DATAFLG                    | ?
| CONFIG_EASTER_EGG                 | include the CBM2 "WAIT 6502" easter egg
| CONFIG_FILE                       | support Commodore PRINT#, INPUT#, GET#, CMD
| CONFIG_IO_MSB                     | all I/O has bit #7 set
| CONFIG_MONCOUT_DESTROYS_Y         | Y needs to be preserved when calling MONCOUT
| CONFIG_NO_CR                      | terminal doesn't need explicit CRs on line ends
| CONFIG_NO_LINE_EDITING            | disable support for Microsoft-style "@", "_", BEL etc.
| CONFIG_NO_POKE                    | don't support PEEK, POKE and WAIT
| CONFIG_NO_READ_Y_IS_ZERO_HACK     | don't do a very volatile trick that saves one byte
| CONFIG_NULL                       | support for the NULL statement
| CONFIG_PEEK_SAVE_LINNUM           | preserve LINNUM on a PEEK
| CONFIG_PRINTNULLS                 | whether PRINTNULLS does anything
| CONFIG_PRINT_CR                   | print CR when line end reached
| CONFIG_RAM                        | optimizations for RAM version of BASIC, only use on 1.x
| CONFIG_ROR_WORKAROUND             | use workaround for buggy 6502s from 1975/1976; not safe for CONFIG_SMALL!
| CONFIG_SAFE_NAMENOTFOUND          | check both bytes of the caller's address in NAMENOTFOUND
| CONFIG_SCRTCH_ORDER               | where in the init code to call SCRTCH
| CONFIG_SMALL                      | use 6 digit FP instead of 9 digit, use 2 character error messages, don't have GET
| CONFIG_SMALL_ERROR                | use 2 character error messages

Changing symbol definitions can alter an existing base configuration, but it not guaranteed to assemble
or work correctly.

## Credits

* Main work by Michael Steil <mist64@mac.com>.
* Additional work by Michael Steil, Robert M. Münch, and others.
* Thanks to the people who have contributed to the [6502.org](http://www.6502.org) wiki.

## TODO
* convert messy init code into completely different
  files without ifdefs (not much in common!)
* move all machine specific code into separate files
* rename all labels that point to RTS to RTSn
* add AppleSoft comments
* look for all " $", i.e. (zeropage) constants, replace them
  with symbols
* convert platform ifdefs in generic files into feature ifdefs or macros
* reconstruct pre-CBM1, i.e. CBM1 without the patches
* add some comments to every file
