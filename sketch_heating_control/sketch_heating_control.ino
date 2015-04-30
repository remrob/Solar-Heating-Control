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

// ################ begin vars S0 Interface #############################

// constant used to identify EEPROM addresses

const int addressOnEEPROM = 7;                     // the EEPROM address used to store the impulse 

// Writing and retrieving double = 1010102.500,this are four bytes of EEPROM memory.
double outputOfEEPROM;

// Interrupt at SZero(S0) Pin
const int SZeroPin = 2;                           // initializing S0 Pin

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
  
  // this is for debugging purpose with onboard LED
  // pinMode(13,OUTPUT);
  
  Bridge.begin();             // Starts Bridge, facilitating communication between the AVR and Linux processor
                              // begin() is a blocking function, this process takes approximately three seconds.
  // ################# end setup bridge ######################################
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
  Bridge.put("celsiusOutdoor", String((int)celsius));  // Send Outdoor Temperature 
  
  Bridge.put("PowerMeterImpulse", String(0.001 + outputOfEEPROM,3));  // Send Powermeter impulse with 3 decimals
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
    Serial.println("No more addresses.");
    Serial.println();
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

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();
 
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
    Serial.println("Interrupt");
   EEPROM.updateDouble(addressOnEEPROM, 0.001 + outputOfEEPROM);
   
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
    Serial.println(outputOfEEPROM,3);
    Serial.print("");
    Serial.println("##################");
 
  }
  // +++ end debugging block
}
