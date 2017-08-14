//
//  quantize.h
//  miniVideo
//
//  Created by Jules on 2017/8/6.
//  Copyright © 2017年 Jules. All rights reserved.
//

#ifndef quantize_h
#define quantize_h

#include "def.h"
#include <cmath>

class Quantize {
    void doit_block(ShadowBlock<int16_t> &block, int h, int w, double qt = 1.2) {
        for (int j = 0; j < w; j++) {
            for (int i = 0; i < h; i++) {
                double tmp = round((double)block.at(i, j) / qt);
                block.set(i, j, (int16_t)tmp);
            }
        }
    }
    
    void undo_block(ShadowBlock<int16_t> &block, int h, int w, double qt = 1.2) {
        for (int j = 0; j < w; j++) {
            for (int i = 0; i < h; i++) {
                double tmp = round((double)block.at(i, j) * qt);
                block.set(i, j, (int16_t)tmp);
            }
        }
    }
    
public:
    void doit(Channel<int16_t> &channel, double qt = 1.2) {
        for (size_t i = 0; i < channel.blocks.size(); i++) {
            doit_block(channel.blocks[i], channel.get_block_height(), channel.get_block_width());
        }
    }
    
    void undo(Channel<int16_t> &channel, double qt = 1.2) {
        for (size_t i = 0; i < channel.blocks.size(); i++) {
            undo_block(channel.blocks[i], channel.get_block_height(), channel.get_block_width());
        }
    }
};


#endif /* quantize_h */
