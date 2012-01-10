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

#include "stdafx.h"
#include "MediaItemData.h"

#include <stdlib.h>
#include <crtdbg.h>
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MediaItemData::MediaItemData(IADORecordBinding* pRB, _RecordsetPtr rs)
{
	RecordBinding = pRB;
	RecordSet = rs;
}

MediaItemData::~MediaItemData()
{
	RecordBinding->Release();
	RecordSet->Close();
	RecordSet.Release();
}

bool MediaItemData::MoveNext()
{
	if (!(RecordSet->EndOfFile))
	{
		RecordSet->MoveNext();
		return true;
	}
	return false;
}

bool MediaItemData::MoveTo(int number)
{
	return (RecordSet->Move(number) == S_OK);
}

bool MediaItemData::MoveFirst()
{
	return (RecordSet->MoveFirst() == S_OK);
}

bool MediaItemData::MoveLast()
{
	return (RecordSet->MoveLast() == S_OK);
}

void MediaItemData::SetStringLengths()
{
	this->Album_length = (int) strlen(this->Album);
	this->Creator_length = (int) strlen(this->Creator);
	this->Genre_length = (int) strlen(this->Genre);
	this->Title_length = (int) strlen(this->Title);
}

/*
bool MediaItemData::Update()
{
	return (RecordSet->UpdateBatch(adAffectCurrent) == S_OK);
}

bool MediaItemData::SupportAddNew()
{
	return (RecordSet->Supports(adAddNew) == VARIANT_TRUE);
} 

bool MediaItemData::Delete()
{
	return (RecordSet->Delete(adAffectCurrent) == S_OK);
}
*/

bool MediaItemData::EndOfFile()
{
	return (RecordSet->EndOfFile != false);
}