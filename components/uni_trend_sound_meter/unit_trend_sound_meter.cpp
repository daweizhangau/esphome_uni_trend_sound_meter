#include "unit_trend_sound_meter.h"

#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include <string>

#ifdef USE_ESP32

namespace esphome {
namespace uni_trend_sound_meter {

void UnitTrendSoundMeter::loop() {}

void UnitTrendSoundMeter::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                    esp_ble_gattc_cb_param_t *param) {
}

// void UnitTrendSoundMeter::update() {
//   if (this->node_state != espbt::ClientState::ESTABLISHED) {
//     ESP_LOGW(TAG, "[%s] Cannot poll, not connected", this->get_name().c_str());
//     return;
//   }
//   if (this->handle == 0) {
//     ESP_LOGW(TAG, "[%s] Cannot poll, no service or characteristic found", this->get_name().c_str());
//     return;
//   }

//   auto status =
//       esp_ble_gattc_read_char(this->parent()->gattc_if, this->parent()->conn_id, this->handle, ESP_GATT_AUTH_REQ_NONE);
//   if (status) {
//     this->status_set_warning();
//     this->publish_state(NAN);
//     ESP_LOGW(TAG, "[%s] Error sending read request for sensor, status=%d", this->get_name().c_str(), status);
//   }
// }

}  // namespace ble_client
}  // namespace esphome
#endif
