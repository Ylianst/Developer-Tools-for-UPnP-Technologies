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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "slist.h"

/* #defines used in this file only */

#define     FIRST_TIME      0xFFFFFFFF

#define FALSE 0
#define TRUE  1

/***************************************************************************/
/* List operations                                                         */

/****************************************************************************
//
// PROCEDURE:   ListCreate()
//
// PARAMETERS:
//
// DESCRIPTION:
//              Creates a linked list with no items within.
//
// RETURNS:
//              Pointer to the list created. NULL if calloc fails.
//
****************************************************************************/

LPLIST ListCreate(void)
{
    LPLIST list;

    list = (LPLIST) calloc (1, sizeof(LIST));

#ifdef _LISTDEBUG
    CSTRACE( (CSTRACE_INFO, "ListCreate %X\n",list) );
#endif // _LISTDEBUG

    if (list)
    {
        /* initialize data members */
        list->head = NULL;
        list->tail = NULL;
        list->count = 0;
    }
    
    return list;

} /* ListCreate */


/****************************************************************************
//
// PROCEDURE:   ListAddItem()
//
// PARAMETERS:
//  list        Pointer to a LIST
//
//  data        Pointer to data item to be added
//
// DESCRIPTION:
//              Adds a data item at the end of list.
//
// RETURNS:
//              TRUE if success, FALSE if calloc fails.
//
****************************************************************************/

BOOL ListAddItem(LPLIST list, void * data)
{
    LPLISTITEM newListItem;

    assert(list);
    assert(data);

    newListItem = (LPLISTITEM) calloc (1, sizeof(LISTITEM));

    if (newListItem)
    {
        newListItem->data = data;
        newListItem->next = NULL;

        if (list->head == NULL)
        {
            /* no list items right now */
            list->head = list->tail = newListItem;
            newListItem->prev = NULL;
        }
        else
        {
            /* there are one or more item */
            newListItem->prev = list->tail;
            list->tail->next = newListItem;
            list->tail = newListItem;
        }

        /* increment number of items */
        list->count ++;
        return TRUE;
    }
    else
    {
        return FALSE;
    } /* if listItem */

} /* ListAddItem */


/****************************************************************************
//
// PROCEDURE:   ListRemoveItem()
//
// PARAMETERS:
//  list        Pointer to a LIST
//
//  data        Pointer to data item to be added
//
// DESCRIPTION:
//              Removes a perticular data item from the list
//
// RETURNS:
//              TRUE if that data item is found, else FALSE is returned.
//
****************************************************************************/

BOOL ListRemoveItem(LPLIST list, void * data)
{
    LPLISTITEM current;

    assert(list);
    assert(data);

    current = list->head;

    while(current)
    {
        if (current->data == data)
        {
            if (current->next == NULL && current->prev == NULL)
            {
                /* this is the only item */
                list->head = list->tail = NULL;
            }
            else if (current->next == NULL)
            {
                /* this is the last item */
                list->tail = current->prev;
                current->prev->next = NULL;
            }
            else if (current->prev == NULL)
            {
                /* this is the first item */
                list->head = current->next;
                current->next->prev = NULL;
            }
            else
            {
                /* somewhere in the middle */
                current->next->prev = current->prev;
                current->prev->next = current->next;
            } /* if */

            free(current);
            list->count --;

            return TRUE;
        } /* if */

        /* traverse further */
        current = current->next;
    } /* while */

    return FALSE;

} /* ListRemoveItem */


/****************************************************************************
//
// PROCEDURE:   ListFindItem()
//
// PARAMETERS:
//  list        Pointer to a LIST
//
//  data        Pointer to data item to be added
//
// DESCRIPTION:
//              Searches for the data item in the list.
//
// RETURNS:
//              TRUE if that data item is found, else FALSE is returned.
//
****************************************************************************/

BOOL ListFindItem(LPLIST list, void * data)
{
    LPLISTITEM current;

    assert(list);
    assert(data);

    current = list->head;

    while(current)
    {
        if (current->data == data)
        {
            return TRUE;
        } /* if */

        /* traverse further */
        current = current->next;
    } /* while */

    return FALSE;

} /* ListFindItem */


/****************************************************************************
//
// PROCEDURE:   ListGetFirst()
//
// PARAMETERS:
//  list        Pointer to a LIST
//
// DESCRIPTION:
//              Gets number of items in the list
//
// RETURNS:
//              Number of items in the list
//
****************************************************************************/

void * ListGetFirst(LPLIST list)
{
    if (list->head)
    {
        return list->head->data;
    } /* if */
    else
    {
        return NULL;
    } /* else */
} /* ListGetFirst */


/****************************************************************************
//
// PROCEDURE:   ListGetLast()
//
// PARAMETERS:
//  list        Pointer to a LIST
//
// DESCRIPTION:
//              Gets number of items in the list
//
// RETURNS:
//              Number of items in the list
//
****************************************************************************/

void * ListGetLast(LPLIST list)
{
    if (list->tail)
    {
        return list->tail->data;
    } /* if */
    else
    {
        return NULL;
    } /* else */
} /* ListGetLast */



/****************************************************************************
//
// PROCEDURE:   ListGetCount()
//
// PARAMETERS:
//  list        Pointer to a LIST
//
// DESCRIPTION:
//              Gets number of items in the list
//
// RETURNS:
//              Number of items in the list
//
****************************************************************************/

int ListGetCount(LPLIST list)
{
    assert(list);

    return (list->count);

} /* ListGetCount */


/****************************************************************************
//
// PROCEDURE:   ListDelete()
//
// PARAMETERS:
//  list        Pointer to a LIST
//
// DESCRIPTION:
//              Deletes all the items in the list and the list itself
//
// RETURNS:
//              None
//
****************************************************************************/

void ListDelete(LPLIST list)
{
    LPLISTITEM current;
    LPLISTITEM tmpItem;

    assert(list);

    current = list->head;

    while(current)
    {
        tmpItem = current->next;

        free(current);
        list->count --;

        current = tmpItem;
    } /* while */

#ifdef _LISTDEBUG
    CSTRACE( (CSTRACE_INFO, "ListDelete %X\n",list) );
#endif // _LISTDEBUG

    free(list);

} /* ListDelete */

/****************************************************************************
//
// PROCEDURE:   ListFreeDelete()
//
// PARAMETERS:
//  list        Pointer to a LIST
//
// DESCRIPTION:
//              Deletes all the items in the list, list's user data
//              and the list itself
//
// RETURNS:
//              None
//
****************************************************************************/

void ListFreeDelete(LPLIST list)
{
    LPLISTITEM current;
    LPLISTITEM tmpItem;

    assert(list);

    current = list->head;

    while(current)
    {
        tmpItem = current->next;

        free(current->data);
        free(current);
        list->count --;
        current = tmpItem;
    } /* while */

#ifdef _LISTDEBUG
    CSTRACE( (CSTRACE_INFO, "ListFreeDelete %X\n",list) );
#endif // _LISTDEBUG

    free(list);
}



/**************************************************************************/
/* List Iterator operations                                               */ 


/****************************************************************************
//
// PROCEDURE:   ListItrCreate()
//
// PARAMETERS:
//  list        Pointer to a LIST
//
// DESCRIPTION:
//              Creates an iterator for the list
//
// RETURNS:
//              Pointer to List Iterator or NULL if calloc fails.
//
****************************************************************************/

LPLISTITR ListItrCreate(LPLIST list)
{
    LPLISTITR listItr;
    
    assert(list);

    listItr = (LPLISTITR) calloc(1, sizeof(LISTITR));
#ifdef _LISTDEBUG
    CSTRACE( (CSTRACE_INFO, "ListItrCreate %X\n",listItr) );
#endif // _LISTDEBUG
    
    if (listItr)
    {
        /* initialize data members */
        listItr->list = list;
        listItr->current = (LPLISTITEM) FIRST_TIME;
    }

    return listItr;

} /* ListItrCreate */

/****************************************************************************
//
// PROCEDURE:   ListItrFirst()
//
// PARAMETERS:
//  list        Pointer to a LIST
//
// DESCRIPTION:
//              iterator for the first item in the list
//
// RETURNS:
//              Pointer to List Iterator or NULL if calloc fails.
//
****************************************************************************/

LPLISTITR ListItrFirst(LPLIST list, LPLISTITR listItr)
{
        
    assert(list);
  
    
    if (listItr)
    {
        /* initialize data members */
        listItr->list = list;
        listItr->current = (LPLISTITEM) FIRST_TIME;
    }

    return listItr;

} /* ListItrFirst */



/****************************************************************************
//
// PROCEDURE:   ListItrGetNext()
//
// PARAMETERS:
//  listItr     Pointer to a LISTITR
//
// DESCRIPTION:
//              Gets the data pointed by the "current" pointer in the
//              list iterator. "current" is incremented before.
//
// RETURNS:
//              Pointer to data. NULL if end of list is reached
//
****************************************************************************/

void * ListItrGetNext(LPLISTITR listItr)
{
    assert(listItr);

    if (listItr->current == (LPLISTITEM)FIRST_TIME)
    {
        listItr->current = listItr->list->head;
    }
    else if (listItr->current == NULL)
    {
        return NULL;
    }
    else
    {
        listItr->current = listItr->current->next;
    }

    if (listItr->current)
    {
        return listItr->current->data;
    }
    else
    {
        return NULL;
    }
} /* ListItrGetNext */

/****************************************************************************
//
// PROCEDURE:   ListItrInsertAfter()
//
// PARAMETERS:
//  listItr     Pointer to a LISTITR
//
//  data        Pointer to data item to be iserted
//
// DESCRIPTION:
//              Inserts the data item after the data item pointed by
//              "current" pointer.
//
// RETURNS:
//              TRUE on success. FALSE if calloc fails
//
****************************************************************************/

BOOL ListItrInsertAfter(LPLISTITR listItr, void * data)
{
    LPLISTITEM newListItem;

    assert(listItr);
    assert(data);
    assert(((unsigned int)listItr->current) != FIRST_TIME);

    newListItem = (LPLISTITEM) calloc (1, sizeof(LISTITEM));

    if (newListItem)
    {
        newListItem->data = data;

        if (listItr->current == NULL)
        {
            if (listItr->list->count == 0)
            {
                /* there are no items right now */
                listItr->list->head = listItr->list->tail = newListItem;
                listItr->current = listItr->list->head;
            } /* if */
            else
            {
                /* there are some items, current points to null */
                newListItem->prev = listItr->list->tail;
                newListItem->next = NULL;
                listItr->list->tail->next = newListItem;
                listItr->list->tail = newListItem;
                listItr->current = newListItem;
            } /* else */
        } /* if */
        else
        {
            newListItem->prev = listItr->current;
            newListItem->next = listItr->current->next;
            
            if (listItr->current->next)
            {
                /* there are items after */
                listItr->current->next->prev = newListItem;
            } /* if */
            else
            {
                /* last item so move the tail */
                listItr->list->tail = newListItem;
            } /* else */

            listItr->current->next = newListItem;
        } /* else */

        /* increment number of items */
        listItr->list->count ++;
        return TRUE;
    } /* if */
    else
    {
        return FALSE;
    } /* else */

} /* ListItrInsertAfter */


/****************************************************************************
//
// PROCEDURE:   ListItrInsertBefore()
//
// PARAMETERS:
//  listItr     Pointer to a LISTITR
//
//  data        Pointer to data item to be iserted
//
// DESCRIPTION:
//              Inserts the data item before the data item pointed by
//              "current" pointer.
//
// RETURNS:
//              TRUE on success. FALSE if calloc fails
//
****************************************************************************/

BOOL ListItrInsertBefore(LPLISTITR listItr, void * data)
{
    LPLISTITEM newListItem;

    assert(listItr);
    assert(data);
    assert(((unsigned int)listItr->current) != FIRST_TIME);

    newListItem = (LPLISTITEM) calloc (1, sizeof(LISTITEM));

    if (newListItem)
    {
        newListItem->data = data;

        if (listItr->current == NULL)
        {
            if (listItr->list->count == 0)
            {
                /* there are no items right now */
                listItr->list->head = listItr->list->tail = newListItem;
                listItr->current = listItr->list->head;
            } /* if */
            else
            {
                /* there are some items, current points to null */
                newListItem->prev = listItr->list->tail;
                newListItem->next = NULL;
                listItr->list->tail->next = newListItem;
                listItr->list->tail = newListItem;
                listItr->current = newListItem;
            } /* else */
        } /* if */
        else
        {
            newListItem->next = listItr->current;
            newListItem->prev = listItr->current->prev;
            
            if (listItr->current->prev)
            {
                /* there are items before */
                listItr->current->prev->next = newListItem;
            } /* if */
            else
            {
                /* first item so move the head */
                listItr->list->head = newListItem;
            } /* else */

            listItr->current->prev = newListItem;
        } /* else */

        /* increment number of items */
        listItr->list->count ++;
        return TRUE;
    } /* if */
    else
    {
        return FALSE;
    } /* else */

} /* ListItrInsertBefore */


/****************************************************************************
//
// PROCEDURE:   ListItrRemoveItem()
//
// PARAMETERS:
//  listItr     Pointer to a LISTITR
//
// DESCRIPTION:
//              Removes the data item pointed by "current" pointer.
//
// RETURNS:
//              TRUE on success. FLASE if current is invalid.
//
****************************************************************************/

BOOL ListItrRemoveItem(LPLISTITR listItr)
{
    LPLISTITEM tmpItem;

    assert(listItr);
    assert(listItr->current);

    /* not yet iterated */
    if (listItr->current == (LPLISTITEM) FIRST_TIME)
    {
        return FALSE;
    }

    if (listItr->current->next == NULL && listItr->current->prev == NULL)
    {
        /* this is the only item */
        listItr->list->head = listItr->list->tail = NULL;
        tmpItem = NULL;
    }
    else if (listItr->current->next == NULL)
    {
        /* this is the last item */
        listItr->list->tail = listItr->current->prev;
        listItr->current->prev->next = NULL;
        tmpItem = listItr->current->prev;
    }
    else if (listItr->current->prev == NULL)
    {
        /* this is the first item */
        listItr->list->head = listItr->current->next;
        listItr->current->next->prev = NULL;
        tmpItem = (LPLISTITEM) FIRST_TIME;
    }
    else
    {
        /* somewhere in the middle */
        listItr->current->next->prev = listItr->current->prev;
        listItr->current->prev->next = listItr->current->next;
        tmpItem = listItr->current->prev;
    } /* if */

    free(listItr->current);
    listItr->current = tmpItem;
    listItr->list->count --;

    return TRUE;

} /* ListItrRemove */


/****************************************************************************
//
// PROCEDURE:   ListItrDelete()
//
// PARAMETERS:
//  listItr     Pointer to a LISTITR
//
// DESCRIPTION:
//              Frees the LISTITR
//
// RETURNS:
//              None
//
****************************************************************************/

void ListItrDelete(LPLISTITR listItr)
{
    assert(listItr);

#ifdef _LISTDEBUG
    CSTRACE( (CSTRACE_INFO, "ListItrDelete %X\n",listItr) );
#endif // _LISTDEBUG

    free(listItr);
} /* ListItrDelete */
