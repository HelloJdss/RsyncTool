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

#define RT_ASSERT(cond, ...) \
    do { \
        if (!(cond)) { \
            fflush(stdout); \
            fprintf(stderr, "\33[1;31m"); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\33[0m\n"); \
            assert(cond); \
        } \
    } while (0)

#define RT_PANIC(format, ...) \
    RT_ASSERT(0, format, ## __VA_ARGS__)

#define TODO() RT_PANIC("Please implement me!")

#endif //RSYNCTOOL_CM_DEFINE_H
