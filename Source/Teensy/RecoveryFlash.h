#pragma once

#ifdef Fab04_SpecialButton
static const char RecoveryFirmwarePath[] = "/RESTORE.HEX";
static const uint32_t RecoveryHoldMilliseconds = 10000;

static bool RecoveryButtonsHeld()
{
   return ReadButton == 0 && ReadDotClkDebug == 0;
}

static FLASHMEM __attribute__((noinline)) void RecoveryFlashAtPowerOn()
{
   if (!RecoveryButtonsHeld()) return;
   uint32_t StartedAt = millis();
   uint32_t LastBlinkAt = StartedAt;
   bool LEDOn = false;
   while (RecoveryButtonsHeld())
   {
      const uint32_t Now = millis();
      if (Now - StartedAt >= RecoveryHoldMilliseconds) break;
      if (Now - LastBlinkAt >= 250)
      {
         LEDOn = !LEDOn;
         if (LEDOn) SetLEDOn;
         else SetLEDOff;
         LastBlinkAt = Now;
      }
      delay(10);
   }
   if (!RecoveryButtonsHeld())
   {
      SetLEDOff;
      return;
   }
   SetLEDOn;
   if (!SDFullInit())
   {
      Serial.println("Recovery: SD card unavailable; normal boot continues.");
      return;
   }
   if (!SD.exists(RecoveryFirmwarePath))
   {
      Serial.println("Recovery: /RESTORE.HEX not found; normal boot continues.");
      return;
   }
   Serial.printf("Recovery: validating and flashing %s\n", RecoveryFirmwarePath);
   const bool PreviousSendC64Msgs = SendC64Msgs;
   SendC64Msgs = false;
   DoFlashUpdate(&SD, RecoveryFirmwarePath);
   SendC64Msgs = PreviousSendC64Msgs;
   Serial.println("Recovery: image rejected; normal boot continues.");
}
#endif
