
#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include <list>
#include <regex>
#include <random>
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

string get_song_button_name(Song* s, Player *plr)
{
    string out;
    char filler = SONG_TAG_FILLER_CHAR;

    for (int i = 0; i < sizeof(song_tag_lengths) / sizeof(song_tag_lengths[0]); i++)
    {
        switch (song_tag_order[i])
        {
        case ARTIST:
        {
            string f_artist = s->artist;
            f_artist.resize(song_tag_lengths[i], filler);
            out.append(f_artist);
            break;
        }
        case NAME:
        {
            string f_name = s->name;
            f_name.resize(song_tag_lengths[i], filler);
            out.append(f_name);
            break;
        }
        case ALBUM:
        {
            string f_album = s->album;
            f_album.resize(song_tag_lengths[i], filler);
            out.append(f_album);
            break;
        }
        case TRACKNUM:
        {
            string f_tnum = to_string(s->trackNum);
            f_tnum.resize(song_tag_lengths[i], filler);
            out.append(f_tnum);
            break;
        }
        case DURATION:
        {
            string f_dur = sec_to_minsec(s->duration);
            f_dur.resize(song_tag_lengths[i], filler);
            out.append(f_dur);
            break;
        }
        case STATUS_ICON:
        {
            int sq = plr->get_song_queue_num(s);
            string f_si;
            if (plr->song == *s)
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
    int scroll = 0;
    int scrollpos = 0;
    bool searchopen = false;
    char searchtext[64];
    Song jumptosong;
    default_random_engine generator;
    bool renameopen = false;
    char renametext[64];
    int renamepl = 0;
    bool canclicksong = true;

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
    totalwidth += sizeof(song_tag_lengths) / sizeof(song_tag_lengths[0]) - 1;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(((totalwidth - 2) * 8), 450, "bmusp");
    GuiLoadStyle("assets/cyber.rgs");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // pre draw logic
        sWidth = GetScreenWidth();
        sHeight = GetScreenHeight();

        // song end action
        if (plr.is_song_over())
        {
            switch(lib.get_playlist(playlist)->end_action)
            {
            case REPEAT:
                plr.play(&plr.song);
                break;
            case STOP:
                plr.stop();
                break;
            case NEXT:
            {
                int songIndex = lib.get_song_number(playlist, &plr.song) + 1;
                if (lib.get_playlist_songs(playlist)->size()  == songIndex)
                    songIndex = 0;
                Song* nSong = lib.get_song_at(playlist, songIndex);
                plr.play(nSong);
                break;
            }
            case RANDOM:
            {
                uniform_int_distribution<int> distribution(0,lib.get_playlist_songs(playlist)->size() - 1);
                int rnd = distribution(generator);
                Song* nSong = lib.get_song_at(playlist, rnd);
                plr.play(nSong);
                break;
            }
            }
        }

        // check for command
        if (fmod(floor(GetTime() * 60), FRAMES_BETWEEN_CMD_CHECK) == 0)
            cli.check_command(&plr, &lib, &playlist, &lib.get_playlist(playlist)->end_action, &generator);

        BeginDrawing();
        ClearBackground(BLACK);
        GuiSetStyle(BUTTON, BORDER_WIDTH, 1);

        if (fileddedit)
            GuiLock();

        // draw song list
        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 12);

        int songnum = 0;
        for (Song s : (searchopen ? lib.search_query(playlist, searchtext) : *lib.get_playlist_songs(playlist)))
        {
            if (songnum * (SONG_HEIGHT + 1) + scroll < sHeight && songnum * (SONG_HEIGHT + 1) + scroll + HEADER_HEIGHT > 0)
            {
                Rectangle sbtn = {0, (float)songnum * (SONG_HEIGHT + 1) + HEADER_HEIGHT + scroll, (float)sWidth - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH), SONG_HEIGHT};
                if (s == plr.song)
                    GuiSetStyle(BUTTON, BASE, 0x111155ff);
                else if ((multistartsong <= songnum && multiendsong >= songnum && multiss) || (s == selectedsong && songoptionsopen))
                    GuiSetStyle(BUTTON, BASE, 0x552200ff);
                else
                    GuiSetStyle(BUTTON, BASE, 0x000000ff);
                if (GuiButton(sbtn, get_song_button_name(&s, &plr).c_str()))
                {
                    if (GetMouseY() > HEADER_HEIGHT && !songoptionsopen && !multiss && canclicksong)
                    {
                        plr.clear_queue();
                        plr.stop();
                        plr.add_to_queue(&s);
                    }
                }
                // song right click
                if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), sbtn) && multistartsong == -1 && canclicksong)
                {
                    songoptionsopen = true;
                    selectedsong = s;
                    songoptiony = sbtn.y + SONG_HEIGHT;
                    multistartsong = songnum;
                }
                else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), sbtn) && !multiss && songoptionsopen && canclicksong)
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

        // scrollbar stuff
        scrollpos = GuiScrollBar({(float)sWidth - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH), HEADER_HEIGHT, (float)GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH), (float)sHeight - HEADER_HEIGHT}, -scroll, 0, songnum * (SONG_HEIGHT + 1) - (SONG_HEIGHT * 10));
        scroll = -scrollpos;

        if (GetMouseY() > HEADER_HEIGHT && -scroll < songnum * (SONG_HEIGHT + 1) - (SONG_HEIGHT * 10))
            scroll += GetMouseWheelMove() * 50;
        if (scroll > 0)
            scroll = 0;

        // jump to currently playing song
        if (IsKeyPressed(KEY_J))
        {
            Song playing = plr.song;
            int spos = lib.get_song_number(playlist, &playing);
            scroll = -(spos * (SONG_HEIGHT + 1));
        }

        // cout << "multiss: " << (multiss ? "true" : "false") << " songoptionsopen: " << (songoptionsopen ? "true" : "false") << endl;
        // play pause button
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
        if (GuiButton({65, 0, 15, HEADER_HEIGHT}, plr.is_playing() ? "#132#" : "#131#") || (IsKeyPressed(KEY_SPACE)))
            plr.toggle_pause();

        // previous button
        if (GuiButton({50, 0, 15, HEADER_HEIGHT}, "#129#") || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_LEFT)))
        {
            if (plr.is_playing())
            {
                Song* pSong = lib.get_song_at(playlist, lib.get_song_number(playlist, &plr.song) - 1);
                plr.clear_queue();
                plr.stop();
                plr.add_to_queue(pSong);
            }
        }

        // next button
        if (GuiButton({80, 0, 15, HEADER_HEIGHT}, "#134#") || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_RIGHT)))
        {
            if (plr.is_playing())
            {
                if (plr.has_queue())
                {
                    plr.jump_to(plr.song.duration);
                    plr.is_song_over(); // force check if song over to quickly go to next song
                }
                else
                    plr.jump_to(plr.song.duration);
            }
        }

        // end action toggle
        if(GuiButton({95, 0, 15, HEADER_HEIGHT}, string("#" + to_string(lib.get_playlist(playlist)->end_action) + "#").c_str()))
            lib.get_playlist(playlist)->end_action++;

        // current song progress bar
        if (IsKeyPressed(KEY_RIGHT))
            plr.jump_to(plr.current_position() + 5);
        if (IsKeyPressed(KEY_LEFT))
            plr.jump_to(plr.current_position() - 5);
        string pbTime = sec_to_minsec(plr.current_position()) + "/" + sec_to_minsec(plr.song.duration);
        GuiSetStyle(PROGRESSBAR, TEXT_PADDING, 3);
        GuiSetStyle(SLIDER, SLIDER_RIGHT_TEXT_CONTAINED, 1);
        GuiSetStyle(SLIDER, SLIDER_LEFT_TEXT_CONTAINED, 1);
        if (!searchopen) {
            if (!(plr.song.name == ""))
            {
                string pbSong = "";
                pbSong.append(to_string(plr.song.trackNum) + ". ");
                pbSong.append(plr.song.name);
                float pos = GuiSliderPro({110, 0, (float)sWidth - 128 - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH, HEADER_HEIGHT}, pbSong.c_str(),
                                         pbTime.c_str(), plr.current_position(), 0, plr.song.duration, 10);
                plr.jump_to(pos);
            }
            else
            {
                GuiSliderPro({110, 0, (float)sWidth - 128 - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH, HEADER_HEIGHT}, "Stopped",
                             "0:00/0:00", 1, 0, 2, 10);
            }
        }

        // volume slider
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_EQUAL))
            plr.set_volume(plr.get_volume() + 10);
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_MINUS))
            plr.set_volume(plr.get_volume() - 10);
        if (GetMouseY() < HEADER_HEIGHT && GetMouseX() > sWidth - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH - 18 && GetMouseWheelMove() > 0)
            plr.set_volume(plr.get_volume() + 5);
        if (GetMouseY() < HEADER_HEIGHT && GetMouseX() > sWidth - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH - 18 && GetMouseWheelMove() < 0)
            plr.set_volume(plr.get_volume() - 5);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
        float vol = GuiSliderPro({(float)sWidth - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH - 18, 0, VOLUME_SLIDER_WIDTH, HEADER_HEIGHT}, "",
                                 to_string(plr.get_volume()).c_str(), plr.get_volume(), 0, 100, 5);
        GuiLabel({(float)sWidth - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH - 18, 0, VOLUME_SLIDER_WIDTH, HEADER_HEIGHT}, "#122#");
        plr.set_volume(vol);

        // search button
        if (GuiButton({(float)sWidth - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH - 18, 0, 18, HEADER_HEIGHT}, "#42#") || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F)))
        {
            searchopen = !searchopen;
            strcpy(searchtext, "");
        }

        if (searchopen)
        {
            bool tbenter = GuiTextBox({95, 0, (float)sWidth - 113 - VOLUME_SLIDER_WIDTH - NUM_OF_PLAYLISTS * PLAYLIST_TAB_WIDTH, HEADER_HEIGHT}, searchtext, 64, true);
            if (tbenter && !IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                searchopen = false;
        }

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
                lib.get_playlist(playlist)->scroll = scroll;
                playlist = i;
                scroll = lib.get_playlist(playlist)->scroll;
                lib.save_library();
            }
            if (sWidth - ((NUM_OF_PLAYLISTS - i) * PLAYLIST_TAB_WIDTH) < GetMouseX() && sWidth - ((NUM_OF_PLAYLISTS - i) * PLAYLIST_TAB_WIDTH) + PLAYLIST_TAB_WIDTH > GetMouseX() && GetMouseY() < HEADER_HEIGHT)
            {
                string plname = lib.get_playlist(i)->name;
                DrawRectangle(GetMouseX() - (plname.size()*13), HEADER_HEIGHT, plname.size()*13, HEADER_HEIGHT, BLACK);
                GuiTextBox({(float)GetMouseX() - (plname.size()*13), (float)HEADER_HEIGHT, (float)plname.size()*13, HEADER_HEIGHT}, plname.data(), 13, false);
            }
            if ((sWidth - ((NUM_OF_PLAYLISTS - i) * PLAYLIST_TAB_WIDTH) < GetMouseX() && sWidth - ((NUM_OF_PLAYLISTS - i) * PLAYLIST_TAB_WIDTH) + PLAYLIST_TAB_WIDTH > GetMouseX() && GetMouseY() < HEADER_HEIGHT && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)))
            {
                renamepl = i;
                renameopen = true;
                canclicksong = false;
                strcpy(renametext, "");
            }
            if(renameopen) {
                int btnnum = GuiTextInputBox({(float)sWidth/2 - 100, (float)sHeight/2 - 50, 200, 100}, "rename playlist", "", "ok", renametext);
                cout << renametext << endl;
                if(btnnum == 1) {
                    lib.get_playlist(renamepl)->name = string(renametext);
                    lib.save_library();
                    renameopen = false;
                    canclicksong = true;
                } else if(btnnum == 0) {
                    renameopen = false;
                    canclicksong = true;
                }
            }
        }

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

            list<Song> multiselsongs;
            int si = 0;
            if (multiss)
                for (const Song &s : *lib.get_playlist_songs(playlist))
                {
                    if (si >= multistartsong && si <= multiendsong)
                        multiselsongs.push_back(s);
                    si++;
                }

            switch (selectedoption)
            {
            case EXIT: case NONE:
                break;
            case ADD_TO_QUEUE:
                if (multiss)
                    for (int i = multistartsong; i <= multiendsong; i++)
                        plr.add_to_queue(lib.get_song_at(playlist, i));
                else
                    plr.add_to_queue(&selectedsong);
                break;
            case ADD_TO_PLAYLIST:
                if (multiss)
                    for (const Song &s : multiselsongs)
                        lib.add_song_to_playlist(playlistbtn, s);
                else
                    lib.add_song_to_playlist(playlistbtn, selectedsong);
                break;
            case REMOVE:
                if (multiss)
                {
                    for (const Song &s : multiselsongs)
                        lib.remove_song_from_playlist(playlist, s);
                }
                else
                    lib.remove_song_from_playlist(playlist, selectedsong);
                break;
            }
            if (selectedoption != NONE)
                songoptionsopen = false, multiss = false, multistartsong = -1, multiendsong = -1;
        }

        // file dropdown
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
        GuiUnlock();
        if (GuiDropdownBox((Rectangle){0, 0, 50, HEADER_HEIGHT}, "File;Add Song;Add Folder;Import Playlist;Empty Songlist;Quit", &fileddnum, fileddedit))
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
                if (mpath != NULL)
                    lib.quick_add_file_folder(playlist, mpath);
                break;
            }
            case 2:
            {
                auto mpath = tinyfd_selectFolderDialog("select song folder", "");
                if (mpath != NULL)
                    lib.quick_add_file_folder(playlist, mpath);
                break;
            }
            case 3:
            {
                const char *f[256];
                for (int i = 0; i < lib.get_playlist_filter().size(); i++)
                {
                    f[i] = lib.get_playlist_filter()[i].c_str();
                }
                auto mpath = tinyfd_openFileDialog("select playlist file", NULL, lib.get_playlist_filter().size(), &f[0], "", 1);
                lib.import_playlist(playlist, mpath);
                break;
            }
            case 4:
                if (tinyfd_messageBox("clear song list?", "are you sure you want to clear all playlists?", "yesno", "warning", 0) == 1)
                {
                    lib.clear_mdir();
                    lib.populate(playlist, true);
                }
                break;
            case 5:
                exit(0);
            }
            fileddnum = 0;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
