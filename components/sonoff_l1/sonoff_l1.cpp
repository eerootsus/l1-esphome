#include "sonoff_l1.h"

namespace esphome {
namespace sonoff_l1 {

static const char *const TAG = "sonoff_l1";

light::LightTraits SonoffL1Output::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
  return traits;
}

void SonoffL1Output::write_state(light::LightState *state) {
  bool binary;
  float brightness;

  // Fill our variables with the device's current state
  state->current_values_as_binary(&binary);
  state->current_values_as_brightness(&brightness);

  // Convert ESPHome's brightness (0-1) to the device's internal brightness (0-100)
  const uint8_t calculated_brightness = (uint8_t) roundf(brightness * 100);

  if (calculated_brightness == 0) {
    // if(binary) ESP_LOGD(TAG, "current_values_as_binary() returns true for zero brightness");
    binary = false;
  }
}

void SonoffL1Output::dump_config() {
  ESP_LOGCONFIG(TAG, "L1 instance name: '%s'", this->light_state_ ? this->light_state_->get_name().c_str() : "");
}

void SonoffL1Output::loop() {
  int count = 64;
  while (this->available() && count--) {
    uint8_t byte = this->read();
    if (byte == 27){
      this->log_string(this->bytes_);
      this->bytes_.clear();
    } else {
      this->bytes_.push_back(byte);
    }
  }
}

void SonoffL1Output::log_string(std::vector<uint8_t> bytes) {
  size_t len = bytes.size();
  if(len == 0) return;

  std::string res;
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

  ESP_LOGV(TAG, "%s", res.c_str());
  delay(10);
}

}  // namespace sonoff_l1
}  // namespace esphome
