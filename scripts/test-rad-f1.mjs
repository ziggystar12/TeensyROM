// SPDX-License-Identifier: GPL-3.0-or-later
// Compile the pinned RAD source as a scalar reference, retaining its original
// palette/block selection code. No Raspberry Pi/NEON runtime is required.
import fs from 'node:fs';import path from 'node:path';import assert from 'node:assert/strict';
import {spawnSync} from 'node:child_process';
const root=path.resolve(import.meta.dirname,'..'),out=path.join(root,'build/rad-f1');
fs.mkdirSync(out,{recursive:true});
const commit='81752f4347e8080ea76a9c49d8da0cd53dfbfa22';
const original=path.join(out,commit+'.c');
if(!fs.existsSync(original)){
 const response=await fetch(`https://raw.githubusercontent.com/frntc/RAD-Doom/${commit}/Source/Doom/doomgeneric_rad.c`);
 assert.equal(response.status,200);fs.writeFileSync(original,await response.text());
}
const src=fs.readFileSync(original,'utf8');
let pre=src.slice(src.indexOf('#define RGB_QUANTIZE_BITS'),src.indexOf('const int16x4_t quantMax'));
let convert=src.slice(src.indexOf('void doImageConversion()'),src.indexOf('static uint8_t firstSoundMix'));
const from=convert.indexOf('uint8x8_t  rgba_2'),to=convert.indexOf('histo[ c64color ] ++;',from);
assert.ok(from>0&&to>from);
// This is only the NEON arithmetic translated literally to scalar operations.
// The upstream Bayer, background, histogram merging and encoding stay verbatim.
convert=convert.slice(0,from)+`int c64color;
 const uint32_t aRGB=DG_ScreenBuffer[x+a+(y+c)*320], bRGB=DG_ScreenBuffer[x+a+1+(y+c)*320];
 const int d=(dm[((a/2+x/2)&dmaskX)+((((c+y)*2)&dmaskY)<<dshift)]-dofs/2)*dmul;
 int q[3];
 for(int channel=0;channel<3;channel++) {
  int value=(((aRGB>>(channel*8))&255)+((bRGB>>(channel*8))&255))*brightnessScale+d;
  value>>=9;q[channel]=value<0?0:value>15?15:value;
 }
 c64color=mapRGB2C64[(q[2]*16+q[1])*16+q[0]];
 `+convert.slice(to);
const ref=src.slice(0,src.indexOf('#include'))+`\n#include <cmath>\n#include <cstring>\n#include <cstdint>\nnamespace rad_reference {\n#define DOOMGENERIC_RESX 320\n#define DOOMGENERIC_RESY 200\nstatic uint32_t DG_ScreenBuffer[64000];\nstatic uint8_t bluenoise256[65536];\nstatic const float invGamma=0.96111111111f;\nstatic const int exposure=350;\nstatic int brightnessScale=20,ditherMode=2,flickerMode=0,alternatePattern=0,bgColor=0;\n`+pre+convert+'\n}\n';
fs.writeFileSync(path.join(out,'rad_reference.h'),ref);
const compiler=process.env.CXX??'C:/msys64/mingw64/bin/g++.exe';
const env={...process.env},key=Object.keys(env).find(k=>k.toUpperCase()==='PATH');
env[key]=path.dirname(compiler)+path.delimiter+env[key];
function run(exe,args){const r=spawnSync(exe,args,{cwd:root,env,windowsHide:true,encoding:'utf8',maxBuffer:8*1024*1024});
 assert.ifError(r.error);process.stdout.write(r.stdout);process.stderr.write(r.stderr);assert.equal(r.status,0);}
for(const name of ['rad_f1_test','rad_f1_host_test','indexed_ram2_source_test','mpe_video_live_test','mpe_video_crop_test','mpe_video_detail_test','mpe_video_sprite_test','indexed_host_test','full_video_converter_test','full_video_host_test']){
 const exe=path.join(out,name+'.exe');
 run(compiler,['-std=c++17','-O2','-I.', '-I'+out,'vm/tests/'+name+'.cpp','-o',exe]);
 run(exe,name==='rad_f1_test'?process.argv.slice(2):[]);
}
