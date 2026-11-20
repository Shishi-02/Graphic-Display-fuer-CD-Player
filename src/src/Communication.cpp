#include "Communication.h"
#include <Arduino.h>
#include <string.h>

namespace {
constexpr uint32_t STATUS_INTERVAL_MS = 2000;
constexpr uint32_t LINK_TIMEOUT_MS = 6000;
constexpr uint32_t ACK_TIMEOUT_MS = 3000;
constexpr unsigned STATUS_COMMAND = 7;
const char* const COMMANDS[] = {
  "PREVIOUS", "STOP", "PLAY", "PAUSE", "NEXT", "EJECT", "REPEAT", "GET_STATUS"
};
PlayerState player;
uint8_t count = 0;
uint16_t duration = 0, discDuration = 0;
bool pending[8] = {};
uint32_t sentAt[8] = {};
uint32_t lastRx = 0, lastQuery = 0, lastByte = 0;
bool receivedState = false, querying = false, discardLine = false;
bool navigationAccepted = false;
char line[64];
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
    if (elapsed + remaining > 5999) return false;
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
  player.elapsedKnown = player.durationKnown = false;
  duration = 0;
  trackTimes = TimePair{};
}

void clearMedium() {
  navigationAccepted = false;
  player.trackKnown = player.countKnown = player.discKnown = false;
  player.repeat = RepeatMode::Off;
  player.loopEnabled = false;
  count = 0;
  discDuration = 0;
  discTimes = TimePair{};
  clearTrackTime();
}

void send(unsigned index, uint32_t now) {
  if (pending[index]) return;
  const size_t length = strlen(COMMANDS[index]);
  if (Serial0.availableForWrite() < static_cast<int>(length + 1)) return;
  Serial0.print(COMMANDS[index]);
  Serial0.write('\n');
  pending[index] = true;
  sentAt[index] = now;
  if (index == STATUS_COMMAND) {
    lastQuery = now;
    querying = true;
    memset(snapshot,0,sizeof(snapshot));
  }
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

  if (!strcmp(value,"INIT")) {
    // Expliziter Neustart der Gegenstelle beendet alte offene Befehle.
    memset(pending,0,sizeof(pending));
    querying = false;
  }
  if (mode == PlayerMode::Reading || mode == PlayerMode::Eject) clearMedium();
  // Ein periodischer STOP-Schnappschuss behaelt die manuelle Titelauswahl.
  if (!(querying && mode == PlayerMode::Toc && player.mode == PlayerMode::Select))
    player.mode = mode;
  if (mode == PlayerMode::Reading) player.readingSinceMs = now;
  receivedState = true;
  return true;
}

bool parse(char* text, uint32_t now) {
  if (!strcmp(text,"STATUS_END")) {
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
    for (unsigned i=0; i<8; ++i) {
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
    if (elapsed) {
      player.elapsedMs = value * 1000UL;
      player.elapsedKnown = true;
    }
    if (trackTimes.update(elapsed,value,duration))
      player.durationKnown = duration > 0;
    return true;
  }
  if (!strcmp(text,"DISC_ELAPSED") || !strcmp(text,"DISC_REMAINING")) {
    if (discTimes.update(!strcmp(text,"DISC_ELAPSED"),value,discDuration))
      player.discKnown = true;
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
  Serial0.setRxBufferSize(512);
  Serial0.begin(115200, SERIAL_8N1, 44, 43);
  send(STATUS_COMMAND,nowMs);
}

void tick(uint32_t nowMs) {
  if (used && nowMs-lastByte >= 250) discardLine = true;
  for (unsigned budget=0; budget<256 && Serial0.available(); ++budget) {
    int byte = Serial0.read();
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
  if (nowMs-lastRx >= LINK_TIMEOUT_MS) {
    player.mode = PlayerMode::Offline;
    receivedState = false;
    clearMedium();
  }
  for (unsigned i=0; i<8; ++i) {
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
  if (index >= 7 || pending[index] || !receivedState) return false;
  if (player.mode == PlayerMode::Offline || player.mode == PlayerMode::Waiting) return false;
  if (key == PlayerCommand::Eject) return true;
  if (player.mode == PlayerMode::Reading || player.mode == PlayerMode::Eject ||
      player.mode == PlayerMode::Error) return false;
  if (key == PlayerCommand::Pause) return player.mode == PlayerMode::Play;
  if (key == PlayerCommand::Play) return player.mode != PlayerMode::Play;
  return true;
}
void command(PlayerCommand key, uint32_t nowMs) {
  if (enabled(key)) send(static_cast<unsigned>(key),nowMs);
}
uint8_t trackCount() { return count; }
uint16_t trackLengthSeconds(uint8_t track) {
  return player.trackKnown && player.durationKnown && track == player.track ? duration : 0;
}
uint32_t discLengthSeconds() { return discDuration; }
const PlayerState& state() { return player; }
const char* notice() { return message; }
}
