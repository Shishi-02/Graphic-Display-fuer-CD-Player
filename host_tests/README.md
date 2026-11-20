# Host-Tests

Die Tests simulieren Arduino, FreeRTOS-Queue und Display-API. Sie testen
Parser, Zustandsuebernahme, in die Queue eingestellte Nachrichten und
Touch-Gesten. Die TX-Task selbst wird nicht ausgefuehrt und die echte
Grafikbibliothek wird nicht gelinkt. Aus dem Projektverzeichnis mit g++:

```sh
for usb in 0 1; do
  for ext in 0 1; do
    g++ -std=c++11 -Wall -Wextra -DARDUINO_USB_CDC_ON_BOOT=1 \
      -DCD_USE_USB_CDC=$usb -DCD_EXTENDED_PROTOCOL=$ext \
      -Ihost_tests/mocks -Isrc/include src/src/Communication.cpp \
      host_tests/test_communication.cpp -o /tmp/cd_test && /tmp/cd_test
  done
done
g++ -std=c++11 -Wall -Wextra -DARDUINO_USB_CDC_ON_BOOT=1 \
  -Ihost_tests/mocks -Isrc/include src/src/Communication.cpp \
  src/src/Display.cpp host_tests/test_display.cpp -o /tmp/cd_touch && /tmp/cd_touch
```

Ergebnis bei Erstellung: alle vier Kommunikationsvarianten und Touch-Test OK.
