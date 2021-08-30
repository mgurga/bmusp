
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <random>

#ifndef CONFIG_H
#define CONFIG_H
#include "../config.h"
#endif

#ifndef PLAYER_CPP
#define PLAYER_CPP
#include "player.cpp"
#endif

#ifndef LIBRARY_CPP
#define LIBRARY_CPP
#include "library.cpp"
#endif

using namespace std;

class Cli
{
public:
    void send_message(int argc, char *argv[])
    {
        if (strcmp(argv[1], "ping") == 0)
        {
            cout << "pong" << endl;
        }
        else if (strcmp(argv[1], "next") == 0 || strcmp(argv[1], "prev") == 0 || strcmp(argv[1], "play") == 0 || strcmp(argv[1], "pause") == 0 || strcmp(argv[1], "toggle-pause") == 0 || strcmp(argv[1], "stop") == 0)
        {
            ofstream cmdfile;
            cmdfile.open(string(CONFIG_LOCATION) + "bmuspcmd");
            cmdfile << argv[1];
            cmdfile.close();
        }
        else if (strcmp(argv[1], "change-volume") == 0 || strcmp(argv[1], "set-volume") == 0)
        {
            if (argv[2] == NULL)
            {
                cout << "2 arguments are required" << endl;
                return;
            }
            ofstream cmdfile;
            cmdfile.open(string(CONFIG_LOCATION) + "bmuspcmd");
            cmdfile << argv[1] << endl;
            cmdfile << argv[2];
            cmdfile.close();
        }
        else
        {
            cout << "invalid command" << endl;
            cout << "valid commands: play,pause,next,prev,toggle-pause,stop" << endl;
            cout << "                set-volume <vol>,change-volume <diff>" << endl;
        }
    }

    void check_command(Player *plr, Library *lib, int *playlist, EndAction *end_action, default_random_engine *generator)
    {
        ifstream cmdfile(string(CONFIG_LOCATION) + "bmuspcmd");
        if (cmdfile.is_open())
        {
            string cmd;
            getline(cmdfile, cmd);
            remove(string(string(CONFIG_LOCATION) + "bmuspcmd").c_str());

            if (cmd == "play")
            {
                plr->unpause();
            }
            else if (cmd == "pause")
            {
                plr->pause();
            }
            else if (cmd == "next")
            {
                plr->jump_to(plr->song.duration);
                switch(*end_action)
                {
                case REPEAT:
                    plr->play(plr->song);
                    break;
                case STOP:
                    plr->stop();
                    break;
                case NEXT:
                {
                    int songIndex = lib->get_song_number(*playlist, plr->song) + 1;
                    if (lib->get_playlist_songs(*playlist)->size()  == songIndex)
                        songIndex = 0;
                    Song nSong = lib->get_song_at(*playlist, songIndex);
                    plr->play(nSong);
                    break;
                }
                case RANDOM:
                {
                    uniform_int_distribution<int> distribution(0,lib->get_playlist_songs(*playlist)->size() - 1);
                    int rnd = distribution(*generator);
                    Song nSong = lib->get_song_at(*playlist, rnd);
                    plr->play(nSong);
                    break;
                }
                }
            }
            else if (cmd == "prev")
            {
                if (plr->is_playing())
                {
                    Song pSong = lib->get_song_at(0, lib->get_song_number(0, plr->song) - 1);
                    plr->play(pSong);
                }
            }
            else if (cmd == "toggle-pause")
            {
                plr->toggle_pause();
            }
            else if (cmd == "stop")
            {
                plr->stop();
            }
            else if (cmd == "set-volume")
            {
                string volstr;
                getline(cmdfile, volstr);
                int newvol = stoi(volstr);
                if (newvol > 100)
                    newvol = 100;
                if (newvol < 0)
                    newvol = 0;
                plr->set_volume(newvol);
            }
            else if (cmd == "change-volume")
            {
                string volstr;
                getline(cmdfile, volstr);
                int newvol = stoi(volstr);
                newvol = plr->get_volume() + newvol;
                if (newvol > 100)
                    newvol = 100;
                if (newvol < 0)
                    newvol = 0;
                plr->set_volume(newvol);
            }
        }
    }
};
