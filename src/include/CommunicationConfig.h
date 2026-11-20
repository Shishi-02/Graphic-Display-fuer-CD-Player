#pragma once

// 1 = temporaerer USB-C / PC-Test (native USB CDC)
// 0 = spaetere Hardware-UART zum PIC24
#ifndef CD_USE_USB_CDC
#define CD_USE_USB_CDC 1
#endif

// Bestehende Pinbelegung des Projekts: UART0, FFC 23/24.
#ifndef CD_UART_PORT
#define CD_UART_PORT Serial0
#endif
#ifndef CD_UART_RX_PIN
#define CD_UART_RX_PIN 44
#endif
#ifndef CD_UART_TX_PIN
#define CD_UART_TX_PIN 43
#endif
#define CD_BAUD 115200

// 0 = Ereignisprotokoll nach 3. Zwischenbericht (20.11.2026).
// 1 = zusaetzlich alter GET_STATUS/ACK/STATUS_END-Vertrag.
// Nur aktivieren, wenn die Steuerungsgruppe ihn bestaetigt.
#ifndef CD_EXTENDED_PROTOCOL
#define CD_EXTENDED_PROTOCOL 0
#endif

#define CD_MAX_FRAME_BYTES 64
#define CD_LONG_PRESS_MS 500
