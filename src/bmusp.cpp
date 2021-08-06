
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
int totalwidth = 0;

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

string get_song_button_name(Song s, Player *plr)
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
        case STATUS_ICON:
        {
            int sq = plr->get_song_queue_num(s);
            string f_si;
            if (plr->song == s)
                f_si.append(">>");
            else if (sq != -1)
                f_si.append(">" + to_string(sq));
            else
                f_si.append("");
            f_si.resize(song_tag_lengths[i], filler);
            out.append(f_si);
        }
        }

        out.append(song_tag_sepatator);
    }

    return out.substr(0, out.length() - 1);
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
    bool songoptionsopen = false;
    Song selectedsong;
    int songoptiony;
    bool multiss = false;
    int multistartsong = -1;
    int multiendsong = -1;
    int sWidth, sHeight;
    int playlist = 0;
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
        lib.populate(playlist, true);
    }
    else
        cout << "loaded existing library" << endl;

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
            Song nSong = lib.get_song_at(playlist, lib.get_song_number(playlist, plr.song) + 1);
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
        panelContentRec = {0, HEADER_HEIGHT, (float)sWidth - 12, (float)(lib.get_playlist_songs(playlist)->size()) * (SONG_HEIGHT + 1)};
        Rectangle view = GuiScrollPanel(panelRec, panelContentRec, &panelScroll);

        // play pause button
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
        if (GuiButton({65, 0, 15, HEADER_HEIGHT}, plr.is_playing() ? "#132#" : "#131#"))
        {
            plr.toggle_pause();
        }

        // previous button
        if (GuiButton({50, 0, 15, HEADER_HEIGHT}, "#129#"))
        {
            if (plr.is_playing())
            {
                Song pSong = lib.get_song_at(playlist, lib.get_song_number(playlist, plr.song) - 1);
                plr.clear_queue();
                plr.stop();
                plr.add_to_queue(pSong);
            }
        }

        // next button
        if (GuiButton({80, 0, 15, HEADER_HEIGHT}, "#134#"))
        {
            if (plr.is_playing())
            {
                if (plr.has_queue())
                {
                    plr.jump_to(plr.song.duration);
                    plr.is_song_over(); // force check if song over to quickly go to next song
                }
                else
                {
                    Song nSong = lib.get_song_at(playlist, lib.get_song_number(playlist, plr.song) + 1);
                    plr.clear_queue();
                    plr.stop();
                    plr.add_to_queue(nSong);
                }
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
            float pos = GuiSliderPro({95, 0, (float)sWidth - 95 - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH, HEADER_HEIGHT}, pbSong.c_str(),
                                     pbTime.c_str(), plr.current_position(), 0, plr.song.duration, 10);
            plr.jump_to(pos);
        }
        else
        {
            string status = plr.paused ? "Paused" : "Stopped";
            GuiSliderPro({95, 0, (float)sWidth - 95 - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH, HEADER_HEIGHT}, status.c_str(),
                         "0:00/0:00", 1, 0, 2, 10);
        }

        // volume slider
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
        float vol = GuiSliderPro({(float)sWidth - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH, 0, VOLUME_SLIDER_WIDTH, HEADER_HEIGHT}, "",
                                 to_string(plr.get_volume()).c_str(), plr.get_volume(), 0, 100, 5);
        GuiLabel({(float)sWidth - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH, 0, VOLUME_SLIDER_WIDTH, HEADER_HEIGHT}, "#122#");
        plr.set_volume(vol);

        // playlist tab buttons
        for (int i = 0; i < NUM_OF_PLAYLISTS; i++)
        {
            string pltabname;
            if (i == 0)
                pltabname = "A";
            else
                pltabname = to_string(i);
            if (GuiButton({(float)sWidth - ((NUM_OF_PLAYLISTS - i) * PLAYLIST_TAB_WIDTH), 0, PLAYLIST_TAB_WIDTH, HEADER_HEIGHT}, pltabname.c_str()))
            {
                playlist = i;
            }
        }

        // draw song list
        BeginScissorMode(0, HEADER_HEIGHT, sWidth, sHeight - HEADER_HEIGHT);
        GuiGrid((Rectangle){panelRec.x + panelScroll.x, panelRec.y + panelScroll.y, panelContentRec.width, panelContentRec.height}, 16, 3);
        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 12);

        int songnum = 0;
        for (Song s : *lib.get_playlist_songs(playlist))
        {
            if (songnum * (SONG_HEIGHT + 1) + panelScroll.y < view.x + view.height + SONG_HEIGHT && songnum * (SONG_HEIGHT + 1) + panelScroll.y + HEADER_HEIGHT > view.x)
            {
                Rectangle sbtn = {0, (float)songnum * (SONG_HEIGHT + 1) + panelScroll.y + HEADER_HEIGHT, (float)sWidth - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH), SONG_HEIGHT};
                if (s == plr.song)
                    GuiSetStyle(BUTTON, BASE, 0x111155ff);
                else if ((multistartsong <= songnum && multiendsong >= songnum && multiss) || (s == selectedsong && songoptionsopen))
                    GuiSetStyle(BUTTON, BASE, 0x552200ff);
                else
                    GuiSetStyle(BUTTON, BASE, 0x000000ff);
                if (GuiButton(sbtn, get_song_button_name(s, &plr).c_str()))
                {
                    if (GetMouseY() > HEADER_HEIGHT && !songoptionsopen && !multiss)
                    {
                        cout << "playing " << s.name << endl;
                        plr.clear_queue();
                        plr.stop();
                        plr.add_to_queue(s);
                    }
                }
                // song right click
                if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), sbtn) && multistartsong == -1)
                {
                    songoptionsopen = true;
                    selectedsong = s;
                    songoptiony = sbtn.y + SONG_HEIGHT;
                    multistartsong = songnum;
                }
                else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), sbtn) && !multiss && songoptionsopen)
                {
                    // second right click
                    multiss = true;
                    multiendsong = songnum;
                    songoptiony = sbtn.y + SONG_HEIGHT;
                }
            }
            songnum++;
        }
        GuiSetStyle(BUTTON, BASE, 0x000000);
        EndScissorMode();

        // cout << "multiss: " << (multiss ? "true" : "false") << " songoptionsopen: " << (songoptionsopen ? "true" : "false") << endl;

        // song options
        if (songoptionsopen)
        {
            // songoptiony = sHeight - SONG_HEIGHT * 2;
            SongOption selectedoption = NONE;
            int playlistbtn;

            GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
            GuiDrawRectangle({0, (float)songoptiony, (float)sWidth, SONG_HEIGHT * 2}, 1, BLUE, BLACK);
            if (GuiButton({0, (float)songoptiony, SONG_HEIGHT * 2, SONG_HEIGHT * 2}, "X"))
                selectedoption = EXIT;

            if (GuiButton({SONG_HEIGHT * 2, (float)songoptiony, 100, SONG_HEIGHT * 2}, "Add to queue"))
                selectedoption = ADD_TO_QUEUE;

            // playlist buttons
            for (int i = 0; i < NUM_OF_PLAYLISTS / 2; i++)
            {
                string plbtn = "";
                if (i == 0)
                    plbtn.append("A");
                else
                    plbtn.append(to_string(i));
                if (playlist != i)
                    if (GuiButton({(float)SONG_HEIGHT * 2 + 100 + (i * SONG_HEIGHT), (float)songoptiony, SONG_HEIGHT, SONG_HEIGHT}, plbtn.c_str()))
                    {
                        selectedoption = ADD_TO_PLAYLIST;
                        playlistbtn = i;
                    }
            }
            for (int i = NUM_OF_PLAYLISTS / 2; i < NUM_OF_PLAYLISTS; i++)
            {
                if (playlist != i)
                    if (GuiButton({(float)SONG_HEIGHT * 2 + 100 + ((i - (NUM_OF_PLAYLISTS / 2)) * SONG_HEIGHT), (float)songoptiony + SONG_HEIGHT, SONG_HEIGHT, SONG_HEIGHT}, to_string(i).c_str()))
                    {
                        selectedoption = ADD_TO_PLAYLIST;
                        playlistbtn = i;
                    }
            }

            // delete song button
            if (GuiButton({SONG_HEIGHT * 2 + 100 + (NUM_OF_PLAYLISTS / 2) * SONG_HEIGHT, (float)songoptiony, 60, SONG_HEIGHT * 2}, "Remove"))
            {
                selectedoption = REMOVE;
            }

            switch (selectedoption)
            {
            case EXIT:
                break;
            case ADD_TO_QUEUE:
                if (multiss)
                    for (int i = multistartsong; i <= multiendsong; i++)
                        plr.add_to_queue(lib.get_song_at(playlist, i));
                else
                    plr.add_to_queue(selectedsong);
                plr.play();
                break;
            case ADD_TO_PLAYLIST:
                if (multiss)
                    for (int i = multistartsong; i <= multiendsong; i++)
                        lib.add_song_to_playlist(playlistbtn, lib.get_song_at(playlist, i));
                else
                    lib.add_song_to_playlist(playlistbtn, selectedsong);
                break;
            case REMOVE:
                if (multiss)
                    for (int i = multistartsong; i <= multiendsong; i++)
                        lib.remove_song_from_playlist(playlist, lib.get_song_at(playlist, i));
                else
                    lib.remove_song_from_playlist(playlist, selectedsong);
                break;
            }
            if (selectedoption != NONE)
                songoptionsopen = false, multiss = false, multistartsong = -1, multiendsong = -1;
        }

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
                lib.populate(playlist);
                break;
            }
            case 2:
            {
                auto mpath = tinyfd_selectFolderDialog("select song folder", "");
                if (mpath != NULL)
                {
                    cout << "adding music folder: " << mpath << endl;
                    lib.add_mdir(mpath);
                    lib.populate(playlist);
                }
                break;
            }
            case 3:
                if (tinyfd_messageBox("clear song list?", "are you sure you want to clear all playlists?", "yesno", "warning", 0) == 1)
                {
                    lib.clear_mdir();
                    lib.populate(playlist, true);
                }
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