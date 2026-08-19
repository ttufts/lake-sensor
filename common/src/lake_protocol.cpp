#include "lake_protocol.h"

namespace lake {
namespace {

void put_u16(uint8_t*& out, uint16_t value) {
  *out++ = static_cast<uint8_t>(value);
  *out++ = static_cast<uint8_t>(value >> 8);
}

void put_u32(uint8_t*& out, uint32_t value) {
  for (int shift = 0; shift < 32; shift += 8) {
    *out++ = static_cast<uint8_t>(value >> shift);
  }
}

uint16_t get_u16(const uint8_t*& in) {
  const uint16_t value = static_cast<uint16_t>(in[0]) |
                         (static_cast<uint16_t>(in[1]) << 8);
  in += 2;
  return value;
}

uint32_t get_u32(const uint8_t*& in) {
  uint32_t value = 0;
  for (int shift = 0; shift < 32; shift += 8) {
    value |= static_cast<uint32_t>(*in++) << shift;
  }
  return value;
}

}  // namespace

EncodedPacketV1 encode_packet(const PacketV1& packet) {
  EncodedPacketV1 encoded{};
  uint8_t* out = encoded.data();
  put_u16(out, kPacketMagic);
  *out++ = kPacketVersion;
  *out++ = packet.node_id;
  put_u32(out, packet.sequence);
  put_u32(out, static_cast<uint32_t>(packet.depth_mm));
  put_u16(out, packet.sense_mv);
  put_u16(out, packet.loop_ua);
  put_u16(out, packet.battery_mv);
  put_u16(out, static_cast<uint16_t>(packet.temperature_centi_c));
  put_u16(out, packet.flags);
  return encoded;
}

bool decode_packet(const uint8_t* data, std::size_t size, PacketV1& packet) {
  if (data == nullptr || size != kPacketV1Size) return false;
  const uint8_t* in = data;
  if (get_u16(in) != kPacketMagic || *in++ != kPacketVersion) return false;
  packet.node_id = *in++;
  packet.sequence = get_u32(in);
  packet.depth_mm = static_cast<int32_t>(get_u32(in));
  packet.sense_mv = get_u16(in);
  packet.loop_ua = get_u16(in);
  packet.battery_mv = get_u16(in);
  packet.temperature_centi_c = static_cast<int16_t>(get_u16(in));
  packet.flags = get_u16(in);
  return true;
}

}  // namespace lake
