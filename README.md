# l1-esphome
Sonoff L1 RGB UART Component for ESPHome

## Acknowledgements
This is based on the amazing work by:
- [Emory Dunn](https://emorydunn.com/blog/2020/08/10/sonoff-l1-&-home-assistant/) and his [custom light component](https://gist.github.com/emorydunn/db410db8bf68c8a335f3362d69624aaa)
- [Anatoly Savchenkov](https://github.com/anatoly-savchenkov) and his [Sonoff D1 component](https://esphome.io/components/light/sonoff_d1)
- [ESPHome](https://esphome.io/) and their [UARTDebugger component](https://esphome.io/components/uart#debugging)

While Emory's implementation works, I still wanted to have bidirectional serial communication so changes with the IR remote would
be reflected back to Home Assistant. Furthermore, I wanted to have a more streamlined development process and include custom
source from GitHub.
