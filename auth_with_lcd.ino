#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define LED_PIN 7 // PIN FOR LED

#define RST_PIN         9           // PIN FOR RFID SCANNER
#define SS_PIN          10          // PIN FOR RFID SCANNER

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
String expectedSecretCode = "777#"; // Initialize expected secret code

LiquidCrystal lcd(8, 6, 5, 4, 3, 2); // PINS FOR LCD SCREEN

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  SPI.begin();
  mfrc522.PCD_Init();
  randomSeed(analogRead(A0)); // Seed the random number generator with an analog input value
  Serial.println(F("Scan card to gain access:"));
}

void loop() {
  lcd.clear();
  lcd.print("Please scan card");
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Initializing variables
  byte block;
  byte len;
  MFRC522::StatusCode status;

  if (!mfrc522.PICC_IsNewCardPresent()) { // Scanning for card 
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("\nExpected Secret Code: " + expectedSecretCode); // Displaying expected secret code

  lcd.clear();
  lcd.print("*Card Detected*");
  Serial.println(F("**Card Detected:**"));
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));

  byte bufferSecret[34]; // Buffer Initialized
  block = 6; // The block where the secret code is stored
  String secretCodeClone = ""; // Initializing variable for secret code to be stored

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  len = 18;
  status = mfrc522.MIFARE_Read(block, bufferSecret, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  for (uint8_t i = 0; i < len; i++) {
    if (bufferSecret[i] >= 32 && bufferSecret[i] <= 126) {
      secretCodeClone += (char)bufferSecret[i];
    }
  }
  secretCodeClone = secretCodeClone.substring(0, secretCodeClone.indexOf('#'));
  expectedSecretCode = expectedSecretCode.substring(0, expectedSecretCode.indexOf('#'));


  secretCodeClone.trim();
  Serial.print("\nSecret Code: " + secretCodeClone);


  if (secretCodeClone.equals(expectedSecretCode)) { // If statement to check if the secret code matches the expected secret code
    String newSecretCode = generateRandomCode(); // Generating a new secret code using randomisation function

    // Convert the new secret code to bytes
    for (int i = 0; i < newSecretCode.length(); ++i) { 
      bufferSecret[i] = newSecretCode.charAt(i);
    }
    
    // Update the secret code on the card
    status = mfrc522.MIFARE_Write(block, bufferSecret, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Writing failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    } else {
      // Update the expected secret code on Arduino
      expectedSecretCode = newSecretCode;
      // Using LCD and LED to show that the secret code matches
      lcd.clear();
      lcd.print("Auth granted");
      digitalWrite(LED_PIN, HIGH);
      delay(1000);
      digitalWrite(LED_PIN, LOW);
      lcd.clear();
      Serial.println(F("\nAuthorization granted!"));
    }
  } else {
      // Using LCD and LED to show that the secret code does not match
      lcd.clear();
      lcd.print("Auth denied");
      delay(1000);
      lcd.clear();
    Serial.println(F("\nAuthorization denied!"));
  }

  Serial.println(F("\n**End Reading**\n"));
  // Clear buffer
  for (int i = 0; i < 34; i++) {
    bufferSecret[i] = 0;
  }
  delay(1000);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

String generateRandomCode() { // Function to generate a new secret code 
  String randomCode;
  for (int i = 0; i < 3; i++) {  // Generate a 3-digit random code
    randomCode += char(random(48, 58)); // ASCII codes for digits 0-9
  }
  randomCode += "#"; // Append '#' character
  Serial.println("\nGenerated Random Code: " + randomCode); // Display the generated random code
  return randomCode;
}
