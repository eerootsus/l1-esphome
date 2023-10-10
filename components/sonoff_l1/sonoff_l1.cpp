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
  ESP_LOGCONFIG(TAG, "Sonoff L1 LED strip: '%s'", this->light_state_ ? this->light_state_->get_name().c_str() : "");
}

void SonoffL1Output::loop() {
  uint8_t buffer[32] = {0};
  size_t len = sizeof(buffer);
  int count = static_cast<int>(len);
  ESP_LOGV(TAG, "Trying to read %04d bytes from UART", count);
  while (this->available() && count--) {
    this->read_byte(&buffer[len - count - 1]);
  }

  ESP_LOGV(TAG, "Got from strip: %s", format_hex_pretty(buffer, len).c_str());
}

}  // namespace sonoff_l1
}  // namespace esphome