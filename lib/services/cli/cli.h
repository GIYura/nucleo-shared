#ifndef CLI_H
#define CLI_H

#include <stdint.h>

typedef void (*CliCommandHandler_t)(int argc, char** argv);

typedef struct
{
    const char* name;
    CliCommandHandler_t handler;
    const char* help;
} CliCommand_t;

void CliInit(void);
void CliProcessLine(const char* buffer);
void CliRegisterCommand(const char* name, CliCommandHandler_t handler, const char* help);

#endif /* CLI_H */

