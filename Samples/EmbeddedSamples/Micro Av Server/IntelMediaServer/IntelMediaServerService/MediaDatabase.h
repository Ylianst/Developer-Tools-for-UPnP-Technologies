#pragma once

#include <icrsint.h>
#include "MediaItemData.h"
#include "SpecialContainerData.h"
#import "C:\Program Files\Common Files\System\ADO\msado15.dll" no_namespace rename("EOF", "EndOfFile")

enum MEDIA_DB_SPECIALCONTAINER_KEY_ENUM
{
	MEDIA_DB_SCK_ALLITEMS = 1,
	MEDIA_DB_SCK_MUSIC = 2,
	MEDIA_DB_SCK_PICTURES = 3,
	MEDIA_DB_SCK_VIDEO = 4,
	MEDIA_DB_SCK_MUSIC_ALBUM = 5,
	MEDIA_DB_SCK_MUSIC_ALL = 6,
	MEDIA_DB_SCK_MUSIC_ARTIST = 7,
	MEDIA_DB_SCK_MUSIC_GENRE = 8,
	MEDIA_DB_SCK_PICTURES_ALL = 9,
	MEDIA_DB_SCK_VIDEO_ARTIST = 10,
	MEDIA_DB_SCK_VIDEO_ALL = 11,
	MEDIA_DB_SCK_VIDEO_GENRE = 12,
	MEDIA_DB_SCK_USERFILES = 13
};

enum MEDIA_DB_SPECIALCONTAINER_ENUM
{
	MEDIA_DB_SC_TITLE = 1,
	MEDIA_DB_SC_CREATOR = 2,
	MEDIA_DB_SC_GENRE = 3,
	MEDIA_DB_SC_ALBUM = 4
};

enum MEDIA_DB_CLASS_ENUM
{
	MEDIA_DB_CL_ALL = 0,
	MEDIA_DB_CL_C_STORAGE = 1,
	MEDIA_DB_CL_C_PLAYLIST = 2,
	MEDIA_DB_CL_C_SPECIAL = 3,
	MEDIA_DB_CL_IMAGE = 4,
	MEDIA_DB_CL_AUDIO = 5,
	MEDIA_DB_CL_VIDEO = 6,
	MEDIA_DB_CL_OTHER = 7
};

enum MEDIA_DB_SORT_ENUM
{
	MEDIA_DB_SORT_NONE = 0,
	MEDIA_DB_SORT_KEY = 1,
	MEDIA_DB_SORT_TITLE = 2,
	MEDIA_DB_SORT_CREATOR = 3,
	MEDIA_DB_SORT_GENRE = 4,
	MEDIA_DB_SORT_ALBUM = 5,
	MEDIA_DB_SORT_DESCENDING = 0x80 // OR this with ary sort to get descending order
};

enum MEDIA_DB_PLAYLIST_CONDITION_ENUM
{
	MEDIA_DB_PL_ALL = 0,
	MEDIA_DB_PL_YES = 1,
	MEDIA_DB_PL_NO = 2
};

class MediaDatabase
{
private:
	_ConnectionPtr DB;

public:
	MediaDatabase();
	~MediaDatabase();

	bool Open(char* DatabasePath);
	void Close();

	bool UpdateItem(int Key, int ParentKey, char* Title, char* Creator, char* Genre, char* Album, int Class, bool Playlist, char* Deserialization, char* Path, int UpdateID);
	bool UpdateItemParent(int Key, int ParentKey);
	long AddNewItem(int ParentKey, char* Title, char* Creator, char* Genre, char* Album, int Class, bool Playlist, char* Deserialization, char* Path, int UpdateID);
	long GetArtistCount(int Class);
	long GetAlbumCount(int Class);
	long GetGenreCount(int Class);
	void GetClassCount(int* AudioCount, int* PictureCount, int* VideoCount, int* OtherCount);
	long GetContainerCount(int ParentKey);
	MediaItemData* QueryDirect(char* SqlQuery);
	MediaItemData* QueryAllItems(unsigned int sort, MEDIA_DB_PLAYLIST_CONDITION_ENUM playlist);
	MediaItemData* QueryAllItemsOfClass(MEDIA_DB_CLASS_ENUM Class, unsigned int sort, MEDIA_DB_PLAYLIST_CONDITION_ENUM playlist);
	MediaItemData* QuerySpecialContainerItems(MEDIA_DB_SPECIALCONTAINER_ENUM container, char* containerName, MEDIA_DB_CLASS_ENUM itemclass, unsigned int sort, MEDIA_DB_PLAYLIST_CONDITION_ENUM playlist);
	MediaItemData* QueryContainerItems(int parent, unsigned int sort);
	MediaItemData* QueryEmbeddedContainers(int parent);
	MediaItemData* QueryItem(int key);
	SpecialContainerData* QuerySpecialContainer(MEDIA_DB_SPECIALCONTAINER_ENUM container,MEDIA_DB_CLASS_ENUM itemclass);
	bool QueryContainerExist(int ParentKey, char* Path);
	bool UpdateItemDeser(int Key, char* Deserialization, int UpdateID);
	bool UpdateItemDeser(int Key, int Deserialization, int UpdateID);
	bool DeleteItemsWithParent(int ParentKey);
	bool DeleteItem(int Key);
};


