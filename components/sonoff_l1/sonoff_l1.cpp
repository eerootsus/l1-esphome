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
  ESP_LOGCONFIG(TAG, "Light name: '%s'", this->light_state_ ? this->light_state_->get_name().c_str() : "");
}

void SonoffL1Output::loop() {
  uint8_t buffer[32] = {0};
  size_t len = sizeof(buffer);
  int count = static_cast<int>(len);
  int pos = 0;
  while (this->available() && count--) {
    buffer[pos] = this->read();
    pos++;
  }

  //ESP_LOGV(TAG, "%s", format_hex_pretty(buffer, len).c_str());
  ESP_LOGV(TAG, "0x%02x from the strip", buffer[0]);
}

}  // namespace sonoff_l1
}  // namespace esphome