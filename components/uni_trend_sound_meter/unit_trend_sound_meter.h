#pragma once

#include "esphome/core/component.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"

#ifdef USE_ESP32

namespace esphome {
namespace uni_trend_sound_meter {

namespace espbt = esphome::esp32_ble_tracker;

using namespace ble_client;

espbt::ESPBTUUID uuid128_from_string(std::string value);

class UnitTrendSoundMeter : public Component, public BLEClientNode {
 public:
  UnitTrendSoundMeter() : Component(){};
//   void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
//                            esp_ble_gattc_cb_param_t *param) override;
//   void update() override;
};
}  // namespace uni_trend_sound_meter
}  // namespace esphome

#endif