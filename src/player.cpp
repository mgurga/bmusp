
#ifndef COMMON_H
#define COMMON_H
#include "common.h"
#endif

#ifndef CONFIG_H
#define CONFIG_H
#include "../config.h"
#endif

#include <ctime>
#include <list>
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
        int audio_rate = 44100;
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

    void play()
    {
        if (q.size() != 0)
            play(q.front());
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

    void jump_to(int seek)
    {
        if (current_position() == seek)
            return;
        if (!is_playing())
            return;
        Mix_SetMusicPosition(seek);
        int diff = current_position() - seek;
        starttime += diff;
    }

    void stop()
    {
        Mix_HaltMusic();
        starttime = 0;
        pausestart = 0;
        paused = true;
    }

    void add_to_queue(Song s)
    {
        q.push_back(s);
        if (!is_playing())
        {
            playingq = true;
            play(q.front());
        }
    }

    int get_song_queue_num(Song s)
    {
        int i = 0;
        for (Song qs : q)
        {
            if (qs == s)
            {
                return i;
            }
            i++;
        }
        return -1;
    }

    bool is_playing()
    {
        return (!paused && Mix_PlayingMusic() == 1);
    }

    bool is_song_over()
    {
        // cout << "queue size: " << q.size() << " playing q: " << (playingq ? "true" : "false") << endl;
        if (current_position() == song.duration)
        {
            if (!q.empty() && playingq)
                q.pop_front();
            if (q.empty())
            {
                playingq = false;
                stop();
            }
            else
            {
                play(q.front());
                playingq = true;
                return false;
            }
            return true;
        }
        return false;
    }

    void toggle_pause()
    {
        if (is_playing())
        {
            pause();
        }
        else
        {
            unpause();
        }
    }

    void clear_queue()
    {
        q.clear();
    }

    Song get_queue_song_at(int num)
    {
        list<Song>::iterator it = q.begin();
        advance(it, num);
        return *it;
    }

    bool has_queue()
    {
        if (q.size() > 1)
            return true;
        else
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
    list<Song> q;
    bool playingq = false;
};