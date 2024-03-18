#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // PIN FOR RFID SCANNER
#define SS_PIN          10          // PIN FOR RFID SCANNER

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

void setup() {
  Serial.begin(9600);                                           // initialize serial communications with the PC
  SPI.begin();                                                  // initialize SPI bus
  mfrc522.PCD_Init();                                              // initialize MFRC522 card
  Serial.println(F("Scan first card to get secret code:"));    //shows in serial that it is ready to read
}

void loop() {

  // Preparing key 
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //initialising variables
  byte block;
  byte len;
  MFRC522::StatusCode status;

  // Reset the loop if no new card present on the sensor/reader. 
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Card Detected:**"));

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card


  Serial.print(F("Name: "));

  byte buffer1[18]; // initialize buffer

  block = 4;
  len = 18;

  // GETTING FIRST NAME FROM CARD
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // PRINT FIRST NAME
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
      Serial.write(buffer1[i]);
    }
  }
  Serial.print(" ");

  // GETTING LAST NAME FROM CARD

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++) {
    Serial.write(buffer2[i] );
  }


  // GETTING SECRET CODE FROM CARD

  byte bufferSecret[18];
  block = 6;
  String secretCodeClone = "";


  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 6, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, bufferSecret, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }


  for (uint8_t i = 0; i < 16; i++) {
    secretCodeClone += (char)bufferSecret[i];
  }

  Serial.print("\nSecret Code: "+secretCodeClone);

  Serial.println(F("\n**End Reading**\n"));

  delay(1000);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

   // Save the secret code of the first card
  String secretCodeCloneFirst = secretCodeClone;

  // Ask the user to scan another card
  Serial.println(F("\nScan another card to update its secret code:"));

  // Wait until a new card is detected
  while (!mfrc522.PICC_IsNewCardPresent()) {
    delay(100);
  }

  // Select the second card
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Second Card Detected:**"));

  // Authenticate with the second card
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 6, &key, &(mfrc522.uid)); // use the correct block number
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write the secret code of the first card to the second card
  status = mfrc522.MIFARE_Write(6, secretCodeCloneFirst.c_str(), secretCodeCloneFirst.length());
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Writing failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.print(F("\nSecret Code of Second Card Updated: "));
  Serial.println(secretCodeCloneFirst);

  // Halt and stop crypto as before
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(1000); //change value if you want to read cards faster
}
