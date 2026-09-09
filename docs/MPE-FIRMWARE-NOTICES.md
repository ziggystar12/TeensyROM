# MPE review firmware notices

Travis Smith / Sensorium Embedded retains credit and the original MIT license
for TeensyROM. MHS created the MHS Power Engine system, module format, generic
host and transport. Existing source notices are retained; this review does not
claim authorship of upstream hardware, firmware or third-party algorithms.

The MPE-enabled binary includes the RAD-Doom-derived F1 converter by Carsten
Dachsbacher, distributed under GNU GPL version 3 or later. The combined binary
is not MIT-only. The [RAD notice](../third_party/RAD-Doom/README.md) and
[complete GPL text](../third_party/RAD-Doom/COPYING) accompany it. Corresponding
source is in this same public review branch: `vm/video/mpe_video_rad_f1.h`,
`rad_f1_tables.h`, `rad_f1_lut.h`, `scripts/generate-rad-f1-lut.mjs`, the shared
host and `mpe/Build.ps1` / `mpe/tools/`. Keep these notices and provide the
corresponding source with redistribution as required by the license.

NUFLIX Studio is by Patai Gergely (cobbpg), under MIT. The ported scheduler,
template and transport retain the [NUFLIX notice](../experiments/dosvm-nuflix/THIRD-PARTY-NOTICES.md)
and [license](../experiments/dosvm-nuflix/upstream-pinned/LICENSE).

FlasherX and its Flasher3/4 predecessors retain their notices in
`Source/Teensy/FlashUpdate.ino` and `Source/Teensy/Flash/`. The recovery addition
uses that updater; it does not replace the PJRC hardware bootloader.

No DOS, SCI, SCUMM, NES or other emulator engine, privately supplied game,
Custom GUI desktop asset or VM runtime package is bundled with this firmware.
Other SDK and library notices remain those of their upstream projects.
