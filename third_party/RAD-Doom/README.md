# RAD-Doom F1 converter

Copyright (c) 2022, 2023 Carsten Dachsbacher <frenetic@dachsbacher.de>.

Source: https://github.com/frntc/RAD-Doom
Pinned revision: `81752f4347e8080ea76a9c49d8da0cd53dfbfa22`
File: `Source/Doom/doomgeneric_rad.c`
License: GNU GPL version 3 or later; full text in `COPYING`.

MHS adaptation (September 2026): `vm/video/mpe_video_rad_f1.h`,
`rad_f1_tables.h`, `rad_f1_lut.h` and its generator port the non-alternating
display preset 0 to the firmware's indexed video service. The rectangular
Bayer generation is credited upstream to Joel Yliluoma:
https://bisqwit.iki.fi/story/howto/dither/jy/

The port retains RAD's pair averaging, brightness 20, exposure 350/256,
gamma, Pepto mapping, rare-color merging, next-frame background selection
and color-RAM slot reuse. ARM NEON is replaced with scalar integer math;
the RGB lookup is generated into flash, not recalculated in RAM. Temporal
color mixing, blue noise, RAD hardware, sound and input code are not included.

Only Doom opts in, using `VM_INDEXED_RAD_F1`. Other producers and F3/F5/F7
keep their existing conversion. An older host can reject the new hint;
Doom retries with its original profile. No runtime ABI structure changes.

This adapted code remains GPL-3.0-or-later. Distributions of firmware linked
with it must comply with GPLv3, including corresponding source and this
notice; existing MIT copyright notices must also be retained. Runtime ZIPs
need not contain source if the distribution provides corresponding source
as required by the license. Do not describe the combined firmware as MIT-only.
