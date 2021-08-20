
#include <string>
#include <list>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#ifndef COMMON_H
#define COMMON_H
#include "common.h"
#endif

using namespace std;

class Importer
{
public:
    const vector<string> playlistfilters = {"*.dbpl"};

    list<Song> import_playlist(string path)
    {
        cout << "found file at path: " << path << endl;

        auto entry = filesystem::directory_entry(path);
        string ext = entry.path().filename().extension();

        cout << "loaded playlist file, got ext: " << ext << endl;

        if (ext == ".dbpl")
            return import_dbpl(path);
    }

private:
    list<Song> import_dbpl(string path)
    {
        cout << "loading dbpl" << endl;
        FILE *f = fopen(path.c_str(), "rb");
        list<Song> out;

        // check magic value
        char magicval[4];
        if (fread(magicval, 1, 4, f) != 4)
        {
            cout << "failed to read magic value" << endl;
            goto quit;
        }
        if (!strncmp(magicval, "DBPL", 4))
            cout << "found magic value" << endl;
        else
        {
            cout << "did not find correct magic value, got: '" << magicval << "'" << endl;
            return out;
        }

        // get file versions
        uint8_t majorver;
        uint8_t minorver;
        fread(&majorver, 1, 1, f);
        fread(&minorver, 1, 1, f);
        printf("major version: %d minor version: %d\n", majorver, minorver);

        // get track count
        uint32_t trackcount;
        if (fread(&trackcount, 1, 4, f) != 4)
            cout << "error reading trackcount" << endl;
        cout << "total track count: " << trackcount << endl;

        cout << "----------------------" << endl;

        // start getting tracks
        for (int i = 0; i < (int)trackcount; i++)
        {
            Song nsong;

            uint16_t urilen;
            fread(&urilen, 1, 2, f);
            // cout << "uri len: " << urilen << endl;

            char uri[(int)urilen];
            fread(&uri, 1, urilen, f);
            cout << "uri: " << uri << endl;

            uint8_t declen;
            fread(&declen, 1, 1, f);
            // cout << "dec len: " << (int)declen << endl;

            char decoder[20] = "";
            fread(&decoder, 1, declen, f);
            // cout << "decoder: " << decoder << endl;

            int16_t tracknum;
            fread(&tracknum, 1, 2, f);
            // cout << "tracknum: " << tracknum << endl;

            int32_t startsample, endsample;
            fread(&startsample, 1, 4, f);
            fread(&endsample, 1, 4, f);
            // cout << "start sample: " << (int)startsample << " end sample: " << (int)endsample << endl;

            float duration;
            fread(&duration, 1, 4, f);
            cout << "duration: " << duration << endl;
            nsong.duration = (int)duration;

            uint8_t filetypelen;
            fread(&filetypelen, 1, 1, f);
            // cout << "filetype len: " << (int)filetypelen << endl;

            if (filetypelen)
            {
                char filetype[filetypelen + 1];
                fread(&filetype, 1, filetypelen, f);
                filetype[filetypelen] = 0;
                cout << "filetype: " << filetype << endl;
            }

            // ReplayGain values
            float ag, ap, tg, tp;
            fread(&ag, 1, 4, f);
            fread(&ap, 1, 4, f);
            fread(&tg, 1, 4, f);
            fread(&tp, 1, 4, f);

            if (minorver >= 2)
            {
                uint32_t flags;
                fread(&flags, 1, 4, f);
            }

            // song meta tags
            int16_t meta_count;
            fread(&meta_count, 1, 2, f);
            cout << "get meta count of: " << meta_count << endl;
            for (int m = 0; m < meta_count; m++)
            {
                uint16_t valuelen;
                fread(&valuelen, 1, 2, f);

                char key[valuelen + 1];
                fread(&key, 1, valuelen, f);
                key[valuelen] = 0;

                uint16_t keyvallen;
                fread(&keyvallen, 1, 2, f);
                if ((int)keyvallen >= 20000)
                    fseek(f, keyvallen, SEEK_CUR);
                else
                {
                    char keyval[keyvallen + 1];
                    fread(&keyval, 1, (int)keyvallen, f);
                    // cout << "got meta key: " << key << "->" << keyval << endl;
                    keyval[keyvallen] = 0;
                    string strkey(key);
                    string strkeyval(keyval);
                    if (strkey.find(":URI") != string::npos)
                        nsong.path = strkeyval;
                    else if (strkey.find("title") != string::npos)
                        nsong.name = strkeyval;
                    else if (strkey.find("artist") != string::npos)
                        nsong.artist = strkeyval;
                    else if (strkey.find("track") != string::npos)
                        nsong.trackNum = stoi(strkeyval);
                    else if (strkey.find("album") != string::npos)
                        nsong.album = strkeyval;
                }
            }

            out.push_back(nsong);

            cout << "-----------------------------" << endl;
        }

        int16_t plmetacount;
        fread(&plmetacount, 1, 2, f);
        for (int p = 0; p < (int)plmetacount; p++)
        {
            int16_t keylen;
            fread(&keylen, 1, 2, f);

            char key[keylen + 1];
            fread(&key, 1, (int)keylen, f);
            key[keylen] = 0;

            int16_t vallen;
            fread(&vallen, 1, 2, f);
            if ((int)vallen < 0 || (int)vallen >= 20000)
                fseek(f, (int)vallen, SEEK_CUR);
            else
            {
                char val[(int)vallen];
                fread(&val, 1, (int)vallen, f);
                cout << "got playlist meta key: " << key << "->" << val << endl;
            }
        }

    quit:
        return out;
    }
};