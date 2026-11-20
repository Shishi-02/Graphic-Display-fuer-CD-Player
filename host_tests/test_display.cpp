#include "Arduino.h"
#include "Display.h"
#include "CommunicationConfig.h"
#include "gfx4desp32_gen4_ESP32_35CT.h"
#include <vector>
#include <cassert>
#include <iostream>
uint32_t fakeNow=0;
FakePort Serial, Serial0, Serial1;
std::vector<std::string> queued;
int fakePen=0,fakeX=0,fakeY=275;
void touch(int pen,int x,unsigned delta){
  fakePen=pen; fakeX=x; fakeNow+=delta;
  auto cmd=Display::pollTouch(fakeNow);
  if(cmd!=PlayerCommand::None) Communication::command(cmd,fakeNow);
}
int main(){
  Communication::begin(0); Display::begin();
  Serial.rx="STATE:PLAY\nTRACK:1\nTOTAL_TRACKS:17\nTRACK_ELAPSED:62\nTRACK_REMAINING:140\n";
  fakeNow=10; Communication::tick(fakeNow);
  Display::render(fakeNow);
  touch(TOUCH_PRESSED,377,10);
  assert(queued.empty());
  touch(TOUCH_PRESSED,377,500);
  assert(queued.back()=="SEARCH_FWD_START\n");
  touch(TOUCH_PRESSED,377,500);
  assert(queued.size()==1);
  touch(TOUCH_RELEASED,377,10);
  assert(queued.back()=="SEARCH_STOP\n");
  touch(TOUCH_PRESSED,103,10);
  touch(TOUCH_RELEASED,103,10);
  assert(queued.back()=="PREVIOUS\n");
  touch(TOUCH_PRESSED,34,10);
  touch(TOUCH_PRESSED,34,500);
  assert(queued.back()=="SHUFFLE\n");
  touch(TOUCH_RELEASED,34,10);
  assert(queued.back()=="SHUFFLE\n");
  touch(TOUCH_PRESSED,34,10);
  touch(TOUCH_RELEASED,34,10);
  assert(queued.back()=="REPEAT\n");
  std::cout<<"OK\n";
}
