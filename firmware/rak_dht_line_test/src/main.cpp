#include <Arduino.h>
namespace {
constexpr uint8_t kRxPin = PIN_SERIAL1_RX;  // RAK19003 J7 RXD.
constexpr uint8_t kTxPin = PIN_SERIAL1_TX;  // RAK19003 J7 TXD.
constexpr uint8_t kAlternateRxPin = PIN_SERIAL2_RX;
constexpr uint8_t kAlternateTxPin = PIN_SERIAL2_TX;
constexpr uint8_t kSdaPin = PIN_WIRE_SDA;
constexpr uint8_t kSclPin = PIN_WIRE_SCL;
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(kRxPin, INPUT_PULLUP);
  pinMode(kTxPin, INPUT_PULLUP);
  pinMode(kAlternateRxPin, INPUT_PULLUP);
  pinMode(kAlternateTxPin, INPUT_PULLUP);
  pinMode(kSdaPin, INPUT_PULLUP);
  pinMode(kSclPin, INPUT_PULLUP);
  Serial.println("J7 input test: monitoring both RAK UART mappings");
}

void loop() {
  const int rx = digitalRead(kRxPin);
  const int tx = digitalRead(kTxPin);
  const int alternate_rx = digitalRead(kAlternateRxPin);
  const int alternate_tx = digitalRead(kAlternateTxPin);
  const int sda = digitalRead(kSdaPin);
  const int scl = digitalRead(kSclPin);
  Serial.printf("P0.13/SDA=%s P0.14/SCL=%s P0.15=%s P0.16=%s P0.19=%s P0.20=%s\n",
                sda ? "H" : "L", scl ? "H" : "L",
                rx ? "H" : "L", tx ? "H" : "L",
                alternate_rx ? "H" : "L", alternate_tx ? "H" : "L");
  delay(500);
}
