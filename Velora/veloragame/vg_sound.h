#ifndef VELORAGAME_SOUND_H
#define VELORAGAME_SOUND_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <mmsystem.h>
    #pragma comment(lib, "winmm.lib")
#endif

// ─── Sound Structures ───

typedef struct {
    char path[512];
    int is_music;
    float volume;
    float pitch;
    int looping;
    int playing;
} VgSound;

typedef struct {
    float master_volume;
    float sfx_volume;
    float music_volume;
    int initialized;
    VgSound* current_music;
} VgAudioContext;

static VgAudioContext vg_audio = {1.0f, 1.0f, 1.0f, 0, NULL};

// ─── Audio Initialization ───

static inline int vg_audio_init() {
    vg_audio.initialized = 1;
    vg_audio.master_volume = 1.0f;
    vg_audio.sfx_volume = 1.0f;
    vg_audio.music_volume = 1.0f;
    return 1;
}

static inline void vg_audio_quit() {
    #ifdef _WIN32
        mciSendStringA("stop all", NULL, 0, NULL);
        mciSendStringA("close all", NULL, 0, NULL);
    #endif
    vg_audio.initialized = 0;
}

// ─── Sound Loading ───

static inline VgSound* vg_sound_load(const char* path) {
    VgSound* sound = (VgSound*)calloc(1, sizeof(VgSound));
    strncpy(sound->path, path, sizeof(sound->path) - 1);
    sound->volume = 1.0f;
    sound->pitch = 1.0f;
    sound->looping = 0;
    sound->playing = 0;
    sound->is_music = 0;
    return sound;
}

static inline VgSound* vg_music_load(const char* path) {
    VgSound* sound = vg_sound_load(path);
    sound->is_music = 1;
    return sound;
}

static inline void vg_sound_destroy(VgSound* sound) {
    free(sound);
}

// ─── Sound Playback ───

static inline void vg_sound_play(VgSound* sound, float volume, float pitch) {
    if (!sound || !vg_audio.initialized) return;
    sound->volume = volume * vg_audio.sfx_volume * vg_audio.master_volume;
    sound->playing = 1;

    #ifdef _WIN32
    {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "open \"%s\" alias sfx_%p", sound->path, (void*)sound);
        mciSendStringA(cmd, NULL, 0, NULL);

        char vol_str[64];
        int win_vol = (int)(sound->volume * 1000);
        snprintf(vol_str, sizeof(vol_str), "setaudio sfx_%p volume to %d", (void*)sound, win_vol);
        mciSendStringA(vol_str, NULL, 0, NULL);

        snprintf(cmd, sizeof(cmd), "play sfx_%p", (void*)sound);
        mciSendStringA(cmd, NULL, 0, NULL);
    }
    #endif
}

static inline void vg_music_play(VgSound* music, int loop) {
    if (!music || !vg_audio.initialized) return;
    music->looping = loop;
    music->playing = 1;
    vg_audio.current_music = music;

    #ifdef _WIN32
    {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "open \"%s\" alias music", music->path);
        mciSendStringA(cmd, NULL, 0, NULL);

        int win_vol = (int)(music->volume * vg_audio.music_volume * vg_audio.master_volume * 1000);
        char vol_str[64];
        snprintf(vol_str, sizeof(vol_str), "setaudio music volume to %d", win_vol);
        mciSendStringA(vol_str, NULL, 0, NULL);

        if (loop) {
            mciSendStringA("play music repeat", NULL, 0, NULL);
        } else {
            mciSendStringA("play music", NULL, 0, NULL);
        }
    }
    #endif
}

static inline void vg_music_stop() {
    if (!vg_audio.initialized) return;
    if (vg_audio.current_music) vg_audio.current_music->playing = 0;
    vg_audio.current_music = NULL;

    #ifdef _WIN32
        mciSendStringA("stop music", NULL, 0, NULL);
        mciSendStringA("close music", NULL, 0, NULL);
    #endif
}

static inline void vg_music_pause() {
    #ifdef _WIN32
        mciSendStringA("pause music", NULL, 0, NULL);
    #endif
}

static inline void vg_music_resume() {
    #ifdef _WIN32
        mciSendStringA("resume music", NULL, 0, NULL);
    #endif
}

static inline void vg_music_set_volume(float volume) {
    vg_audio.music_volume = volume;
    #ifdef _WIN32
        int win_vol = (int)(volume * vg_audio.master_volume * 1000);
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "setaudio music volume to %d", win_vol);
        mciSendStringA(cmd, NULL, 0, NULL);
    #endif
}

static inline void vg_audio_set_master_volume(float volume) {
    vg_audio.master_volume = volume;
}

static inline int vg_music_is_playing() {
    return vg_audio.current_music && vg_audio.current_music->playing;
}

#endif
