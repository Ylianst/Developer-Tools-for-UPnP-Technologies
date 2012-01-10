#ifndef _CDS_ERRORS_H
#define _CDS_ERRORS_H

#define CDS_EC_ACTION_FAILED					501
#define CDS_EM_ACTION_FAILED					"Action failed. Internal error encountered."

#define CDS_EC_OBJECT_ID_NO_EXIST				701
#define CDS_EM_OBJECT_ID_NO_EXIST				"ObjectID does not exist."

#define CDS_EC_NO_SUCH_CONTAINER				710
#define CDS_EM_NO_SUCH_CONTAINER				"The specified ObjectID or ContainerID identifies an object that is not a container."

enum Enum_CdsErrors
{
	CdsError_None = 0,
	CdsError_ActionFailed,
	CdsError_NoSuchObject,
	CdsError_NoSuchContainer
};

char *CDS_ErrorStrings[] = 
{
	"",
	CDS_EM_ACTION_FAILED,
	CDS_EM_OBJECT_ID_NO_EXIST,
	CDS_EM_NO_SUCH_CONTAINER
};

int CDS_ErrorCodes[] = 
{
	0,
	CDS_EC_ACTION_FAILED,
	CDS_EC_OBJECT_ID_NO_EXIST,
	CDS_EC_NO_SUCH_CONTAINER
};

#endif