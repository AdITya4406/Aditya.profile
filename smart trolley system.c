#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

// Define RFID pins
#define RST_PIN 9
#define SS_PIN 10

// Push button pins
#define ADD_ITEM_BTN 2
#define RESET_BTN 3
#define REMOVE_ITEM_BTN 4

// Buzzer pin
#define BUZZER_PIN 5

// Initialize LCD and RFID
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);

// Predefined UIDs and corresponding items
const byte UID1[] = {0x93, 0x17, 0x2B, 0x19}; // Example UID 1
const byte UID2[] = {0xCC, 0xA1, 0x51, 0x4A}; // Example UID 2
const byte UID3[] = {0x8C, 0x9F, 0x08, 0x38}; // Example UID 3
const char *items[] = {"Milk", "Bread", "Eggs"};
const float prices[] = {50.0, 30.0, 10.0};

float total = 0.0; // Total cost
bool itemAdded[3] = {false, false, false}; // Track added items (false means not added, true means added)

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Trolley");
  delay(2000);

  pinMode(ADD_ITEM_BTN, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);
  pinMode(REMOVE_ITEM_BTN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.clear();
  lcd.print("Add your item");
}

void loop() {
  // Handle button presses
  if (digitalRead(RESET_BTN) == LOW) {
    resetCart();
    delay(300); // Simple debounce for reset button
  }

  if (digitalRead(REMOVE_ITEM_BTN) == LOW) {
    promptRemoveItem();
    delay(300); // Simple debounce for remove button
  }

  // RFID scanning
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    processCard();
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}

void processCard() {
  if (compareUID(UID1)) {
    addItem(0);
  } else if (compareUID(UID2)) {
    addItem(1);
  } else if (compareUID(UID3)) {
    addItem(2);
  } else {
    lcd.clear();
    lcd.print("Item not found");
    buzzError();
    delay(2000);
    lcd.clear();
    lcd.print("Add your item");
  }
}

bool compareUID(const byte *uid) {
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] != uid[i]) {
      return false;
    }
  }
  return true;
}

void addItem(int itemIndex) {
  if (itemAdded[itemIndex]) {
    // Item already added
    lcd.clear();
    lcd.print(items[itemIndex]);
    lcd.setCursor(0, 1);
    lcd.print("Already added");
    buzzError();
    delay(2000);
  } else {
    // Item not added yet, add to cart
    itemAdded[itemIndex] = true; // Mark the item as added
    total += prices[itemIndex];
    lcd.clear();
    lcd.print(items[itemIndex]);
    lcd.setCursor(0, 1);
    lcd.print("Price: ");
    lcd.print(prices[itemIndex]);
    buzzSuccess();
    delay(2000);
    lcd.clear();
    lcd.print("Total: ");
    lcd.print(total);
    delay(2000);
    lcd.clear();
    lcd.print("Add your item");
  }
}

void resetCart() {
  total = 0.0;
  for (int i = 0; i < 3; i++) {
    itemAdded[i] = false; // Reset the "added" state of all items
  }
  lcd.clear();
  lcd.print("Cart Reset");
  buzzReset();
  delay(2000);
  lcd.clear();
  lcd.print("Add your item");
}

void promptRemoveItem() {
  lcd.clear();
  lcd.print("Scan item to");
  lcd.setCursor(0, 1);
  lcd.print("remove from cart");

  // Wait for RFID scan
  while (true) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      if (compareUID(UID1)) {
        removeSpecificItem(0);
      } else if (compareUID(UID2)) {
        removeSpecificItem(1);
      } else if (compareUID(UID3)) {
        removeSpecificItem(2);
      } else {
        lcd.clear();
        lcd.print("Item not found");
        buzzError();
        delay(2000);
        lcd.clear();
        lcd.print("Scan item to");
        lcd.setCursor(0, 1);
        lcd.print("remove from cart");
        break; // Exit the loop after a failed scan
      }
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      break; // Exit the loop after a successful scan
    }
  }
}

void removeSpecificItem(int itemIndex) {
  if (itemAdded[itemIndex]) {
    itemAdded[itemIndex] = false;  // Mark the item as removed
    total -= prices[itemIndex];
    lcd.clear();
    lcd.print(items[itemIndex]);
    lcd.setCursor(0, 1);
    lcd.print("Removed");
    buzzRemove();
    delay(2000);
    lcd.clear();
    lcd.print("Total: ");
    lcd.print(total);
    delay(2000);
    lcd.clear();
    lcd.print("Add your item");
  } else {
    lcd.clear();
    lcd.print("Item not in cart");
    buzzError();
    delay(2000);
    lcd.clear();
    lcd.print("Scan item to");
    lcd.setCursor(0, 1);
    lcd.print("remove from cart");
  }
}

// Buzzer functions
void buzzSuccess() {
  tone(BUZZER_PIN, 1000, 200); // 1 kHz tone for 200 ms
  delay(200);
}

void buzzError() {
  tone(BUZZER_PIN, 500, 200); // 500 Hz tone for 200 ms
  delay(200);
}

void buzzReset() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1000, 100); // 1 kHz short beep
    delay(100);
  }
}

void buzzRemove() {
  tone(BUZZER_PIN, 750, 200); // 750 Hz tone for 200 ms
  delay(200);
}