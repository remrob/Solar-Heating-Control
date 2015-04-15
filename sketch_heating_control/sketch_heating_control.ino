  /*
 * Heating Control for REMROB-Plattform
 *
 * Reading from S0-impulse Interface(EN 62053-31) of Powermeter, and writing pulses into EEPROM.
 * Transfer data via Yun-Bridge  
 *
 * Author: Michael Macherey
 * 
 */
 
#include <EEPROMex.h>

// these values are saved in EEPROM
const byte EEPROM_ID = 0x99;                       // used to identify if valid data in EEPROM
volatile double impulse = 0.000;                   // store impulse

// constants used to identify EEPROM addresses
const int ID_ADDR = 5;                             // the EEPROM address used to store the ID
const int addressOnEEPROM = 7;                     // the EEPROM address used to store the impulse 

// Writing and retrieving double = 1010102.500,this are four bytes of EEPROM memory.
double outputOfEEPROM;

// Interrupt at SZero(S0) Pin
const int SZeroPin = 2;                            // initializing S0 Pin

// stores first byte of EEPROM.read(ID_ADDR)
byte id;

void setup() {
  Serial.begin(9600);
  
  pinMode(SZeroPin, INPUT);                              // defining SZeroPin as INPUT/OUTPUT
  
  attachInterrupt(1, saveImpulseToEEPROM, FALLING);      // Interrupt settings for Arduino-Yun
  
  id = EEPROM.read(ID_ADDR);                             // read the first byte from the EEPROM
  
  // delay is for debugging purpose at Serial Monitor
  delay(5000);
  
  if(id == EEPROM_ID){
    // here if the id value of EEPROM.read matches the value saved when writing EEPROM
    
    readImpulseFromEEPROM();
    delay(500);
    Serial.print("Output of EEPROM if EEPROM_ID is set = " );
    printDouble(outputOfEEPROM, 1000);
    Serial.println("");
   
  }else {
  // here if the ID is not found, so write the default data
    
    Serial.print("ID is not found , setting ID!");
    EEPROM.write(ID_ADDR, EEPROM_ID);
    delay(200);
    readImpulseFromEEPROM();
    delay(200);
    
    // is for debugging purpose at Serial Monitor
    debugTraceElse();
    
  }
}

void loop() {
  // is for debugging purpose at Serial Monitor
  debugTrace();
}

void readImpulseFromEEPROM(){              // Read impulse from EEPROM

  //Serial.println("inside ImpulseFromEEPROM" );
  outputOfEEPROM = EEPROM.readDouble(addressOnEEPROM);
}

void saveImpulseToEEPROM(){                  // Save impulse to EEPROM 
  // Attention: The EEPROM has an endurance of at least 100,000 write/erase cycles.
   EEPROM.updateDouble(addressOnEEPROM, 0.001 + outputOfEEPROM);
}

void printDouble( double val, unsigned int precision){
// this function is needed to print more than two decimial places e.g. 0.001 at the serial Monitor
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

   Serial.print (int(val));  //prints the int part
   Serial.print("."); // print the decimal point
   unsigned int frac;
   if(val >= 0){
     frac = (val - int(val)) * precision;
   }else{
      frac = (int(val)- val ) * precision;
   }
   int frac1 = frac;
   
   while( frac1 /= 10 )
       precision /= 10;
   precision /= 10;
   while(  precision /= 10)
       Serial.print("0");

   Serial.println(frac,DEC) ;
}

void debugTrace(){
    // +++ the following block is for debugging purpose
   if(digitalRead(SZeroPin)  == 1){
    /* 
    int dR = digitalRead(SZeroPin);
    Serial.print("dR = " );
    Serial.print(dR);
    */
    Serial.print("EEPROMv= " );
    readImpulseFromEEPROM();
    delay(1500);
    printDouble(outputOfEEPROM, 1000);
    Serial.print("");
    Serial.println("##################");
 
  }
  // +++ end debugging block
}

void debugTraceElse(){
  
    Serial.print("Output of EEPROM = ");
    printDouble(outputOfEEPROM, 1000);
    Serial.print("");
}
