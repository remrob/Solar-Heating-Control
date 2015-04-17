  /*
 * Heating Control for REMROB-Plattform
 *
 * Reading from S0-impulse Interface(EN 62053-31) of Powermeter, and writing pulses into EEPROM.
 * Transfer data via Yun-Bridge  
 *
 * Author: Michael Macherey
 * 
 */
 
// OneWire DS18B20 Temperature
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library
 
 
#include <EEPROMex.h>
#include <OneWire.h>
#include <Bridge.h>

// ################# begin bridge ######################################

char switchValue[2];        // digital pin 12 for switching relay

// ################# end bridge ########################################

// ################# begin vars DS18B20 Temperature Sensor #############

OneWire  ds(10);  // on pin 10 (a 4.7K resistor is necessary)

float celsius;    // temperature of Sensor

// ################# end DS18B20 Temperature Sensor ####################

// ################ begin vars S0 Interface#############################

// these values are saved in EEPROM
const byte EEPROM_ID = 0x99;                       // used to identify if valid data in EEPROM
volatile double impulse = 0.000;                   // store impulse

// constants used to identify EEPROM addresses
const int ID_ADDR = 5;                             // the EEPROM address used to store the ID
const int addressOnEEPROM = 7;                     // the EEPROM address used to store the impulse 

// Writing and retrieving double = 1010102.500,this are four bytes of EEPROM memory.
double outputOfEEPROM;

// Interrupt at SZero(S0) Pin
const int SZeroPin = 2;                           // initializing S0 Pin

// stores first byte of EEPROM.read(ID_ADDR)
byte id;

//################ end S0 Interface ####################################

void setup() {
  // needed only for debugging purpose at Serial Monitor
  Serial.begin(9600);
  
  // ################# setup S0 Interface ################################
  
  pinMode(SZeroPin, INPUT);                            // defining SZeroPin as INPUT/OUTPUT
  
  attachInterrupt(1, saveImpulseToEEPROM, FALLING);   // Interrupt settings for Arduino-Yun
  
  // ################# end setup S0 Interface ################################
  
  // ################# setup bridge ##########################################
  
  pinMode(12,OUTPUT);          // defining pin 12 as OUTPUT for bridge communication
  
  // this is for debugging purpose
  // pinMode(13,OUTPUT);
  
  Bridge.begin();             // Starts Bridge, facilitating communication between the AVR and Linux processor
                              // begin() is a blocking function, this process takes approximately three seconds.
  // ################# end setup bridge ######################################
  
  // ####  initializing S0 interface and set EEPROM_ID #######################
  
  id = EEPROM.read(ID_ADDR);                     // read the first byte from the EEPROM
  
  // delay is for debugging purpose at Serial Monitor
  //delay(5000);
  
  if(id == EEPROM_ID){
    // here if the id value of EEPROM.read matches the value saved when writing EEPROM
    
    readImpulseFromEEPROM();
    delay(500);
/*    
    Serial.print("Output of EEPROM if EEPROM_ID is set = " );
    printDouble(outputOfEEPROM, 1000);
    Serial.println("");
*/   
  }else {
  // here if the ID is not found, so write the default data
    
  // Serial.print("ID is not found , setting ID!");
    EEPROM.write(ID_ADDR, EEPROM_ID);
    delay(200);
    readImpulseFromEEPROM();
    delay(200);
    
    // is for debugging purpose at Serial Monitor
    // debugTraceElse();
    
  }
  // ####  end initializing S0 interface #########################################
}


void loop() {
  
  // ######### begin bridge ##########
  
  Bridge.get("switch1",switchValue,2);
  int D12int = atoi(switchValue);   // atoi = Alpha to integer
  digitalWrite(12,D12int);
  
  // this is for debugging purpose
  // digitalWrite(13,D12int);
  // Serial.print("String(celsius) = ");
  // Serial.print(String(celsius));
  
  Bridge.put("celsiusOutdoor", String(celsius));  // Send Outdoor Temperature 
  
  // ######### end bridge ##########
  
  // is for debugging purpose of S0 interface at Serial Monitor
  //delay(1000);
  debugTrace();

  // Read temperature Sensor
   readTemperature();

}

void readTemperature() {
  
byte i;
byte present = 0;
byte type_s = 0;
byte data[12];
byte addr[8];

  if ( !ds.search(addr)) {
//    Serial.println("No more addresses.");
//    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

// Serial.print("  Data = ");
//  Serial.print(present, HEX);
//  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
//    Serial.print(data[i], HEX);
//    Serial.print(" ");
  }
  
/*  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();
*/ 
    // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  Serial.print("Temperature = ");
  Serial.print(celsius);
  Serial.println(" Celsius");
 
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
   delay(500);
   if(digitalRead(SZeroPin)  == 1){
    /* 
    int dR = digitalRead(SZeroPin);
    Serial.print("dR = " );
    Serial.print(dR);
    */
    readImpulseFromEEPROM();
    delay(50);
    Serial.print("EEPROMv= " );
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
