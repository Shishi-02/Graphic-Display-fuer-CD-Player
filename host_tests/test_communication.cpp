#include "Arduino.h"
#include "Communication.h"
#include "CommunicationConfig.h"
#include <cassert>
#include <vector>
#include <iostream>
uint32_t fakeNow=0;
FakePort Serial,Serial0,Serial1;
std::vector<std::string> queued;
#if CD_USE_USB_CDC
#define PORT Serial
#else
#define PORT CD_UART_PORT
#endif
void feed(const std::string& bytes) {
  PORT.rx+=bytes; fakeNow+=10; Communication::tick(fakeNow);
}
int main(){
  Communication::begin(0);
#if !CD_USE_USB_CDC
  assert(PORT.rxPin==CD_UART_RX_PIN && PORT.txPin==CD_UART_TX_PIN);
#endif
#if CD_EXTENDED_PROTOCOL
  assert(queued.size()==1 && queued.back()=="GET_STATUS\n");
  feed("ACK:GET_STATUS\nSTATE:STOP\nTOTAL_TRACKS:17\nTRACK:1\nTRACK_ELAPSED:0\nTRACK_REMAINING:202\nDISC_ELAPSED:0\nDISC_REMAINING:2552\nSTATUS_END\n");
#else
  assert(queued.empty());
  feed("STATE:INIT\nSTATE:LOADING\nSTATE:STOP\nTOTAL_TRACKS:17\nTRACK:1\nTRACK_ELAPSED:0\nTRACK_REMAINING:202\nDISC_ELAPSED:0\nDISC_REMAINING:2552\n");
#endif
  assert(Communication::state().mode==PlayerMode::Toc);
  assert(Communication::trackCount()==17);
  assert(Communication::discLengthSeconds()==2552);
  assert(Communication::trackLengthSeconds(0)==202);
  Communication::command(PlayerCommand::Play,fakeNow);
  assert(queued.back()=="PLAY\n");
  assert(Communication::state().mode==PlayerMode::Toc);
  feed("STATE:PLAY\nTRACK_ELAPSED:62\nTRACK_REMAINING:140\nDISC_ELAPSED:62\nDISC_REMAINING:2490\n");
  assert(Communication::state().elapsedMs==62000);
  assert(Communication::state().trackRemainingSeconds==140);
  assert(Communication::state().discElapsedSeconds==62);
  assert(Communication::state().discRemainingSeconds==2490);
  Communication::command(PlayerCommand::SearchFwdStart,fakeNow);
  assert(queued.back()=="SEARCH_FWD_START\n");
  Communication::command(PlayerCommand::SearchStop,fakeNow);
  assert(queued.back()=="SEARCH_STOP\n");
  Communication::command(PlayerCommand::Shuffle,fakeNow);
  assert(queued.back()=="SHUFFLE\n");
  assert(Communication::selectTrack(5,fakeNow));
  assert(queued.back()=="TRACK:5\n");
  assert(!Communication::selectTrack(18,fakeNow));
  feed("TRACK:2\r\nTRACK:0\nTRACK:99\n");
  assert(Communication::state().track==0);
  feed(std::string(64,'A')+"TRACK:2\n");
  assert(Communication::state().track==0);
  feed("TRA"); feed("CK:2\n");
  assert(Communication::state().track==1);
  assert(!Communication::state().durationKnown);
  feed("STATE:PAUSE\n");
#if !CD_EXTENDED_PROTOCOL
  fakeNow+=10000; Communication::tick(fakeNow);
  assert(Communication::state().mode==PlayerMode::Pause);
#endif
  feed("STATE:NO_DISC\n");
  assert(!Communication::state().trackKnown);
  std::cout<<"OK\n";
}
