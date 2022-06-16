#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"

#ifdef USE_ESP32

namespace esphome {
namespace uni_trend_sound_meter {

namespace espbt = esphome::esp32_ble_tracker;

using namespace ble_client;

espbt::ESPBTUUID uuid128_from_string(std::string value);

class UnitTrendSoundMeter : public sensor::Sensor, public PollingComponent, public BLEClientNode {

public:
//   float get_setup_priority() const override { return setup_priority::LATE; }
//   void setup() override;
  void dump_config() override;

  void loop() override;
  void update() override;
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

protected:
  optional<float> parse_data_(uint8_t *value, uint16_t value_len);

private:
  bool notify_ = true;
  espbt::ESPBTUUID service_uuid_ = espbt::ESPBTUUID::from_uint16(0xFF12);
  espbt::ESPBTUUID input_char_uuid_ = espbt::ESPBTUUID::from_uint16(0xFF01);
  uint16_t input_handle_;
  espbt::ESPBTUUID output_char_uuid_ = espbt::ESPBTUUID::from_uint16(0xFF02);
  uint16_t output_handle_;
};
}  // namespace uni_trend_sound_meter
}  // namespace esphome

#endif