/*   
Copyright 2006 - 2011 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "StdAfx.h"
#include "mediadatabase.h"

#include <icrsint.h>
#include "stdio.h"
#include "MediaItemData.h"
#include "SpecialContainerData.h"
//#import "C:\Program Files\Common Files\System\ADO\msado15.dll" no_namespace rename("EOF", "EndOfFile")

MediaDatabase::MediaDatabase()
{
	DB = NULL;
}

MediaDatabase::~MediaDatabase()
{
	Close();
}

bool MediaDatabase::Open(char* DatabasePath)
{
	char constr[2048];
	if (DB != NULL) Close();
	DB.CreateInstance("ADODB.Connection");
	sprintf(constr,"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%sIntelMediaServer.mdb;",DatabasePath);
	DB->ConnectionString = constr;
	HRESULT r;
	try
	{
		r = DB->Open("","","",adConnectUnspecified);
	}
	catch (_com_error ex)
	{
		DB.Release();
		DB = NULL;
		return false;
	}
	if (r != S_OK) return false;

	return true;
}

MediaItemData* MediaDatabase::QueryDirect(char* SqlQuery)
{
	_RecordsetPtr RecordSet;
	MediaItemData* item;
	IADORecordBinding* pRB = NULL;
	RecordSet.CreateInstance("ADODB.Recordset");
	_variant_t SQLSTMT(SqlQuery);

	//RecordSet->Open(SQLSTMT,_variant_t((IDispatch *)DB, true),adOpenForwardOnly,adLockOptimistic,adCmdText);
	RecordSet->Open(SQLSTMT,_variant_t((IDispatch *)DB, true),adOpenForwardOnly,adLockReadOnly,adCmdText);
	RecordSet->QueryInterface(__uuidof(IADORecordBinding), (LPVOID*)&pRB);
	item = new MediaItemData(pRB,RecordSet);
	pRB->BindToRecordset(item);

	if(!(RecordSet->EndOfFile))
	{
		RecordSet->MoveFirst();
		return item;
	}
	delete item;
	return NULL;
}

long MediaDatabase::AddNewItem(int ParentKey, char* Title, char* Creator, char* Genre, char* Album, int Class, bool Playlist, char* Deserialization, char* Path, int UpdateID)
{
	char SqlQuery[4048];
	char* SqlQueryPtr = SqlQuery;
	char* PlaylistStr = Playlist?"True":"False";
	long result = 0;

	// Truncate the fields to max database size
	if (strlen(Title)   > 80) Title[80] = 0;
	if (strlen(Creator) > 50) Creator[50] = 0;
	if (strlen(Genre)   > 50) Genre[50] = 0;
	if (strlen(Album)   > 50) Album[50] = 0;

	SqlQueryPtr += sprintf(SqlQueryPtr,"INSERT INTO MediaItems ( ParentKey, Title, Creator, Genre, Album, Class, Playlist, Deserialization, Path, UpdateID ) VALUES (%d, \"%s\", \"%s\", \"%s\", \"%s\", %d, %s, \"%s\", \"%s\", %d);", ParentKey, Title, Creator, Genre, Album, Class, PlaylistStr, Deserialization, Path, UpdateID);
	_RecordsetPtr RecordSet = DB->Execute(SqlQuery,NULL,adCmdText);
	if (RecordSet == NULL) return 0;
	
	RecordSet->Open("SELECT @@Identity AS NewID",_variant_t((IDispatch *)DB, true),adOpenForwardOnly,adLockOptimistic,adCmdText);
	_variant_t val = RecordSet->Fields->GetItem("NewID")->Value;
	result = val.iVal;
	RecordSet->Close();
	RecordSet.Release();

	return result;
}

long MediaDatabase::GetArtistCount(int Class)
{
	char SqlQuery[4048];
	long result = 0;
	_RecordsetPtr RecordSet;

	RecordSet.CreateInstance("ADODB.Recordset");
	sprintf(SqlQuery,"SELECT COUNT(MediaItems.Creator) AS val FROM [SELECT DISTINCT MediaItems.Creator FROM MediaItems WHERE (CLASS=%d)]. AS Creator1;", Class);
	RecordSet->Open(SqlQuery,_variant_t((IDispatch *)DB, true),adOpenForwardOnly,adLockOptimistic,adCmdText);
	_variant_t val = RecordSet->Fields->GetItem("val")->Value;
	result = val.iVal;
	RecordSet->Close();
	RecordSet.Release();
	return result;
}

long MediaDatabase::GetAlbumCount(int Class)
{
	char SqlQuery[4048];
	long result = 0;
	_RecordsetPtr RecordSet;

	RecordSet.CreateInstance("ADODB.Recordset");
	sprintf(SqlQuery,"SELECT COUNT(MediaItems.Album) AS val FROM [SELECT DISTINCT MediaItems.Album FROM MediaItems WHERE (CLASS=%d)]. AS Album1;", Class);
	RecordSet->Open(SqlQuery,_variant_t((IDispatch *)DB, true),adOpenForwardOnly,adLockOptimistic,adCmdText);
	_variant_t val = RecordSet->Fields->GetItem("val")->Value;
	result = val.iVal;
	RecordSet->Close();
	RecordSet.Release();
	return result;
}

long MediaDatabase::GetGenreCount(int Class)
{
	char SqlQuery[4048];
	long result = 0;
	_RecordsetPtr RecordSet;

	RecordSet.CreateInstance("ADODB.Recordset");
	sprintf(SqlQuery,"SELECT COUNT(MediaItems.Genre) AS val FROM [SELECT DISTINCT MediaItems.Genre FROM MediaItems WHERE (CLASS=%d)]. AS Genre1;", Class);
	RecordSet->Open(SqlQuery,_variant_t((IDispatch *)DB, true),adOpenForwardOnly,adLockOptimistic,adCmdText);
	_variant_t val = RecordSet->Fields->GetItem("val")->Value;
	result = val.iVal;
	RecordSet->Close();
	RecordSet.Release();
	return result;
}

void MediaDatabase::GetClassCount(int* AudioCount, int* PictureCount, int* VideoCount, int* OtherCount)
{
	*AudioCount   = 0;
	*PictureCount = 0;
	*VideoCount   = 0;
	*OtherCount   = 0;
	_RecordsetPtr RecordSet;

	RecordSet.CreateInstance("ADODB.Recordset");
	RecordSet->Open("SELECT DISTINCTROW MediaItems.Class, Count(*) AS [CClass] FROM MediaItems GROUP BY MediaItems.Class;",_variant_t((IDispatch *)DB, true),adOpenForwardOnly,adLockOptimistic,adCmdText);
	do
	{
		_variant_t val1 = RecordSet->Fields->GetItem("Class")->Value;
		_variant_t val2 = RecordSet->Fields->GetItem("CClass")->Value;
		switch (val1.iVal)
		{
			case MEDIA_DB_CL_IMAGE:
				*AudioCount = val2.iVal;
				break;
			case MEDIA_DB_CL_AUDIO:
				*PictureCount = val2.iVal;
				break;
			case MEDIA_DB_CL_VIDEO:
				*VideoCount = val2.iVal;
				break;
			case MEDIA_DB_CL_OTHER:
				*OtherCount = val2.iVal;
				break;
		}
		RecordSet->MoveNext();
	}
	while (RecordSet->EndOfFile == 0);

	RecordSet->Close();
	RecordSet.Release();
}

long MediaDatabase::GetContainerCount(int ParentKey)
{
	char SqlQuery[4048];
	long result = 0;
	_RecordsetPtr RecordSet;

	RecordSet.CreateInstance("ADODB.Recordset");
	sprintf(SqlQuery,"SELECT COUNT(*) AS val FROM MediaItems WHERE ParentKey=0;");
	RecordSet->Open(SqlQuery,_variant_t((IDispatch *)DB, true),adOpenForwardOnly,adLockOptimistic,adCmdText);
	_variant_t val = RecordSet->Fields->GetItem("val")->Value;
	result = val.iVal;
	RecordSet->Close();
	RecordSet.Release();
	return result;
}

bool MediaDatabase::UpdateItem(int Key, int ParentKey, char* Title, char* Creator, char* Genre, char* Album, int Class, bool Playlist, char* Deserialization, char* Path, int UpdateID)
{
	char SqlQuery[2048];
	char* SqlQueryPtr = SqlQuery;
	char* PlaylistStr = Playlist?"True":"False";
	bool result;

	// Truncate the fields to max database size
	if (strlen(Title)   > 80) Title[80] = 0;
	if (strlen(Creator) > 50) Creator[50] = 0;
	if (strlen(Genre)   > 50) Genre[50] = 0;
	if (strlen(Album)   > 50) Album[50] = 0;

	SqlQueryPtr += sprintf(SqlQueryPtr,"UPDATE MediaItems SET ParentKey=%d, Title='%s', Creator='%s', Genre='%s', Album='%s', Class=%d, Playlist=%s, Deserialization='%s', Path='%s', UpdateID=%d WHERE Key=%d;", ParentKey, Title, Creator, Genre, Album, Class, PlaylistStr, Deserialization, Path, UpdateID, Key);
	_RecordsetPtr r = DB->Execute(SqlQuery,NULL,adCmdText);
	result = (r != NULL);
	r.Release();

	return result;
}

bool MediaDatabase::UpdateItemParent(int Key, int ParentKey)
{
	char SqlQuery[2048];
	bool result;

	sprintf(SqlQuery,"UPDATE MediaItems SET ParentKey=%d WHERE Key=%d;", ParentKey, Key);
	_RecordsetPtr r = DB->Execute(SqlQuery,NULL,adCmdText);
	result = (r != NULL);
	r.Release();

	return result;
}

bool MediaDatabase::UpdateItemDeser(int Key, char* Deserialization, int UpdateID)
{
	char SqlQuery[2048];
	bool result;

	sprintf(SqlQuery,"UPDATE MediaItems SET Deserialization='%s', UpdateID=%d WHERE Key=%d;", Deserialization, UpdateID, Key);
	_RecordsetPtr r = DB->Execute(SqlQuery,NULL,adCmdText);
	result = (r != NULL);
	r.Release();

	return result;
}

bool MediaDatabase::UpdateItemDeser(int Key, int Deserialization, int UpdateID)
{
	char SqlQuery[512];
	bool result;

	sprintf(SqlQuery,"UPDATE MediaItems SET Deserialization='%d', UpdateID=%d WHERE Key=%d;", Deserialization, UpdateID, Key);
	_RecordsetPtr r = DB->Execute(SqlQuery,NULL,adCmdText);
	result = (r != NULL);
	r.Release();

	return result;
}

MediaItemData* MediaDatabase::QueryAllItems(unsigned int sort, MEDIA_DB_PLAYLIST_CONDITION_ENUM playlist)
{
	char SqlQuery[2048];
	char* SqlQueryPtr = SqlQuery;

	SqlQueryPtr += sprintf(SqlQueryPtr,"SELECT * FROM MediaItems WHERE (Class > 3)");
	if (playlist == MEDIA_DB_PL_YES)   SqlQueryPtr += sprintf(SqlQueryPtr," AND (Playlist = True)");
	if (playlist == MEDIA_DB_PL_NO)    SqlQueryPtr += sprintf(SqlQueryPtr," AND (Playlist = False)");

	if (sort != 0) SqlQueryPtr += sprintf(SqlQueryPtr," ORDER BY ");
	for (int i=0;i<4;i++)
	{
		int srt = (sort >> (i*8)) & 0x7F;
		if (srt != 0)
		{
			if (i != 0)SqlQueryPtr += sprintf(SqlQueryPtr,", ");
			if (srt == MEDIA_DB_SORT_KEY)     SqlQueryPtr += sprintf(SqlQueryPtr,"Key ");
			if (srt == MEDIA_DB_SORT_TITLE)   SqlQueryPtr += sprintf(SqlQueryPtr,"Title ");
			if (srt == MEDIA_DB_SORT_CREATOR) SqlQueryPtr += sprintf(SqlQueryPtr,"Creator ");
			if (srt == MEDIA_DB_SORT_GENRE)   SqlQueryPtr += sprintf(SqlQueryPtr,"Genre ");
			if (srt == MEDIA_DB_SORT_ALBUM)   SqlQueryPtr += sprintf(SqlQueryPtr,"Album ");
			if ((sort >> (i*8)) & 0x80)       SqlQueryPtr += sprintf(SqlQueryPtr,"DESC ");
		}
	}

	SqlQueryPtr += sprintf(SqlQueryPtr,";");

	return QueryDirect(SqlQuery);
}

MediaItemData* MediaDatabase::QueryAllItemsOfClass(MEDIA_DB_CLASS_ENUM Class, unsigned int sort, MEDIA_DB_PLAYLIST_CONDITION_ENUM playlist)
{
	char SqlQuery[2048];
	char* SqlQueryPtr = SqlQuery;

	SqlQueryPtr += sprintf(SqlQueryPtr,"SELECT * FROM MediaItems WHERE (Class = %d)", (int)Class);
	if (playlist == MEDIA_DB_PL_YES)   SqlQueryPtr += sprintf(SqlQueryPtr," AND (Playlist = True)");
	if (playlist == MEDIA_DB_PL_NO)    SqlQueryPtr += sprintf(SqlQueryPtr," AND (Playlist = False)");

	if (sort != 0) SqlQueryPtr += sprintf(SqlQueryPtr," ORDER BY ");
	for (int i=0;i<4;i++)
	{
		int srt = (sort >> (i*8)) & 0x7F;
		if (srt != 0)
		{
			if (i != 0)SqlQueryPtr += sprintf(SqlQueryPtr,", ");
			if (srt == MEDIA_DB_SORT_KEY)     SqlQueryPtr += sprintf(SqlQueryPtr,"Key ");
			if (srt == MEDIA_DB_SORT_TITLE)   SqlQueryPtr += sprintf(SqlQueryPtr,"Title ");
			if (srt == MEDIA_DB_SORT_CREATOR) SqlQueryPtr += sprintf(SqlQueryPtr,"Creator ");
			if (srt == MEDIA_DB_SORT_GENRE)   SqlQueryPtr += sprintf(SqlQueryPtr,"Genre ");
			if (srt == MEDIA_DB_SORT_ALBUM)   SqlQueryPtr += sprintf(SqlQueryPtr,"Album ");
			if ((sort >> (i*8)) & 0x80)       SqlQueryPtr += sprintf(SqlQueryPtr,"DESC ");
		}
	}

	SqlQueryPtr += sprintf(SqlQueryPtr,";");

	return QueryDirect(SqlQuery);
}

MediaItemData* MediaDatabase::QueryContainerItems(int parent, unsigned int sort)
{
	char SqlQuery[2048];
	char* SqlQueryPtr = SqlQuery;

	SqlQueryPtr += sprintf(SqlQueryPtr,"SELECT * FROM MediaItems WHERE ParentKey=%d",parent);

	if (sort != 0) SqlQueryPtr += sprintf(SqlQueryPtr," ORDER BY ");
	for (int i=0;i<4;i++)
	{
		int srt = (sort >> (i*8)) & 0x7F;
		if (srt != 0)
		{
			if (i != 0)SqlQueryPtr += sprintf(SqlQueryPtr,", ");
			if (srt == MEDIA_DB_SORT_KEY)     SqlQueryPtr += sprintf(SqlQueryPtr,"Key ");
			if (srt == MEDIA_DB_SORT_TITLE)   SqlQueryPtr += sprintf(SqlQueryPtr,"Title ");
			if (srt == MEDIA_DB_SORT_CREATOR) SqlQueryPtr += sprintf(SqlQueryPtr,"Creator ");
			if (srt == MEDIA_DB_SORT_GENRE)   SqlQueryPtr += sprintf(SqlQueryPtr,"Genre ");
			if (srt == MEDIA_DB_SORT_ALBUM)   SqlQueryPtr += sprintf(SqlQueryPtr,"Album ");
			if ((sort >> (i*8)) & 0x80)       SqlQueryPtr += sprintf(SqlQueryPtr,"DESC ");
		}
	}

	SqlQueryPtr += sprintf(SqlQueryPtr,";");

	return QueryDirect(SqlQuery);
}

MediaItemData* MediaDatabase::QueryEmbeddedContainers(int parent)
{
	char SqlQuery[2048];
	sprintf(SqlQuery,"SELECT * FROM MediaItems WHERE (Class<4) AND (ParentKey=%d);",parent);
	return QueryDirect(SqlQuery);
}

MediaItemData* MediaDatabase::QueryItem(int key)
{
	char SqlQuery[2048];
	sprintf(SqlQuery,"SELECT * FROM MediaItems WHERE (Key=%d);",key);
	return QueryDirect(SqlQuery);
}

MediaItemData* MediaDatabase::QuerySpecialContainerItems(MEDIA_DB_SPECIALCONTAINER_ENUM container, char* containerName, MEDIA_DB_CLASS_ENUM itemclass, unsigned int sort, MEDIA_DB_PLAYLIST_CONDITION_ENUM playlist)
{
	char SqlQuery[2048];
	char* SqlQueryPtr = SqlQuery;

	SqlQueryPtr += sprintf(SqlQueryPtr,"SELECT * FROM MediaItems WHERE ");

	if (container == MEDIA_DB_SC_TITLE)   SqlQueryPtr += sprintf(SqlQueryPtr,"(Title=\"%s\") ",containerName);
	if (container == MEDIA_DB_SC_CREATOR) SqlQueryPtr += sprintf(SqlQueryPtr,"(Creator=\"%s\") ",containerName);
	if (container == MEDIA_DB_SC_GENRE)   SqlQueryPtr += sprintf(SqlQueryPtr,"(Genre=\"%s\") ",containerName);
	if (container == MEDIA_DB_SC_ALBUM)   SqlQueryPtr += sprintf(SqlQueryPtr,"(Album=\"%s\") ",containerName);

	if (itemclass > 0) SqlQueryPtr += sprintf(SqlQueryPtr,"AND (Class=%d) ",(int)itemclass);

	if (playlist == MEDIA_DB_PL_YES)   SqlQueryPtr += sprintf(SqlQueryPtr,"AND (Playlist=True) ");
	if (playlist == MEDIA_DB_PL_NO)    SqlQueryPtr += sprintf(SqlQueryPtr,"AND (Playlist=False) ");

	if (sort != 0) SqlQueryPtr += sprintf(SqlQueryPtr," ORDER BY ");
	for (int i=0;i<4;i++)
	{
		int srt = (sort >> (i*8)) & 0x7F;
		if (srt != 0)
		{
			if (i != 0)SqlQueryPtr += sprintf(SqlQueryPtr,", ");
			if (srt == MEDIA_DB_SORT_KEY)     SqlQueryPtr += sprintf(SqlQueryPtr,"Key ");
			if (srt == MEDIA_DB_SORT_TITLE)   SqlQueryPtr += sprintf(SqlQueryPtr,"Title ");
			if (srt == MEDIA_DB_SORT_CREATOR) SqlQueryPtr += sprintf(SqlQueryPtr,"Creator ");
			if (srt == MEDIA_DB_SORT_GENRE)   SqlQueryPtr += sprintf(SqlQueryPtr,"Genre ");
			if (srt == MEDIA_DB_SORT_ALBUM)   SqlQueryPtr += sprintf(SqlQueryPtr,"Album ");
			if ((sort >> (i*8)) & 0x80)       SqlQueryPtr += sprintf(SqlQueryPtr,"DESC ");
		}
	}

	SqlQueryPtr += sprintf(SqlQueryPtr,";");

	return QueryDirect(SqlQuery);
}

SpecialContainerData* MediaDatabase::QuerySpecialContainer(MEDIA_DB_SPECIALCONTAINER_ENUM container, MEDIA_DB_CLASS_ENUM itemclass)
{
	_RecordsetPtr RecordSet;
	SpecialContainerData* item;
	IADORecordBinding* pRB = NULL;
	RecordSet.CreateInstance("ADODB.Recordset");
	char SqlQuery[2048];
	char* SqlQueryPtr = SqlQuery;
	
	if (container == MEDIA_DB_SC_TITLE)   SqlQueryPtr += sprintf(SqlQueryPtr,"SELECT DISTINCTROW MediaItems.Title, Count(*) AS [Count Of MediaItems] FROM MediaItems");
	if (container == MEDIA_DB_SC_CREATOR) SqlQueryPtr += sprintf(SqlQueryPtr,"SELECT DISTINCTROW MediaItems.Creator, Count(*) AS [Count Of MediaItems] FROM MediaItems");
	if (container == MEDIA_DB_SC_GENRE)   SqlQueryPtr += sprintf(SqlQueryPtr,"SELECT DISTINCTROW MediaItems.Genre, Count(*) AS [Count Of MediaItems] FROM MediaItems");
	if (container == MEDIA_DB_SC_ALBUM)   SqlQueryPtr += sprintf(SqlQueryPtr,"SELECT DISTINCTROW MediaItems.Album, Count(*) AS [Count Of MediaItems] FROM MediaItems");

	if (itemclass > 0) SqlQueryPtr += sprintf(SqlQueryPtr," WHERE (Class=%d)",(int)itemclass);

	if (container == MEDIA_DB_SC_TITLE)   SqlQueryPtr += sprintf(SqlQueryPtr," GROUP BY MediaItems.Title ORDER BY Title;");
	if (container == MEDIA_DB_SC_CREATOR) SqlQueryPtr += sprintf(SqlQueryPtr," GROUP BY MediaItems.Creator ORDER BY Creator;");
	if (container == MEDIA_DB_SC_GENRE)   SqlQueryPtr += sprintf(SqlQueryPtr," GROUP BY MediaItems.Genre ORDER BY Genre;");
	if (container == MEDIA_DB_SC_ALBUM)   SqlQueryPtr += sprintf(SqlQueryPtr," GROUP BY MediaItems.Album ORDER BY Album;");

	if (SqlQuery == NULL) return NULL;
	_variant_t SQLSTMT(SqlQuery);

	RecordSet->Open(SQLSTMT,_variant_t((IDispatch *)DB, true),adOpenDynamic,adLockOptimistic,adCmdText);
	RecordSet->QueryInterface(__uuidof(IADORecordBinding), (LPVOID*)&pRB);
	item = new SpecialContainerData(pRB,RecordSet);
	pRB->BindToRecordset(item);

	if(!(RecordSet->EndOfFile))
	{
		RecordSet->MoveFirst();
		return item;
	}
	delete item;
	return NULL;
}

bool MediaDatabase::QueryContainerExist(int ParentKey, char* Path)
{
	char SqlQuery[2048];
	sprintf(SqlQuery,"SELECT * FROM MediaItems WHERE (ParentKey=%d) AND (Path=\"%s\");",ParentKey,Path);
	MediaItemData* items = QueryDirect(SqlQuery);
	if (items == NULL) return false;
	delete items;
	return true;
}

bool MediaDatabase::DeleteItemsWithParent(int ParentKey)
{
	char SqlQuery[2048];
	bool result;

	sprintf(SqlQuery,"DELETE FROM MediaItems WHERE ParentKey=%d;", ParentKey);
	_RecordsetPtr r = DB->Execute(SqlQuery,NULL,adCmdText);
	result = (r != NULL);
	if (r != NULL) r.Release();
	return result;
}

bool MediaDatabase::DeleteItem(int Key)
{
	char SqlQuery[2048];
	bool result;

	sprintf(SqlQuery,"DELETE FROM MediaItems WHERE Key=%d;", Key);
	_RecordsetPtr r = DB->Execute(SqlQuery,NULL,adCmdText);
	result = (r != NULL);
	if (r != NULL) r.Release();
	return result;	
}

void MediaDatabase::Close()
{
	if (DB != NULL)
	{
		DB->Close();
		DB.Release();
		DB = NULL;
	}
}
