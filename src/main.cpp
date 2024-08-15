#define DEBUG     0

#define RST_PIN         D0           // Configurable, see typical pin layout above
#define SS_PIN          D8          // Configurable, see typical pin layout above


#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
MFRC522::MIFARE_Key key;          //create a MIFARE_Key struct named 'key', which will hold the card information

byte sector         = 1;
byte blockAddr      = 4;

byte trailerBlock   = 7;
MFRC522::StatusCode status;
byte buffer[18];
byte size = sizeof(buffer);


byte readDataBlock[] = {
    0x00, 0x00, 0x00, 0x00, // 0,  1,  2,  3
    0x00, 0x00, 0x00, 0x00, // 4,  5,  6,  7
    0x00, 0x00, 0x00, 0x00, // 8,  9,  10, 11
    0x00, 0x00, 0x00, 0x00  // 12, 13, 14, 15
};


/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
      Serial.print(buffer[i] < 0x10 ? " 0" : " ");
      Serial.print(buffer[i], HEX);
  }
}

void readSector(){
  // Show the whole sector as it currently is
  if (DEBUG){Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();
  }
}

void readBlock(){
  // Read data from the block
  if (DEBUG){
    Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
  }
    
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (DEBUG){Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
  dump_byte_array(buffer, 16); Serial.println();
  Serial.println();
  }
  for (byte i = 0; i < 16; i++) {
      readDataBlock[i] = buffer[i];
  }
  for (byte i = 0; i < 16; i++) {
      Serial.print(readDataBlock[i]); Serial.print(", ");

  } Serial.println();
}


void authenticate(){
    // Authenticate using key A
    if (DEBUG){ Serial.println(F("Authenticating using key A...")); }
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
}

// Clear Array after processing
void clearArray(){
  for (int i=0; i<16; i++){
    readDataBlock[i] = 0x00;
  }
  if (DEBUG){
    Serial.println();
    Serial.println("READ DATA");
    for(int i=0; i<16; i++){
      Serial.print(readDataBlock[i]); Serial.print(", ");
    }
    Serial.println();
  }
}

// Initialize.

void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }
  if (DEBUG){
    Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
    Serial.print(F("Using key (for A and B):"));
  }
  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
  if (DEBUG){
    Serial.println();
    Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));
  }
}


void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()){ return; }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) { return; }

  authenticate();
  readSector();
  readBlock();
  clearArray();

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}
