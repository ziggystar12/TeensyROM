#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>

static uint32_t clockNow, started, releaseAfter=UINT32_MAX;
static bool menuHeld, specialHeld, sdReady, imagePresent, ledOn, SendC64Msgs=true;
static unsigned sdCalls, existsCalls, updateCalls;
static uint32_t millis(){return clockNow;}
static void delay(uint32_t duration){clockNow+=duration;}
static bool held(bool button){return button&&uint32_t(clockNow-started)<releaseAfter;}
static bool SDFullInit(){++sdCalls;return sdReady;}
struct FakeSD {
   bool exists(const char *filename){++existsCalls;assert(!std::strcmp(filename,"/RESTORE.HEX"));return imagePresent;}
} SD;
struct FakeSerial {
   void println(const char *){}
   void printf(const char *,const char *){}
} Serial;
static void DoFlashUpdate(FakeSD *storage,const char *filename){
   assert(storage==&SD&&!SendC64Msgs&&ledOn);
   assert(!std::strcmp(filename,"/RESTORE.HEX"));++updateCalls;
}
#define Fab04_SpecialButton
#define FLASHMEM
#define ReadButton (!held(menuHeld))
#define ReadDotClkDebug (!held(specialHeld))
#define SetLEDOn (ledOn=true)
#define SetLEDOff (ledOn=false)
#include "../../Source/Teensy/RecoveryFlash.h"

static void reset(){
   clockNow=started=0;releaseAfter=UINT32_MAX;
   menuHeld=specialHeld=sdReady=imagePresent=SendC64Msgs=true;
   ledOn=false;sdCalls=existsCalls=updateCalls=0;
}
int main(){
   reset();menuHeld=specialHeld=false;RecoveryFlashAtPowerOn();assert(clockNow==0&&sdCalls==0);
   reset();specialHeld=false;RecoveryFlashAtPowerOn();assert(clockNow==0&&sdCalls==0);
   reset();menuHeld=false;RecoveryFlashAtPowerOn();assert(clockNow==0&&sdCalls==0);
   reset();releaseAfter=9990;RecoveryFlashAtPowerOn();assert(sdCalls==0&&!ledOn&&updateCalls==0);
   reset();sdReady=false;RecoveryFlashAtPowerOn();assert(clockNow==10000&&sdCalls==1&&existsCalls==0&&updateCalls==0);
   reset();imagePresent=false;RecoveryFlashAtPowerOn();assert(sdCalls==1&&existsCalls==1&&updateCalls==0);
   reset();RecoveryFlashAtPowerOn();assert(clockNow==10000&&updateCalls==1&&SendC64Msgs);
   reset();SendC64Msgs=false;RecoveryFlashAtPowerOn();assert(updateCalls==1&&!SendC64Msgs);
   reset();clockNow=started=UINT32_MAX-5000;RecoveryFlashAtPowerOn();assert(uint32_t(clockNow-started)==10000&&updateCalls==1);
   std::puts("PASS: recovery idle, single buttons, early release, missing SD/file, root-only route, message restoration and timer wrap; no real flash");
}
