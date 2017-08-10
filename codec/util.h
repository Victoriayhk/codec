#pragma once

#include <map>
#include <iostream>
using namespace std;


template<class T>
class CyclePool {
    int pool_cap;
    T *pool;
    int pfront, pend;
	T init_value;
    
public:
    CyclePool();
	CyclePool(const CyclePool<T> & other);
	CyclePool(int pool_cap,const  T & init_value);
    ~CyclePool();
    
	// 初始化, 会重新申请内存, 最好运行一次once for all
    void init(int pool_cap);

    // 获取池的容量
    int capicity();

	// 计算当前池内元素个数
    int size();

	// 池尾追加一个空间并返回, 如果池已经满了, 会自动循环使用之前的空间
    T& new_back();

	// 获取池的第一个元素
    T& front();

	// 获取池的第一个元素, 这个空间会在经过capicity次new_back后
    // 被再利用
    T& pop_front();

	// 按照在队列里的从首到尾的顺序获取第order个元素
	// 不检查order的正确性
	T& at(int order);

    T& operator [](int i);

	void operator=(const CyclePool<T> & another);
};

template<class T>
CyclePool<T>::CyclePool() {
    pool_cap = 0;
    pool = nullptr;
    pfront = pend = 0;
}

template<class T>
CyclePool<T>::CyclePool(const CyclePool<T> & other) {
	pfront = other.pfront;
	pend = other.pend;
	init_value = other.init_value;
	init(other.pool_cap);
	if(pool != nullptr)
	memcpy(pool,other.pool,sizeof(T) * pool_cap);
}


template<class T>
CyclePool<T>::CyclePool(int pool_cap, const T & init_value):pool_cap(pool_cap),init_value(init_value) {
    this->pool_cap = 0;
    this->pool = nullptr;
    this->pfront = this->pend = 0;
	init(pool_cap);
}

template<class T>
CyclePool<T>::~CyclePool() {
   if (pool_cap > 0 && this->pool != nullptr) {
        delete []pool;
		pool = nullptr;
		pool_cap = 0;
    }
}

template<class T>
void CyclePool<T>::init(int pool_cap) {
    if (this->pool_cap > 0 && this->pool != nullptr) {
        delete pool;
		this->pool = nullptr;
    }
    this->pool_cap = pool_cap;
	if(pool_cap > 0) pool = new T[pool_cap];
	for(int i = 0; i < pool_cap; ++i){
		pool[i] = init_value;
	}
    pfront = pend = 0;
}

template<class T>
int CyclePool<T>::capicity() {
    return pool_cap;
}

template<class T>
int CyclePool<T>::size() {
    return pend - pfront;
}

template<class T>
T& CyclePool<T>::new_back() {
    if (pend - pfront == pool_cap) {
        pfront++;
    }
    pend++;
    return pool[(pend - 1) % pool_cap];
}

template<class T>
T& CyclePool<T>::front() {
    return pool[pfront % pool_cap];
}

template<class T>
T& CyclePool<T>::pop_front() {
    pfront++;
    return pool[(pfront - 1) % pool_cap];
}

template<class T>
T& CyclePool<T>::at(int order) {
	return pool[(pfront + order) % pool_cap];
}

template<class T>
T& CyclePool<T>::operator [] (int i) {
    if (i < 0) {
        return pool[(pend - i) % pool_cap];
    } else {
        return pool[(pfront + i) % pool_cap];
    }
}

template<class T>
void CyclePool<T>::operator=(const CyclePool<T> & other)  
{  
	this->pfront = other.pfront;
	this->pend = other.pend;
	this->init_value = other.init_value;
	this->init(pool_cap);
	if(pool != nullptr)
	memcpy(this->pool,other.pool,sizeof(T) * this->pool_cap);
}  

