//
//  utils.h
//  miniVideo
//
//  Created by Jules on 2017/8/5.
//  Copyright © 2017年 Jules. All rights reserved.
//

#ifndef utils_h
#define utils_h

#include <cstdio>
using namespace std;


///////////////////////////////////////////////////////////////
// 所有的申明
///////////////////////////////////////////////////////////////

// 连续存储的可一维读取, 也可二维读取的数组
template<class T>
class Array {
    int height, width, size;
    T default_val;
    T *arr1, **arr2;
public:
    Array();
    ~Array();
    
    // 初始化为 h * w的二维矩阵, 使用read_stream(), at()等方法前必须先调用
    // default_val表示读取下标越界时的默认值
    void init(const int h, const int w, T defualt_val);
    
    // 销毁内存式清空
    void destroy();
    
    // 从fin读取一段流
    void read_stream(const int i, const int j, const int length, FILE *fin);
    
    // 写入一段流到fout
    void write_stream(const int i, const int j, const int length, FILE *fin);
    
    // 按下标返回相应值, 越界时, 返回default_val
    T at(const int i, const int j);
    T at(const int i);
    int get_height() { return height; }
    int get_width() { return width; }
    
    T* get_arr1() { return arr1; }
    T** get_arr2() { return arr2; }

    
    // 设置值
    void set(const int i, T val);
    void set(const int i, const int j, T val);
};


// 使用内存引用的方法由Array的数据构建一个二维块
template<class T> class ShadowBlock {
    int offset_i, offset_j;
    Array<T> *from_arr;
public:
    ShadowBlock();
    ~ShadowBlock();
    
    // 初始化, 指定Array, 和块在Array中的起始点坐标(offset)
    void init(Array<T> & arr, const int &offset_i, const int &offset_j);
    
    // 返回位于块内位置(i, j)上的元素
    inline T at(int i, int j);
    
    // 设置值
    void set(int i, int j, T val);
};


// 循环队列版帧池, 最终一直到了项目中
template<class T>
class ShadowPool {
    int list_size, pool_size;
    T **pool;
    T **list;
    int *index_of_pool;
    
    int pfront, pend;
    
public:
    ShadowPool();
    ~ShadowPool();
    
    void init(int list_size, int pool_size);
    int item_counts();
    void push_back(T *item, int idx);
    T* front();
    void pop();
    T* at(int idx);
};

///////////////////////////////////////////////////////////////
// Array
///////////////////////////////////////////////////////////////



template<class T>
Array<T>::Array(){
    arr1 = 0;
    arr2 = 0;
    height = width = size = 0;
}

template<class T>
Array<T>::~Array() {
    if (size > 0) {
        delete arr1;
        delete arr2;
        size = 0;
    }
}

template<class T>
void Array<T>::init(const int h, const int w, T default_val) {
    height = h;
    width  = w;
    size   = h * w;
    this->default_val = default_val;
    arr1 = new T[size];
    arr2 = new T*[h];
    arr2[0] = arr1;
    for (int i = 1; i < h; i++) {
        arr2[i] = arr2[i - 1] + w;
    }
}

template<class T>
void Array<T>::destroy() {
    if (size > 0) {
        delete arr1;
        delete arr2;
        size = 0;
    }
}

template<class T>
void Array<T>::read_stream(const int i, const int j, const int length, FILE *fin) {
    // 读出fin: 长度为length的流数据到起始坐标从(i, j)开始的位置
    fread(arr2[i] + j, length, 1, fin);
}

template<class T>
void Array<T>::write_stream(const int i, const int j, const int length, FILE *fout) {
    // 写入fout: 长度为length的流数据到起始坐标从(i, j)开始的位置
    fwrite(arr2[i] + j, length, 1, fout);
}

template<class T>
T Array<T>::at(const int i, const int j) {
    if (i < 0 || i >= height || j < 0 || j >= width) {
        return default_val;
    }
    return arr2[i][j];
}

template<class T>
void Array<T>::set(const int i, T val) {
    if (i < 0 || i >= size) {
        return default_val;
    }
    arr1[i] = val;
}

template<class T>
void Array<T>::set(const int i, const int j, T val) {
    arr2[i][j] = val;
}


template<class T>
T Array<T>::at(const int i) {
    return arr1[i];
}


///////////////////////////////////////////////////////////////
// ShadowBlcok
///////////////////////////////////////////////////////////////

 

template<class T>
ShadowBlock<T>::ShadowBlock() {
    from_arr = nullptr;
}

template<class T>
ShadowBlock<T>::~ShadowBlock() {
}

template<class T>
void ShadowBlock<T>::init(Array<T> & arr, const int &offset_i, const int &offset_j) {
    from_arr = &arr;
    this->offset_i = offset_i;
    this->offset_j = offset_j;
}

template<class T>
inline T ShadowBlock<T>::at(int i, int j) {
    return from_arr->at(i + offset_i, j + offset_j);
}


template<class T>
void ShadowBlock<T>::set(const int i, const int j, T val) {
    from_arr->set(i + offset_i, j + offset_j, val);
}



///////////////////////////////////////////////////////////////
// ShadowPool
///////////////////////////////////////////////////////////////



template<class T>
ShadowPool<T>::ShadowPool() {
    pool_size = list_size = 0;
    pool = list = nullptr;
}

template<class T>
ShadowPool<T>::~ShadowPool() {
    if (pool_size > 0) {
        delete list;
        delete pool;
        delete index_of_pool;
    }
}


template<class T>
void ShadowPool<T>::init(int list_size, int pool_size) {
    this->list_size = list_size;
    this->pool_size = pool_size;
    list = new T*[list_size];
    pool = new T*[pool_size];
    index_of_pool = new int[pool_size];
    pfront = pend = 0;
    
    for (int i = 0; i < list_size; i++) {
        list[i] = nullptr;
    }
    
    for (int i = 0; i <pool_size; i++) {
        pool[i] = nullptr;
        index_of_pool[i] = -1;
    }
}

    
template<class T>
int ShadowPool<T>::item_counts() {
    return pend - pfront;
}

template<class T>
void ShadowPool<T>::push_back(T *item, int idx) {
//#ifdef DEBUG0
//    if (item_counts() >= pool_size) {
//        printf("Pool is full.\n");
//    }
//    if (idx >= list_size) {
//        printf("ShadowPool: bad index");
//    }
//#endif
    list[idx] = item;
    pool[pend % pool_size] = item;
    index_of_pool[pend % pool_size] = idx;
    pend++;
}

template<class T>
T* ShadowPool<T>:: front() {
    return pool[pfront % pool_size];
}

template<class T>
void ShadowPool<T>::pop() {
    list[index_of_pool[pfront % pool_size]] = nullptr;
    pool[pfront % pool_size] = nullptr;
    index_of_pool[pfront % pool_size] = -1;
    pfront++;
}

template<class T>
T* ShadowPool<T>::at(int idx) {
//#ifdef DEBUG0
//    if (idx >= list_size) {
//        printf("ShadowPool: bad index");
//    }
//    #endif
    return list[idx];
}


#endif /* utils_h */
