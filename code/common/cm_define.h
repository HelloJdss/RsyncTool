#pragma once
#ifndef RSYNCTOOL_CM_DEFINE_H
#define RSYNCTOOL_CM_DEFINE_H

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

#endif //RSYNCTOOL_CM_DEFINE_H
