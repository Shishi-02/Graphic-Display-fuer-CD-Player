#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include <stddef.h>
using QueueHandle_t=void*;
extern std::vector<std::string> queued;
inline QueueHandle_t xQueueCreate(unsigned,size_t){return reinterpret_cast<void*>(1);}
inline int xQueueReceive(QueueHandle_t,void*,unsigned){return 0;}
inline int xQueueSend(QueueHandle_t,const void* data,unsigned){
  auto bytes=static_cast<const unsigned char*>(data);
  // TxFrame: length, 3 alignment bytes, queuedAt, bytes.
  queued.emplace_back(reinterpret_cast<const char*>(bytes+8),bytes[0]);
  return pdTRUE;
}
inline void vQueueDelete(QueueHandle_t){}
