// clearing EEPROM for debugging purpose

#include <EEPROM.h>
void setup() {
  Serial.begin(9600);
  // ############## EEPROM CLEAR ######################################
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < 512; i++){
    EEPROM.write(i, 0);
  }
    
  delay(5000);
  Serial.println("EEPROM cleared");
    
  // ############## end EEPROM CLEAR ##################################
}

void loop() {
  // put your main code here, to run repeatedly:

}
