#pragma once

namespace gateway_config {
constexpr char kWifiSsid[] = "replace-me";
constexpr char kWifiPassword[] = "replace-me";
constexpr char kMqttHost[] = "192.168.1.10";
constexpr unsigned short kMqttPort = 1883;
constexpr char kMqttUsername[] = "lake-gateway";
constexpr char kMqttPassword[] = "replace-me";
constexpr char kTopicPrefix[] = "lake-monitor";
constexpr char kGatewayId[] = "heltec-gateway-1";
constexpr char kHttpEndpoint[] =
    "https://lake.emiliamarie.net/api/v1/lake-level";
constexpr char kHttpApiKey[] = "replace-me";
}  // namespace gateway_config
