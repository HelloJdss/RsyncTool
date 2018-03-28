//
// Created by carrot on 18-3-28.
//

#ifndef RSYNCTOOL_THREADBASE_H
#define RSYNCTOOL_THREADBASE_H

#include <pthread.h>
#include "LogHelper.h"

/*typedef struct
{
    void *_this;
    void *_args;
} thread_params;

class Thread
{
public:
    void RunThread(void* _this, void* args, bool async = true);     //启动线程处理, 默认采用异步方式

    virtual void onThreadCreated(void *args){LOG_TRACE("Thread Created!");}

    virtual void onThreadDestroyed(void *args){LOG_TRACE("Thread Created!");}
};

template<typename TYPE>
void *t_Thread(void *params)//线程启动函数，声明为模板函数
{
    thread_params *params1 = (thread_params *) params;
    Thread *This = (TYPE *) params1->_this;     //父类指针指向子类对象
    This->onThreadCreated(params1->_args);
    This->onThreadDestroyed(params1->_args);
    return nullptr;
}*/



class Thread
{
private:
    //当前线程的线程ID
    pthread_t m_tid;
    //线程的状态
    int m_threadStatus;

    //获取执行方法的指针
    static void *thread_proxy_func(void *args);

    //内部执行方法
    void *run1();

public:
    //构造函数
    Thread();

    //线程的运行实体
    virtual void Runnable() = 0;

    //开始执行线程
    bool Start();

    //等待线程直至退出
    void Join();

    //等待线程退出或者超时
    void Join(unsigned long millisTime);

    //分离线程
    void Detach();

    //终止线程
    void Abort();

    //获取线程ID
    pthread_t GetThreadID();

    //获取线程状态
    int GetState();

    enum ThreadStatus
    {
        //线程的状态－运行结束
                THREAD_STATUS_EXIT = -1,
        //线程的状态－新建
                THREAD_STATUS_NEW = 0,
        //线程的状态－正在运行
                THREAD_STATUS_RUNNING = 1,
        //线程的状态－分离执行
                THREAD_STATUS_RUNNING_DETACHED = 2,
    };
};

#endif //RSYNCTOOL_THREADBASE_H
