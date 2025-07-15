

EPprogmr.exe is a companion tool used with the Arduino EPROM programmer
or Arrduino Flash (EEPROM) Programmer
discribed here: https://sites.google.com/site/ericmklaus/projects-1/unoepromprogrammer
and here:  https://sites.google.com/site/ericmklaus/projects-1/uno-flash-programmer

This is a command line program (so simply clicking on it won't help much)

From a command window enter:   EPprogmr {filename} {port#}
where {filename} is the name of an intel hex file and {port#} is the number 
from the COM port your Arduino is connected to (use device manager to find this)

NOTE: the {port#} paramater is just the number like "3" NOT "COM3"

Example:  EPprogmr rom01.hex 5
  this will upload the file rom01.hex to the programmer connected to COM5

EPprogmr.cbp is a CodeBlocks project file if you want to use the CodeBlocks
IDE to build the utility.  
 
