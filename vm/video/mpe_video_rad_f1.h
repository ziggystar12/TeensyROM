// SPDX-License-Identifier: GPL-3.0-or-later
// Adapted from RAD-Doom Source/Doom/doomgeneric_rad.c, preset 0.
// Copyright (c) 2022, 2023 Carsten Dachsbacher <frenetic@dachsbacher.de>
// MHS Teensy/indexed-frame adaptation, 2026. See third_party/RAD-Doom/README.md.
// This program is free software under GNU GPL version 3 or any later version,
// WITHOUT ANY WARRANTY; see third_party/RAD-Doom/COPYING.
#pragma once
#include "rad_f1_tables.h"
#include "rad_f1_lut.h"
namespace mpe_video { namespace rad_f1 {
// RAD's rectangular Bayer generator (credited upstream to Joel Yliluoma,
// https://bisqwit.iki.fi/story/howto/dither/jy/), fixed at M=3/L=4.
// Preset 0 samples only even matrix rows; no frame-phase alternation.
constexpr uint8_t threshold(unsigned x,unsigned y){
    unsigned v=0,offset=0,maskX=3,maskY=4;
    const unsigned yc=y^((x<<4)>>3);
    for(unsigned bit=0;bit<7;){
        v|=((x>>--maskX)&1)<<bit++;
        for(offset+=4;offset>=3;offset-=3)v|=((yc>>--maskY)&1)<<bit++;
    }
    return uint8_t(v);
}
struct Pattern {
    int16_t value[64]{};
    constexpr Pattern(){for(unsigned y=0;y<8;y++)for(unsigned x=0;x<8;x++)
        value[y*8+x]=(int(threshold(x,y*2))-31)*16;}
};
static constexpr Pattern pattern MPE_VIDEO_RAD_RODATA {};
inline unsigned quantize(unsigned pairSum,int dither){
    const int v=int(pairSum)*20+dither;
    return v<=0?0:v>=15*512?15:unsigned(v)/512;
}
}
MPE_VIDEO_CODE void LiveConverter::renderRadF1(const IndexedSource &s,LiveFrame &out,const LiveFrame *previous){
    // Reuse the inactive nearest-color map for counts, with no workspace
    // growth. Normal prepare/cache restores overwrite the marker (VIC colors
    // are always <16). Distances remain intact for cached Full F5 conversion.
    const bool continuing=previous&&previous->mode==0&&map_[32]==254;
    uint16_t occurrence[16]{};
    if(continuing)memcpy(occurrence,map_,sizeof occurrence);
    unsigned background=0;
    if(continuing)for(unsigned c=1;c<15;c++)
        if(occurrence[c]>occurrence[background])background=c;
    memset(occurrence,0,sizeof occurrence);
    auto read=[&](unsigned x,unsigned y){
        const auto i=s.read_pixel?s.read_pixel(s.context,x,y):s.pixels[y*s.stride+x];
        return unsigned(i)<s.colors?unsigned(i):0u;
    };
    for(unsigned cell=0;cell<1000;cell++){
        uint8_t hist[16]{},pixels[32],map[16];
        const unsigned x=(cell%40)*8,y=(cell/40)*8;
        for(unsigned row=0;row<8;row++)for(unsigned col=0;col<4;col++){
            const auto a=read(x+col*2,y+row)*3,b=read(x+col*2+1,y+row)*3;
            const int d=rad_f1::pattern.value[row*8+((x/2+col)&7)];
            const auto r=rad_f1::quantize(unsigned(s.palette[a])+s.palette[b],d);
            const auto g=rad_f1::quantize(unsigned(s.palette[a+1])+s.palette[b+1],d);
            const auto blue=rad_f1::quantize(unsigned(s.palette[a+2])+s.palette[b+2],d);
            const auto color=rad_f1::rgbToVic[(r*16+g)*16+blue];
            ++hist[color];pixels[row*4+col]=color;
        }
        for(unsigned c=0;c<16;c++)if(hist[c])++occurrence[c];
        hist[background]=32;
        unsigned colors=0;
        for(unsigned c=0;c<16;c++){colors+=hist[c]!=0;map[c]=c;}
        while(colors>4){
            unsigned least=0,count=256;
            for(unsigned c=0;c<16;c++)if(hist[c]&&hist[c]<count){count=hist[c];least=c;}
            hist[least]=0;--colors;
            const auto ranked=rad_f1::closest+least*16;
            unsigned rank=0;while(rank<15&&!hist[ranked[rank]])++rank;
            const auto target=ranked[rank];map[least]=target;
            for(unsigned c=0;c<16;c++)if(!hist[c]&&map[c]==least)map[c]=target;
            hist[target]+=count;
        }
        // Read before writing: the firmware deliberately uses previous==out.
        const auto old=continuing?previous->cells[cell][9]:0;
        unsigned slot=0;uint8_t screen=0,color=0;
        if(old!=background&&old<16&&hist[old]){slot=1;hist[old]=0;color=old;}
        for(unsigned c=0;c<16;c++)if(c!=background&&hist[c]){
            if(slot==2)screen|=c<<4;
            else if(slot==1)screen|=c;
            else if(slot==0)color=c;
            ++slot;
        }
        auto dst=out.cells[cell];dst[8]=screen;dst[9]=color;
        for(unsigned row=0;row<8;row++){
            uint8_t bits=0;
            for(unsigned col=0;col<4;col++){
                const auto c=map[pixels[row*4+col]];
                const unsigned value=c==color?3:c==(screen&15)?2:c==(screen>>4)?1:0;
                bits=(bits<<2)|value;
            }
            dst[row]=bits;
        }
    }
    out.mode=0;out.background=background;out.mask=0;
    out.overlays=out.multicolor=0;memset(out.split,0,sizeof out.split);
    memcpy(map_,occurrence,sizeof occurrence);map_[32]=254;
}
}
