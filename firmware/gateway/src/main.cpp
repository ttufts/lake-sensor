#include <Arduino.h>
#include <PubSubClient.h>
#include <RadioLib.h>
#include <SPI.h>
#include <WiFi.h>

#include "gateway_config_select.h"
#include "lake_protocol.h"
#include "pins.h"

namespace {
constexpr float kFrequencyMhz = 915.0F;
constexpr float kBandwidthKhz = 125.0F;
constexpr uint8_t kSpreadingFactor = 9;
constexpr uint8_t kCodingRate = 5;
constexpr uint8_t kSyncWord = 0x12;
constexpr uint8_t kAckType = 2;
constexpr size_t kAckSize = 7;

SPIClass radio_spi(FSPI);
SX1262 radio = new Module(node_pins::kLoRaNss, node_pins::kLoRaDio1,
                          node_pins::kLoRaReset, node_pins::kLoRaBusy,
                          radio_spi);
WiFiClient network;
PubSubClient mqtt(network);
uint32_t last_wifi_attempt_ms = 0;
uint32_t last_mqtt_attempt_ms = 0;
uint32_t last_sequence[256]{};
bool sequence_seen[256]{};
bool discovery_published[256]{};

struct DiscoverySensor {
  const char* object_id;
  const char* name;
  const char* json_key;
  const char* unit;
  const char* device_class;
  const char* state_class;
  const char* entity_category;
};

constexpr DiscoverySensor kDiscoverySensors[] = {
    {"depth", "Lake Level", "depth_m", "m", "distance", "measurement", ""},
    {"temperature", "Temperature", "temperature_c", "°C", "temperature", "measurement", ""},
    {"battery", "Battery Voltage", "battery_v", "V", "voltage", "measurement", "diagnostic"},
    {"loop_current", "Loop Current", "loop_ma", "mA", "current", "measurement", "diagnostic"},
    {"rssi", "LoRa RSSI", "rssi_dbm", "dBm", "signal_strength", "measurement", "diagnostic"},
    {"snr", "LoRa SNR", "snr_db", "dB", "", "measurement", "diagnostic"},
    {"flags", "Status Flags", "flags", "", "", "", "diagnostic"},
    {"sequence", "Sequence", "sequence", "", "", "", "diagnostic"},
};

void make_topic(char* out, size_t size, uint8_t node_id, const char* suffix) {
  snprintf(out, size, "%s/node/%u/%s", gateway_config::kTopicPrefix, node_id,
           suffix);
}

void service_connections() {
  const uint32_t now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (now - last_wifi_attempt_ms >= 10000U) {
      last_wifi_attempt_ms = now;
      Serial.printf("wifi connecting to %s\n", gateway_config::kWifiSsid);
      WiFi.begin(gateway_config::kWifiSsid, gateway_config::kWifiPassword);
    }
    return;
  }
  if (!mqtt.connected() && now - last_mqtt_attempt_ms >= 5000U) {
    last_mqtt_attempt_ms = now;
    char availability[128];
    snprintf(availability, sizeof(availability), "%s/gateway/%s/availability",
             gateway_config::kTopicPrefix, gateway_config::kGatewayId);
    const bool connected = mqtt.connect(
        gateway_config::kGatewayId, gateway_config::kMqttUsername,
        gateway_config::kMqttPassword, availability, 1, true, "offline");
    Serial.printf("mqtt connect: %s\n", connected ? "ok" : "failed");
    if (connected) mqtt.publish(availability, "online", true);
  }
  if (mqtt.connected()) mqtt.loop();
}

void send_ack(uint32_t sequence) {
  uint8_t ack[kAckSize] = {'L', 'T', kAckType, 0, 0, 0, 0};
  for (int i = 0; i < 4; ++i) ack[3 + i] = sequence >> (8 * i);
  delay(25);
  const int state = radio.transmit(ack, sizeof(ack));
  Serial.printf("ack seq=%lu state=%d\n", static_cast<unsigned long>(sequence),
                state);
}

bool publish_discovery(uint8_t node_id) {
  if (discovery_published[node_id]) return true;
  char state_topic[128];
  char availability_topic[128];
  char discovery_topic[160];
  char payload[768];
  make_topic(state_topic, sizeof(state_topic), node_id, "state");
  snprintf(availability_topic, sizeof(availability_topic),
           "%s/gateway/%s/availability", gateway_config::kTopicPrefix,
           gateway_config::kGatewayId);
  for (const auto& sensor : kDiscoverySensors) {
    snprintf(discovery_topic, sizeof(discovery_topic),
             "homeassistant/sensor/lake_node_%u/%s/config", node_id,
             sensor.object_id);
    snprintf(payload, sizeof(payload),
             "{\"name\":\"%s\",\"unique_id\":\"lake_node_%u_%s\","
             "\"state_topic\":\"%s\",\"value_template\":"
             "\"{{ value_json.%s }}\",\"availability_topic\":\"%s\","
             "\"device\":{\"identifiers\":[\"lake_node_%u\"],"
             "\"name\":\"Lake Monitor Node %u\",\"manufacturer\":"
             "\"Lake Monitor\",\"model\":\"RAK4631 lake node\"}%s%s%s%s%s%s}",
             sensor.name, node_id, sensor.object_id, state_topic,
             sensor.json_key, availability_topic, node_id, node_id,
             sensor.unit[0] ? ",\"unit_of_measurement\":\"" : "",
             sensor.unit, sensor.unit[0] ? "\"" : "",
             sensor.device_class[0] ? ",\"device_class\":\"" : "",
             sensor.device_class, sensor.device_class[0] ? "\"" : "");
    // Append optional state/entity fields separately to keep the table readable.
    const size_t used = strlen(payload);
    if (used == 0 || payload[used - 1] != '}') return false;
    payload[used - 1] = '\0';
    snprintf(payload + used - 1, sizeof(payload) - used + 1,
             "%s%s%s%s%s%s}",
             sensor.state_class[0] ? ",\"state_class\":\"" : "",
             sensor.state_class, sensor.state_class[0] ? "\"" : "",
             sensor.entity_category[0] ? ",\"entity_category\":\"" : "",
             sensor.entity_category,
             sensor.entity_category[0] ? "\"" : "");
    if (!mqtt.publish(discovery_topic, payload, true)) {
      Serial.printf("discovery publish failed: %s\n", sensor.object_id);
      return false;
    }
  }
  discovery_published[node_id] = true;
  Serial.printf("Home Assistant discovery published for node %u\n", node_id);
  return true;
}

void publish_packet(const lake::PacketV1& packet, float rssi, float snr) {
  if (!mqtt.connected()) {
    Serial.println("mqtt offline; packet not published");
    return;
  }
  if (!publish_discovery(packet.node_id)) return;
  char topic[128];
  char payload[384];
  make_topic(topic, sizeof(topic), packet.node_id, "state");
  snprintf(payload, sizeof(payload),
           "{\"node_id\":%u,\"sequence\":%lu,\"depth_m\":%.3f,"
           "\"sense_v\":%.3f,\"loop_ma\":%.3f,\"battery_v\":%.3f,"
           "\"temperature_c\":%.2f,\"flags\":%u,\"rssi_dbm\":%.1f,"
           "\"snr_db\":%.2f}",
           packet.node_id, static_cast<unsigned long>(packet.sequence),
           packet.depth_mm / 1000.0F, packet.sense_mv / 1000.0F,
           packet.loop_ua / 1000.0F, packet.battery_mv / 1000.0F,
           packet.temperature_centi_c / 100.0F, packet.flags, rssi, snr);
  const bool published = mqtt.publish(topic, payload, true);
  Serial.printf("mqtt publish %s: %s\n", topic, published ? "ok" : "failed");
}
}  // namespace

void setup() {
  pinMode(node_pins::kVextControl, OUTPUT);
  digitalWrite(node_pins::kVextControl, HIGH);
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_STA);
  mqtt.setServer(gateway_config::kMqttHost, gateway_config::kMqttPort);
  mqtt.setBufferSize(1024);
  radio_spi.begin(node_pins::kLoRaSck, node_pins::kLoRaMiso,
                  node_pins::kLoRaMosi, node_pins::kLoRaNss);
  const int state = radio.begin(kFrequencyMhz, kBandwidthKhz,
                                kSpreadingFactor, kCodingRate, kSyncWord, 14);
  Serial.printf("gateway radio init: %d\n", state);
  if (state != RADIOLIB_ERR_NONE) while (true) delay(1000);
  radio.setCRC(true);
  last_wifi_attempt_ms = millis() - 10000U;
}

void loop() {
  service_connections();
  uint8_t bytes[lake::kPacketV1Size]{};
  const int state = radio.receive(bytes, sizeof(bytes), 500);
  if (state == RADIOLIB_ERR_RX_TIMEOUT) return;
  const float rssi = radio.getRSSI();
  const float snr = radio.getSNR();
  lake::PacketV1 packet{};
  if (state != RADIOLIB_ERR_NONE ||
      !lake::decode_packet(bytes, sizeof(bytes), packet)) {
    Serial.printf("radio packet rejected state=%d\n", state);
    return;
  }
  Serial.printf("rx node=%u seq=%lu rssi=%.1f snr=%.2f flags=0x%04x\n",
                packet.node_id, static_cast<unsigned long>(packet.sequence),
                rssi, snr, packet.flags);
  send_ack(packet.sequence);
  if (sequence_seen[packet.node_id] &&
      last_sequence[packet.node_id] == packet.sequence) {
    Serial.println("duplicate acknowledged but not republished");
    return;
  }
  sequence_seen[packet.node_id] = true;
  last_sequence[packet.node_id] = packet.sequence;
  publish_packet(packet, rssi, snr);
}
