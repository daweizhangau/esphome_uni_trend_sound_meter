#include "unit_trend_sound_meter.h"

#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include <string>

#ifdef USE_ESP32    

namespace esphome {
namespace uni_trend_sound_meter {

static const char *const TAG = "uni_trend_sound_meter";
static const unsigned short CMD_QUERY = 0x5E;
static const uint16_t CMD_LENGTH = 1;
static const uint8_t MEASUREMENT_VALUE_START_INDEX = 6;
static const uint16_t MEASUREMENT_VALUE_LENGTH = 5;

namespace espbt = esphome::esp32_ble_tracker;

using namespace ble_client;

espbt::ESPBTUUID uuid128_from_string(std::string value);

void UnitTrendSoundMeter::loop() {}

void UnitTrendSoundMeter::gattc_event_handler(
    esp_gattc_cb_event_t event, 
    esp_gatt_if_t gattc_if,
    esp_ble_gattc_cb_param_t *param) {
  ESP_LOGV(TAG, "[%s] Handling event: %d", this->get_name().c_str(), event);
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(TAG, "[%s] Connected successfully!", this->get_name().c_str());
        break;
      }
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGW(TAG, "[%s] Disconnected!", this->get_name().c_str());
      this->node_state = espbt::ClientState::IDLE;
      this->status_set_warning();
      this->publish_state(NAN);
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      ESP_LOGI(TAG, "[%s] Service discovery is completed", this->get_name().c_str());
      // Look for output handle
      this->output_handle_ = 0;
      auto chr_output = this->parent()->get_characteristic(this->service_uuid_, this->output_char_uuid_);
      if (chr_output == nullptr) {
        this->status_set_warning();
        ESP_LOGW(TAG, "No characteristic found at service %s char %s", this->service_uuid_.to_string().c_str(),
                 this->output_char_uuid_.to_string().c_str());
        break;
      }
      this->output_handle_ = chr_output->handle;

      // Look for input handle
      this->input_handle_ = 0;
      auto chr_input = this->parent()->get_characteristic(this->service_uuid_, this->input_char_uuid_);
      if (chr_input == nullptr) {
        this->status_set_warning();
        ESP_LOGW(TAG, "No characteristic found at service %s char %s", this->service_uuid_.to_string().c_str(),
                 this->input_char_uuid_.to_string().c_str());
        break;
      }
      this->input_handle_ = chr_input->handle;

      if (this->notify_) {
        ESP_LOGI(TAG, "Registering for notification");
        auto status =
            esp_ble_gattc_register_for_notify(this->parent()->gattc_if, this->parent()->remote_bda, this->output_handle_);
        if (status) {
          ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
        }
      } else {
        this->node_state = espbt::ClientState::ESTABLISHED;
      }
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      if (param->notify.conn_id != this->parent()->conn_id || param->notify.handle != this->output_handle_)
        break;
      ESP_LOGV(TAG, "[%s] GATT Notification: handle=0x%x, value_length=%d", this->get_name().c_str(),
               param->notify.handle, 
               param->notify.value_len);
      auto parsed = this->parse_data_(param->notify.value, param->notify.value_len);
      if(parsed.has_value()) {
        this->publish_state(parsed.value());
      }
      break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      ESP_LOGI(TAG, "[%s] Register for notification completed", this->get_name().c_str());
      this->node_state = espbt::ClientState::ESTABLISHED;
      break;
    }
    default:
      break;
  }
}

// Sample data 0xAABB10013B203130312E316442413D3400041A for 101.1dBA
//             0xAABB10013B202033372E396442413D3400041A for  37.9dBA
optional<float> UnitTrendSoundMeter::parse_data_(uint8_t *value, uint16_t value_len) {
  if(value_len != 19)
    return nullopt;
  
  uint8_t* sub_value = value + MEASUREMENT_VALUE_START_INDEX;
  std::string text(sub_value, sub_value + MEASUREMENT_VALUE_LENGTH);
  ESP_LOGV(TAG, "[%s] Parsing %s", this->get_name().c_str(), text.c_str());

  return parse_number<float>(text.c_str());
}

void UnitTrendSoundMeter::dump_config() {
  LOG_SENSOR("", "UNI-T UT353BT Mini Sound Meter", this);
  ESP_LOGCONFIG(TAG, "  MAC address        : %s", this->parent()->address_str().c_str());
  ESP_LOGCONFIG(TAG, "  Service UUID       : %s", this->service_uuid_.to_string().c_str());
  ESP_LOGCONFIG(TAG, "  Input character UUID: %s", this->input_char_uuid_.to_string().c_str());
  ESP_LOGCONFIG(TAG, "  Output character UUID: %s", this->output_char_uuid_.to_string().c_str());
  LOG_UPDATE_INTERVAL(this);
}

void UnitTrendSoundMeter::update() {
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    ESP_LOGW(TAG, "[%s] Cannot poll, not connected", this->get_name().c_str());
    return;
  }
  if (this->input_handle_ == 0) {
    ESP_LOGW(TAG, "[%s] Cannot poll, no service or characteristic found for input", this->get_name().c_str());
    return;
  }

  esp_err_t status = ::esp_ble_gattc_write_char(
    this->parent()->gattc_if, 
    this->parent()->conn_id, 
    this->input_handle_, 
    CMD_LENGTH, 
    (uint8_t*)(&CMD_QUERY),
    ESP_GATT_WRITE_TYPE_NO_RSP,
    ESP_GATT_AUTH_REQ_NONE);

  if (status != ESP_OK) {
    this->status_set_warning();
    ESP_LOGW(TAG, "[%s] Error sending write request for cover, status=%d", this->get_name().c_str(), status);
  }
}

}  // namespace ble_client
}  // namespace esphome
#endif
