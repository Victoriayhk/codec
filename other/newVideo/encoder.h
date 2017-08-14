//
//  encoder.h
//  miniVideo
//
//  Created by Jules on 2017/8/6.
//  Copyright © 2017年 Jules. All rights reserved.
//

#ifndef encoder_h
#define encoder_h

#include "def.h"
#include "utils.h"
#include "dct.h"
#include "predict.h"
#include "quantize.h"
#include <cmath>

class FrameSolver {
public:
    vector<uint8> patterns[3];
    
    void encode_chanel(Channel<uint8> &channel, Channel<int16_t> &channel2, ShadowPool< Channel<uint8> > &pool, vector<uint8> &patterns, int ctype) {
       for (int i = 0; i < channel.get_frame_height(); i++) {
           for (int j = 0; j < channel.get_frame_width(); j++) {
               channel2.frame.set(i, j, (int16_t)channel.frame.at(i, j));
           }
        }
        
        // Predict predict_solver;
        // patterns = predict_solver.doit_intra(channel, channel2);

        DCT4x4Solver dct_solver;
        dct_solver.dct(channel2.frame.get_arr2(), channel2.get_frame_height(), channel2.get_frame_width());

        Quantize quantize_solver;
        double qt = 1.2;
        if (ctype != 0) qt = 4;
        quantize_solver.doit(channel2);
    }
    
    
    void decode_chanel(Channel<int16_t> &channel, Channel<uint8> &channel2, const vector<uint8> &patterns, int ctype) {
        Quantize quantize_solver;
        double qt = 1.2;
        if (ctype != 0) qt = 4;
        quantize_solver.undo(channel);

        DCT4x4Solver dct_solver;
        dct_solver.reverse_dct(channel.frame.get_arr2(), channel.get_frame_height(), channel.get_frame_width());
        
        // Predict predict_solver;
        // predict_solver.undo_intra(channel, channel2, patterns);

        for (int i = 0; i < channel.get_frame_height(); i++) {
           for (int j = 0; j < channel.get_frame_width(); j++) {
               channel.frame.set(i, j, (uint8)channel2.frame.at(i, j));
           }
        }
    }
    
    void encode(Frame<uint8> &frame, Frame<int16_t> &frame2, ShadowPool<Channel<uint8> > pools []) {
        encode_chanel(frame.Y, frame2.Y, pools[0], patterns[0], 0);
        encode_chanel(frame.U, frame2.U, pools[1], patterns[1], 1);
        encode_chanel(frame.V, frame2.V, pools[2], patterns[2], 2);
    }

    void decode(Frame<int16_t> &frame, Frame<uint8> &frame2) {
        decode_chanel(frame.Y, frame2.Y, patterns[0], 0);
        decode_chanel(frame.U, frame2.U, patterns[0], 1);
        decode_chanel(frame.V, frame2.V, patterns[0], 2);
    }
};


#endif /* encoder_h */
