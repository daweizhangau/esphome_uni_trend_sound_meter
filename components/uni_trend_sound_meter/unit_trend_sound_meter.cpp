#include "unit_trend_sound_meter.h"

#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include <string>

#ifdef USE_ESP32    

namespace esphome {
namespace uni_trend_sound_meter {

static const char *const TAG = "uni_trend_sound_meter";

namespace espbt = esphome::esp32_ble_tracker;

using namespace ble_client;

espbt::ESPBTUUID uuid128_from_string(std::string value);

void UnitTrendSoundMeter::loop() {}

void UnitTrendSoundMeter::gattc_event_handler(
    esp_gattc_cb_event_t event, 
    esp_gatt_if_t gattc_if,
    esp_ble_gattc_cb_param_t *param) {
  ESP_LOGI(TAG, "[%s] Handling event: %d", this->get_name().c_str(), event);
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
      this->status_set_warning();
      this->publish_state(NAN);
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      ESP_LOGI(TAG, "[%s] Handling event: ESP_GATTC_SEARCH_CMPL_EVT (6)", this->get_name().c_str());
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
        ESP_LOGI(TAG, "Register for notification");
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
    case ESP_GATTC_READ_CHAR_EVT: {
      if (param->read.conn_id != this->parent()->conn_id)
        break;
      if (param->read.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error reading char at handle %d, status=%d", param->read.handle, param->read.status);
        break;
      }
      if (param->read.handle == this->output_handle_) {
        this->status_clear_warning();
        // this->publish_state(this->parse_data_(param->read.value, param->read.value_len));
      }
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      if (param->notify.conn_id != this->parent()->conn_id || param->notify.handle != this->output_handle_)
        break;
      ESP_LOGI(TAG, "[%s] ESP_GATTC_NOTIFY_EVT: handle=0x%x, value=0x%x%x%x%x%x%x%x%x%x%x%x%x%x%x", this->get_name().c_str(),
               param->notify.handle, 
               param->notify.value[0],
               param->notify.value[1],
               param->notify.value[2],
               param->notify.value[3],
               param->notify.value[4],
               param->notify.value[5],
               param->notify.value[6],
               param->notify.value[7],
               param->notify.value[8],
               param->notify.value[9],
               param->notify.value[10],
               param->notify.value[11],
               param->notify.value[12],
               param->notify.value[13]);
      // this->publish_state(this->parse_data_(param->notify.value, param->notify.value_len));
      break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      this->node_state = espbt::ClientState::ESTABLISHED;
      break;
    }
    default:
      break;
  }
}

float UnitTrendSoundMeter::parse_data_(uint8_t *value, uint16_t value_len) {
  if (this->data_to_value_func_.has_value()) {
    std::vector<uint8_t> data(value, value + value_len);
    return (*this->data_to_value_func_)(data);
  } else {
    return value[0];
  }
}

void UnitTrendSoundMeter::dump_config() {
  LOG_SENSOR("", "UNI-T UT353BT", this);
  ESP_LOGCONFIG(TAG, "  MAC address        : %s", this->parent()->address_str().c_str());
  ESP_LOGCONFIG(TAG, "  Service UUID       : %s", this->service_uuid_.to_string().c_str());
  ESP_LOGCONFIG(TAG, "  Input UUID: %s", this->input_char_uuid_.to_string().c_str());
  ESP_LOGCONFIG(TAG, "  Output UUID: %s", this->output_char_uuid_.to_string().c_str());
  LOG_UPDATE_INTERVAL(this);
}

void UnitTrendSoundMeter::write_value_(uint16_t handle, unsigned short value) {
  uint8_t data[2];
  data[0] = value;
  data[1] = value >> 8;

  esp_err_t status = ::esp_ble_gattc_write_char(this->parent()->gattc_if, this->parent()->conn_id, handle, 2, data,
                                                ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);

  if (status != ESP_OK) {
    this->status_set_warning();
    ESP_LOGW(TAG, "[%s] Error sending write request for cover, status=%d", this->get_name().c_str(), status);
  }
}

void UnitTrendSoundMeter::update() {
  // if (this->input_handle_ == 0) {
  //   ESP_LOGW(TAG, "[%s] Cannot poll, input characteristic found", this->get_name().c_str());
  //   return;
  // }
  // if (this->output_char_uuid_ == 0) {
  //   ESP_LOGW(TAG, "[%s] Cannot poll, output characteristic found", this->get_name().c_str());
  //   return;
  // }

  this->write_value_(this->input_handle_, 0x5E);
  // int value = 0x5E;
  // uint8_t data[2];
  // data[0] = value;
  // data[1] = value >> 8;

  // auto status =
  //     esp_ble_gattc_write_char(this->parent()->gattc_if, this->parent()->conn_id, handle, 2, value,
  //                                               ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
  // if (status) {
  //   this->status_set_warning();
  //   this->publish_state(NAN);
  //   ESP_LOGW(TAG, "[%s] Error sending read request for sensor, status=%d", this->get_name().c_str(), status);
  // }
}

}  // namespace ble_client
}  // namespace esphome
#endif
