#include "Communication.h"
#include "CommunicationConfig.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <atomic>
#include <stdio.h>
#include <Arduino.h>
#include <string.h>

namespace {
constexpr uint32_t STATUS_INTERVAL_MS = 2000;
constexpr uint32_t LINK_TIMEOUT_MS = 6000;
constexpr uint32_t ACK_TIMEOUT_MS = 3000;
constexpr unsigned STATUS_COMMAND = 11;
constexpr unsigned COMMAND_COUNT = 12;
#if CD_USE_USB_CDC
#if !defined(ARDUINO_USB_CDC_ON_BOOT) || !ARDUINO_USB_CDC_ON_BOOT
#error "USB CDC On Boot muss fuer CD_USE_USB_CDC aktiviert sein."
#endif
#define CD_PORT Serial
#else
#define CD_PORT CD_UART_PORT
#endif

// USB-Ausgabe kann im Core warten. Daher TX in eigener Task,
// niemals Serial-Verbindungsprüfung oder write() in der GUI-Schleife.
struct TxFrame {
  uint8_t length;
  uint32_t queuedAt;
  char bytes[CD_MAX_FRAME_BYTES];
};
QueueHandle_t txQueue = nullptr;
std::atomic<bool> txFailed{false};

void txTask(void*) {
  TxFrame frame;
  for (;;) {
    if (xQueueReceive(txQueue, &frame, portMAX_DELAY) != pdTRUE) continue;
    // Keine alten Bedienbefehle nach langer Blockierung nachsenden.
    if (uint32_t(millis() - frame.queuedAt) > 250 ||
        CD_PORT.write(reinterpret_cast<const uint8_t*>(frame.bytes),
                      frame.length) != frame.length) {
      txFailed.store(true);
    }
  }
}

bool enqueue(const char* text, uint32_t now) {
  const size_t length = strlen(text);
  if (!length || length >= CD_MAX_FRAME_BYTES || !txQueue) return false;
  TxFrame frame{};
  frame.length = length + 1;
  frame.queuedAt = now;
  memcpy(frame.bytes, text, length);
  frame.bytes[length] = '\n';
  return xQueueSend(txQueue, &frame, 0) == pdTRUE;
}
const char* const COMMANDS[] = {
  "PREVIOUS", "STOP", "PLAY", "PAUSE", "NEXT", "EJECT", "REPEAT",
  "SEARCH_FWD_START", "SEARCH_BWD_START", "SEARCH_STOP", "SHUFFLE", "GET_STATUS"
};
PlayerState player;
uint8_t count = 0;
uint16_t duration = 0, discDuration = 0;
bool pending[COMMAND_COUNT] = {};
uint32_t sentAt[COMMAND_COUNT] = {};
uint32_t lastRx = 0, lastQuery = 0, lastByte = 0;
bool receivedState = false, querying = false, discardLine = false;
bool navigationAccepted = false;
char line[CD_MAX_FRAME_BYTES];
// GET_STATUS hat keine feste Feldreihenfolge. Bis STATUS_END sammeln und
// zuerst Medium/Zustand/Track, danach die zugehoerigen Zeiten uebernehmen.
const char* const STATUS_FIELDS[] = {
  "STATE", "TOTAL_TRACKS", "TRACK", "MODE", "TRACK_ELAPSED",
  "TRACK_REMAINING", "DISC_ELAPSED", "DISC_REMAINING", "ERROR_CODE"
};
char snapshot[9][64] = {};
bool applyingSnapshot = false;
unsigned used = 0;
const char* message = "";

struct TimePair {
  uint32_t elapsed = 0, remaining = 0;
  bool hasElapsed = false, hasRemaining = false;
  bool update(bool isElapsed, unsigned seconds, uint16_t& total) {
    if (isElapsed) {
      elapsed = seconds;
      hasElapsed = true;
    } else {
      remaining = seconds;
      hasRemaining = true;
    }
    if (!hasElapsed || !hasRemaining) return false;
    hasElapsed = hasRemaining = false;
    if (elapsed + remaining > 5999) { total = 0; return true; }
    total = static_cast<uint16_t>(elapsed + remaining);
    return true;
  }
} trackTimes, discTimes;

bool number(const char* text, unsigned maximum, unsigned& value) {
  if (!*text) return false;
  value = 0;
  while (*text) {
    if (*text < '0' || *text > '9') return false;
    unsigned digit = *text++ - '0';
    if (value > maximum / 10 ||
        (value == maximum / 10 && digit > maximum % 10)) return false;
    value = value * 10 + digit;
  }
  return true;
}

void clearTrackTime() {
  player.elapsedMs = 0;
  player.trackRemainingSeconds = 0;
  player.trackRemainingKnown = false;
  player.elapsedKnown = player.durationKnown = false;
  duration = 0;
  trackTimes = TimePair{};
}

void clearMedium() {
  navigationAccepted = false;
  player.trackKnown = player.countKnown = player.discKnown = false;
  player.repeat = RepeatMode::Off;
  player.searching = false;
  player.loopEnabled = false;
  count = 0;
  discDuration = 0;
  player.discElapsedSeconds = player.discRemainingSeconds = 0;
  player.discElapsedKnown = player.discRemainingKnown = false;
  discTimes = TimePair{};
  clearTrackTime();
}

bool send(unsigned index, uint32_t now) {
  if (index >= COMMAND_COUNT || (CD_EXTENDED_PROTOCOL && pending[index])) return false;
  if (!enqueue(COMMANDS[index], now)) {
    message = "SENDEN NICHT MOEGLICH";
    return false;
  }
  if (CD_EXTENDED_PROTOCOL) {
    pending[index] = true;
    sentAt[index] = now;
    if (index == STATUS_COMMAND) {
      lastQuery = now;
      querying = true;
      memset(snapshot,0,sizeof(snapshot));
    }
  }
  return true;
}

bool readState(const char* value, uint32_t now) {
  PlayerMode mode;
  if (!strcmp(value,"INIT") || !strcmp(value,"LOADING")) mode = PlayerMode::Reading;
  else if (!strcmp(value,"NO_DISC")) mode = PlayerMode::Eject;
  else if (!strcmp(value,"STOP")) mode = PlayerMode::Toc;
  else if (!strcmp(value,"PLAY") || !strcmp(value,"SEARCH_FWD") ||
           !strcmp(value,"SEARCH_BWD")) mode = PlayerMode::Play;
  else if (!strcmp(value,"PAUSE")) mode = PlayerMode::Pause;
  else if (!strcmp(value,"ERROR")) mode = PlayerMode::Error;
  else return false;

  player.searching = !strcmp(value,"SEARCH_FWD") || !strcmp(value,"SEARCH_BWD");
  if (!strcmp(value,"INIT")) {
    // Expliziter Neustart der Gegenstelle beendet alte offene Befehle.
    memset(pending,0,sizeof(pending));
    querying = false;
  }
  if (mode == PlayerMode::Reading || mode == PlayerMode::Eject) clearMedium();
  // Ein periodischer STOP-Schnappschuss behaelt die manuelle Titelauswahl.
  if (!(mode == PlayerMode::Toc && player.mode == PlayerMode::Select))
    player.mode = mode;
  if (mode == PlayerMode::Reading) player.readingSinceMs = now;
  receivedState = true;
  return true;
}

bool parse(char* text, uint32_t now) {
  if (CD_EXTENDED_PROTOCOL && !strcmp(text,"STATUS_END")) {
    if (!querying) return false;
    applyingSnapshot = true;
    const bool hadTrack = player.trackKnown;
    const uint8_t previousTrack = player.track;
    player.trackKnown = false;
    player.countKnown = player.discKnown = false;
    clearTrackTime();
    discTimes = TimePair{};
    bool accepted = false;
    for (auto& field : snapshot)
      if (field[0]) accepted = parse(field,now) || accepted;
    if (player.mode == PlayerMode::Toc && hadTrack &&
        player.trackKnown && player.track != previousTrack)
      player.mode = PlayerMode::Select;
    applyingSnapshot = false;
    querying = false;
    return accepted;
  }
  if (querying && !applyingSnapshot) {
    for (unsigned i=0; i<9; ++i) {
      const size_t length = strlen(STATUS_FIELDS[i]);
      if (!strncmp(text,STATUS_FIELDS[i],length) && text[length] == ':') {
        strcpy(snapshot[i],text); // Empfang begrenzt die Zeile auf 63 Zeichen.
        return false; // Aktualitaet erst nach Pruefung bei STATUS_END.
      }
    }
  }
  char* separator = strchr(text,':');
  if (!separator) return false;
  *separator++ = 0;
  if (!strcmp(text,"ACK")) {
    for (unsigned i=0; i<COMMAND_COUNT; ++i) {
      if (!strcmp(separator,COMMANDS[i])) {
        pending[i] = false;
        if (i == 0 || i == 4) navigationAccepted = true;
        if (i == 1) navigationAccepted = false;
        message = "";
        return true;
      }
    }
    return false;
  }
  if (!strcmp(text,"ERR")) {
    // ERR hat laut Bericht keine eindeutige Befehlszuordnung.
    // Offene Bedienbefehle bleiben bis zu ihrem ACK gesperrt.
    message = "STEUERUNG MELDET FEHLER";
    return true;
  }
  if (!strcmp(text,"STATE")) return readState(separator,now);
  if (!strcmp(text,"MODE")) {
    RepeatMode repeat;
    if (!strcmp(separator,"REPEAT_TRACK")) repeat = RepeatMode::Track;
    else if (!strcmp(separator,"REPEAT_DISC")) repeat = RepeatMode::Disc;
    else if (!strcmp(separator,"NORMAL") || !strcmp(separator,"SHUFFLE") ||
             !strcmp(separator,"PROGRAMMING") || !strcmp(separator,"PROGRAM_PLAY") ||
             !strcmp(separator,"AB_REPEAT")) repeat = RepeatMode::Off;
    else return false;
    player.repeat = repeat;
    player.loopEnabled = repeat != RepeatMode::Off;
    return true;
  }

  unsigned value;
  if (!strcmp(text,"TRACK")) {
    if (!number(separator,99,value) || value == 0) return false;
    if (player.countKnown && value > count) return false;
    const bool changed = !player.trackKnown || player.track != value-1;
    const bool hadTrack = player.trackKnown;
    if (changed) {
      clearTrackTime();
      player.track = value-1;
    }
    if ((navigationAccepted || (hadTrack && changed)) && player.mode == PlayerMode::Toc)
      player.mode = PlayerMode::Select;
    navigationAccepted = false;
    player.trackKnown = true;
    return true;
  }
  if (!strcmp(text,"TOTAL_TRACKS")) {
    if (!number(separator,99,value)) return false;
    count = value;
    player.countKnown = true;
    if (player.trackKnown && player.track >= count) {
      player.trackKnown = false;
      clearTrackTime();
    }
    return true;
  }
  if (!number(separator,5999,value)) return false;
  if (!strcmp(text,"TRACK_ELAPSED") || !strcmp(text,"TRACK_REMAINING")) {
    if (!player.trackKnown) return false;
    const bool elapsed = !strcmp(text,"TRACK_ELAPSED");
    if (!elapsed) {
      player.trackRemainingSeconds = value;
      player.trackRemainingKnown = true;
    }
    if (elapsed) {
      player.elapsedMs = value * 1000UL;
      player.elapsedKnown = true;
    }
    if (trackTimes.update(elapsed,value,duration))
      player.durationKnown = duration > 0;
    return true;
  }
  if (!strcmp(text,"DISC_ELAPSED") || !strcmp(text,"DISC_REMAINING")) {
    const bool elapsed = !strcmp(text,"DISC_ELAPSED");
    if (elapsed) { player.discElapsedSeconds = value; player.discElapsedKnown = true; }
    else { player.discRemainingSeconds = value; player.discRemainingKnown = true; }
    if (discTimes.update(elapsed,value,discDuration))
      player.discKnown = discDuration > 0;
    return true;
  }
  if (!strcmp(text,"ERROR_CODE")) {
    message = value ? "LAUFWERKSFEHLER" : "";
    return true;
  }
  // Fernbedienungsfunktionen ohne Anzeige in diesem UI werden ignoriert.
  return false;
}
}

namespace Communication {
void begin(uint32_t nowMs) {
  player = PlayerState{};
  clearMedium();
  receivedState = querying = discardLine = false;
  memset(pending,0,sizeof(pending));
  used = 0;
  message = "";
  lastRx = lastQuery = lastByte = nowMs;
#if CD_USE_USB_CDC
  CD_PORT.setRxBufferSize(512);
  CD_PORT.begin(CD_BAUD);
  CD_PORT.setTxTimeoutMs(20);
#else
  CD_PORT.setRxBufferSize(512);
  CD_PORT.begin(CD_BAUD, SERIAL_8N1, CD_UART_RX_PIN, CD_UART_TX_PIN);
#endif
  txFailed.store(false);
  txQueue = xQueueCreate(8, sizeof(TxFrame));
  if (!txQueue || xTaskCreate(txTask, "cdTx", 4096, nullptr, 1, nullptr) != pdPASS) {
    if (txQueue) vQueueDelete(txQueue);
    txQueue = nullptr;
    message = "TX INITIALISIERUNG FEHLER";
  }
  if (CD_EXTENDED_PROTOCOL) send(STATUS_COMMAND,nowMs);
}

void tick(uint32_t nowMs) {
  if (txFailed.exchange(false)) message = "SENDEN FEHLGESCHLAGEN";
  if (used && nowMs-lastByte >= 250) discardLine = true;
  for (unsigned budget=0; budget<256 && CD_PORT.available(); ++budget) {
    int byte = CD_PORT.read();
    if (byte < 0) break;
    lastByte = nowMs;
    if (byte == '\n') {
      line[used] = 0;
      if (!discardLine && parse(line,nowMs)) lastRx = nowMs;
      used = 0;
      discardLine = false;
    } else if (byte < 32 || byte > 126 || used == sizeof(line)-1) {
      discardLine = true;
    } else if (!discardLine) {
      line[used++] = static_cast<char>(byte);
    }
  }
  // Bei STOP/PAUSE garantiert der neue Bericht keine zyklischen Meldungen.
  // Keine Offline-Erkennung allein wegen stiller Gegenstelle.
  if ((CD_EXTENDED_PROTOCOL || (receivedState && player.mode == PlayerMode::Play)) &&
      nowMs-lastRx >= LINK_TIMEOUT_MS && player.mode != PlayerMode::Offline) {
    player.mode = PlayerMode::Offline;
    receivedState = false;
    clearMedium();
    memset(pending,0,sizeof(pending));
    querying = false;
  }
  if (!CD_EXTENDED_PROTOCOL) return;
  for (unsigned i=0; i<COMMAND_COUNT; ++i) {
    if (pending[i] && nowMs-sentAt[i] >= ACK_TIMEOUT_MS)
      message = "BESTAETIGUNG FEHLT";
  }
  if (querying && !pending[STATUS_COMMAND] &&
      nowMs-sentAt[STATUS_COMMAND] >= ACK_TIMEOUT_MS) {
    message = "STATUS UNVOLLSTAENDIG";
    querying = false;
  }
  // Abfragen nur nach beendeter Antwort. Kein Wiederholen offener Befehle.
  if (!querying && nowMs-lastQuery >= STATUS_INTERVAL_MS)
    send(STATUS_COMMAND,nowMs);
}
bool enabled(PlayerCommand key) {
  const unsigned index = static_cast<unsigned>(key);
  if (index >= STATUS_COMMAND || pending[index]) return false;
  if (key == PlayerCommand::SearchStop) return true;
  if (!receivedState) return false;
  if (player.mode == PlayerMode::Offline || player.mode == PlayerMode::Waiting) return false;
  if (key == PlayerCommand::Eject) return true;
  if (player.mode == PlayerMode::Reading || player.mode == PlayerMode::Eject ||
      player.mode == PlayerMode::Error) return false;
  if (key == PlayerCommand::SearchFwdStart || key == PlayerCommand::SearchBwdStart)
    return player.mode == PlayerMode::Play || player.mode == PlayerMode::Pause;
  if (key == PlayerCommand::Pause) return player.mode == PlayerMode::Play;
  if (key == PlayerCommand::Play) return player.mode != PlayerMode::Play;
  return true;
}
void command(PlayerCommand key, uint32_t nowMs) {
  if (enabled(key)) send(static_cast<unsigned>(key),nowMs);
}
bool selectTrack(uint8_t trackNumber, uint32_t nowMs) {
  if (!enabled(PlayerCommand::Next) || trackNumber == 0 || trackNumber > 99 ||
      (player.countKnown && trackNumber > count)) return false;
  char frame[16];
  snprintf(frame, sizeof(frame), "TRACK:%u", unsigned(trackNumber));
  if (!enqueue(frame, nowMs)) {
    message = "SENDEN NICHT MOEGLICH";
    return false;
  }
  return true;
}
uint8_t trackCount() { return count; }
uint16_t trackLengthSeconds(uint8_t track) {
  return player.trackKnown && player.durationKnown && track == player.track ? duration : 0;
}
uint32_t discLengthSeconds() { return discDuration; }
const PlayerState& state() { return player; }
const char* notice() { return message; }
}
