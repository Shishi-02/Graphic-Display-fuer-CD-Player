#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string>
#define SERIAL_8N1 0
extern uint32_t fakeNow;
inline uint32_t millis(){return fakeNow;}
inline void delay(unsigned){}
struct FakePort {
  std::string rx;
  size_t pos=0;
  int rxPin=-1, txPin=-1;
  void setRxBufferSize(size_t){}
  void begin(unsigned long){}
  void begin(unsigned long,int,int rx,int tx){rxPin=rx;txPin=tx;}
  void setTxTimeoutMs(unsigned){}
  int available(){return int(rx.size()-pos);}
  int read(){return pos<rx.size()? (unsigned char)rx[pos++]:-1;}
  size_t write(const uint8_t*,size_t n){return n;}
};
extern FakePort Serial,Serial0,Serial1;
