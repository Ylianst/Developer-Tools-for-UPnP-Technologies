struct MyListNode
{
	char * ID;
	char * Description;
	struct MyListNode *next;
};

struct MyList
{
	struct MyListNode *head;
	struct MyListNode *tail;
};


struct MyList * MyList_Create()


void MyList_Destroy(struct UDNList * toDestroy)


void MyList_Append(char *UDN)
char * MyList_Get(int index)
