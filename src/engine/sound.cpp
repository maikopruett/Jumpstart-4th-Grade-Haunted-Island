#include "engine/sound.h"

#include <SDL3/SDL.h>

#include "engine/hostfs.h"
#include "engine/input.h"
#include "engine/sys.h"

namespace {

const int kRate = 22050;
const int kChannels = 4;
const u32 kLatency = 0x1588; // the original schedules sounds 250 ms ahead

struct Chan {
    const s8* data;
    u32 len;
    int loop;
    u64 start;
};

SDL_AudioStream* g_stream;
SDL_AudioDeviceID g_dev;
SDL_Mutex* g_mutex;
Chan g_ch[kChannels];
u32 g_prio[kChannels];   // 0x43be58
int g_handles[kChannels]; // 0x43be98
u64 g_pos;               // samples handed to the device so far
int g_nextHandle = 1;    // 0x437ae8
int g_enabled = 1;       // 0x437acc
bool g_device = false;   // strategy 4 (waveOut) vs 1 (none)
int g_pauseCount = 0;    // 0x437ae4

FILE* g_dump; // JS4_AUDIO_DUMP: raw unsigned 8-bit mono 22050 Hz output

// Mix exactly like the asm at 0x4018e0..0x401a41: sum signed samples,
// re-bias by 0x80 and saturate.
void SDLCALL Callback(void*, SDL_AudioStream* s, int additional, int) {
    if (additional <= 0) return;
    u8 buf[4096];
    while (additional > 0) {
        int n = additional < (int)sizeof buf ? additional : (int)sizeof buf;
        SDL_LockMutex(g_mutex);
        for (int i = 0; i < n; i++) {
            u64 pos = g_pos + i;
            int sum = 0, used = 0;
            for (const Chan& c : g_ch) {
                if (!c.data || pos < c.start) continue;
                u64 off = pos - c.start;
                if (off >= c.len) {
                    if (!c.loop || c.len == 0) continue;
                    off %= c.len;
                }
                sum += c.data[off];
                used++;
            }
            if (used == 0) buf[i] = 0x80;
            else if (sum < -128) buf[i] = 0x00;
            else if (sum > 127) buf[i] = 0xff;
            else buf[i] = (u8)(sum + 0x80);
        }
        g_pos += n;
        SDL_UnlockMutex(g_mutex);
        if (g_dump) fwrite(buf, 1, n, g_dump);
        SDL_PutAudioStreamData(s, buf, n);
        additional -= n;
    }
}

} // namespace

void Sound_Init() {
    int v = Ini_GetInt("Options", "SoundOn");
    g_enabled = (v == 0 || v == 2) ? 1 : 0;
    g_mutex = SDL_CreateMutex();
    if (const char* d = SDL_getenv("JS4_AUDIO_DUMP")) g_dump = Host_FOpen(d, "wb");
    SDL_AudioSpec spec = {SDL_AUDIO_U8, 1, kRate};
    g_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, Callback, nullptr);
    if (g_stream) {
        g_dev = SDL_GetAudioStreamDevice(g_stream);
        g_device = true;
        SDL_ResumeAudioStreamDevice(g_stream);
    } else {
        SDL_Log("audio unavailable: %s", SDL_GetError());
    }
    for (int i = 0; i < kChannels; i++) {
        g_prio[i] = 0;
        g_handles[i] = 0;
    }
}

void Sound_Shutdown() {
    Sound_StopAllChannels();
    if (g_stream) SDL_DestroyAudioStream(g_stream);
    g_stream = nullptr;
    g_device = false;
}

void Sound_Update() {}

bool Sound_Enabled() { return g_enabled != 0; }

int Sound_PlayOnChannel(const void* kwav, int loop, u32 priority) {
    if (!g_enabled) return -1;
    int best = -1;
    u32 lowest = priority + 1;
    for (int i = 0; i < kChannels; i++) {
        if (g_prio[i] != 0 && !Sound_ChannelPlaying(i)) g_prio[i] = 0;
        if (g_prio[i] < lowest) {
            lowest = g_prio[i];
            best = i;
        }
    }
    if (priority < lowest || best < 0) return -1;
    Sound_StopChannel(best);
    g_prio[best] = priority;
    if (!g_device) return -1;
    const u8* p = (const u8*)kwav;
    SDL_LockMutex(g_mutex);
    g_ch[best].data = (const s8*)(p + 8);
    g_ch[best].len = RdU32(p + 4);
    g_ch[best].loop = loop;
    g_ch[best].start = g_pos + kLatency;
    SDL_UnlockMutex(g_mutex);
    return best;
}

void Sound_StopChannel(int ch) {
    if (ch < 0) return;
    g_prio[ch] = 0;
    if (!g_device) return;
    SDL_LockMutex(g_mutex);
    g_ch[ch].data = nullptr;
    SDL_UnlockMutex(g_mutex);
}

void Sound_StopAllChannels() {
    for (int i = 0; i < kChannels; i++) Sound_StopChannel(i);
}

bool Sound_ChannelPlaying(int ch) {
    if (!g_device || g_fastQuit) return false;
    SDL_LockMutex(g_mutex);
    const Chan& c = g_ch[ch];
    bool playing = c.data && (c.loop || g_pos < c.start + c.len);
    SDL_UnlockMutex(g_mutex);
    return playing;
}

int Sound_Play(const void* kwav, int loop, u32 priority) {
    int ch = Sound_PlayOnChannel(kwav, loop, priority);
    if (ch < 0) return 0;
    int h = g_nextHandle++;
    g_handles[ch] = h;
    return h;
}

int Sound_FindChannel(int handle) {
    for (int i = 0; i < kChannels; i++)
        if (g_handles[i] == handle) return i;
    return -1;
}

void Sound_Stop(int handle) {
    int ch = Sound_FindChannel(handle);
    if (ch >= 0) {
        Sound_StopChannel(ch);
        g_handles[ch] = 0;
    }
}

bool Sound_IsPlaying(int handle) {
    int ch = Sound_FindChannel(handle);
    return ch >= 0 && Sound_ChannelPlaying(ch);
}

int Sound_ChannelHandle(int ch) {
    if (ch >= 0 && ch < kChannels) return g_handles[ch];
    return 0;
}

void Sound_Pause() {
    if (g_device) SDL_PauseAudioDevice(g_dev);
    g_pauseCount++;
}

void Sound_Resume() {
    g_pauseCount--;
    if (g_pauseCount < 1) {
        g_pauseCount = 0;
        if (g_device) SDL_ResumeAudioDevice(g_dev);
    }
}

void Sound_Reset() {
    Sound_StopAllChannels();
    g_pauseCount = 0;
}

void Sound_SetEnabled(int on, int save) {
    if (!on) Sound_Reset();
    g_enabled = on;
    if (save) Ini_WriteInt("Options", "SoundOn", on ? 2 : 1);
}

SDL_AudioStream* Sound_OpenStream(int freq, int channels, int bits) {
    if (!g_device) return nullptr;
    SDL_AudioSpec src = {bits == 16 ? SDL_AUDIO_S16LE : SDL_AUDIO_U8, channels, freq};
    SDL_AudioStream* s = SDL_CreateAudioStream(&src, nullptr);
    if (!s) return nullptr;
    if (!SDL_BindAudioStream(g_dev, s)) {
        SDL_DestroyAudioStream(s);
        return nullptr;
    }
    return s;
}

void Sound_CloseStream(SDL_AudioStream* s) {
    if (s) SDL_DestroyAudioStream(s);
}
