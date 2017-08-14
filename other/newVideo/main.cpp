//
//  main.cpp
//  miniVideo
//
//  Created by Jules on 2017/8/5.
//  Copyright © 2017年 Jules. All rights reserved.
//

#define DEBUG0

#include "def.h"
#include "utils.h"
#include "encoder.h"
#include <cstdio>
using namespace std;

int main() {
    FILE *fin = fopen("/Users/apple/Desktop/newVideo/data/dongman.yuv", "rb");
    FILE *fout = fopen("/Users/apple/Desktop/newVideo/data/dongman2.yuv", "wb");
    
    Frame<uint8> frame[10];
    Frame<int16_t> frame2[10];

    ShadowPool<Channel<uint8> > pools[3];
    for (int i = 0; i < 3; i++) pools[i].init(750, 10);
    
    
    for (int i = 0; i < 1; i++) {
        frame[i].init(i, 720, 1280, 16, 16, 128);
        frame2[i].init(i, 720, 1280, 16, 16, 128);
        frame[i].read(fin);
        
        frame[i].display(16, 16);

        FrameSolver fsolver;
        fsolver.encode(frame[i], frame2[i], pools);
        frame2[i].display(16, 16);
        
        fsolver.decode(frame2[i], frame[i]);
        
        frame[i].display(16, 16);
        //frame2[i].display(16, 16);
        
        //pools[0].push_back(&frame[i].Y, frame[i].get_id());
        //pools[1].push_back(&frame[i].U, frame[i].get_id());
        //pools[2].push_back(&frame[i].V, frame[i].get_id());
        
        frame[i].write(fout);
    }
    
    int a;
    scanf("%d", &a);
}
