// SPDX-License-Identifier: GPL-3.0-or-later
#include <cassert>
#include <cstdio>
#include <fstream>
#include <vector>
#include <chrono>
#include "../video/mpe_video_live.cpp"
#include "rad_reference.h"
using namespace mpe_video;
static uint8_t pixels[64000],palette[768];
static uint8_t shown(const LiveFrame &f,unsigned x,unsigned y){
 const auto cell=f.cells[y/8*40+x/8];const uint8_t colors[]={f.background,uint8_t(cell[8]>>4),uint8_t(cell[8]&15),cell[9]};
 return colors[(cell[y%8]>>(6-2*((x%8)/2)))&3];
}
static void save(const char *name,const LiveFrame &f){
 std::ofstream out(name,std::ios::binary);out<<"P6\n320 200\n255\n";
 for(unsigned y=0;y<200;y++)for(unsigned x=0;x<320;x++)out.write((const char *)rad_f1::pepto[shown(f,x,y)],3);
}
static uint8_t reader(void *,uint16_t x,uint16_t y){assert(x<320&&y<200);return pixels[y*320+x];}
int main(int argc,char **argv){
 rad_reference::precomputeColorQuantization();
 assert(!memcmp(rad_reference::mapRGB2C64,rad_f1::rgbToVic,4096));
 LiveConverter cv;LiveFrame out{};IndexedSource s{pixels,palette,320,200,320,256};s.rad_f1=true;
 uint32_t rng=7;auto random=[&](){rng=rng*1664525+1013904223;return uint8_t(rng>>24);};
 for(unsigned frame=0;frame<12;frame++){
  for(auto &v:palette)v=random();for(auto &v:pixels)v=random();
  if(frame==0){memset(palette,0,sizeof palette);memset(pixels,0,sizeof pixels);}
  if(frame==1)for(unsigned i=0;i<256;i++)palette[i*3]=palette[i*3+1]=palette[i*3+2]=i;
  for(unsigned i=0;i<64000;i++){const auto p=palette+pixels[i]*3;rad_reference::DG_ScreenBuffer[i]=(unsigned(p[0])<<16)|(unsigned(p[1])<<8)|p[2];}
  rad_reference::doImageConversion();assert(cv.render(s,0,out,frame?&out:nullptr));
  for(unsigned c=0;c<1000;c++){
   assert(!memcmp(out.cells[c],rad_reference::koalaData+c*8,8));
   assert(out.cells[c][8]==rad_reference::koalaData[8000+c]);
   assert(out.cells[c][9]==rad_reference::koalaData[9000+c]);
  }
  assert(out.background==rad_reference::koalaData[10000]);
 }
 puts("PASS RAD reference: all 4096 lookup values and 120012 frame bytes identical over 12 frames");
 // Modes/profile isolation, callback parity and in-place previous aliasing.
 LiveFrame before{},after{};LiveConverter plain,rad;
 for(uint16_t geometry:{0,32,64,128})for(uint8_t mode:{1,2,3}){
  s.geometry=geometry;s.rad_f1=false;assert(plain.render(s,mode,before));
  s.rad_f1=true;assert(rad.render(s,mode,after));assert(!memcmp(&before,&after,sizeof before));
 }
 s.geometry=0;LiveConverter bytes,callback;LiveFrame a{},b{};
 for(unsigned i=0;i<4;i++){
  assert(bytes.render(s,0,a,i?&a:nullptr));auto r=s;r.pixels=nullptr;r.read_pixel=reader;
  assert(callback.render(r,0,b,i?&b:nullptr));assert(!memcmp(&a,&b,sizeof a));
 }
 const auto stable=a;assert(bytes.render(s,0,a,&a));assert(!memcmp(&stable,&a,sizeof a));
 // F7 round-trip restarts the RAD background estimate, not old mode state.
 assert(bytes.render(s,3,a,&a));assert(bytes.render(s,0,a,&a));LiveConverter fresh;
 assert(fresh.render(s,0,b));assert(!memcmp(&a,&b,sizeof a));
 puts("PASS F3/F5/F7 isolation, reader parity, in-place history, static repeat and mode round-trip");
 FullFrame full{};full.invalidate();LiveConverter shared;bool changed=false;
 assert(shared.renderFull(s,full,&changed)&&changed);const auto fullBefore=full;
 assert(shared.render(s,0,a));assert(shared.renderFull(s,full,&changed)&&!changed);
 assert(!memcmp(&full.frame,&fullBefore.frame,sizeof full.frame));
 assert(!memcmp(full.extra,fullBefore.extra,sizeof full.extra));
 puts("PASS cached Full F5 survives intervening RAD conversion without workspace growth");
 for(int f=1;f<argc;f++){
  std::ifstream in(argv[f],std::ios::binary);std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)),{});
  assert(data.size()>=64768);memcpy(pixels,data.data(),64000);memcpy(palette,data.data()+64000,768);
  s.rad_f1=false;assert(plain.render(s,0,before));s.rad_f1=true;
  assert(rad.render(s,0,after));assert(rad.render(s,0,after,&after));
  save("build/rad-f1/doom-f1-before.ppm",before);save("build/rad-f1/doom-f1-rad.ppm",after);
  for(bool enabled:{false,true}){
   s.rad_f1=enabled;const auto start=std::chrono::steady_clock::now();
   for(unsigned n=0;n<100;n++)assert(rad.render(s,0,after,&after));
   printf("%s F1 host conversion %.3f ms/frame (not Teensy timing)\n",enabled?"RAD":"Legacy",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/100);
  }
 }
}
