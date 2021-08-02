#ifndef COMMON_H
#define COMMON_H
#include "src/common.h"
#endif

/* CHANGE SETTINGS BEYOND THIS LINE */
#define MUSIC_DIRECTORIES {"%HOME%/Music"}
#define SONG_TAG_ORDER {STATUS_ICON, ARTIST, ALBUM, TRACKNUM, NAME, DURATION}
#define SONG_TAG_LENGTHS {2, 18, 20, 3, 30, 5}
#define SONG_SORT_ORDER {ARTIST, ALBUM, TRACKNUM}
#define SONG_TAG_SEPARATOR "|"
#define SONG_TAG_FILLER_CHAR ' '
#define HEADER_HEIGHT 20
#define SONG_HEIGHT 15
#define VOLUME_SLIDER_WIDTH 75
#define DEFAULT_VOLUME 50
#define CONFIG_LOCATION "" // just uses build directory, should end in slash
#define FRAMES_BETWEEN_CMD_CHECK 10