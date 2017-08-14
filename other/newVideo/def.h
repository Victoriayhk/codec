//
//  def.h
//  miniVideo
//
//  Created by Jules on 2017/8/6.
//  Copyright © 2017年 Jules. All rights reserved.
//

#ifndef def_h
#define def_h

#include "utils.h"
#include <vector>
#include <cstdlib>
#include <string>
using namespace std;



///////////////////////////////////////////////////////////////
// 所有的类型申明
///////////////////////////////////////////////////////////////

typedef unsigned char uint8;
typedef unsigned short uint16;


struct AVFormat {
    static int num_frame, height, width;
    string input_file_name = "/Users/apple/Desktop/miniVideo/data/dongman.yuv";
    string mid_file_name = "/Users/apple/Desktop/miniVideo/data/dongman.mid";
    string output_file_name = "/Users/apple/Desktop/miniVideo/data/dongman2.yuv";
};

// 存储一个通道的数据
template<class T> class Channel {
    int cid;
    int frame_height, frame_width, block_height, block_width;
public:
    Array<T> frame;
    vector< ShadowBlock<T> > blocks;     Channel() {}
    ~Channel() {}

    // 初始化帧, 包括通道编码, 帧大小, 和block大小
    void init(int frame_height, int frame_width, int block_height, int block_width);
    
    // 从fin读入该通道的数据
    void read(FILE *fin);
    
    void extend(T val);
    
    // 写入该通道的数据到fout
    void write(FILE *fout);
    
    T* get_arr1() { return frame.get_arr1(); }
    T** get_arr2() { return frame.get_arr2(); }
    
    int get_frame_height() { return frame_height; }
    int get_frame_width() { return frame_width; }
    int get_block_height() { return block_height; }
    int get_block_width() { return block_width; }
 };


// 存储一个包含三个完整通道的数据的整帧
template<class T>
class Frame {
    int fid;
public:
    Channel<T> Y, U, V;
    Frame() {}
    ~Frame() {}

    // 初始化帧, 包括通道帧大小, 和block(Y通道, U/V通道为1/4)大小
    void init(int fid, int frame_height, int frame_width, int block_height, int block_width, T extend_val);
    
    // 从fin读入该帧(共三个通道)的数据
    void read(FILE *fin);
    
    // 写入该帧(共三个通道)的数据到fout
    void write(FILE *fout);
        
    int get_id();
    
    // 输出查看
    void display(int h, int w) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                printf("%3d ", Y.frame.at(i, j));
            }
            printf(" :(Y)\n");
        }
        printf("\n");
        
        for (int i = 0; i < h/2; i++) {
            for (int j = 0; j < w/2; j++) {
                printf("%3d ", U.frame.at(i, j));
            }
            printf(" :(U)\n");
        }
        printf("\n");
        
        for (int i = 0; i < h/2; i++) {
            for (int j = 0; j < w/2; j++) {
                printf("%3d ", V.frame.at(i, j));
            }
            printf(" :(V)\n");
        }
        printf("\n");
    }
};


///////////////////////////////////////////////////////////////
// Channel
///////////////////////////////////////////////////////////////


template<class T>
void Channel<T>::init(int frame_height, int frame_width, int block_height, int block_width) {
    this->frame_height = frame_height;
    this->frame_width  = frame_width;
    this->block_height = block_height;
    this->block_width  = block_width;
    int num_block_row = (frame_height + block_height - 1) / block_height;
    int num_block_col = (frame_width  + block_width  - 1) / block_width;
    frame.init(num_block_row * block_height, num_block_col * block_width, 128);
    for (int i = 0; i < frame_height; i += block_height) {
        for (int j = 0; j < frame_width; j += block_width) {
            blocks.push_back(ShadowBlock<T>());
            blocks.back().init(frame, i, j);
        }
    }
}

template<class T>
void Channel<T>::read(FILE *fin) {
    for (int i = 0; i < frame_height; i++) {
        frame.read_stream(i, 0, frame_width, fin);
    }
}

template<class T>
void Channel<T>::extend(T val) {
    for (int i = frame_height; i < frame.get_height(); i++) {
        for (int j = frame_width; j < frame.get_width(); j++) {
            frame.set(i, j, val);
        }
    }
}

template<class T>
void Channel<T>::write(FILE *fout) {
    for (int i = 0; i < frame_height; i++) {
        frame.write_stream(i, 0, frame_width, fout);
    }
}




///////////////////////////////////////////////////////////////
// Frame
///////////////////////////////////////////////////////////////

template<class T>
void Frame<T>::init(int fid, int frame_height, int frame_width, int block_height, int block_width, T val) {
    this->fid = fid;
    Y.init(frame_height,frame_width, block_height, block_width);
    U.init(frame_height/2,frame_width/2, block_height/2, block_width/2);
    V.init(frame_height/2,frame_width/2, block_height/2, block_width/2);
    Y.extend(val);
    U.extend(val);
    V.extend(val);
}

template<class T>
int Frame<T>::get_id() {
    return fid;
}

template<class T>
void Frame<T>::read(FILE *fin) {
    Y.read(fin);
    U.read(fin);
    V.read(fin);
}

template<class T>
void Frame<T>::write(FILE *fout) {
    Y.write(fout);
    U.write(fout);
    V.write(fout);
}



#endif /* def_h */
