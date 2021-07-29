
#ifndef COMMON_H
#define COMMON_H
#include "common.h"
#endif

#ifndef CONFIG_H
#define CONFIG_H
#include "../config.h"
#endif

#include <ctime>
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

using namespace std;

class Player
{
public:
    Song song;
    bool paused;

    Player()
    {
        int audio_rate = MIX_DEFAULT_FREQUENCY;
        Uint16 audio_format = MIX_DEFAULT_FORMAT;
        int audio_channels = MIX_DEFAULT_CHANNELS;
        int audio_buffers = 4096;

        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            cout << "Couldn't initialize SDL: " << SDL_GetError() << endl;
        }

        if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0)
        {
            cout << "Couldn't open audio: " << SDL_GetError() << endl;
        }
        else
        {
            Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
            // SDL_Log("Opened audio at %d Hz %d bit%s %s %d bytes audio buffer\n", audio_rate,
            //         (audio_format & 0xFF),
            //         (SDL_AUDIO_ISFLOAT(audio_format) ? " (float)" : ""),
            //         (audio_channels > 2) ? "surround" : (audio_channels > 1) ? "stereo"
            //                                                                  : "mono",
            //         audio_buffers);
        }

        Mix_VolumeMusic(DEFAULT_VOLUME);
        vol = DEFAULT_VOLUME;
    }

    void play(Song s)
    {
        if (is_playing())
        {
            stop();
        }
        music = Mix_LoadMUS(s.path.c_str());
        Mix_PlayMusic(music, 0);
        starttime = time(0);
        paused = false;
        song = s;
    }

    int current_position()
    {
        if (paused)
            return pausepos;
        if (starttime == 0)
            return -1;
        if (Mix_PlayingMusic == 0)
            return -1; // no music playing
        time_t now;
        time(&now);
        double diff = difftime(now, starttime);
        return (int)diff;
    }

    void pause()
    {
        Mix_PauseMusic();
        pausepos = current_position();
        pausestart = time(0);
        paused = true;
    }

    void unpause()
    {
        Mix_ResumeMusic();
        paused = false;
        time_t now;
        time(&now);
        starttime = starttime + difftime(now, pausestart);
        pausestart = 0;
    }

    void stop()
    {
        Mix_HaltMusic();
        starttime = 0;
        pausestart = 0;
        paused = true;
    }

    bool is_playing()
    {
        return (!paused && Mix_PlayingMusic() == 1);
    }

    bool is_song_over() {
        if (current_position() == song.duration)
        {
            stop();
            return true;
        }
        return false;
    }

    int get_volume()
    {
        return vol;
    }

    void set_volume(int v)
    {
        vol = v;
        Mix_VolumeMusic(v);
    }

private:
    Mix_Music *music = NULL;
    time_t starttime;
    time_t pausestart;
    double pausepos;
    int vol;
};