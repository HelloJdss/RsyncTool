//
// Created by carrot on 18-3-28.
//

#include <unistd.h>
#include <csignal>
#include "ThreadBase.h"

/*void Thread::RunThread(void* (*t_thread)(void*), void *args, bool async)
{
    pthread_t pid;
    thread_params params;
    params._this = this;
    params._args = args;

    if (pthread_create(&pid, nullptr, t_thread, &params) == 0)
    {
        if(async)
        {
            pthread_detach(pid);
        }
        else
        {
            pthread_join(pid, nullptr);
        }
    }
    else
    {
        LOG_ERROR("Thread Create Failed: %s", strerror(errno));
    }

}

void *t_Thread(void *params)//线程启动函数，声明为模板函数
{
    thread_params *params1 = (thread_params *) params;
    Thread *This = (Thread *) params1->_this;     //父类指针指向子类对象
    This->onThreadCreated(params1->_args);
    This->onThreadDestroyed(params1->_args);
    return nullptr;
}

void Thread::RunThread(void* _this, void *_args, bool async)
{

    pthread_t pid;
    thread_params params;
    params._this = _this;
    params._args = _args;

    if (pthread_create(&pid, nullptr, t_Thread, &params) == 0)
    {
        if(async)
        {
            pthread_detach(pid);
        }
        else
        {
            pthread_join(pid, nullptr);
        }
    }
    else
    {
        LOG_ERROR("Thread Create Failed: %s", strerror(errno));
    }
}*/

void *Thread::run1()
{
    struct sigaction acct;

    acct.sa_handler = Thread::onRecvSignal;
    sigemptyset(&acct.sa_mask);
    acct.sa_flags = 0;
    sigaction(SIGUSR1, &acct, nullptr);

    LOG_TRACE("Thread Start!");
    m_tid = pthread_self();
    Runnable();
    m_tid = 0;
    m_threadStatus = THREAD_STATUS_EXIT;
    LOG_TRACE("Thread Stop!");
    pthread_exit(nullptr);
}

Thread::Thread()
{
    m_tid = 0;
    m_threadStatus = THREAD_STATUS_NEW;
}

bool Thread::Start()
{
    auto ret = pthread_create(&m_tid, nullptr, thread_proxy_func, this) == 0;
    if(ret)
    {
        m_threadStatus = THREAD_STATUS_RUNNING;
    }
    return ret;
}

pthread_t Thread::GetThreadID()
{
    return m_tid;
}

int Thread::GetState()
{
    return m_threadStatus;
}

void Thread::Join()
{
    if (m_tid > 0)
    {
        pthread_join(m_tid, nullptr);
    }
}

void *Thread::thread_proxy_func(void *args)
{
    Thread *pThread = static_cast<Thread *>(args);

    pThread->run1();

    return nullptr;
}

void Thread::Join(unsigned long millisTime)
{
    if (m_tid == 0)
    {
        return;
    }
    if (millisTime == 0)
    {
        Join();
    }
    else
    {
        unsigned long k = 0;
        while (m_threadStatus != THREAD_STATUS_EXIT && k <= millisTime)
        {
            usleep(100);
            k++;
        }
    }
}

void Thread::Detach()
{
    if (m_tid > 0)
    {
        pthread_detach(m_tid);
        m_threadStatus = THREAD_STATUS_RUNNING_DETACHED;
    }
}

void Thread::Abort()
{
    if(m_tid > 0)
    {
        pthread_kill(m_tid, SIGUSR1);
        m_tid = 0;
        m_threadStatus = THREAD_STATUS_EXIT;
    }
}

void Thread::onRecvSignal(int sig)
{
    if(sig == SIGUSR1)
    {
        LOG_TRACE("Thread[%d] Exit ", pthread_self());
        pthread_exit(nullptr);
    }
}
