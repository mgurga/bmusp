
#include <string>
#include <list>
#include <filesystem>
#include <iostream>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

#ifndef COMMON_H
#define COMMON_H
#include "common.h"
#endif

using namespace std;

class Library
{
public:
    list<Song> songlist; // generated from music directories
    list<string> mdirs;  // music directories and single song files
    const vector<string> filters = {"*.mp3", "*.flac", "*.m4a", "*.ogg", "*.wav"};

    void populate()
    {
        totaladditions = 0;
        songlist.clear();
        cout << "total music directories: " << mdirs.size() << endl;
        for (auto const path : mdirs)
        {
            const filesystem::path file(path);
            if (filesystem::is_regular_file(path))
            {
                cout << "added single file" << path << endl;
                add_regular_file(path);
            }
            else
            {
                cout << "starting recursive search on: " << path << endl;
                populate_rec(path, 0);
            }
        }
        // cout << "added " << totaladditions << " songs to songlist" << endl;
        cout << "songlist has a total of " << songlist.size() << " songs" << endl;
        songlist.sort(song_compare);
    }

    int get_song_number(Song &s)
    {
        list<Song>::iterator it = songlist.begin();
        for (int i = 0; i < songlist.size(); i++)
        {
            if (s == *it)
                return i;
            std::advance(it, 1);
        }
    }

    Song get_song_at(int n)
    {
        list<Song>::iterator it = songlist.begin();
        advance(it, n);
        return *it;
    }

private:
    int totaladditions;

    static bool song_compare(const Song &s1, const Song &s2)
    {
        list<SongTag> sortorder = {ARTIST, ALBUM, TRACKNUM}; // how to sort songlist

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
            }
        }

        return true;
    }

    void populate_rec(string path, int depth)
    {
        if (depth > 5)
            return;
        cout << "searching path: " << path << endl;

        for (const auto &entry : filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                populate_rec(entry.path(), depth + 1);
            }
            else if (entry.is_regular_file())
            {
                add_regular_file(entry.path());
            }
        }
    }

    void add_regular_file(string path)
    {
        auto entry = filesystem::directory_entry(path);
        string ext = entry.path().filename().extension();

        bool supported = false;
        for (string i : filters)
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
            songlist.push_back(ns);
            totaladditions++;
        }
    }
};