//
//  predict.h
//  miniVideo
//
//  Created by Jules on 2017/8/6.
//  Copyright © 2017年 Jules. All rights reserved.
//

#ifndef predict_h
#define predict_h

#include "def.h"

class Predict {
    double block_intro_predict(ShadowBlock<uint8> &block, ShadowBlock<uint16> &block2, int h, int w, int pattern_type) {
        double score = 0;
        if (pattern_type == 0) {
            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    block2.set(i, j, (uint16)block.at(i, j) - block.at(-1, j));
                    score += block.at(i, j) * block.at(i, j);
                }
            }
        } else if (pattern_type == 1) {
            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    block2.set(i, j, (uint16)block.at(i, j) - block.at(i, -1));
                    score += block.at(i, j) * block.at(i, j);
                }
            }
        } else if (pattern_type == 2) {
            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    int c_i = max(-1, j - i - 1);
                    int c_j = max(-1, i - j - 1);
                    block2.set(i, j, (uint16)block.at(i, j) - block.at(c_i, c_j));
                    score += block.at(i, j) * block.at(i, j);
                }
            }
        }
        return score;
    }
    
    void undo_block_intro_predict(ShadowBlock<uint16> &block, ShadowBlock<uint8> &block2, int h, int w, int pattern_type) {
        double score = 0;
        if (pattern_type == 0) {
            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    block2.set(i, j, (uint8)(block.at(i, j) + block2.at(-1, j)));
                    score += block.at(i, j) * block.at(i, j);
                }
            }
        } else if (pattern_type == 1) {
            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    block2.set(i, j, (uint8)(block.at(i, j) + block2.at(i, -1)));
                    score += block.at(i, j) * block.at(i, j);
                }
            }
        } else if (pattern_type == 2) {
            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    int c_i = max(-1, j - i - 1);
                    int c_j = max(-1, i - j - 1);
                    block2.set(i, j, (uint8)(block.at(i, j) + block2.at(c_i, c_j)));
                    score += block.at(i, j) * block.at(i, j);
                }
            }
        }
    }
    
    int best_block_intro_predict_pattern(ShadowBlock<uint8> &block, ShadowBlock<uint16> &block2, int h, int w) {
        int ptype = 0;
        double best_score = __DBL_MAX__;
        for (int i = 0; i < 3; i++) {
            double score = block_intro_predict(block, block2, h, w, i);
            if (score < best_score) {
                best_score = score;
                ptype = i;
            }
        }
        return ptype;
    }

public:
    vector<uint8> doit_intra(Channel<uint8> &channel, Channel<uint16> &channel2) {
        vector<uint8> patterns;
        const int b_h = channel.get_block_height();
        const int b_w = channel.get_frame_width();
        for (size_t i = 0; i < channel.blocks.size(); i++) {
            uint8 pattern_type = (uint8)best_block_intro_predict_pattern(channel.blocks[i], channel2.blocks[i], b_h, b_w);
            patterns.push_back(pattern_type);
        }
        return patterns;
    }
    
    void undo_intra(Channel<uint16> &channel, Channel<uint8> &channel2, const vector<uint8> &patterns) {
        int cnt = 0;
        const int b_h = channel.get_block_height();
        const int b_w = channel.get_frame_width();
        for (size_t i = 0; i < channel.blocks.size(); i++) {
            undo_block_intro_predict(channel.blocks[i], channel2.blocks[i], b_h, b_w, patterns[cnt]);
            cnt ++;
        }
    }
};

#endif /* predict_h */
