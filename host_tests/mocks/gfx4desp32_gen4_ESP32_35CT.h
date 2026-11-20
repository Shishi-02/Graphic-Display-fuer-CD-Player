#pragma once
#include <stdint.h>
#include <string.h>
#define WHITE 0xffff
#define LANDSCAPE 0
#define LANDSCAPE_R 1
#define FONT2 0
#define TOUCH_ENABLE 1
#define TOUCH_PRESSED 1
#define TOUCH_RELEASED 2
#define NOTOUCH 0
extern int fakePen, fakeX, fakeY, fakeOrientation;
struct gfx4desp32_gen4_ESP32_35CT {
  template<typename... T> void RectangleFilled(T...){}
  template<typename... T> void CircleFilled(T...){}
  template<typename... T> void TriangleFilled(T...){}
  template<typename... T> void Line(T...){}
  template<typename... T> void TextColor(T...){}
  template<typename... T> void TextSize(T...){}
  template<typename... T> void MoveTo(T...){}
  template<typename... T> void print(T...){}
  template<typename... T> void Cls(T...){}
  void Orientation(int value){fakeOrientation=value;}
  template<typename... T> void ScrollEnable(T...){}
  template<typename... T> void BacklightOn(T...){}
  template<typename... T> void Font(T...){}
  template<typename... T> void touch_Set(T...){}
  void begin(){}
  void touch_Update(){}
  int touch_GetPen(){return fakePen;}
  int touch_GetX(){return fakeX;}
  int touch_GetY(){return fakeY;}
  int strWidth(const char* s){return int(strlen(s))*8;}
  int charHeight(char){return 16;}
};
