// MediaItemData.h: interface for the MediaItemData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATA_H__8D063A43_688C_4E62_BFAA_675361FD4B48__INCLUDED_)
#define AFX_DATA_H__8D063A43_688C_4E62_BFAA_675361FD4B48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#import "C:\Program Files\Common Files\System\ADO\msado15.dll" no_namespace rename("EOF", "EndOfFile")

#include <icrsint.h>

class MediaItemData : public CADORecordBinding  
{
	BEGIN_ADO_BINDING(MediaItemData)
		ADO_NUMERIC_ENTRY(1, adInteger, Key, sizeof(Key), Key_st, Key_length, TRUE)
		ADO_NUMERIC_ENTRY(2, adInteger, ParentKey, sizeof(ParentKey), ParentKey_st, ParentKey_length, TRUE)
		ADO_VARIABLE_LENGTH_ENTRY(3, adVarChar, Title, sizeof(Title), Title_st, Title_length, TRUE)
		ADO_VARIABLE_LENGTH_ENTRY(4, adVarChar, Creator, sizeof(Creator), Creator_st, Creator_length, TRUE)
		ADO_VARIABLE_LENGTH_ENTRY(5, adVarChar, Genre, sizeof(Genre), Genre_st, Genre_length, TRUE)
		ADO_VARIABLE_LENGTH_ENTRY(6, adVarChar, Album, sizeof(Album), Album_st, Album_length, TRUE)
		ADO_NUMERIC_ENTRY(7, adInteger, Class, sizeof(Class), Class_st, Class_length, TRUE)
		ADO_FIXED_LENGTH_ENTRY(8, adBoolean, Playlist, Playlist_st, TRUE)
		ADO_VARIABLE_LENGTH_ENTRY(9, adVarChar, Deserialization, sizeof(Deserialization), Deserialization_st, Deserialization_length, TRUE)
		ADO_VARIABLE_LENGTH_ENTRY(10, adVarChar, Path, sizeof(Path), Path_st, Path_length, TRUE)
		ADO_NUMERIC_ENTRY(11, adInteger, UpdateID, sizeof(UpdateID), UpdateID_st, UpdateID_length, TRUE)
	END_ADO_BINDING()

private:
	IADORecordBinding* RecordBinding;
	_RecordsetPtr RecordSet;

public:
	MediaItemData(IADORecordBinding* pRB, _RecordsetPtr rs);
	~MediaItemData();
	//bool Update();
	//bool SupportAddNew();
	//bool Delete();
	bool MoveTo(int number);
	bool EndOfFile();
	bool MoveNext();
	bool MoveFirst();
	bool MoveLast();
	void SetStringLengths();

	long Key;
	BYTE Key_st;
	int  Key_length;

	long ParentKey;
	BYTE ParentKey_st;
	int  ParentKey_length;

	// This field must NEVER have the quote (") or forward-slash (/) characters in it.
	CHAR Title[255];
	BYTE Title_st;
	int  Title_length;

	// This field must NEVER have the quote (") or forward-slash (/) characters in it.
	CHAR Creator[255];
	BYTE Creator_st;
	int  Creator_length;

	// This field must NEVER have the quote (") or forward-slash (/) characters in it.
	CHAR Genre[255];
	BYTE Genre_st;
	int  Genre_length;

	// This field must NEVER have the quote (") or forward-slash (/) characters in it.
	CHAR Album[255];
	BYTE Album_st;
	int  Album_length;

	long Class;
	BYTE Class_st;
	int  Class_length;

	bool Playlist;
	BYTE Playlist_st;

	CHAR Deserialization[2048];
	BYTE Deserialization_st;
	int  Deserialization_length;
	
	CHAR Path[2048];
	BYTE Path_st;
	int  Path_length;

	long UpdateID;
	BYTE UpdateID_st;
	int  UpdateID_length;

};

#endif // !defined(AFX_DATA_H__8D063A43_688C_4E62_BFAA_675361FD4B48__INCLUDED_)
