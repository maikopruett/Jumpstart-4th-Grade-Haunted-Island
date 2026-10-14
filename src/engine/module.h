// Game module dispatch. The original keeps a jmp_buf (0x43959c) and switches
// activities with longjmp(jb, moduleNumber); the port uses an exception
// caught by the dispatcher in game/main.cpp.
#pragma once

struct ModuleJump {
    int module;
};

extern int g_module;      // 0x4395bc
extern int g_prevModule;  // 0x4395c0

[[noreturn]] void Module_Jump(int module); // longjmp(0x43959c, module)
void App_Quit();                            // 0x4025c6: jump to module 20 (quit)

enum { MODULE_QUIT = 20 };
