// 4-channel 22050 Hz 8-bit software mixer (0x42469c..0x424d6e).
// Sounds are 'KWAV' resources: {'KWAV', u32 length, signed 8-bit samples}.
#pragma once
#include "engine/core.h"

void Sound_Init();                    // 0x42469c
void Sound_Shutdown();                // 0x424728
// Keeps the original's buffer refill hook; mixing runs on the audio thread.
void Sound_Update();                  // 0x424bb6

int Sound_PlayOnChannel(const void* kwav, int loop, u32 priority); // 0x4248c6, -1 if none
void Sound_StopChannel(int ch);       // 0x42499c
void Sound_StopAllChannels();         // 0x4249e2
bool Sound_ChannelPlaying(int ch);    // 0x4249fd
int Sound_Play(const void* kwav, int loop, u32 priority); // 0x424a53 handle or 0
void Sound_Stop(int handle);          // 0x424a86
bool Sound_IsPlaying(int handle);     // 0x424aac
int Sound_FindChannel(int handle);    // 0x424ace
int Sound_ChannelHandle(int ch);      // 0x424aef
void Sound_Pause();                   // 0x424b0e
void Sound_Resume();                  // 0x424b2e
void Sound_Reset();                   // 0x424b60
void Sound_SetEnabled(int on, int save); // 0x424b7a
bool Sound_Enabled();

// Extra output stream used by the Smacker player.
struct SDL_AudioStream;
SDL_AudioStream* Sound_OpenStream(int freq, int channels, int bits);
void Sound_CloseStream(SDL_AudioStream* s);
