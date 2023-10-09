# l1-esphome
Sonoff L1 RGB UART

## Acknowledgements
This is based on the amazing work by [Emory Dunn](https://emorydunn.com/blog/2020/08/10/sonoff-l1-&-home-assistant/),
[gist](https://gist.github.com/emorydunn/db410db8bf68c8a335f3362d69624aaa) and combines it with 
[Sonoff D1 component](https://esphome.io/components/light/sonoff_d1).

While Emory's implementation works, I still wanted to have bidirectional serial communication so changes with the IR remote would
be reflected back to Home Assistant. Furthermore, I wanted to have a more streamlined development process and include custom
source from Github.
