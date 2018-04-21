//
// Created by carrot on 18-4-21.
//

#ifndef RSYNCTOOL_CM_LOGGER_H
#define RSYNCTOOL_CM_LOGGER_H


/*
 * 在本文件中定义了所有的日志格式以及其解析正则
 */

enum LoggerType
{
    UnKnown,
    ViewDir
};

typedef struct {
    LoggerType type;
    const char* context; //内容
    const char* pattern; //正则
} Logger;

static Logger g_loggerdef[] =
        {
                {ViewDir ,"ViewDir[%d]: Path: [%s] Size: [%lld] Modify: [%llu]", R"(ViewDir\[\d*\]: Path: \[(.*)\] Size: \[(\d*)\] Modify: \[(\d*)\])"},
        };

#define NR_LOGGERS (sizeof(g_loggerdef) / sizeof(g_loggerdef[0]))

#endif //RSYNCTOOL_CM_LOGGER_H
