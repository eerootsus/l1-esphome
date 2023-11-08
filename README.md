# l1-esphome
Sonoff L1 RGB UART Component for ESPHome

## Acknowledgements
This is based on the amazing work by:
- [Emory Dunn](https://emorydunn.com/blog/2020/08/10/sonoff-l1-&-home-assistant/) and his [custom light component](https://gist.github.com/emorydunn/db410db8bf68c8a335f3362d69624aaa)
- [Theo Arends](https://github.com/arendst) and his amazing [Tasmota project](https://tasmota.github.io/docs/), which includes [Sonoff L1 support](https://github.com/arendst/Tasmota/blob/development/tasmota/tasmota_xlgt_light/xlgt_05_sonoff_l1.ino)
- [Anatoly Savchenkov](https://github.com/anatoly-savchenkov) and his [Sonoff D1 component](https://esphome.io/components/light/sonoff_d1)
- [ESPHome](https://esphome.io/) project

While Emory's implementation works, I still wanted to have bidirectional serial communication so changes with the IR remote would
be reflected back to Home Assistant. Furthermore, I wanted to have a more streamlined development process and not having to 
copy files to the HA server every time I made a change, so custom component was the way to go.

## Installation
Configure the ESPHome device with the YAML found in `example.yaml`. Update the instance name to your preference and configure
OTA, Wi-Fi, etc. per your needs. All light parameters, like gamma correct and transition time should work as well.
Compile and upload the firmware to your device.

## Disclaimer
This is my first attempt at an ESPHome custom component, so it is far from being perfect. Furthermore, it's my first time ever
writing C++, so it might be a weird mix of vectors and strings, but my objective was to get it to work.

I'm ofcourse open to suggestions, forks and improvements.

Use at your own peril.
