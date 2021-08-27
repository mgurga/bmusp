
#include <iostream>
#include <ostream>

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
    DURATION,
    STATUS_ICON // should not be used as a song tag, only used for tab info
};

enum SongOption
{
    NONE,
    EXIT,
    ADD_TO_QUEUE,
    ADD_TO_PLAYLIST,
    REMOVE
};

enum EndAction
{
    NEXT = 115,
    REPEAT = 61,
    RANDOM = 193,
    STOP = 113
};

inline EndAction& operator++(EndAction& ea, int)
{
    switch(ea)
    {
    case EndAction::NEXT: return ea = EndAction::REPEAT;
    case EndAction::REPEAT: return ea = EndAction::RANDOM;
    case EndAction::RANDOM: return ea = EndAction::STOP;
    case EndAction::STOP: return ea = EndAction::NEXT;
    }
}
