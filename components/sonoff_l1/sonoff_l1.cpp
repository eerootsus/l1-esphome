#include "sonoff_l1.h"

namespace esphome {
namespace sonoff_l1 {

static const char *const TAG = "sonoff_l1";
static const float COLOR_SCALE = 1.0f / 255.f;

light::LightTraits SonoffL1Output::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::RGB});
  return traits;
}

void SonoffL1Output::setup_state(light::LightState *state) {
  ESP_LOGD(TAG, "Setting up light initial state");
  this->light_state_ = state;
  this->light_color_values_.set_state(state->current_values.is_on());
  this->light_color_values_.set_brightness(state->current_values.get_brightness());
}

void SonoffL1Output::write_state(light::LightState *state) {
  ESP_LOGV(TAG, "Received next light state");
  this->next_light_state_ = state;
}

void SonoffL1Output::send_next_state() {
  uint64_t sequence = micros();
  std::string update_command = "AT+UPDATE=\"sequence\":\"" + std::to_string(sequence) + "\"";

  ESP_LOGV(TAG, "Sending next light state");

  // Current known states of the light parameters
  bool current_on;
  float current_brightness;
  float current_red;
  float current_green;
  float current_blue;

  // Next states of the light parameters received from the frontend
  bool next_on;
  float next_brightness;
  float next_red;
  float next_green;
  float next_blue;

  this->light_color_values_.as_binary(&current_on);
  this->next_light_state_->current_values.as_binary(&next_on);

  this->light_color_values_.as_brightness(&current_brightness);
  this->next_light_state_->current_values.as_brightness(&next_brightness);

  this->light_color_values_.as_rgb(&current_red, &current_green, &current_blue);
  this->next_light_state_->current_values.as_rgb(&next_red, &next_green, &next_blue);

  ESP_LOGV(TAG, "Current state: %s, %f, R%f, G%f, B%f", ONOFF(current_on), current_brightness, current_red, current_green, current_blue);
  ESP_LOGV(TAG, "Next state: %s, %f, R%f, G%f, B%f", ONOFF(next_on), next_brightness, next_red, next_green, next_blue);

  if (next_on != current_on) {
    ESP_LOGD(TAG, "Setting state: %s", ONOFF(next_on));
    update_command += ",\"switch\":\"";
    update_command += (next_on ? "on" : "off");
    update_command += "\"";
    this->light_color_values_.set_state(next_on);
  }

  if (next_on && next_brightness != current_brightness) {
    const uint8_t calculated_brightness = (uint8_t) roundf(next_brightness * 100);
    ESP_LOGD(TAG, "Setting brightness: %d", calculated_brightness);
    update_command += ",\"bright\":";
    update_command += std::to_string(calculated_brightness);
    this->light_color_values_.set_brightness(next_brightness);
  }

  if (next_on && next_red != current_red) {
    const uint8_t calculated_red = light::to_uint8_scale(next_red);
    ESP_LOGD(TAG, "Setting red: %d", calculated_red);
    update_command += ",\"colorR\":";
    update_command += std::to_string(calculated_red);
    this->light_color_values_.set_red(next_red);
  }

  if (next_on && next_green != current_green) {
    const uint8_t calculated_green = light::to_uint8_scale(next_green);
    ESP_LOGD(TAG, "Setting green: %d", calculated_green);
    update_command += ",\"colorG\":";
    update_command += std::to_string(calculated_green);
    this->light_color_values_.set_green(next_green);
  }

  if (next_on && next_blue != current_blue) {
    const uint8_t calculated_blue = light::to_uint8_scale(next_blue);
    ESP_LOGD(TAG, "Setting blue: %d", calculated_blue);
    update_command += ",\"colorB\":";
    update_command += std::to_string(calculated_blue);
    this->light_color_values_.set_blue(next_blue);
  }

  this->next_light_state_ = nullptr;
  this->last_sequence_ = sequence;

  this->send_at_command(update_command.c_str());
}

void SonoffL1Output::send_at_command(const char *str) {
  ESP_LOGV(TAG, "Sending AT command: %s", str);
  this->write_str(str);
  this->write_byte(0x1B);
  this->flush();
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
      bool values_have_changed = false;
      auto call_ = this->light_state_->make_call();
      call_.set_transition_length(0);

      if(header == "AT+RESULT"){
        ESP_LOGV(TAG, "Received AT+RESULT, sending ACK");
        this->send_at_command("AT+SEND=ok");
        // TODO: compare if the received sequence matches with our last sequence (maybe need to resend?)
      }

      else if(header == "AT+UPDATE"){
        ESP_LOGV(TAG, "Received AT+UPDATE, parsing attributes");
        message.push_back(','); // Add a comma to the end of the message so we can parse the last attribute
        while(message.length()){
          std::string value = message.substr(0, message.find(","));
          message.erase(0, message.find(",") + 1);

          std::string attribute = value.substr(0, value.find(":"));
          value.erase(0, value.find(":") + 1);

          if(attribute == "\"switch\""){
            if (value == "\"on\"" && !this->light_color_values_.is_on()) {
              call_.set_state(true);
              values_have_changed = true;
            } else if (value == "\"off\"" && this->light_color_values_.is_on()) {
              call_.set_state(false);
              values_have_changed = true;
            }
          } else if(attribute == "\"bright\""){
            float brightness = std::stof(value)/100.0;
            if(this->light_color_values_.get_brightness() != brightness){
              call_.set_brightness(brightness);
              values_have_changed = true;
            }
          } else if(attribute == "\"colorR\"") {
            float red = std::stof(value) * COLOR_SCALE;
            if(this->light_color_values_.get_red() != red){
              call_.set_red(red);
              values_have_changed = true;
            }
          } else if(attribute == "\"colorG\"") {
            float green = std::stof(value) * COLOR_SCALE;
            if(this->light_color_values_.get_green() != green){
              call_.set_green(green);
              values_have_changed = true;
            }
          } else if(attribute == "\"colorB\"") {
            float blue = std::stof(value) * COLOR_SCALE;
            if(this->light_color_values_.get_blue() != blue){
              call_.set_blue(blue);
              values_have_changed = true;
            }
          } else {
            continue;
          }

          ESP_LOGV(TAG, "Attribute %s has value %s", attribute.c_str(), value.c_str());
        }
      }
      else {
        ESP_LOGD(TAG, "Received message header: %s; no handling implemented", header.c_str());
      }


      if (values_have_changed) {
        ESP_LOGD(TAG, "Light has changed, publishing light state to frontend");
        call_.perform();
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
