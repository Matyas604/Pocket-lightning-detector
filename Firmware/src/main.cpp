#include <Arduino.h> 
#include <Wire.h> // Include the Wire library for I2C communication
#include <SparkFun_AS3935.h> // Include the AS3935 library for lightning sensor
#include <BLEDevice.h> // Include the BLE library
#include <BLEServer.h> // Include the BLE Server library
#include <BLEUtils.h> // Include the BLE Utils library
#include <BLE2902.h> // Include the BLE Descriptor library
#include <time.h> // Include time library for timestamping lightning events
#include <sys/time.h> 
#include <esp_attr.h> // Include ESP32 attributes for RTC memory

// Pin definitions
#define PIN_SDA 4 //Pin for I2C SDA
#define PIN_SCL 5 //Pin for I2C SCL
#define PIN_IRQ 2 //Pin for AS3935 IRQ output
#define BT_SWITCH_PIN 3 //Switch to toggle Bluetooth
#define BUZZER_PIN 7 //Pin for buzzer

// Function prototypes
void runBluetoothMode();
void goToSleep();
void processLightning();

// Lightning event structure
struct LightningEvent {
  time_t timestamp;
  uint8_t distance;
  uint32_t energy;
};

// Store lightning events in RTC memory
RTC_DATA_ATTR LightningEvent history[50]; // Store up to 50 lightning events
RTC_DATA_ATTR int eventCount = 0;

// Initialize AS3935 lightning sensor and BLE server
SparkFun_AS3935 Lightning;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Buzzer functions
void beep(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
  delay(duration);
  noTone(BUZZER_PIN);
}

// Alert function for lightning events
void alertLightning() {
  beep(2500, 100);
  delay(50);
  beep(2500, 100);
}

// Process lightning events
void processLightning() {
  uint8_t interuptSource = Lightning.readInterruptReg(); // Read the interrupt source from the AS3935

  if (interuptSource == 0x08) { // Lightning detected
    uint8_t distance = Lightning.distanceToStorm(); // Distance in km
    uint32_t energy = Lightning.lightningEnergy(); // Energy of the lightning

    if (eventCount < 50) { // Store the lightning event in RTC memory
      history[eventCount] = {time(NULL), distance, energy};
      eventCount++;
    }
    alertLightning();
  }
}

// BLE server callbacks
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    beep(1500, 300); // Beep on connection
  }
  void onDisconnect(BLEServer* pServer) { 
    deviceConnected = false;
    BLEDevice::startAdvertising();
  }
};

// BLE characteristic callbacks
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue(); 
    if (rxValue.length() > 0 && rxValue[0] == 'T') {
      String timestampStr = "";
      for (size_t i = 1; i < rxValue.length(); i++) timestampStr += rxValue[i]; 

      struct timeval tv = { .tv_sec = (time_t)timestampStr.toInt() }; // Convert the received timestamp string to time_t and set the system time
      settimeofday(&tv, NULL);
      beep(2000, 50); // Beep on time sync
    }
  }
};

// Setup function
void setup() {
  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output
  pinMode(BT_SWITCH_PIN, INPUT_PULLUP); // Set Bluetooth switch pin as input
  pinMode(PIN_IRQ, INPUT); // Set AS3935 IRQ pin as input

  Wire.begin(PIN_SDA, PIN_SCL); // Initialize I2C communication

  if (!Lightning.begin()) { 
    beep(500, 1000); // Beep on initialization failure
  }

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
    processLightning();
  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED) {
    beep(3000, 50); // Beep on undefined wakeup
  }

// Checks the Bluetooth switch state and either run Bluetooth mode or go to sleep
  if (digitalRead(BT_SWITCH_PIN) == LOW) {
    runBluetoothMode();
  } else {
    goToSleep();
  }
}

// Bluetooth mode function
void runBluetoothMode() {
  BLEDevice::init("Lightning Detector"); // Bluetooth device name
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  pCharacteristic = pService->createCharacteristic(
                      "beb5483e-36e1-4688-b7f5-ea07361b26a8",
                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  pServer->getAdvertising()->start();

  // Sends lightning event history every 3 seconds if a device is connected
  while (digitalRead(BT_SWITCH_PIN) == LOW) {
    if (deviceConnected) {
      String dataOut = "LIGHTNINGS:\n";
      if (eventCount == 0) {
        dataOut += "There are no lightnings yet.";
      } else {
        for (int i = 0; i < eventCount; i++) {
          struct tm *tm_info = localtime(&history[i].timestamp);
          char timeBuf[20];
          strftime(timeBuf, 20, "%H:%M:%S", tm_info);
          dataOut += String(timeBuf) + " | " + String(history[i].distance) + "km | " + String(history[i].energy) + "\n";
        }
      }
      pCharacteristic->setValue(dataOut.c_str());
      pCharacteristic->notify();
    }
    delay(3000);
  }
  goToSleep();
}

// Function to put the ESP32 into deep sleep mode
void goToSleep() {
  esp_deep_sleep_enable_gpio_wakeup(1 << PIN_IRQ, ESP_GPIO_WAKEUP_GPIO_HIGH); // Wake up on logic high of the lightning sensor's IRQ pin
  esp_deep_sleep_start(); 
}

// Loop function (not used in this project)
void loop() {
}