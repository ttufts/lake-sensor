#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>
#include <esp_sleep.h>

#include "pins.h"

namespace {

SPIClass radio_spi(FSPI);
SX1262 radio = new Module(node_pins::kLoRaNss, node_pins::kLoRaDio1,
                          node_pins::kLoRaReset, node_pins::kLoRaBusy,
                          radio_spi);

void set_vext(bool enabled) {
  pinMode(node_pins::kVextControl, OUTPUT);
  digitalWrite(node_pins::kVextControl, enabled ? LOW : HIGH);
}

void print_identity() {
  Serial.println("\n=== Heltec V3 lake-sensor diagnostic ===");
  Serial.printf("chip: %s, revision %u, %u core(s)\n", ESP.getChipModel(),
                ESP.getChipRevision(), ESP.getChipCores());
  Serial.printf("CPU: %u MHz; flash: %u bytes; free heap: %u bytes\n",
                ESP.getCpuFreqMHz(), ESP.getFlashChipSize(), ESP.getFreeHeap());
  Serial.printf("reset reason: %d; wake cause: %d\n",
                static_cast<int>(esp_reset_reason()),
                static_cast<int>(esp_sleep_get_wakeup_cause()));
}

void test_vext() {
  Serial.println("Vext OFF");
  set_vext(false);
  delay(500);
  Serial.println("Vext ON for 500 ms (GPIO36 LOW)");
  set_vext(true);
  delay(500);
  set_vext(false);
  Serial.println("Vext OFF (GPIO36 HIGH)");
}

void test_radio_init() {
  Serial.println("Initializing SX1262 without transmitting...");
  radio_spi.begin(node_pins::kLoRaSck, node_pins::kLoRaMiso,
                  node_pins::kLoRaMosi, node_pins::kLoRaNss);
  const int state = radio.begin(915.0F, 125.0F, 9, 5, 0x12, 14);
  Serial.printf("SX1262 begin result: %d (%s)\n", state,
                state == RADIOLIB_ERR_NONE ? "PASS" : "FAIL");
  if (state == RADIOLIB_ERR_NONE) {
    const int sleep_state = radio.sleep();
    Serial.printf("SX1262 sleep result: %d (%s)\n", sleep_state,
                  sleep_state == RADIOLIB_ERR_NONE ? "PASS" : "FAIL");
  }
  Serial.println("No RF packet was transmitted.");
}

void print_help() {
  Serial.println("Commands: i=identity, v=Vext pulse, r=radio init/sleep, s=10 s deep sleep, h=help");
}

void deep_sleep_test() {
  Serial.println("Entering timer deep sleep for 10 seconds...");
  set_vext(false);
  Serial.flush();
  esp_sleep_enable_timer_wakeup(10ULL * 1000000ULL);
  esp_deep_sleep_start();
}

}  // namespace

void setup() {
  set_vext(false);
  Serial.begin(115200);
  delay(1000);
  print_identity();
  test_vext();
  test_radio_init();
  print_help();
}

void loop() {
  if (!Serial.available()) {
    delay(20);
    return;
  }
  switch (Serial.read()) {
    case 'i': print_identity(); break;
    case 'v': test_vext(); break;
    case 'r': test_radio_init(); break;
    case 's': deep_sleep_test(); break;
    case 'h': print_help(); break;
    case '\r':
    case '\n': break;
    default: Serial.println("Unknown command; type h for help."); break;
  }
}

