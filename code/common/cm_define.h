//
// Created by carrot on 18-3-11.
//

#pragma once
#ifndef RSYNCTOOL_CM_DEFINE_H
#define RSYNCTOOL_CM_DEFINE_H

#include <cassert>

#define DECLARE_SINGLETON(T) \
    private: \
        T(){}\
        static T *m_instance; \
    public:\
        static T *Instance() \
        { \
            if (nullptr == m_instance) \
                m_instance = new T(); \
            return m_instance; \
        }

#define IMPLEMENT_SINGLETON(T) \
    T* T::m_instance = nullptr;

//use following macro instead, if the singleton need destructor

#define DECLARE_SINGLETON_EX(T) \
    private: \
        T(){}\
    public:\
        static T *Instance() \
        { \
            static T instance; \
            return &instance; \
        }

#define RT_RAWTIP(...) \
    do{ \
        fflush(stdout); \
        fprintf(stderr, "\33[1;31m"); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\33[0m\n"); \
    } while(0)

#define RT_ASSERT(cond, ...) \
    do { \
        if (!(cond)) { \
            RT_RAWTIP(__VA_ARGS__); \
            assert(cond); \
        } \
    } while (0)

#define RT_PANIC(format, ...) \
    RT_ASSERT(0, format, ## __VA_ARGS__)



#define TODO() RT_PANIC("Please implement me!")

typedef struct {
    void* _this;
    void* _args;
    bool  _needdel;
    void* (*_del_func)(void*);
} thread_params;

template <typename TYPE, void (TYPE::*RunThread)(void*) >
void* t_Thread(void* params)//线程启动函数，声明为模板函数
{
    thread_params* params1 = (thread_params*)params;
    TYPE* This = (TYPE*)params1->_this;
    This->RunThread(params1->_args);
    if (params1->_needdel)
    {
        params1->_del_func(params1->_args);
    }
    return NULL;
}

#endif //RSYNCTOOL_CM_DEFINE_H
