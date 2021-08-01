
#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include <list>
#include <regex>
#include "raylib.h"
#include "cli.cpp"
#include "tinyfiledialogs.h"
#include "ricons.h"
#include "raygui.h"

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

SongTag song_tag_order[] = SONG_TAG_ORDER;
int song_tag_lengths[] = SONG_TAG_LENGTHS;
string song_tag_sepatator = SONG_TAG_SEPARATOR;

string sec_to_minsec(int i)
{
    string fancyTimes;
    fancyTimes.append(to_string(i / 60) + ":");
    if ((i % 60) > 9)
        fancyTimes.append(to_string((i % 60)));
    else
        fancyTimes.append("0" + to_string((i % 60)));
    return fancyTimes;
}

string get_song_button_name(Song s)
{
    string out;
    char filler = SONG_TAG_FILLER_CHAR;

    for (int i = 0; i < sizeof(song_tag_lengths) / sizeof(song_tag_lengths[0]); i++)
    {
        switch (song_tag_order[i])
        {
        case ARTIST:
        {
            string f_artist = s.artist;
            f_artist.resize(song_tag_lengths[i], filler);
            out.append(f_artist);
            break;
        }
        case NAME:
        {
            string f_name = s.name;
            f_name.resize(song_tag_lengths[i], filler);
            out.append(f_name);
            break;
        }
        case ALBUM:
        {
            string f_album = s.album;
            f_album.resize(song_tag_lengths[i], filler);
            out.append(f_album);
            break;
        }
        case TRACKNUM:
        {
            string f_tnum = to_string(s.trackNum);
            f_tnum.resize(song_tag_lengths[i], filler);
            out.append(f_tnum);
            break;
        }
        case DURATION:
        {
            string f_dur = sec_to_minsec(s.duration);
            f_dur.resize(song_tag_lengths[i], filler);
            out.append(f_dur);
            break;
        }
        }

        out.append(song_tag_sepatator);
    }

    return out;
}

int main(int argc, char *argv[])
{
    Cli cli;
    if (argc > 1)
    {
        cli.send_message(argc, argv);
        exit(0);
    }

    Library lib;
    Player plr;

    int fileddnum = 0;
    bool fileddedit = false;
    int sWidth, sHeight;
    Rectangle panelRec = {0, 0, 0, 0};
    Rectangle panelContentRec = {0, 0, 1000, 1000};
    Vector2 panelScroll = {0, 0};

    if (!lib.load_library())
    {
        cout << "library database not detected, creating one" << endl;
        string configmdirs[] = MUSIC_DIRECTORIES;
        for (string dir : configmdirs)
        {
            dir = regex_replace(dir, regex("\\%HOME%"), string(getenv("HOME")));
            lib.add_mdir(dir);
        }
        lib.populate();
    }
    else
        cout << "loaded existing library" << endl;

    int totalwidth = 0;
    for (int i : song_tag_lengths)
    {
        totalwidth += i;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow((totalwidth * 8) + 15, 450, "bmusp");
    GuiLoadStyle("assets/cyber.rgs");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // pre draw logic
        sWidth = GetScreenWidth();
        sHeight = GetScreenHeight();
        if (plr.is_song_over())
        {
            Song nSong = lib.get_song_at(lib.get_song_number(plr.song) + 1);
            plr.play(nSong);
        }

        // check for command
        if (fmod(floor(GetTime() * 60), FRAMES_BETWEEN_CMD_CHECK) == 0)
            cli.check_command(&plr, &lib);

        BeginDrawing();
        ClearBackground(BLACK);
        GuiSetStyle(BUTTON, BORDER_WIDTH, 1);

        if (fileddedit)
            GuiLock();

        // set scrollbar variables
        panelRec = {0, HEADER_HEIGHT, (float)sWidth, (float)sHeight - HEADER_HEIGHT};
        panelContentRec = {0, HEADER_HEIGHT, (float)sWidth - 12, (float)(lib.songlist.size()) * (SONG_HEIGHT + 1)};
        Rectangle view = GuiScrollPanel(panelRec, panelContentRec, &panelScroll);

        // play pause button
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
        if (GuiButton({65, 0, 15, HEADER_HEIGHT}, plr.is_playing() ? "#132#" : "#131#"))
        {
            if (plr.is_playing())
            {
                plr.pause();
                // cout << "pausing" << endl;
            }
            else
            {
                plr.unpause();
                // cout << "resuming" << endl;
            }
        }

        // previous button
        if (GuiButton({50, 0, 15, HEADER_HEIGHT}, "#129#"))
        {
            if (plr.is_playing())
            {
                Song pSong = lib.get_song_at(lib.get_song_number(plr.song) - 1);
                plr.play(pSong);
            }
        }

        // next button
        if (GuiButton({80, 0, 15, HEADER_HEIGHT}, "#134#"))
        {
            if (plr.is_playing())
            {
                Song nSong = lib.get_song_at(lib.get_song_number(plr.song) + 1);
                plr.play(nSong);
            }
        }

        // current song progress bar
        string pbTime = sec_to_minsec(plr.current_position()) + "/" + sec_to_minsec(plr.song.duration);
        GuiSetStyle(PROGRESSBAR, TEXT_PADDING, 3);
        GuiSetStyle(SLIDER, SLIDER_RIGHT_TEXT_CONTAINED, 1);
        GuiSetStyle(SLIDER, SLIDER_LEFT_TEXT_CONTAINED, 1);
        if (plr.is_playing())
        {
            string pbSong = "";
            pbSong.append(to_string(plr.song.trackNum) + ". ");
            pbSong.append(plr.song.name);
            float pos = GuiSliderPro({95, 0, (float)sWidth - 95 - VOLUME_SLIDER_WIDTH, HEADER_HEIGHT}, pbSong.c_str(),
                                     pbTime.c_str(), plr.current_position(), 0, plr.song.duration, 10);
            plr.jump_to(pos);
        }
        else
        {
            string status = plr.paused ? "Paused" : "Stopped";
            GuiSliderPro({95, 0, (float)sWidth - 95 - VOLUME_SLIDER_WIDTH, HEADER_HEIGHT}, status.c_str(),
                         "0:00/0:00", 1, 0, 2, 10);
        }

        // volume slider
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
        float vol = GuiSliderPro({(float)sWidth - VOLUME_SLIDER_WIDTH, 0, VOLUME_SLIDER_WIDTH, HEADER_HEIGHT}, "",
                                 to_string(plr.get_volume()).c_str(), plr.get_volume(), 0, 100, 5);
        GuiLabel({(float)sWidth - VOLUME_SLIDER_WIDTH, 0, VOLUME_SLIDER_WIDTH, HEADER_HEIGHT}, "#122#");
        plr.set_volume(vol);

        // draw song list
        BeginScissorMode(0, HEADER_HEIGHT, sWidth, sHeight - HEADER_HEIGHT);
        GuiGrid((Rectangle){panelRec.x + panelScroll.x, panelRec.y + panelScroll.y, panelContentRec.width, panelContentRec.height}, 16, 3);
        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 12);

        int songnum = 0;
        for (Song s : lib.songlist)
        {
            if (songnum * (SONG_HEIGHT + 1) + panelScroll.y < view.x + view.height + SONG_HEIGHT && songnum * (SONG_HEIGHT + 1) + panelScroll.y + HEADER_HEIGHT > view.x)
                if (GuiButton({0, (float)songnum * (SONG_HEIGHT + 1) + panelScroll.y + HEADER_HEIGHT, (float)sWidth - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH), SONG_HEIGHT}, get_song_button_name(s).c_str()))
                {
                    // cout << "playing " << s.name << endl;
                    if (GetMouseY() > HEADER_HEIGHT)
                    {
                        plr.play(s);
                    }
                }
            songnum++;
        }
        EndScissorMode();

        // file dropdown
        Vector2 mouse = GetMousePosition();
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
        GuiUnlock();
        if (GuiDropdownBox((Rectangle){0, 0, 50, HEADER_HEIGHT}, "File;Add Song;Add Folder;Empty Songlist;Quit", &fileddnum, fileddedit))
        {
            fileddedit = !fileddedit;
            switch (fileddnum)
            {
            case 1:
            {
                const char *f[256];
                for (int i = 0; i < lib.filters.size(); i++)
                {
                    f[i] = lib.filters[i].c_str();
                }
                auto mpath = tinyfd_openFileDialog("select music files", NULL, lib.filters.size(), &f[0], "", 1);
                lib.add_mdir(mpath);
                lib.populate();
                break;
            }
            case 2:
            {
                auto mpath = tinyfd_selectFolderDialog("select song folder", "");
                if (mpath != NULL)
                {
                    cout << "adding music folder: " << mpath << endl;
                    lib.add_mdir(mpath);
                    lib.populate();
                }
                break;
            }
            case 3:
                lib.clear_mdir();
                lib.populate();
                break;
            case 4:
                exit(0);
            }
            fileddnum = 0;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}