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
  int count = 32;
  while (this->available() && count--) {
    uint8_t byte = this->read();
    this->bytes_.push_back(0x55);
  }

  this->log_string();
}

void SonoffL1Output::log_string() {
  size_t len = this->bytes_.size();
  if(len == 0) return;

  std::string res;
  char buf[5];
  for (size_t i = 0; i < len; i++) {
    if (this->bytes_[i] == 7) {
      res += "\\a";
    } else if (this->bytes_[i] == 8) {
      res += "\\b";
    } else if (this->bytes_[i] == 9) {
      res += "\\t";
    } else if (this->bytes_[i] == 10) {
      res += "\\n";
    } else if (this->bytes_[i] == 11) {
      res += "\\v";
    } else if (this->bytes_[i] == 12) {
      res += "\\f";
    } else if (this->bytes_[i] == 13) {
      res += "\\r";
    } else if (this->bytes_[i] == 27) {
      res += "\\e";
    } else if (this->bytes_[i] == 34) {
      res += "\\\"";
    } else if (this->bytes_[i] == 39) {
      res += "\\'";
    } else if (this->bytes_[i] == 92) {
      res += "\\\\";
    } else if (this->bytes_[i] < 32 || this->bytes_[i] > 127) {
      sprintf(buf, "\\x%02X", this->bytes_[i]);
      res += buf;
    } else {
      res += this->bytes_[i];
    }
  }

  this->bytes_.clear();
  ESP_LOGV(TAG, "%s", res.c_str());
  delay(10);
}

}  // namespace sonoff_l1
}  // namespace esphome