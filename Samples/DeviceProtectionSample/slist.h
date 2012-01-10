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

#if !defined SLIST_HDR
#define SLIST_HDR

//#include <tucommon.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef int BOOL;

/* this is an item in the list and stores pointers to data, next and
   previous list items */
typedef struct tag_LISTITEM
{
    void *      data;
    struct tag_LISTITEM *  next;      
    struct tag_LISTITEM *  prev;      
} LISTITEM, * LPLISTITEM;

typedef const LISTITEM * LPCLISTITEM;
    

/* this is created when Createlist is called and stores the head and tail
   of the list. */
typedef struct tag_LIST
{
    LPLISTITEM  head;
    LPLISTITEM  tail;
    short       count;
} LIST, * LPLIST;

typedef const LIST * LPCLIST;
    

/* this is created whenever an iterator is needed for iterating the list */
typedef struct tag_LISTITR
{
    LPLIST      list;
    LPLISTITEM  current;
} LISTITR, * LPLISTITR;

typedef const LISTITR * LPCLISTITR;
    
/* Function prototypes */

/* List operations */
extern LPLIST ListCreate(void);
extern BOOL ListAddItem(LPLIST list, void * data);
extern BOOL ListRemoveItem(LPLIST list, void * data);
extern BOOL ListFindItem(LPLIST list, void * data);
extern void * ListGetFirst(LPLIST list);
extern void * ListGetLast(LPLIST list);
extern int ListGetCount(LPLIST list);
extern void ListDelete(LPLIST list);
extern void ListFreeDelete(LPLIST list);


/* List Iterator operations */
extern LPLISTITR ListItrCreate(LPLIST list);
extern LPLISTITR ListItrFirst(LPLIST list, LPLISTITR listItr);
extern void * ListItrGetNext(LPLISTITR listItr);
extern BOOL ListItrInsertAfter(LPLISTITR listItr, void * data);
extern BOOL ListItrInsertBefore(LPLISTITR listItr, void * data);
extern BOOL ListItrRemoveItem(LPLISTITR listItr);
extern void ListItrDelete(LPLISTITR listItr);

#ifdef __cplusplus
}
#endif


#endif /* SLIST_HDR */

