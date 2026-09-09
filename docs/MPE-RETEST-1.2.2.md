# Text-interface MPE host 1.2.2: NUFLIX and SD recovery

This is the updated maintainer-review firmware, retaining Travis's text menu
and ordinary features. It imports shared host revision
`9585d08136935877aed84255129ae0bccb9c56ff`, not the Custom GUI application or
emulator engines. The separate DOSVM DESERT correction is in its downloadable
engine module; no DOS engine is embedded in this firmware.

## Ready-to-test firmware

[Download the combined full HEX](../mpe/review/TeensyROM+_0.8.0.4_MPE-1.2.2-NUFLIX_full.hex).

SHA-256: `bb465916d26533a451da4a1bf83805195a571ab69ceebf12eb6603791420016b`.
Size: 7,057,532 bytes. The upstream visible version remains 0.8.0.4;
MPE-1.2.2-NUFLIX and this hash identify the candidate. Use **TeensyROM+ Fab0.4**.
Original-TR comparison firmware does not provide MPE support or this two-button
TR+ recovery route. Existing older review artifacts remain historical only.

## Changes

- Full-width 320x200 NUFLIX double-buffer transport, incremental conversion,
  separate visible/inactive bank histories and bounded PAL/NTSC grants.
- SID sideband packets can travel while an inactive picture uploads; a pending
  SID acknowledgment no longer unnecessarily stops permitted picture transfers.
- Immutable RAM2 video sources and opt-in RAD-Doom F1 remain generic host
  capabilities; older negotiated video modes are retained.
- Hold **Menu + Alternate/Special for ten seconds at power-on** to load a
  trusted full `/RESTORE.HEX` through the ordinary validated updater. The
  [full recovery procedure and limitations](MPE-VM.md#two-button-sd-recovery-tr-fab04)
  are important: this is not an independent hardware bootloader.
- Stricter Intel HEX parsing and buffer/file cleanup on failed updates.

## Measured build and host checks

MPE (three images), stock-plus and original-TR comparison builds all pass using
Teensy core 1.61.0 / GCC 11.3.1. The MPE host's actual ITCM span is **91,324 of
98,304 bytes**, with zero RAM2/PSRAM globals. Its 16 KiB heap, 192 KiB module
data window and 48 KiB execution stack reservation remain separate.

The ordinary main application remains in seven ITCM banks. Its linked static
DTCM usage is 268,992 bytes, with **25,920 bytes** from static end to stack top.
The printed 35,676-byte local-variable figure excludes 9,756 bytes of
non-cacheable Ethernet data; it is not the actual linked gap. Ordinary RAM2
static allocation stays 17,600 bytes. No extra main-image ITCM bank is taken.

All 219 protected upstream menu/asset/configuration files remain unchanged.
The suite passes file operations, replay, existing graphics, AUX ownership,
real EasyFlash swap functions, launch routing and current five-package
header/CRC/service preflight, including the new DOSVM engine. No package or
private game data is included in this review.

Three synthetic changing frames per PAL/NTSC standard exercise the actual
double-buffer host: visible bytes remain intact, grant limits hold, stale and
early grants are rejected, sources remain immutable, and DMA failures release
the bus. A separate test compiles the exact transformed firmware scheduler
and verifies SID publication during uploads, DMA behind pending SID, ACK,
quiet/replay and failure handling. DMA/register I/O is simulated in these tests.

Recovery tests cover no/single buttons, early release, missing SD/file,
root-only routing, message-state restoration and timer wrap. **25 parser
safety scenarios pass.** Flash writes/reboots are stubbed; no hardware was
flashed or physically recovered by these tests.

[Verification, source hashes and memory record](../mpe/review/host-1.2.2-verification.json)
and [test output](../mpe/review/host-1.2.2-tests.log) are included. Build with
`mpe/Build.ps1`; run `mpe/tools/verify.mjs` as described in the integration guide.

## Before acceptance

Physical testing remains pending: normal/large cartridges, USB/network/MIDI,
REU/freezer/KERNAL, settings retention, VM boot/reset/menu return, PAL/NTSC
graphics and audio, ordinary firmware updating and intentional SD recovery.
Neither the GUI user's acceptance nor these host checks establish acceptance
of this refreshed text firmware. No Teensy FPS or listening-quality claim is made.

The combined MPE binary includes GPL-3.0-or-later RAD-Doom conversion. The
license, attribution, generator and corresponding source are included; original
MIT notices remain. Read the [component notices](MPE-FIRMWARE-NOTICES.md).
