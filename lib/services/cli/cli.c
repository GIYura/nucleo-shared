#include <string.h>

#include "assert.h"
#include "cli.h"
#include "logger.h"
#include "uart-service.h"

#define CLI_COMMANDS_MAX  10
#define CLI_ARGS_MAX       5

static CliCommand_t m_cmdList[CLI_COMMANDS_MAX];
static uint8_t m_cmdCount = 0;

static char m_cmd[64];
static uint8_t m_index = 0;

static uint8_t SplitLine(char* line, char** argv, int maxArgs)
{
    uint8_t argc = 0;
    char* p = line;
    bool inToken = false;

    while (*p != '\0')
    {
        if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        {
            *p = '\0';
            inToken = false;
        }
        else if (!inToken)
        {
            if (argc < maxArgs)
            {
                argv[argc++] = p;
                inToken = true;
            }
        }
        p++;
    }

    return argc;
}

static void OnUartRxCompleted(const uint8_t* data, uint8_t len)
{
    ASSERT(data);

    uint8_t ch;

    for (uint8_t i = 0; i < len; i++)
    {
        ch = data[i];

        LogPrint("%c", ch);

        if (ch == '\r' || ch == '\n')
        {
            LogPrint("\r\n");

            m_cmd[m_index] = '\0';
            CliProcessLine(m_cmd);
            m_index = 0;
        }
        else if (m_index < sizeof(m_cmd) - 1)
        {
            m_cmd[m_index++] = ch;
        }
    }
}

void CliInit(void)
{
    UartServiceRegisterRxCallback(&OnUartRxCompleted);
}

void CliProcessLine(const char* buffer)
{
    char* argv[CLI_COMMANDS_MAX];
    uint8_t argc = 0;
    char buff[64];

    strncpy(buff, buffer, sizeof(buff) - 1);

    argc = SplitLine(buff, argv, CLI_COMMANDS_MAX);

    if (argc == 0)
    {
        return;
    }

    for (uint8_t i = 0; i < m_cmdCount; i++)
    {
        if (strcmp(argv[0], m_cmdList[i].name) == 0)
        {
            m_cmdList[i].handler(argc, argv);

            return;
        }
    }

    LogPrint("Unknown command: %s\r\n", argv[0]);
}

void CliRegisterCommand(const char* name, CliCommandHandler_t handler, const char* help)
{
    if (m_cmdCount < CLI_COMMANDS_MAX)
    {
        m_cmdList[m_cmdCount].name = name;
        m_cmdList[m_cmdCount].help = help;
        m_cmdList[m_cmdCount].handler = handler;

        ++m_cmdCount;
    }
}

