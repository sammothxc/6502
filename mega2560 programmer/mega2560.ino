// ******************************************************************
// Flash EPROM Programmer for Arduino MEGA   V1  1/28/23 by E. Klaus
// Designed for AT28C256 Flash EEPROM
// *** SET BOARD = "Genuino Mega or Mega 2560" ******
// ******************************************************************

#define PIN_DATA0   49   // EEPROM D0    PORTL0
#define PIN_DATA1   48   // EEPROM D1    PORTL1
#define PIN_DATA2   47   // EEPROM D2    PORTL2
#define PIN_DATA3   46   // EEPROM D3    PORTL3
#define PIN_DATA4   45   // EEPROM D4    PORTL4
#define PIN_DATA5   44   // EEPROM D5    PORTL5
#define PIN_DATA6   43   // EEPROM D6    PORTL6
#define PIN_DATA7   42   // EEPROM D7    PORTL7

#define DATA_PORT   PORTL  // PORT for D0-D7 PORTL
#define DATA_DDR    DDRL   // Data Direction Register for D0-D7
#define DATA_PIN    PINL   // Data READ Register for D0-D7

#define PIN_ADDR0   37   // EEPROM A0    PORTC 0
#define PIN_ADDR1   36   // EEPROM A1    PORTC 1
#define PIN_ADDR2   35   // EEPROM A2    PORTC 2
#define PIN_ADDR3   34   // EEPROM A3    PORTC 3
#define PIN_ADDR4   33   // EEPROM A4    PORTC 4
#define PIN_ADDR5   32   // EEPROM A5    PORTC 5
#define PIN_ADDR6   31   // EEPROM A6    PORTC 6
#define PIN_ADDR7   30   // EEPROM A7    PORTC 7
#define PIN_ADDR8   22   // EEPROM A8    PORTA 0
#define PIN_ADDR9   23   // EEPROM A9    PORTA 1
#define PIN_ADDR10  24   // EEPROM A10   PORTA 2
#define PIN_ADDR11  25   // EEPROM A11   PORTA 3
#define PIN_ADDR12  26   // EEPROM A12   PORTA 4
#define PIN_ADDR13  27   // EEPROM A13   PORTA 5
#define PIN_ADDR14  28   // EEPROM A14   PORTA 6

#define ADDR_LO_PORT  PORTC   // PORT for A0-A7  PORTC 
#define ADDR_LO_DDR   DDRC    // Data Direction Register for A0-A7
#define ADDR_HI_PORT  PORTA   // PORT for A8-A14  PORTA
#define ADDR_HI_DDR   DDRA    // Data Direction Register for A8-A14


#define PIN_EPROM_OE  39 // EEPROM OE    PORTG 2  
#define PIN_EPROM_WE  40 // EEPROM WE    PORTG 1
#define PIN_EPROM_CE  41 // EEPROM CE    PORTG 0
#define PIN_LED       53 // LED          PORTB 0

#define MAX_READ_COUNT    10
#define MAX_PGM_TRIES     25
#define MAX_IHEX_REC      32

// Global Variables
unsigned char addrLOW, addrHIGH, dataByte, outByte, led;
unsigned char dataBuf[36];
unsigned int  address;
unsigned char test16[16]= {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB,0xCC, 0xDD, 0xEE, 0xF0};
int pgmLoopCt, recLen, pgmLen;
// the setup routine runs once when you press reset:
void setup() {                
  int x;
	Serial.begin(9600);    // initialize the Serial port object

 pinMode (PIN_EPROM_WE, OUTPUT);
 digitalWrite(PIN_EPROM_WE, HIGH);  // WE = OFF 

  pinMode(PIN_EPROM_CE, OUTPUT);   // EPROM Enable HIGH (Diasbled)
  digitalWrite(PIN_EPROM_CE,HIGH);

  pinMode(PIN_EPROM_OE, OUTPUT);   // Output Enable HIGH (Disabled)
  digitalWrite(PIN_EPROM_OE,HIGH);

  pinMode(PIN_LED, OUTPUT);    // Shield LED on Pin 53 (PB0)
  digitalWrite(PIN_LED, LOW);  

  for(x=0; x<5; x++)
     {
      digitalWrite(PIN_LED, HIGH);
      delay(50);
      digitalWrite(PIN_LED, LOW);
      delay(50);
     } 

  setDataPinsAsInput();      // Set data pins to high impedance state (INPUT)  
  setAddressPinsAsOutput();  // Set address pins as OUTPUTS = 7FFF 
	showMenu();            // print the menu                                     
	Serial.println();      // print a blank line
}

// *** Main Program Loop *****
void loop() {
	char k;
  char outBuf[40];
  byte data = 0;
  unsigned char dataIn, dataOut;
  int x, i, v;
  unsigned int addr;

  if(++address > 500)   // Blink the activity indicator
    {
      address=0;
      if(led & 0x01)
        digitalWrite(PIN_LED, HIGH);
      else
        digitalWrite(PIN_LED, LOW);

      led++;    
    }
  
	if(Serial.available() )
	{
		k = Serial.read();
		Serial.println(k); 
		
		switch(k)
		{
      case 'B':
        Serial.println("\r\n Blinking...");
        for(x=0; x<20; x++)
           {
            digitalWrite(PIN_LED, HIGH);
            delay(100);
            digitalWrite(PIN_LED, LOW);
            delay(100);
           } 
        Serial.println(" Done\r\n");
      break;  
      case 'Q': // DEMO:toggel control signals
        Serial.println("\r\n Enable Line Test...");
        for(x=0; x<20; x++)
           {
            enableOutputs(true);     // set OE LOW
            enableEPROM(true);       // Set CE LOW
            enableWrite(true);       // Set WE LOW
            delay(200);
            enableOutputs(false);     // set OE High
            enableEPROM(false);       // Set CE HIGH
            enableWrite(false);       // Set WE HIGH
            delay(200);
           } 
        Serial.println(" Done\r\n");     
      break;
      case 'J':
        Serial.println("\r\n Disable Software Protect...");
        disableDataProtection();
        Serial.println(" Done\r\n");     
      break;

      case '#':
        Serial.println("\r\n Software Chip Erase... ARE YOU SURE? (Y/N)");
        while((k != 'Y')&&(k != 'N')&&(k != 0x1B))  
            { 
             while(!Serial.available()) ;      //wait for a byte from terminal
             k = Serial.read();               //read a byte
            } 
        if(k=='Y')
          {
           softwareChipErase();
           Serial.println(" Done\r\n");     
          }
        else
          Serial.println(" Aborted\r\n");       
      break;
      
      case 'A':  //DEMO: Write 1 byte with pauses to measure signals
        enableEPROM(true);         //  Set CE High
        enableWrite(false);         // Set WE High
        enableOutputs(false);       //  Set OE HIGH
        addr = getStartAddress();
        Serial.print(F("\r\n  Data Value (In Hex x00- xFF):"));
        data = SerialGetHexValue(2,1); // Get 2hex bytes from the terminal convert to a value.
        
        writeDataByte(data);
        
        flushSerialInput();
        Serial.println("\r\n Press Enter to start write pulse");
        while((k != 0x0D)&&(k != 0x0A)&&(k != 0x1B))  
            { 
             while(!Serial.available()) ;      //wait for a byte from terminal
             k = Serial.read();               //read a byte
            } 

         // 1st part of sendWritePulse()
         enableEPROM(true);         //  Set CE LOW
         enableWrite(true);         // Set WE LOW
        
        k=0;
        flushSerialInput();
        Serial.println("\r\n Press Enter to end write pulse");
        while((k != 0x0D)&&(k != 0x0A)&&(k != 0x1B))  
            { 
             while(!Serial.available()) ;      //wait for a byte from terminal
             k = Serial.read();               //read a byte
            } 
            
        // 2nd part of sendWritePulse()
        enableWrite(false);        // Set WE HIGH
        enableEPROM(false);        //  Set CE HIGH
        delay(500);
        
        Serial.print(" Done  ");
        
        setDataPinsAsInput();     // Prepare IO for read
        enableEPROM(true);        //  Set CE Low
        enableOutputs(true);      //  Set OE Low
        delay(1);
        k = readDataByte();
        Serial.println(k, HEX);
      break; 
      
      case 'W':  // Write a fixed set of 16 bytes to flash
        getStartAddress();
        enableOutputs(false);  //Inhibit programming E=HIGH G=HIGH
        enableEPROM(false);
        enableWrite(false);    // Set WE HIGH
        delayMicroseconds(10); // necessary delay... 
        
        for(x=0; x<16; x++)
           {
            setAddress(address);        // Send the address to the EPROM address lines
            writeDataByte(test16[x]);   // Set data value
            sendWritePulse();
            address++;
           }

        enableWrite(false);
        setDataPinsAsInput();       // Prepare IO for read
        enableEPROM(true);  // 
        enableOutputs(true);  
        Serial.println("Done");
      break; 
			case 'd':   // ***** Dump 16 bytes  *****
        getStartAddress();
        read16(address);
        Serial.println();
      break;

      case 'D':  // *****  Dump 256 bytes  *****
        getStartAddress();
        read256(address);
        Serial.println();
      break;
      
      case 'p':   // ***** Program 1 byte  *****
        getStartAddress(); // prompt, get 4 hex bytes,set address variable and set addr lines.
        Serial.print(F("\r\n  Data Value (In Hex x00- xFF):"));
        dataIn = SerialGetHexValue(2,1); // Get 2hex bytes from the terminal convert to a value.
        Serial.println();
        
        writeData(dataIn);          // Write data to EEPROM
        
        enableWrite(false);         // Set WE pin HIGH
        setDataPinsAsInput();       // Prepare IO for read
        enableOutputs(true);        // Set OE pin LOW
        enableEPROM(true);          // Set CE pin LOW
        delay(10);                  // Wait for chip to respond (we really need this)
        
        dataOut = readDataByte();   // Read the EPROM Data lines
        
        if(dataOut == dataIn)
           Serial.println("OK");
        else
          {
           sprintf(outBuf, " ERROR: %02X  !=  %02X",dataOut, dataIn);
           Serial.println(outBuf);
          }
      break;

      case 'P':   // *****  Program 16 bytes   *******
        
        getStartAddress();  //Prompt user for start address and send to EPROM address lines
        
        Serial.print(F("\r\nEnter 16 HEX Bytes (with spaces between each byte):\r\n"));
        flushSerialInput();
        
        for(i=0; i<16; i++)   //Populate dataBuf[] with 16 bytes of input data
           {
            v = SerialGetHexByte(true);
            if(v == -1)
               break;
               
            dataBuf[i] = (unsigned char)v;
           }
           
         k=0;
         
        if(i > 15)   // If user did not abort input (press enter or escape)
          {
           Serial.print(F("\r\n programming "));     
           for(x=0; x<16; x++)
              {
               dataByte= dataBuf[x];      // Get 1 byte from input buffer
               writeData(dataByte);       //Program the byte 
               Serial.print(".");
               address++;               // Increment the address  
               setAddress(address);     // Send the address to the EPROM address lines  
              }
          }
		         
        enableWrite(false);
        Serial.println("Done..");
      break;
      
      case 'I':  // *****  program iHex Record (interactive test mode)  *****
        Serial.print(F("\r\n :"));
        
        recLen = get_ihex_rec(dataBuf, &address, true);
        if((recLen > 0)&&(recLen <33))
          {
           pgmLen = programBuffer(dataBuf, recLen, address);
           if(pgmLen != recLen)
             {
              Serial.print(F("\r\n Programming Error at byte#: "));
              Serial.println(pgmLen);
             }
           else
             {
              Serial.print(F("\r\n Programmed ")); 
              Serial.print(pgmLen); 
              Serial.println(F(" bytes")); 
             }
          }
        else
          {
           Serial.print("\r\n Invalid Input Record ");
           sprintf(outBuf, "%04X",address);
           Serial.println(outBuf);
          }   
      break;
      
      case ':':  // *****  program iHex Record (no echo)  *****
        Serial.println();  //respond with : LF&CR

        recLen = get_ihex_rec(dataBuf, &address, false);  //get the record

        if((recLen > 0)&&(recLen <33))
          {
           pgmLen = programBuffer(dataBuf, recLen, address);
           if(pgmLen != recLen)
              Serial.print("\r\nE*P\r\n");   // Programming error response
           else
              Serial.print("\r\nOK \r\n");   // Programming SUCCESS response
          }
        else
          {
           Serial.print("\r\n?");      // checksum error response
           Serial.println(address, HEX);
          }   
      break;

      case ';':  // *****  send the requested iHex Record *****
        Serial.print(" ");  //respond with single space

        recLen = send_ihex_rec(false);  //get the length and address and send the record

        if(recLen == 0)
            Serial.println("?");   // error response
        else
            Serial.println();     
   
      break;
      
      case '@':    // System attention command (sent from PC)
           Serial.print("$$$");    //respond with "$$$" to verify connection.
      break;
        
			case 'm':
			case 'M':
				showMenu();
				break;
		}
		
	}
	
	delay(1);  
}

// ******************************************************************
// getStartAddress()
// Ask user for a starting address in HEX, set global address value
// ******************************************************************
unsigned int getStartAddress(void)
{
  flushSerialInput();     // disregard any trailing LF/CR chars.
  enableEPROM(false);     // Set CE pin High
  Serial.print(F("  Enter Start Address (In Hex x0000 - xFFFF):"));
  address = SerialGetHexValue(4,1); // Get 4 hex bytes from the terminal convert to a value.
  setAddress(address);     // Send the address to the EPROM address lines 
  return address;          // Return the addres we just set.
}

// ******************************************************************
// read1Byte(unsigned int address)
// ******************************************************************
unsigned char read1Byte(unsigned int address)
{
  setAddress(address);        // Send the address to the EPROM address lines 
  enableEPROM(true);          // Set CE pin LOW
  enableWrite(false);         // Set WE pin High
  enableOutputs(true);        // Set OE pin LOW
  dataByte = readDataByte();  // Read the EPROM Data lines
  return dataByte;            // Return the byte read
}

// ***********************************************************
// read16(address)
// Read and report 16 bytes 
// ***********************************************************
void read16(unsigned int address)
{
 unsigned char x, d, dataBuf[17];
 char outBuf[15];

 // Print the Address
 sprintf(outBuf, "\r\n %04X  ",address);
 Serial.print(outBuf);
 
 setDataPinsAsInput();
 enableEPROM(true);          // Set CE pin LOW
 enableWrite(false);         // Set WE pin HIGH

 // Read 16 bytes -> dataBuf
 for(x=0; x<16; x++)
   {
    enableOutputs(false);        // Set OE pin HIGH
    setAddress(address);         // Send the address to the EPROM address lines 
    enableOutputs(true);         // Set OE pin LOW
    delayMicroseconds(100);      // Wait for chip to respond to new address
    
    d = readDataByte();          // Read the EPROM Data lines
    
    sprintf(outBuf,"%02X ", d);  // Print the data byte
    if(x==7)
       outBuf[2]='-';
       
    Serial.print(outBuf);
    delayMicroseconds(100);      // Wait for serial output
    
    dataBuf[x] = d;              
    address++;
   }
    
 Serial.print("    ");         
 for(x=0; x<16; x++)           // Print the 16 ASCII Values 
    {
     d = dataBuf[x];
     if((d<0x20)||(d>0x7F))    // Show non-printables as "."
       d = '.'; 
       
     Serial.print((char)d);
    }     
 }

// ***********************************************************
// read256(address)
// Read and report 256 bytes 
// ***********************************************************
void read256(unsigned int address)
{
 int x;
 for(x=0; x<16; x++)
    {
     read16(address);
     address+=16;   
    }
}

// ***********************************************************
// disableDataProtection()
// Send the command to disable the software protection mode 
// ***********************************************************
void disableDataProtection(void)
{
  enableOutputs(false);      //Disable outputs OE = HIGH
  enableEPROM(false);        // CE = HIGH
  enableWrite(false);        // WE = HIGH
  delay(10);
  
  setAddress(0x5555);     // Send the address to the EPROM address lines
  writeDataByte(0xAA);     // Set data value
  sendWritePulse();        // Send write signal
  
  setAddress(0x2AAA);     // Send the address to the EPROM address lines
  writeDataByte(0x55);     // Set data value
  sendWritePulse();        // Send write signal

  setAddress(0x5555);     // Send the address to the EPROM address lines
  writeDataByte(0x80);     // Set data value
  sendWritePulse();        // Send write signal        

  setAddress(0x5555);     // Send the address to the EPROM address lines
  writeDataByte(0xAA);     // Set data value
  sendWritePulse();        // Send write signal
          

  setAddress(0x2AAA);     // Send the address to the EPROM address lines
  writeDataByte(0x55);     // Set data value
  sendWritePulse();        // Send write signal

  setAddress(0x5555);     // Send the address to the EPROM address lines
  writeDataByte(0x20);     // Set data value
  sendWritePulse();        // Send write signal        

  setAddress(0x0000);     // Send the address to the EPROM address lines
  writeDataByte(0xFF);     // Set data value
  sendWritePulse();        // Send write signal        
}

// ***********************************************************
// softwareChipErase()
// Send the command to erase the entire chip
// ***********************************************************
void softwareChipErase(void)
{
  enableOutputs(false);      //Disable outputs OE = HIGH
  enableEPROM(false);        // CE = HIGH
  enableWrite(false);        // WE = HIGH
  delay(10);
  
  setAddress(0x5555);     // Send the address to the EPROM address lines
  writeDataByte(0xAA);     // Set data value
  sendWritePulse();        // Send write signal
  
  setAddress(0x2AAA);     // Send the address to the EPROM address lines
  writeDataByte(0x55);     // Set data value
  sendWritePulse();        // Send write signal

  setAddress(0x5555);     // Send the address to the EPROM address lines
  writeDataByte(0x80);     // Set data value
  sendWritePulse();        // Send write signal        

  setAddress(0x5555);     // Send the address to the EPROM address lines
  writeDataByte(0xAA);     // Set data value
  sendWritePulse();        // Send write signal
          

  setAddress(0x2AAA);     // Send the address to the EPROM address lines
  writeDataByte(0x55);     // Set data value
  sendWritePulse();        // Send write signal

  setAddress(0x5555);     // Send the address to the EPROM address lines
  writeDataByte(0x10);     // Set data value
  sendWritePulse();        // Send write signal  

  delay(30);
  disableDataProtection();
}


// ******************************************************************
// writeData(inData)
//  *Assumes address has already been set.
// ******************************************************************
void writeData(unsigned char inData)
{
  enableOutputs(false);    // OE = HIGH
  enableEPROM(false);      // CE = HIGH
  enableWrite(false);      // WE = HIGH
        
  writeDataByte(inData);   // Set data value
  sendWritePulse();        // Send write signal
}

// ******************************************************************
// programBuffer(buf, buflen, address)
// Send bytes from buf to EEEPROM at specified address.
// NOTE: 64 byte MAX    
// ******************************************************************
int programBuffer(unsigned char *buf, int buflen, unsigned int address)
{
 int i, x;
 unsigned int addrHold = address;  // Save starting address
 unsigned char dataByte, outByte;

 enableOutputs(false);        // Set OE pin HIGH
 enableEPROM(false);
 enableWrite(false);       // Set WE HIGH
 
 for(i=0; i<buflen; i++)
    {
     setAddress(address);     // Send the address to the EPROM address lines
     dataByte = buf[i];       // Get 1 byte from input buffer
     writeData(dataByte);     // Write the byte
     address++;               // Increment the address  
    }

  setDataPinsAsInput();
  enableEPROM(true);          // Set CE pin LOW
  enableWrite(false);         // Set WE pin HIGH
  enableOutputs(true);        // Set OE pin LOW for READ

  for(i=0; i < MAX_READ_COUNT; i++)    // Check DATA POLLING feature for end of write cycle
     {                                 // Wrire may take up to 10ms 
      delay(1);
      outByte = readDataByte();        
      if(outByte == dataByte)          // This will not be true until the write cycle is complete
        break;
     }

  //  Read data to verify the write operation  
  address=addrHold;             // Restore starting address  
 
  for(i=0; i<buflen; i++)
    {
     setAddress(address);       // Send the address to the EPROM address lines
     dataByte = buf[i];         // Get 1 byte from input buffer
     outByte = readDataByte();  // Read the EPROM Data
     if(outByte != dataByte)    // Check for ERROR
        break;
        
     address++;                // Increment the address  
    }
    
  return i;   // return # of bytes programmed.
}

// ******************************************************************
//  sendWritePulse()   Send signal to initiate a byte write
//  * outputs must be DISABLED:  enableOutputs(false)
// ******************************************************************
void sendWritePulse(void)
{
  enableEPROM(true);         // Set CE LOW
  enableWrite(true);         // Set WE LOW
  delayMicroseconds(10);
  enableWrite(false);        // Set WE HIGH
  enableEPROM(false);        // Set CE HIGH
}

// ******************************************************************
// enableEPROM(boolean bEnable)
// set the E (chip enable) pin on the EPROM High or LOW 
// (LOW=Enabled High=Disabled)  
// ******************************************************************
void enableEPROM(boolean bEnable)
{
  if(bEnable)
    digitalWrite(PIN_EPROM_CE, LOW);
  else
    digitalWrite(PIN_EPROM_CE, HIGH);   
}

// ******************************************************************
// enableOutputs(boolean bEnable)
// set the G (output enable) pin on the EPROM High or LOW 
// (LOW=Enabled High=Disabled)
// ******************************************************************
void enableOutputs(boolean bEnable)
{
  if(bEnable)
     digitalWrite(PIN_EPROM_OE, LOW);
  else
    digitalWrite(PIN_EPROM_OE, HIGH);   
}

// ******************************************************************
// enableWrite(boolean bEnable)
// ******************************************************************
void enableWrite(boolean bEnable)
{
  if(bEnable)
    digitalWrite(PIN_EPROM_WE, LOW);
  else
    digitalWrite(PIN_EPROM_WE, HIGH);   
}

// ******************************************************************
//  setDataPinsAsInput() 
//  Configure data bus pins as INPUTS (high impedance state)
//  Arduino MEGA pins 42-49  PORTL0-PORTL8
// ******************************************************************
void setDataPinsAsInput(void)
{
 DATA_DDR  = 0x00;   // This sets Pins 42-49 = INPUTS
 DATA_PORT = 0xFF;   // engage pull-up
}

// ******************************************************************
//  setDataPinsAsOutPut() 
//  Configure data bus pins as OUTPUTS 
//  Arduino MEGA pins 42-49  PORTL bits 0-7 
//  NOTE Make sure EPROM outputs are DISABLED   OE=1 or CE=1
// ******************************************************************
void setDataPinsAsOutput(void)
{
 DATA_DDR = 0xFF;   // This sets Pins 42-49 = OUTPUTS
 DATA_PORT = 0xFF;   // Set all bits HIGH
}

// ******************************************************************
//  setAddressPinsAsOutPut() 
//  Configure address pins A0-A14 as OUTPUTS 
//  Arduino MEGA pins 30-37 (A0-A7) pins 22-28 (A8-A14) 
// ******************************************************************
void setAddressPinsAsOutput(void)
{
 ADDR_LO_DDR  = 0xFF;    // DDRC = OUTPUTs   C0-C7
 ADDR_LO_PORT = 0xFF;    // PORTC = HIGH     C0-C7
 ADDR_HI_DDR  = 0x7F;    // DDRA  = OUTPUTS  A0-A6
 ADDR_HI_PORT = 0x7F;    // PORTA = HIGH     A0-A6
}


// ******************************************************************
// setAddress(addr)
// Clock the High & Low bytes of the address into the shift register
//  chips attached to the EPROM address lines
// ******************************************************************
void setAddress(unsigned int addr)
{
 unsigned char x, addrHi, addrLo;
 addrHi = (addr>>8) & 0x00FF;      //Get address High 8 bits -> addrHi
 addrLo = (addr & 0x00FF);         //Get address Low 8 bits -> addrLo
 
 ADDR_LO_PORT = addrLo;            // Send to pins 30-37
 ADDR_HI_PORT = addrHi;            // Send to pins 22-28
}

// ******************************************************************
// readDataByte(void)
// read the EPROM data pins until 2 reading match
// Assumes chip is in the proper enabled state and address is stable.
// ******************************************************************
unsigned char readDataByte(void)
{
 unsigned char read, lastRead;
 int i;
 
 setDataPinsAsInput();  // This sets Pins PL0-PL7 = INPUTS
 lastRead = read;       // read up to 10 times until 2 consecutive readings match
 for(i=0; i < MAX_READ_COUNT; i++) 
    {
     delayMicroseconds(10);                        
     read = DATA_PIN;        // int read PORTL
     
     if(read == lastRead)   // Compare last reading with this reading
       break;               // if they match we're done..
     else
       lastRead = read;  
    }
     
 return read;
}

// ******************************************************************
// writeDataByte(unsigned char data)
// set the EPROM data pins
// ******************************************************************
void writeDataByte(unsigned char data)
{
 setDataPinsAsOutput(); // This sets Pins4-11 = OUTPUTS
 DATA_PORT = data;
}

// ******************************************************************
// showMenu()
// Display a menu on the serial monitor
// ******************************************************************
void showMenu(void)
{
  Serial.println(F("** AT28C256 MEGA Flash Programmer Main Menu v1.0  **"));
  Serial.print(F("B  Blink LED\r\n"));
  Serial.print(F("d  Dump 16 Bytes\r\n"));
  Serial.print(F("D  Dump 256 Bytes\r\n"));
  Serial.print(F("p  Program 1 Byte\r\n"));
  Serial.print(F("P  Program 16 Bytes\r\n"));
  Serial.print(F("J  Disable Software Protect\r\n"));
  Serial.print(F("Q  Toggle Control Signals\r\n"));
  Serial.print(F("A  Write 1 byte with pauses\r\n"));
  Serial.print(F("W  Write 16 bytes (fixed set of data)\r\n"));
  Serial.print(F("I  Program Intel Hex Record(Interactive Mode)\r\n"));
  Serial.print(F(":  Program Intel Hex Record\r\n"));
  Serial.print(F("#  Software Chip Erase\r\n"));
  Serial.print(F("M  Re-Display Menu\r\n"));
}

// ***********************************************************
//  get_ihex_rec(unsigned char *buf, unsigned int *addr, bEcho)
//  input and store data from Intel Hex File record
//  NOTE: *buf MUST point to an array large enough to store expected data
//  iHex Format:  :ssaaaattdddddddd.....cs
//     where ss= record length, aaaa=address, tt=type dd=data cs=checksum
//  e.g   :10200000CE2C8ABDFBDC86F79701961484F79714D3
//  length=0x10 address=2000 type=00 {16 bytes of data} D3=checksum   
// ***********************************************************
int get_ihex_rec(unsigned char *buf, unsigned int *addr, char bEcho)
{
 int p, h, cs_sum=0;
 unsigned char data, recLen, recType, addr_hi, addr_lo, cs1, cs2;

 *addr =0;  //Error Return Address
 
 // Get Record Length  
 recLen = (unsigned char)SerialGetHexByte(bEcho);
 if(recLen > MAX_IHEX_REC)  // Record too large
    return recLen;
  
 // Get Address  
 h = SerialGetHexByte(bEcho);
 if(h == -1)
   return 0;
   
 addr_hi=(unsigned char)h;
 
 h = SerialGetHexByte(bEcho);
 if(h == -1)
   return 0;

 addr_lo=(unsigned char)h;
 
 
 // Get Record Type  
 h = SerialGetHexByte(bEcho);
 if(h == -1)
   return 0;
 
 recType=(unsigned char)h;
  
 // Get Data Bytes
 for(p=0; p<recLen; p++)
 {
  h = SerialGetHexByte(bEcho);
  if(h == -1)
     return 0;
     
  data = (unsigned char)h;
  buf[p]=data;
  cs_sum+=data;   
 }
 
 // Get Record Checksum compare to checksum calculated  
 cs1=(unsigned char)SerialGetHexByte(bEcho);
 cs_sum += (recLen + addr_hi + addr_lo + recType);
 cs2 =(unsigned char)((~cs_sum)&0x00FF)+1;
 
 //** TESTING Messages ***
 //Serial.print(F("\r\n checksum: 0x"));
 //Serial.println(cs2,HEX);
 
 if((cs2 != cs1)&&(cs1 !=0))  //Test data has zero checksum - ignore it
   {
    recLen=0;
    addr_hi=cs1;
    addr_lo=cs2;             // set address to failed checksum values
   }
 
 *addr = (addr_hi <<8)+ addr_lo;
 return recLen;
}

// ***********************************************************
//  send_ihex_rec(bEcho)
//  send data requested for Intel Hex File record
//  message from host PC is ";ssaaaa" (rec size & address)
//  here we respond with the data followed by the checksum
//  iHex Format:  :ssaaaattdddddddd.....cs
//     where ss= record length, aaaa=address, tt=type dd=data cs=checksum
//  e.g   :10200000CE2C8ABDFBDC86F79701961484F79714D3
//  length=0x10 address=2000 type=00 {16 bytes of data} D3=checksum   
// ***********************************************************
int send_ihex_rec(char bEcho)
{
 int p, h;
 unsigned int cs_sum=0;
 unsigned char data, recLen=0, addr_hi, addr_lo, cs2;
 char outBuf[4];

 enableEPROM(true);          // Set E pin LOW
 enableOutputs(true);        // Set G pin LOW
 
 // Get Record Length  
 recLen = (unsigned char)SerialGetHexByte(bEcho);
 if(recLen > MAX_IHEX_REC)  // Record too large
    return recLen;
  
 // Get Address  
 h = SerialGetHexByte(bEcho);
 if(h == -1)
   return 0;
   
 addr_hi=(unsigned char)h;
 
 h = SerialGetHexByte(bEcho);
 if(h == -1)
   return 0;

 addr_lo=(unsigned char)h;
 address = (addr_hi <<8)+ addr_lo;
  
 // Note: Record Type is always zero
   
 // Get Data Bytes
 cs_sum=0;
 for(p=0; p<recLen; p++)
 {
  setAddress(address);       // Send the address to the EPROM address lines
  data = readDataByte();     // Read the EPROM Data lines
  cs_sum+=data;              // Add data value to checksum total
  address++;                 // point to next address
  SerialSendHexByte(data);   //send data as 2 hex characters
 }
 
 // Calculate Checksum   
 cs_sum += (recLen + addr_hi + addr_lo);
 cs2 =(unsigned char)((~cs_sum)&0x00FF)+1;
 SerialSendHexByte(cs2);     //send checksum as 2 hex characters
  
 return recLen;
}

// ***************************************************************************
// flushSerialInput()
// Wait for any pendint incoming bytes to be recieved by the serial port.
// ***************************************************************************
void flushSerialInput(void)
{
  delay(10);                 // wait a bit..
  while(Serial.available())  // if any data available
       Serial.read();        //read byte and discard
}

// ***************************************************************************
// SerialGetHexChar()
// Wait for  a character from terminal in the rance of '0'-'9' or 'A'-'F'
// If Esc, LF or CR are encountered exit.
// Return the character entered.
// ***************************************************************************
char SerialGetHexChar(char bEcho)
{
  char inByte=0; 
  
  //do until CR,LF, or Esc  is encountered
  while((inByte != 0x0D)&&(inByte != 0x0A)&&(inByte != 0x1B))  
      { 
       while(!Serial.available()) ;      //wait for a byte from terminal
       inByte= toupper(Serial.read());    //read a byte and convert to UPPERCASE
       
       if(bEcho)
         Serial.write(inByte);
       
       if((inByte >= '0')&&(inByte <= '9'))
            return inByte;
            
       if((inByte >= 'A')&&(inByte <= 'F'))
            return inByte;
      } 
      
  return inByte;
}

// ***************************************************************************
// hexChar2Val(inChar)
// ***************************************************************************
unsigned char hexChar2Val(char inChar)
{
 char inVal; 
 inVal = inChar - '0';
 if(inVal > 9)
   inVal-=7;
 return inVal; 
}

// ***************************************************************************
// SerialGetHexByte()   Wait for 2 Hex characters and return the byte value.
// Returns 0-255 (0x00 - 0xFF) OR -1 if Enter or Esc was entered
// ***************************************************************************
int SerialGetHexByte(char bEcho)
{
  char  k;
  unsigned char val;
   
  k = SerialGetHexChar(bEcho);
  if(k < '0')
     return -1;
    
  val = hexChar2Val(k);
  val = val << 4;
            
  k = SerialGetHexChar(bEcho);
  if(k < '0')
     return -1;
     
  val += hexChar2Val(k);
  
  return val; 
 }  
          
// ***************************************************************************
// SerialGetHexValue()
// Wait for up to 4 HEX characters from terminal.
// Terminate input when LF, CR, Esc or 4 Hex bytes have been entered.
// Otherwise return the Value Entered.
// NOTE: This function call flushSerialInput() so it's designed for user input.
//       not reccommended for automated or machine (PC) input. 
// ***************************************************************************
unsigned int SerialGetHexValue(char digits, char bEcho)
{
  char inByte=0; 
  char inVal;
  char valBuf[4]={0,0,0,0};
  int ctDigits=0, bufIndex=0;
  unsigned int retVal=0;
   
  flushSerialInput(); //clear any leftover data.
  
  while(inByte != 0x1B)  //do until Esc  is encountered
       {
        inByte = SerialGetHexChar(bEcho);
        if(inByte >= '0')
          {
           inVal = inByte-'0';
           if(inVal > 9)
             inVal-=7;
           
           for(bufIndex=3; bufIndex >0; bufIndex--)
              valBuf[bufIndex]=valBuf[bufIndex-1];
              
           valBuf[bufIndex] = inVal;
           
           ctDigits++;
          }
           
        if((inByte < '0')||(ctDigits > 3)||(ctDigits == digits))
          {
           retVal =  valBuf[0] + (valBuf[1]*16)+ (valBuf[2]*256)+ (valBuf[3]*4096);  
           break;
          }
      } 
      
 return retVal;
}

// ***************************************************************************
// SerialSendHexByte(data)   send data as 2 Hex characters 
// ***************************************************************************
void SerialSendHexByte(unsigned char data)
{
 char  k;
  
 k = (data >> 4) & 0x0F;
 if(k > 9)
    k += 0x37;
 else
    k += 0x30;  
  
 Serial.print(k);
  
 k = data & 0x0F;
 if(k > 9)
    k += 0x37;
 else
    k += 0x30;  
  
 Serial.print(k);
}  
