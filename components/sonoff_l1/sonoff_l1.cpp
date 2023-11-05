#include "sonoff_l1.h"

namespace esphome {
namespace sonoff_l1 {

static const char *const TAG = "sonoff_l1";

light::LightTraits SonoffL1Output::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
  return traits;
}

void SonoffL1Output::setup_state(light::LightState *state) {
  ESP_LOGD(TAG, "Setting light initial state");
  this->next_light_state_ = state;
}

void SonoffL1Output::write_state(light::LightState *state) {
  ESP_LOGD(TAG, "Writing light state");
  this->next_light_state_ = state;
}

void SonoffL1Output::send_next_state() {
  std::string update_command = "AT+UPDATE=";

  this->light_state_ = this->next_light_state_;
  this->next_light_state_ = nullptr;
  this->last_sequence_ = micros();

  update_command += "\"sequence\":\"" + std::to_string(this->last_sequence_) + "\"";
  ESP_LOGD(TAG, "Setting light state:");

  bool current_state = this->parent_->remote_values.is_on();
  if (this->light_state_.value_or(current_state) != current_state) {
    ESP_LOGD(TAG, "  Setting state: %s", ONOFF(this->light_state_.is_on()));
    update_command += ",\"switch\":\"" + (this->light_state_.is_on() ? "on" : "off") + "\"";
  }


  // Convert ESPHome's brightness (0-1) to the device's internal brightness (0-100)
  /*const uint8_t calculated_brightness = (uint8_t) roundf(brightness * 100);

  if (calculated_brightness == 0) {
    binary = false;
  }*/


  this->send_at_command(update_command.c_str());
}

void SonoffL1Output::send_at_command(const char *str) {
  ESP_LOGV(TAG, "Sending AT command: %s", str);
  this->write_str(str);
  this->write_byte(0x1B);
  this->flush();
}

void SonoffL1Output::dump_config() {
  ESP_LOGCONFIG(TAG, "L1 instance name: '%s'", this->light_state_ ? this->light_state_->get_name().c_str() : "");
}

void SonoffL1Output::loop() {
  int count = 32;
  while (this->available() && count--) {
    uint8_t byte = this->read();

    // End of message, handle it and clear the buffer
    if (byte == 27){
      std::string message = this->uart_bytes_to_string(this->bytes_);
      this->bytes_.clear();
      ESP_LOGV(TAG, "Received from UART: %s", message.c_str());

      std::string header = message.substr(0, message.find("="));
      message.erase(0, message.find("=") + 1);
      bool state_has_changed = false;
      ESP_LOGV(TAG, "Message header: %s", header.c_str());
      auto call = light_state_->make_call();

      if(header == "AT+RESULT"){
        ESP_LOGV(TAG, "Received AT+RESULT, sending ACK");
        this->send_at_command("AT+SEND=ok");
        // TODO: compare if the received sequence matches with our last sequence (maybe need to resend?)
      }

      if(header == "AT+UPDATE"){
        ESP_LOGV(TAG, "Received AT+UPDATE, parsing attributes");
        message.push_back(','); // Add a comma to the end of the message so we can parse the last attribute
        while(message.length()){
          std::string value = message.substr(0, message.find(","));
          message.erase(0, message.find(",") + 1);

          std::string attribute = value.substr(0, value.find(":"));
          value.erase(0, value.find(":") + 1);

          if(attribute == "\"switch\""){
            if (value == "\"on\"" && !this->light_state_->current_values.is_on()) {
              call.set_state(true);
              state_has_changed = true;
            } else if (value == "\"off\"" && this->light_state_->current_values.is_on()) {
              call.set_state(false);
              state_has_changed = true;
            }
          } else {
            continue;
          }

          ESP_LOGV(TAG, "Attribute %s has value %s", attribute.c_str(), value.c_str());
        }
      }


      if (state_has_changed && this->light_state_) {
          ESP_LOGV(TAG, "Publishing light state to frontend");
          call.set_transition_length(0);
          call.perform();
        }

    } else {
      this->bytes_.push_back(byte);
    }
  }

  if(this->next_light_state_ != nullptr){
    this->send_next_state();
  }
}

std::string SonoffL1Output::uart_bytes_to_string(std::vector<uint8_t> bytes) {
  std::string res;
  size_t len = bytes.size();
  char buf[5];

  for (size_t i = 0; i < len; i++) {
    if (bytes[i] == 7) {
      res += "\\a";
    } else if (bytes[i] == 8) {
      res += "\\b";
    } else if (bytes[i] == 9) {
      res += "\\t";
    } else if (bytes[i] == 10) {
      res += "\\n";
    } else if (bytes[i] == 11) {
      res += "\\v";
    } else if (bytes[i] == 12) {
      res += "\\f";
    } else if (bytes[i] == 13) {
      res += "\\r";
    } else if (bytes[i] == 27) {
      res += "\\e";
    } else if (bytes[i] == 34) {
      res += "\"";
    } else if (bytes[i] == 39) {
      res += "'";
    } else if (bytes[i] == 92) {
      res += "\\";
    } else if (bytes[i] < 32 || bytes[i] > 127) {
      sprintf(buf, "\\x%02X", bytes[i]);
      res += buf;
    } else {
      res += bytes[i];
    }
  }

  return res;

}

}  // namespace sonoff_l1
}  // namespace esphome
