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

#include "WscCommon.h"
#include "WscError.h"
#include "WscTlvBase.h"
#include "tutrace.h"

/******************************************************************************
 *                           Buffer class methods                             *
 ******************************************************************************/

BufferObj::BufferObj()
        :pBase(NULL), 
         m_bufferLength(BUF_BLOCK_SIZE), 
         m_currentLength(0), 
         m_dataLength(0), 
         m_allocated(true)
{
    pBase = (uint8 *) malloc(m_bufferLength);
    if(!pBase)
        throw "memory allocate error";
    pCurrent = pBase;
}

uint8 * BufferObj::Advance(uint32 offset)
{
    //Advance the pCurrent pointer. update current length
    //Don't disturb the dataLength variable here
    if(m_currentLength+offset > m_bufferLength)
        return NULL;
    
    m_currentLength += offset;
    pCurrent+=offset;
    return pCurrent;
}

uint8 * 
BufferObj::Append(uint32 length, uint8 *pBuff)
{
    if((pBuff == NULL) || (length == 0))
        return pCurrent;

    //IMPORTANT: if this buffer was not allocated by us
    //and points to an existing buffer, then we should be extremely careful
    //how much data we append
    if((!m_allocated) && (Remaining() < length))
    {
        TUTRACE((TUTRACE_INFO, "TLV: Explicitly allocating memory\n"));

        //now we need to explicitly allocate memory. 
        //while in the process, allocate some extra mem
        uint8 *ptr = (uint8 *) malloc(m_bufferLength+BUF_BLOCK_SIZE);
        if(!ptr)
            throw "memory allocate error";

        //copy the existing data
        memcpy(ptr, pBase, m_currentLength);

        //update internal variables
        pBase = ptr;
        pCurrent = pBase + m_currentLength;
        m_bufferLength += BUF_BLOCK_SIZE;
        m_allocated = true;
    }

    if(m_bufferLength - m_currentLength < length)
    {
        //the available bufferspace isn't sufficient for the current data

        //determine how much more space we need, either the block size or
        //the data length, whichever is bigger
        int tempLen = (length>BUF_BLOCK_SIZE)?length:BUF_BLOCK_SIZE;

        //Allocate more memory
        //pBase = (uint8 *)realloc(pBase, m_bufferLength+tempLen);
        pBase = (uint8 *)realloc(pBase, m_currentLength+tempLen);
        if(!pBase)
            throw "Realloc error";

        //m_bufferLength += BUF_BLOCK_SIZE;
        m_bufferLength = (m_currentLength+tempLen);
        pCurrent = pBase + m_currentLength;
    }
    
    memcpy(pCurrent, pBuff, length);
    pCurrent += length;
    m_currentLength += length;

    //the data length needs to be updated based on the pointer locations,
    //since the pointers could have been moved around (using rewind and
    //advance) before the call to append.
    m_dataLength = pCurrent - pBase;

    return pCurrent-length; //return the location at which the data was copied
}

uint8 *
BufferObj::Set(uint8 *pos) 
{
    if(pos < pBase)
        throw "Buffer underflow";
    if(pos > pBase + m_bufferLength)
        throw "Buffer overflow";
    
    //Perform operation as if pos lies before pCurrent. 
    //If it lies after pCurrent, offset will be negative, so we'll be OK
    int32 offset = (int32)(pCurrent-pos);
    pCurrent = pos;
    m_currentLength -= offset;
    return pCurrent;
}

uint16 
BufferObj::NextType() 
{
    if(Remaining() < 4)//at least size of the header
        return 0;

    return WscNtohs(*(__unaligned uint16 *)pCurrent); 
}

uint8 *BufferObj::Reset()
{
    pCurrent = pBase;
    m_currentLength = m_dataLength = 0;
    return pBase;
}

uint8 *BufferObj::Rewind(uint32 length)
{
    if(length > m_currentLength)
        throw "Buffer underflow error";

    m_currentLength-=length;
    pCurrent = pBase + m_currentLength;
    return pCurrent;
}

uint8 *BufferObj::Rewind()
{
    m_currentLength = 0;
    pCurrent = pBase;
    return pCurrent;
}
/******************************************************************************
 *                             TLV Base class                                 *
 ******************************************************************************/
//Deserealizing constructor
tlvbase::tlvbase(uint16 theType, BufferObj & theBuf, uint16 minDataSize) 
        : m_pos(theBuf.Pos()) 
{ 

    uint32 remaining = theBuf.Remaining();
    if ( remaining < sizeof(S_WSC_TLV_HEADER) + minDataSize)
        throw "insufficient buffer size";

    if (theType != WscNtohs(*((__unaligned uint16 *) m_pos)) )
    {
        TUTRACE((TUTRACE_ERR, "TLVBASE: Expected TLV type: %d, received: %d", 
                theType, WscNtohs(*((__unaligned uint16 *) m_pos)) ));
        throw "unexpected type";
    }

    m_type = theType;

    m_pos += sizeof(uint16 ); // advance to length field

    m_len = WscNtohs(*((__unaligned uint16 *) m_pos));
    if (minDataSize > m_len) 
        throw "length field too small";
    
    if (m_len + sizeof(S_WSC_TLV_HEADER) > remaining) 
        throw "buffer overflow error";
    
    m_pos += sizeof(uint16 ); // advance to data field
    theBuf.Advance( 2 * sizeof(uint16) + m_len);
}

tlvbase::tlvbase(uint16 theType, 
                 BufferObj & theBuf, 
                 uint16 minDataSize, 
                 uint16 maxDataSize) 
        : m_pos(theBuf.Pos()) 
{ 

    uint32 remaining = theBuf.Remaining();
    if ( remaining < sizeof(S_WSC_TLV_HEADER) + minDataSize)
        throw "insufficient buffer size";

    if (theType != WscNtohs(*((__unaligned uint16 *) m_pos)) ) 
    {
        TUTRACE((TUTRACE_ERR, "TLVBASE: Expected TLV type: %d, received: %d", 
                theType, WscNtohs(*((__unaligned uint16 *) m_pos)) ));
        throw "unexpected type";
    }

    m_type = theType;

    m_pos += sizeof(uint16); // advance to length field

    m_len = WscNtohs(*((__unaligned uint16 *) m_pos));
    if (m_len < minDataSize) 
        throw "length field too small";
    
    if (maxDataSize && (m_len > maxDataSize))
        throw "length greater than expectated";

    if (m_len + sizeof(S_WSC_TLV_HEADER) > remaining) 
        throw "buffer overflow error";
    
    m_pos += sizeof(uint16 ); // advance to data field
    theBuf.Advance( 2 * sizeof(uint16) + m_len);
}

//Serealizing constructor
tlvbase::tlvbase(uint16 theType, 
                 BufferObj & theBuf, 
                 uint16 dataSize, 
                 uint8 *data)
        :m_type(theType), m_len(dataSize)
{
    serialize(theBuf, data);
}

void tlvbase::serialize(BufferObj &theBuf, uint8 *data)
{
    uint8 temp[sizeof(uint32)];

    if((NULL == data) || (0 == m_len))
        throw WSC_ERR_INVALID_PARAMETERS;
    
    //Copy the type
    *(unsigned short *)temp = WscHtons(m_type);
    theBuf.Append(sizeof(uint16 ), temp);
    
    //Copy the length
    *(unsigned short *)temp = WscHtons(m_len);
    theBuf.Append(sizeof(uint16), temp);

    //Copy the value
    m_pos = theBuf.Append(m_len, data);
}

/******************************************************************************
 *                       Base class for complex TLVs                          *
 ******************************************************************************/
void 
cplxtlvbase::parseHdr(uint16 theType, BufferObj & theBuf, uint16 minDataSize) 
{ 
    //Extracts the type and the length. Positions m_pos to point to the data
    m_pos = theBuf.Pos();
    uint32 remaining = theBuf.Remaining();
    if ( remaining < sizeof(S_WSC_TLV_HEADER) + minDataSize)
        throw "insufficient buffer size";

    if (theType != WscNtohs(*((uint16 *) m_pos)) ) 
        throw "unexpected type";

    m_type = theType;

    m_pos += sizeof(uint16 ); // advance to length field

    m_len = WscNtohs(*((uint16 *) m_pos));
    if (minDataSize > m_len) 
        throw "length field too small";
    
    if (m_len + sizeof(S_WSC_TLV_HEADER) > remaining) 
        throw "buffer overflow error";
    
    m_pos += sizeof(uint16 ); // advance to data field
    theBuf.Advance( 2 * sizeof(uint16 ));
}

void 
cplxtlvbase::writeHdr(uint16 theType, uint16 length, BufferObj & theBuf)
{
    //serializes the type and length. 
    //Positions m_pos to point to the start of length
    uint8 temp[sizeof(uint32)];

    m_type = theType;
    m_len = length;

    //Copy the Type
    *(uint16 *)temp = WscHtons(m_type);
    theBuf.Append(sizeof(uint16), temp);
    
    //Copy the length
    *(uint16 *)temp = WscHtons(m_len);
    m_pos = theBuf.Append(sizeof(uint16), temp);
}

