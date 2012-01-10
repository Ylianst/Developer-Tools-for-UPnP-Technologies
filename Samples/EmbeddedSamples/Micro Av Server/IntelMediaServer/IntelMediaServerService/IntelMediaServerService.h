
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0347 */
/* at Fri Jul 11 13:53:26 2003
 */
/* Compiler settings for IntelMediaServerService.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __IntelMediaServerService_h__
#define __IntelMediaServerService_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IIntelMediaServerConfig_FWD_DEFINED__
#define __IIntelMediaServerConfig_FWD_DEFINED__
typedef interface IIntelMediaServerConfig IIntelMediaServerConfig;
#endif 	/* __IIntelMediaServerConfig_FWD_DEFINED__ */


#ifndef ___IIntelMediaServerConfigEvents_FWD_DEFINED__
#define ___IIntelMediaServerConfigEvents_FWD_DEFINED__
typedef interface _IIntelMediaServerConfigEvents _IIntelMediaServerConfigEvents;
#endif 	/* ___IIntelMediaServerConfigEvents_FWD_DEFINED__ */


#ifndef __IntelMediaServerConfig_FWD_DEFINED__
#define __IntelMediaServerConfig_FWD_DEFINED__

#ifdef __cplusplus
typedef class IntelMediaServerConfig IntelMediaServerConfig;
#else
typedef struct IntelMediaServerConfig IntelMediaServerConfig;
#endif /* __cplusplus */

#endif 	/* __IntelMediaServerConfig_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IIntelMediaServerConfig_INTERFACE_DEFINED__
#define __IIntelMediaServerConfig_INTERFACE_DEFINED__

/* interface IIntelMediaServerConfig */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IIntelMediaServerConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C86E5F5A-876F-44EB-87EF-8246CECF2D4D")
    IIntelMediaServerConfig : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetVersionInfo( 
            /* [retval][out] */ BSTR *info) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSharedFolders( 
            /* [retval][out] */ BSTR *Folders) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddSharedFolder( 
            /* [in] */ BSTR Folder) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RemoveSharedFolder( 
            /* [in] */ BSTR Folder) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IIntelMediaServerConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IIntelMediaServerConfig * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IIntelMediaServerConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IIntelMediaServerConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IIntelMediaServerConfig * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IIntelMediaServerConfig * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IIntelMediaServerConfig * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IIntelMediaServerConfig * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetVersionInfo )( 
            IIntelMediaServerConfig * This,
            /* [retval][out] */ BSTR *info);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSharedFolders )( 
            IIntelMediaServerConfig * This,
            /* [retval][out] */ BSTR *Folders);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddSharedFolder )( 
            IIntelMediaServerConfig * This,
            /* [in] */ BSTR Folder);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *RemoveSharedFolder )( 
            IIntelMediaServerConfig * This,
            /* [in] */ BSTR Folder);
        
        END_INTERFACE
    } IIntelMediaServerConfigVtbl;

    interface IIntelMediaServerConfig
    {
        CONST_VTBL struct IIntelMediaServerConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IIntelMediaServerConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IIntelMediaServerConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IIntelMediaServerConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IIntelMediaServerConfig_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IIntelMediaServerConfig_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IIntelMediaServerConfig_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IIntelMediaServerConfig_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IIntelMediaServerConfig_GetVersionInfo(This,info)	\
    (This)->lpVtbl -> GetVersionInfo(This,info)

#define IIntelMediaServerConfig_GetSharedFolders(This,Folders)	\
    (This)->lpVtbl -> GetSharedFolders(This,Folders)

#define IIntelMediaServerConfig_AddSharedFolder(This,Folder)	\
    (This)->lpVtbl -> AddSharedFolder(This,Folder)

#define IIntelMediaServerConfig_RemoveSharedFolder(This,Folder)	\
    (This)->lpVtbl -> RemoveSharedFolder(This,Folder)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IIntelMediaServerConfig_GetVersionInfo_Proxy( 
    IIntelMediaServerConfig * This,
    /* [retval][out] */ BSTR *info);


void __RPC_STUB IIntelMediaServerConfig_GetVersionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IIntelMediaServerConfig_GetSharedFolders_Proxy( 
    IIntelMediaServerConfig * This,
    /* [retval][out] */ BSTR *Folders);


void __RPC_STUB IIntelMediaServerConfig_GetSharedFolders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IIntelMediaServerConfig_AddSharedFolder_Proxy( 
    IIntelMediaServerConfig * This,
    /* [in] */ BSTR Folder);


void __RPC_STUB IIntelMediaServerConfig_AddSharedFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IIntelMediaServerConfig_RemoveSharedFolder_Proxy( 
    IIntelMediaServerConfig * This,
    /* [in] */ BSTR Folder);


void __RPC_STUB IIntelMediaServerConfig_RemoveSharedFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IIntelMediaServerConfig_INTERFACE_DEFINED__ */



#ifndef __IntelMediaServerServiceLib_LIBRARY_DEFINED__
#define __IntelMediaServerServiceLib_LIBRARY_DEFINED__

/* library IntelMediaServerServiceLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_IntelMediaServerServiceLib;

#ifndef ___IIntelMediaServerConfigEvents_DISPINTERFACE_DEFINED__
#define ___IIntelMediaServerConfigEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IIntelMediaServerConfigEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IIntelMediaServerConfigEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("57C98A04-E827-4BCD-B288-0DA049AF1258")
    _IIntelMediaServerConfigEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IIntelMediaServerConfigEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IIntelMediaServerConfigEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IIntelMediaServerConfigEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IIntelMediaServerConfigEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IIntelMediaServerConfigEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IIntelMediaServerConfigEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IIntelMediaServerConfigEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IIntelMediaServerConfigEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IIntelMediaServerConfigEventsVtbl;

    interface _IIntelMediaServerConfigEvents
    {
        CONST_VTBL struct _IIntelMediaServerConfigEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IIntelMediaServerConfigEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IIntelMediaServerConfigEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IIntelMediaServerConfigEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IIntelMediaServerConfigEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IIntelMediaServerConfigEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IIntelMediaServerConfigEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IIntelMediaServerConfigEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IIntelMediaServerConfigEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_IntelMediaServerConfig;

#ifdef __cplusplus

class DECLSPEC_UUID("8A879A58-EE07-4F3D-B92D-3BE14C9C541A")
IntelMediaServerConfig;
#endif
#endif /* __IntelMediaServerServiceLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


