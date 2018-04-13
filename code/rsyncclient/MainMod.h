//
// Created by carrot on 18-3-19.
//

#ifndef RSYNCTOOL_MAINMOD_H
#define RSYNCTOOL_MAINMOD_H

#include <string>
#include "cm_define.h"
#include "tinyxml2.h"
#include "LogHelper.h"

namespace RsyncClient
{
#define CONFIG_PATH "./config/client.xml"

    using std::string;
    using namespace tinyxml2;

    struct ST_command
    {
        int id;
        string arg;
        string description;
    };

    class Configuration    //本程序的基础配置信息
    {
        DECLARE_SINGLETON(Configuration)

    public:

        int SaveAsXml(const char* xmlPath = CONFIG_PATH);

        int LoadXml(const char* xmlPath = CONFIG_PATH);

    public:
        LOG_LEVEL m_log_lv = LOG_LEVEL_DEFALT;
        string m_log_file = LOG_FILE_PATH;
        string m_view_output = "./view_output.xml";
        int64_t m_view_output_max_size = 10 * 1024 * 1024; //输出文件的最大大小，超过此大小下一次执行任务时将会清空文件
    };

#define g_Configuration Configuration::Instance()

    class MainMod
    {
    public:
        static bool Init(int argc, char *argv[], std::string appName);

        static int Run();

        static void onRecvSignal(int signal);

    private:
        static ST_command m_cmds[];

        static bool cmd_push(string, string); //正向同步任务（客户端==>服务器）

        static bool cmd_pull(string, string); //反向同步任务（服务器==>客户端）

        static bool cmd_f(string); //读取文件

        static bool cmd_v(string); //读取文件列表

#define NR_CMDS (sizeof(m_cmds) / sizeof(m_cmds[0]))
    };
}

#endif //RSYNCTOOL_MAINMOD_H
