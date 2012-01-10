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

#ifndef _WSC_TLV_H
#define _WSC_TLV_H

#include <stdexcept>
#include "Portability.h"
#include "WscError.h"

#ifndef UNDER_CE//to be used for everything other than WinCE
    #ifndef __unaligned
    #define __unaligned
    #endif
#endif

#pragma pack(push, 1)

//Declare TLV header as extern, since it will be defined elsewhere
typedef struct {
    uint16    attributeType;
    uint16    dataLength;
}S_WSC_TLV_HEADER;

/******************************************************************************
 *                             Buffer class                                   *
 ******************************************************************************/
#define BUF_BLOCK_SIZE 256
class BufferObj
{
private:
    uint8 *pBase, *pCurrent;
    uint32 m_bufferLength, m_currentLength, m_dataLength;
    bool m_allocated;
public:
    //Default constructor for Serealizing operations
    BufferObj();

    //Overloaded constructor for Deserialize operations
    BufferObj(uint8 *ptr, uint32 length)
        :pBase(ptr), 
         pCurrent(ptr), 
         m_bufferLength(length),//gives the buffer's length 
         m_currentLength(0), //gives the offset of the pCurrent ptr
         m_dataLength(length),//gives the length of the data stored
         m_allocated(false) {}

    uint8 *Advance(uint32 offset);
    uint8 *Pos() { return pCurrent;  }
    uint32 Length() { return m_dataLength; }
    uint32 Remaining() { return m_bufferLength - m_currentLength;  }
    uint8 *Append(uint32 length, uint8 *pBuff);
    uint8 *GetBuf() { return pBase; }
    uint8 *Set(uint8 *pos);
    uint16 NextType();
    uint8 *Reset();
    uint8 *Rewind(uint32 length);
    uint8 *Rewind();

    ~BufferObj()
    {
        if((m_allocated) && (pBase))
        {
            free(pBase);
            pBase = NULL;
            m_allocated = false;
        }
    }
};

/******************************************************************************
 *                                TLV Base class                              *
 ******************************************************************************/
class tlvbase
{
public:
    uint16 Type() { return m_type; }
    uint16 Length() { return m_len; }

protected:
    uint16 m_type;
    uint16 m_len;
    uint8 * m_pos;

    //Deserializing constructors
    tlvbase(uint16 theType, BufferObj & theBuf, uint16 minDataSize);
    tlvbase(uint16 theType, BufferObj & theBuf, uint16 minDataSize, 
            uint16 maxDataSize);
    //Serializing constructor
    tlvbase(uint16 theType, BufferObj & theBuf, uint16 dataSize, uint8 *data);
    tlvbase() : m_len(0), m_pos(NULL) { };

    void serialize(BufferObj &theBuf, uint8 *data);
};

/******************************************************************************
 *                          TLV Template class: TlvObj                        *
 ******************************************************************************/
template <class T> class TlvObj: public tlvbase
{
public:
    TlvObj():m_data(0) {};
    TlvObj(uint16 wscType, BufferObj &buf)
        :tlvbase(wscType, buf, sizeof(T))
    {   
        m_data = *(__unaligned T *)m_pos;

        if(m_len == sizeof(uint32))
        {
            m_data = WscNtohl(m_data);
        }
        else if(m_len == sizeof(uint16))
        {
            m_data = WscNtohs(m_data);
        }
        else
        {
           ;
        }
        
        return;
    }

    TlvObj(uint16 wscType, BufferObj &buf, T *data)
        :tlvbase(wscType, buf, sizeof(T), (uint8 *)data), m_data(*(__unaligned T *)data)
    {
        //The data has already been stored. 
        //Now convert it to network byte order as appropriate
        if(m_len == sizeof(uint32))
        {
            *(__unaligned uint32 *)m_pos = WscHtonl(*(__unaligned uint32 *)m_pos);
        }
        else if(m_len == sizeof(uint16))
        {
#ifdef UNDER_CE
            //Band-aid to avoid crashes
            uint16 temp;
            memcpy(&temp, m_pos, sizeof(uint16));
            temp = WscHtons(temp);
            memcpy(m_pos, &temp, sizeof(uint16));
#else
            *(uint16 *)m_pos = WscHtons(*(uint16 *)m_pos);
#endif
        }
        else
        {
           ;
        }

        return;
    }

    
    T Value() { return m_data; }
    void Set(uint16 wscType, T value)
    {
        m_type = wscType;
        m_len = sizeof(T);
        m_data = value;
    }

    void Write(BufferObj &theBuf)
    {
        serialize(theBuf, (uint8 *)&m_data);

        //The data has been stored. Now convert it to network byte order as appropriate
        if(m_len == sizeof(uint32))
            *(__unaligned uint32 *)m_pos = WscHtonl(*(__unaligned uint32 *)m_pos);
        else if(m_len == sizeof(uint16))
            *(__unaligned uint16 *)m_pos = WscHtons(*(__unaligned uint16 *)m_pos);
    }
private:
    T m_data;
};

/******************************************************************************
 *                        TLV Template class: TlvPtrObj                       *
 ******************************************************************************/
template <class T> class TlvPtrObj: public tlvbase
{
public:
    TlvPtrObj(bool allocate = false):m_data((T)0), m_allocated(allocate) {};

    //Serializing (writing) constructor
    TlvPtrObj(uint16 wscType, BufferObj &buf, T data, uint16 length)
            :tlvbase(wscType, buf, length, (__unaligned uint8 *)data),
             m_data(data), m_allocated(false)
    { m_data = (T) m_pos; }

    //Deserializing (Parsing) constructors
    //TlvPtrObj(uint16 wscType, BufferObj &buf, uint16 maxLength)
    //        :tlvbase(wscType, buf, sizeof(T), maxLength) 
    //{ m_data = (T) m_pos; }

    TlvPtrObj(uint16 wscType, BufferObj &buf,
              uint16 maxLength = 0, bool allocate = false)
            :tlvbase(wscType, buf, sizeof(*m_data), maxLength), m_allocated(allocate) 
    { 
        if(m_allocated)
        {
            m_data = (T) new uint8[m_len];
            if(!m_data)
                throw WSC_ERR_OUTOFMEMORY;
            memcpy(m_data, m_pos, m_len);
        }
        else
        {
            m_data = (T) m_pos; 
        }
    }

    //TlvPtrObj(uint16 wscType, BufferObj &buf)
    //        :tlvbase(wscType, buf, sizeof(T))
    //{ m_data = (T) m_pos; }

    //TlvPtrObj(uint16 wscType, BufferObj &buf, boolean allocate = false)
    //        :tlvbase(wscType, buf, sizeof(T))
    //{ m_data = (T) m_pos; }
    
    //Destructor
    ~TlvPtrObj()
    {
        if(m_allocated && m_data)
        {
            delete [] (__unaligned uint8 *)m_data;
            m_data = (T)0;
        }
    }

    T Value() { return m_data; }

    bool Allocated() { return m_allocated; }

    void Set(uint16 wscType, T value, uint16 length)
    {
        if(NULL == value)
            throw WSC_ERR_INVALID_PARAMETERS;

        m_type = wscType;
        m_len = length;
        m_data = value;
    }

    void Write(BufferObj &theBuf) { serialize(theBuf, (uint8 *)m_data); }

#ifdef WIN32
    TlvPtrObj & operator= (TlvPtrObj &O)
#else
    TlvPtrObj & operator= (TlvPtrObj O)
#endif
    {
        m_type = O.Type();
        m_len = O.Length();
        m_allocated = O.Allocated();

        if(m_allocated)
        {
            m_data = (T) new uint8[m_len];
            if(!m_data)
                throw WSC_ERR_OUTOFMEMORY;
            memcpy(m_data, O.Value(), m_len);
        }
        else
        {
            m_data = O.Value();
        }
        return *this;
    }

private:
    T m_data;
    bool m_allocated;
};

/******************************************************************************
 *                         Base class for complex TLVs                        *
 ******************************************************************************/
class cplxtlvbase
{
public:
    uint16 Length() { return m_len; }
    uint16 Type() { return m_type; }
protected:
    uint16 m_len;
    uint8 * m_pos;
    uint16 m_type;

    void parseHdr(uint16 theType, BufferObj & theBuf, uint16 minDataSize);
    void writeHdr(uint16 theType, uint16 length, BufferObj & theBuf);
};

#pragma pack(pop)

#endif //WSC_TLV_H

