
#include <iostream>

struct Song
{
    std::string path;
    int plays;

    // gotten from track metadata
    std::string name;
    int duration; // in seconds
    int trackNum;
    std::string album;
    std::string artist;

    bool operator==(const Song s)
    {
        return (s.name == name &&
                s.path == path &&
                s.artist == artist &&
                s.duration == duration &&
                s.trackNum == trackNum &&
                s.album == album);
    }

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(path, duration, trackNum, album, artist, plays, name);
    }
};

enum SongTag
{
    ARTIST,
    ALBUM,
    TRACKNUM,
    NAME,
    DURATION
};