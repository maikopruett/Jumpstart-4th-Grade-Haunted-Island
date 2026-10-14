#include <cstdio>
#include <ctime>

#include "engine/engine.h"

int g_module;
int g_prevModule;

void Module_Jump(int module) { throw ModuleJump{module}; }

void App_Quit() {
    if (g_module != MODULE_QUIT) Module_Jump(MODULE_QUIT);
}

bool App_InitEngine() {
    Event_Init();
    Hotspot_Init();
    snprintf(g_exeDir, sizeof g_exeDir, "%s", Sys_SaveDir().c_str());
    g_resDir[0] = 0; // movies and archives are read from the mounted disc root
    Gfx_Init();
    Pal_InitBlack();
    Spr_Init();
    Anim_Init(Res_Load, Res_Free);
    Sound_Init();
    LipSync_Init();
    SRand((unsigned)time(nullptr));
    return true;
}

void App_ShutdownEngine() {
    Video_Close();
    LipSync_Shutdown();
    Sound_Shutdown();
    Anim_Shutdown();
    Spr_Shutdown();
    Gfx_Shutdown();
    Hotspot_Shutdown();
    Event_Shutdown();
}

void Fatal_Error(const char* text) {
    fprintf(stderr, "fatal: %s\n", text);
    Sys_MessageBox("JumpStart 4th Grade", text);
    exit(1);
}
