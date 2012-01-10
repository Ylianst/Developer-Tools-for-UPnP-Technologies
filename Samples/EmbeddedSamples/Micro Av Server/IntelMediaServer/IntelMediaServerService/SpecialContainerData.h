// SpecialContainerData.h: interface for the SpecialContainerData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATA_H__8D063A43_688C_4E62_BFAA_675361FD4B49__INCLUDED_)
#define AFX_DATA_H__8D063A43_688C_4E62_BFAA_675361FD4B49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#import "C:\Program Files\Common Files\System\ADO\msado15.dll" no_namespace rename("EOF", "EndOfFile")

#include <icrsint.h>

class SpecialContainerData : public CADORecordBinding  
{
	BEGIN_ADO_BINDING(SpecialContainerData)
		ADO_VARIABLE_LENGTH_ENTRY(1, adVarChar, Value, sizeof(Value), Value_st, Value_length, TRUE)
		ADO_NUMERIC_ENTRY(2, adInteger, ValueCount, sizeof(ValueCount), ValueCount_st, ValueCount_length, TRUE)
	END_ADO_BINDING()

private:
	IADORecordBinding* RecordBinding;
	_RecordsetPtr RecordSet;

public:
	SpecialContainerData(IADORecordBinding* pRB, _RecordsetPtr rs);
	~SpecialContainerData();
	int  GetCount();
	bool MoveTo(int number);
	bool EndOfFile();
	bool MoveNext();
	bool MoveFirst();
	bool MoveLast();

	CHAR Value[255];
	BYTE Value_st;
	int  Value_length;

	long ValueCount;
	BYTE ValueCount_st;
	int  ValueCount_length;

	void SetStringLengths();
};

#endif // !defined(AFX_DATA_H__8D063A43_688C_4E62_BFAA_675361FD4B49__INCLUDED_)
