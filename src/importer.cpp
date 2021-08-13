
#include <string>
#include <list>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>

#ifndef COMMON_H
#define COMMON_H
#include "common.h"
#endif

using namespace std;

class Importer
{
public:
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
        ifstream in(path, ifstream::binary);
        list<Song> out;

        // check magic value
        char magicval[4];
        in.get(magicval, 5);
        if (strcmp(magicval, "DBPL") == 0)
            cout << "found magic value" << endl;
        else
        {
            cout << "did not find correct magic value, got: " << magicval << endl;
            return out;
        }

        // get file versions
        char majorver[1], minorver[1];
        in.get(majorver, 2);
        in.get(minorver, 2);
        cout << "got major version: " << (unsigned)majorver[0] << " and minor version: " << (unsigned)minorver[0] << endl;

        // get track count
        char trackcount[1];
        in.get(trackcount, 4);
        cout << "total track count: " << (unsigned) trackcount[0] << endl;

    quit:
        in.close();
        return out;
    }
};