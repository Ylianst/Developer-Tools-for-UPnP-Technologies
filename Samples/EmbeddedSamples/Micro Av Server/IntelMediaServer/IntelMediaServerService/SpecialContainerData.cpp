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
#include "SpecialContainerData.h"

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

SpecialContainerData::SpecialContainerData(IADORecordBinding* pRB, _RecordsetPtr rs)
{
	RecordBinding = pRB;
	RecordSet = rs;
}

SpecialContainerData::~SpecialContainerData()
{
	RecordBinding->Release();
	RecordSet->Close();
}

bool SpecialContainerData::MoveNext()
{
	if (!(RecordSet->EndOfFile))
	{
		RecordSet->MoveNext();
		return true;
	}
	return false;
}

bool SpecialContainerData::MoveTo(int number)
{
	return (RecordSet->Move(number) == S_OK);
}

bool SpecialContainerData::MoveFirst()
{
	return (RecordSet->MoveFirst() == S_OK);
}

bool SpecialContainerData::MoveLast()
{
	return (RecordSet->MoveLast() == S_OK);
}

int SpecialContainerData::GetCount()
{
	long count;
	if (RecordSet->get_RecordCount(&count) == S_OK) return count;
	return -1;
}

bool SpecialContainerData::EndOfFile()
{
	return (RecordSet->EndOfFile != false);
}

void SpecialContainerData::SetStringLengths()
{
	this->Value_length = (int) strlen(this->Value);
}