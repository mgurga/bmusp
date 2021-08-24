
#include <string>
#include <list>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/complex.hpp>

#ifndef COMMON_H
#define COMMON_H
#include "common.h"
#endif

#ifndef CONFIG_H
#define CONFIG_H
#include "../config.h"
#endif

#include "importer.cpp"

using namespace std;

class Library
{
public:
    list<list<Song>> playlists;
    const vector<string> filters = {"*.mp3", "*.flac", "*.m4a", "*.ogg", "*.wav"};

    bool load_library()
    {
        Library newlib;
        {
            ifstream ifs(string(CONFIG_LOCATION) + "library");
            if (ifs.good())
            {
                cereal::BinaryInputArchive ia(ifs);
                ia >> newlib;
            }
            else
                return false;
        }
        playlists = newlib.playlists;
        mdirs = newlib.mdirs;
        return true;
    }

    void populate(int plnum, bool clearplaylists = false)
    {
        cout << "populating" << endl;
        totaladditions = 0;
        if (clearplaylists)
        {
            playlists.clear();
            for (int i = 0; i < NUM_OF_PLAYLISTS; i++)
            {
                list<Song> ls;
                playlists.push_front(ls);
            }
        }
        cout << "total music directories: " << mdirs.size() << endl;
        for (auto const &path : mdirs)
        {
            const filesystem::path file(path);
            if (filesystem::is_regular_file(path))
            {
                cout << "added single file" << path << endl;
                add_regular_file(plnum, path);
            }
            else
            {
                cout << "starting recursive search on: " << path << endl;
                populate_rec(plnum, path, 0);
            }
        }
        // cout << "added " << totaladditions << " songs to songlist" << endl;
        cout << "songlist has a total of " << get_playlist_songs(0)->size() << " songs" << endl;
        get_playlist_songs(plnum)->sort(song_compare);
        save_library();
    }

    void quick_add_file_folder(int plnum, string path)
    {
        const filesystem::path file(path);
        if (filesystem::is_regular_file(path))
        {
            cout << "added single file" << path << endl;
            add_regular_file(plnum, path);
        }
        else
        {
            cout << "starting recursive search on: " << path << endl;
            populate_rec(plnum, path, 0);
        }
        get_playlist_songs(plnum)->sort(song_compare);
        save_library();
    }

    void save_library()
    {
        ofstream ofs;
        ofs.open(string(CONFIG_LOCATION) + "library");
        {
            cereal::BinaryOutputArchive archive(ofs);
            archive(*this);
        }
        ofs.close();
    }

    void add_mdir(string nmdir)
    {
        for (const string &i : mdirs)
            if (nmdir == i)
                return;
        mdirs.push_back(nmdir);
    }

    void clear_mdir()
    {
        mdirs.clear();
    }

    vector<string> get_playlist_filter()
    {
        return Importer().playlistfilters;
    }

    list<Song> search_query(int plnum, string query)
    {
        list<Song> pls = *get_playlist_songs(plnum);
        list<Song> out;
        for (const Song &s : pls) {
            if (to_lower(s.name).find(to_lower(query)) != string::npos ||
                to_lower(s.album).find(to_lower(query)) != string::npos ||
                to_lower(s.artist).find(to_lower(query)) != string::npos ||
                to_string(s.trackNum).find(to_lower(query)) != string::npos )
                out.push_front(s);
        }
        out.sort(song_compare);
        return out;
    }

    int get_song_number(int plnum, Song &s)
    {
        list<Song>::iterator it = get_playlist_songs(plnum)->begin();
        for (int i = 0; i < get_playlist_songs(plnum)->size(); i++)
        {
            if (s == *it)
                return i;
            std::advance(it, 1);
        }
        return -1;
    }

    Song get_song_at(int plnum, int n)
    {
        list<Song>::iterator it = get_playlist_songs(plnum)->begin();
        advance(it, n);
        return *it;
    }

    void add_song_to_playlist(int plnum, Song s)
    {
        list<list<Song>>::iterator plit = playlists.begin();
        advance(plit, plnum);
        plit->push_front(s);
        plit->sort(song_compare);
        save_library();
    }

    void remove_song_from_playlist(int plnum, Song s){
        list<list<Song>>::iterator plit = get_playlist_songs(plnum);
        plit->remove(s);
        save_library();
    }

    list<list<Song>>::iterator get_playlist_songs(int plnum)
    {
        list<list<Song>>::iterator plit = playlists.begin();
        advance(plit, plnum);
        return plit;
    }

    void import_playlist(int plnum, string path)
    {
        Importer imp;
        list<Song> npl = imp.import_playlist(path);
        cout << "importing " << npl.size() << " songs" << endl;
        if(npl.size() != 0)
            for(const Song &ns : npl)
                add_song_to_playlist(plnum, ns);

    }

private:
    friend class cereal::access;
    int totaladditions;
    list<string> mdirs; // music directories and single song files

    string to_lower(string s)
    {
        for (auto& c : s)
        {
            c = std::tolower(c);
        }
        return s;
    }

    static bool song_compare(const Song &s1, const Song &s2)
    {
        list<SongTag> sortorder = SONG_SORT_ORDER; // how to sort songlist

        for (SongTag t : sortorder)
        {
            switch (t)
            {
            case NAME:
                if (s2.name.compare(s1.name) > 0)
                    return true;
                else if (s2.name.compare(s1.name) < 0)
                    return false;
                break;
            case ALBUM:
                if (s2.album.compare(s1.album) > 0)
                    return true;
                else if (s2.album.compare(s1.album) < 0)
                    return false;
                break;
            case ARTIST:
                if (s2.artist.compare(s1.artist) > 0)
                    return true;
                else if (s2.artist.compare(s1.artist) < 0)
                    return false;
                break;
            case TRACKNUM:
                if (s2.trackNum > s1.trackNum)
                    return true;
                else if (s2.trackNum < s1.trackNum)
                    return false;
                break;
            case DURATION: case STATUS_ICON:
                break;
            }
        }

        return true;
    }

    void populate_rec(int plnum, string path, int depth)
    {
        if (depth > 5)
            return;
        cout << "searching path: " << path << endl;

        for (const auto &entry : filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                populate_rec(plnum, entry.path(), depth + 1);
            }
            else if (entry.is_regular_file())
            {
                add_regular_file(plnum, entry.path());
            }
        }
    }

    void add_regular_file(int plnum, string path)
    {
        auto entry = filesystem::directory_entry(path);
        string ext = entry.path().filename().extension();

        bool supported = false;
        for (const string &i : filters)
            if (ext == i.substr(1))
                supported = true;

        if (supported)
        {
            Song ns;
            ns.path = entry.path();
            ns.plays = 0;

            TagLib::FileRef f(entry.path().c_str());
            TagLib::AudioProperties *properties = f.audioProperties();

            if (!f.isNull() && f.tag())
            {
                TagLib::Tag *tag = f.tag();
                ns.trackNum = tag->track();
                ns.name = tag->title().toCString();
                ns.album = tag->album().toCString();
                ns.artist = tag->artist().toCString();
                ns.duration = properties->length();
            }
            add_song(plnum, ns);
            totaladditions++;
        }
    }

    void add_song(int plnum, Song nsong)
    {
        for (const Song &s : *get_playlist_songs(plnum))
            if (nsong.path == s.path)
                return;
        get_playlist_songs(plnum)->push_back(nsong);
    }

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(playlists, mdirs);
    }
};
