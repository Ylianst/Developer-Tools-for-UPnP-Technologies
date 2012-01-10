/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.02.88 */
/* at Wed Apr 10 11:24:15 2002
 */
/* Compiler settings for PlayerOCX.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __PlayerOCX_h__
#define __PlayerOCX_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IWMP_FWD_DEFINED__
#define __IWMP_FWD_DEFINED__
typedef interface IWMP IWMP;
#endif 	/* __IWMP_FWD_DEFINED__ */


#ifndef ___IWMPEvents_FWD_DEFINED__
#define ___IWMPEvents_FWD_DEFINED__
typedef interface _IWMPEvents _IWMPEvents;
#endif 	/* ___IWMPEvents_FWD_DEFINED__ */


#ifndef __WMP_FWD_DEFINED__
#define __WMP_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMP WMP;
#else
typedef struct WMP WMP;
#endif /* __cplusplus */

#endif 	/* __WMP_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL_itf_PlayerOCX_0000
 * at Wed Apr 10 11:24:15 2002
 * using MIDL 3.02.88
 ****************************************/
/* [local] */ 


//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Player Control for Pocket PC
//  Copyright (C) 2001-2002 Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------
typedef /* [public][uuid] */ DWORD VB_OLE_COLOR;



extern RPC_IF_HANDLE __MIDL_itf_PlayerOCX_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_PlayerOCX_0000_v0_0_s_ifspec;

#ifndef __IWMP_INTERFACE_DEFINED__
#define __IWMP_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IWMP
 * at Wed Apr 10 11:24:15 2002
 * using MIDL 3.02.88
 ****************************************/
/* [unique][helpstring][dual][uuid][object] */ 



EXTERN_C const IID IID_IWMP;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("136B66EC-F30D-46A8-88DD-F2D055163E49")
    IWMP : public IDispatch
    {
    public:
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_AutoSize( 
            /* [in] */ VARIANT_BOOL vbool) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_AutoSize( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbool) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_BorderStyle( 
            /* [in] */ long style) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_BorderStyle( 
            /* [retval][out] */ long __RPC_FAR *pstyle) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_Enabled( 
            /* [in] */ VARIANT_BOOL vbool) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_Enabled( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbool) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FileName( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FileName( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Volume( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Volume( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Mute( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Mute( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AutoStart( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AutoStart( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlayCount( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PlayCount( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowStatusBar( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowStatusBar( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowAudioControls( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowAudioControls( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowCaptioning( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowCaptioning( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowControls( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowControls( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowDisplay( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowDisplay( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowGotoBar( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowGotoBar( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowPositionControls( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowPositionControls( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowTracker( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowTracker( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Startup( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Bandwidth( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BaseURL( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BaseURL( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BufferingCount( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BufferingProgress( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BufferingTime( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CanSeek( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CanSeekToMarkers( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ChannelDescription( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ChannelName( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ChannelURL( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClientID( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ConnectionSpeed( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ContactAddress( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ContactEmail( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ContactPhone( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentMarker( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CurrentMarker( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentPosition( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CurrentPosition( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DefaultFrame( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DefaultFrame( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Duration( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EntryCount( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ErrorCode( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ErrorDescription( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasError( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HasMultipleItems( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageSourceHeight( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageSourceWidth( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_InvokeURLs( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_InvokeURLs( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsBroadcast( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsDurationValid( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LostPackets( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MarkerCount( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_OpenState( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlayState( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PreviewMode( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PreviewMode( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReadyState( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReceivedPackets( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReceptionQuality( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RecoveredPackets( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SAMIFileName( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SAMIFileName( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SAMILang( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SAMILang( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SAMIStyle( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SAMIStyle( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SelectionEnd( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SelectionEnd( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SelectionStart( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SelectionStart( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SendErrorEvents( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SendErrorEvents( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SendKeyboardEvents( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SendKeyboardEvents( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SendMouseClickEvents( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SendMouseClickEvents( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SendMouseMoveEvents( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SendMouseMoveEvents( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SendOpenStateChangeEvents( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SendOpenStateChangeEvents( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SendPlayStateChangeEvents( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SendPlayStateChangeEvents( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SendWarningEvents( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SendWarningEvents( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SourceLink( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AboutBox( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCodecDescription( 
            /* [in] */ long nCodec,
            /* [retval][out] */ BSTR __RPC_FAR *pDescription) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCodecInstalled( 
            /* [in] */ BSTR __RPC_FAR *pstrCodec,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pIsInstalled) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCurrentEntry( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMarkerName( 
            /* [in] */ long nMarker,
            /* [retval][out] */ BSTR __RPC_FAR *pMarkerName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMarkerTime( 
            /* [in] */ long nMarker,
            /* [retval][out] */ double __RPC_FAR *pMarkerTime) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMediaInfoString( 
            /* [in] */ long MPMediaInfoType,
            /* [retval][out] */ BSTR __RPC_FAR *pstrMediaInfo) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Next( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Open( 
            BSTR pstrClip) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Play( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Previous( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Rate( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Rate( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisplaySize( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisplaySize( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SourceProtocol( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ErrorCorrection( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FinalConstruct( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AllowChangeDisplaySize( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AllowChangeDisplaySize( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AllowScan( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AllowScan( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnimationAtStart( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AnimationAtStart( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AudioStream( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AudioStream( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AutoRewind( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AutoRewind( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Balance( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Balance( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CanPreview( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CanScan( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CaptioningID( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClickToPlay( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ClickToPlay( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CodecCount( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CreationDate( 
            /* [retval][out] */ DATE __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CursorType( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CursorType( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisplayBackColor( 
            /* [retval][out] */ VB_OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisplayBackColor( 
            /* [in] */ VB_OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisplayForeColor( 
            /* [retval][out] */ VB_OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisplayForeColor( 
            /* [in] */ VB_OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisplayMode( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisplayMode( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableContextMenu( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableContextMenu( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableFullScreenControls( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableFullScreenControls( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnablePositionControls( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnablePositionControls( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnableTracker( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EnableTracker( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Language( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StreamCount( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TransparentAtStart( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TransparentAtStart( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VideoBorder3D( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VideoBorder3D( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VideoBorderColor( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VideoBorderColor( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VideoBorderWidth( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VideoBorderWidth( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FastForward( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FastReverse( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCodecURL( 
            /* [retval][out] */ BSTR __RPC_FAR *pstrCodecURL) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMediaParameter( 
            /* [in] */ long nParam,
            BSTR szParameterName,
            /* [retval][out] */ BSTR __RPC_FAR *pstrParameterValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMediaParameterName( 
            /* [in] */ long nParam,
            long nIndex,
            /* [retval][out] */ BSTR __RPC_FAR *pstrParameterName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMoreInfoURL( 
            /* [retval][out] */ BSTR __RPC_FAR *pstrMoreInfoURL) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetStreamGroup( 
            /* [retval][out] */ BSTR __RPC_FAR *pstrStreamGroup) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetStreamName( 
            /* [retval][out] */ BSTR __RPC_FAR *pstrStreamName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetStreamSelected( 
            /* [in] */ long nStream,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *fIsSelected) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsSoundCardEnabled( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *fIsEnabled) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetCurrentEntry( 
            long nValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowDialog( 
            long nValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StreamSelect( 
            long nSelect) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnWindowMessage( 
            UINT msg,
            WPARAM wParam,
            LPARAM lParam,
            LRESULT __RPC_FAR *plResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWMPVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMP __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMP __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMP __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWMP __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWMP __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWMP __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWMP __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AutoSize )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL vbool);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AutoSize )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbool);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BorderStyle )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long style);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BorderStyle )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pstyle);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Enabled )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL vbool);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Enabled )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbool);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FileName )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FileName )( 
            IWMP __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Volume )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Volume )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Mute )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Mute )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AutoStart )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AutoStart )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PlayCount )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PlayCount )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowStatusBar )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowStatusBar )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowAudioControls )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowAudioControls )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowCaptioning )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowCaptioning )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowControls )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowControls )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowDisplay )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowDisplay )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowGotoBar )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowGotoBar )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowPositionControls )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowPositionControls )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowTracker )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowTracker )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Startup )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Shutdown )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Bandwidth )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BaseURL )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BaseURL )( 
            IWMP __RPC_FAR * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BufferingCount )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BufferingProgress )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BufferingTime )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CanSeek )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CanSeekToMarkers )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ChannelDescription )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ChannelName )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ChannelURL )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ClientID )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ConnectionSpeed )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ContactAddress )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ContactEmail )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ContactPhone )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CurrentMarker )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CurrentMarker )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CurrentPosition )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CurrentPosition )( 
            IWMP __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DefaultFrame )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DefaultFrame )( 
            IWMP __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Duration )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EntryCount )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorCode )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorDescription )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HasError )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HasMultipleItems )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ImageSourceHeight )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ImageSourceWidth )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_InvokeURLs )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_InvokeURLs )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsBroadcast )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsDurationValid )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LostPackets )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MarkerCount )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_OpenState )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PlayState )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PreviewMode )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PreviewMode )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ReadyState )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ReceivedPackets )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ReceptionQuality )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RecoveredPackets )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SAMIFileName )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SAMIFileName )( 
            IWMP __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SAMILang )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SAMILang )( 
            IWMP __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SAMIStyle )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SAMIStyle )( 
            IWMP __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SelectionEnd )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SelectionEnd )( 
            IWMP __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SelectionStart )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SelectionStart )( 
            IWMP __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SendErrorEvents )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SendErrorEvents )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SendKeyboardEvents )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SendKeyboardEvents )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SendMouseClickEvents )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SendMouseClickEvents )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SendMouseMoveEvents )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SendMouseMoveEvents )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SendOpenStateChangeEvents )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SendOpenStateChangeEvents )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SendPlayStateChangeEvents )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SendPlayStateChangeEvents )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SendWarningEvents )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SendWarningEvents )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SourceLink )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AboutBox )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Cancel )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCodecDescription )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long nCodec,
            /* [retval][out] */ BSTR __RPC_FAR *pDescription);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCodecInstalled )( 
            IWMP __RPC_FAR * This,
            /* [in] */ BSTR __RPC_FAR *pstrCodec,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pIsInstalled);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentEntry )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMarkerName )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long nMarker,
            /* [retval][out] */ BSTR __RPC_FAR *pMarkerName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMarkerTime )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long nMarker,
            /* [retval][out] */ double __RPC_FAR *pMarkerTime);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMediaInfoString )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long MPMediaInfoType,
            /* [retval][out] */ BSTR __RPC_FAR *pstrMediaInfo);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IWMP __RPC_FAR * This,
            BSTR pstrClip);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Pause )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Play )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Previous )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stop )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Rate )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Rate )( 
            IWMP __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DisplaySize )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DisplaySize )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SourceProtocol )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorCorrection )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FinalConstruct )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AllowChangeDisplaySize )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AllowChangeDisplaySize )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AllowScan )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AllowScan )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AnimationAtStart )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AnimationAtStart )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AudioStream )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AudioStream )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AutoRewind )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AutoRewind )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Balance )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Balance )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CanPreview )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CanScan )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CaptioningID )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ClickToPlay )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ClickToPlay )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CodecCount )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CreationDate )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ DATE __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CursorType )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CursorType )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DisplayBackColor )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VB_OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DisplayBackColor )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VB_OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DisplayForeColor )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VB_OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DisplayForeColor )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VB_OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DisplayMode )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DisplayMode )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EnableContextMenu )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EnableContextMenu )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EnableFullScreenControls )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EnableFullScreenControls )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EnablePositionControls )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EnablePositionControls )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EnableTracker )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EnableTracker )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Language )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StreamCount )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TransparentAtStart )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TransparentAtStart )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VideoBorder3D )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_VideoBorder3D )( 
            IWMP __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VideoBorderColor )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_VideoBorderColor )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VideoBorderWidth )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_VideoBorderWidth )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FastForward )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FastReverse )( 
            IWMP __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCodecURL )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pstrCodecURL);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMediaParameter )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long nParam,
            BSTR szParameterName,
            /* [retval][out] */ BSTR __RPC_FAR *pstrParameterValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMediaParameterName )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long nParam,
            long nIndex,
            /* [retval][out] */ BSTR __RPC_FAR *pstrParameterName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMoreInfoURL )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pstrMoreInfoURL);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStreamGroup )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pstrStreamGroup);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStreamName )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pstrStreamName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStreamSelected )( 
            IWMP __RPC_FAR * This,
            /* [in] */ long nStream,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *fIsSelected);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSoundCardEnabled )( 
            IWMP __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *fIsEnabled);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCurrentEntry )( 
            IWMP __RPC_FAR * This,
            long nValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowDialog )( 
            IWMP __RPC_FAR * This,
            long nValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StreamSelect )( 
            IWMP __RPC_FAR * This,
            long nSelect);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnWindowMessage )( 
            IWMP __RPC_FAR * This,
            UINT msg,
            WPARAM wParam,
            LPARAM lParam,
            LRESULT __RPC_FAR *plResult);
        
        END_INTERFACE
    } IWMPVtbl;

    interface IWMP
    {
        CONST_VTBL struct IWMPVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMP_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMP_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMP_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMP_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMP_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMP_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMP_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMP_put_AutoSize(This,vbool)	\
    (This)->lpVtbl -> put_AutoSize(This,vbool)

#define IWMP_get_AutoSize(This,pbool)	\
    (This)->lpVtbl -> get_AutoSize(This,pbool)

#define IWMP_put_BorderStyle(This,style)	\
    (This)->lpVtbl -> put_BorderStyle(This,style)

#define IWMP_get_BorderStyle(This,pstyle)	\
    (This)->lpVtbl -> get_BorderStyle(This,pstyle)

#define IWMP_put_Enabled(This,vbool)	\
    (This)->lpVtbl -> put_Enabled(This,vbool)

#define IWMP_get_Enabled(This,pbool)	\
    (This)->lpVtbl -> get_Enabled(This,pbool)

#define IWMP_get_FileName(This,pVal)	\
    (This)->lpVtbl -> get_FileName(This,pVal)

#define IWMP_put_FileName(This,newVal)	\
    (This)->lpVtbl -> put_FileName(This,newVal)

#define IWMP_get_Volume(This,pVal)	\
    (This)->lpVtbl -> get_Volume(This,pVal)

#define IWMP_put_Volume(This,newVal)	\
    (This)->lpVtbl -> put_Volume(This,newVal)

#define IWMP_get_Mute(This,pVal)	\
    (This)->lpVtbl -> get_Mute(This,pVal)

#define IWMP_put_Mute(This,newVal)	\
    (This)->lpVtbl -> put_Mute(This,newVal)

#define IWMP_get_AutoStart(This,pVal)	\
    (This)->lpVtbl -> get_AutoStart(This,pVal)

#define IWMP_put_AutoStart(This,newVal)	\
    (This)->lpVtbl -> put_AutoStart(This,newVal)

#define IWMP_get_PlayCount(This,pVal)	\
    (This)->lpVtbl -> get_PlayCount(This,pVal)

#define IWMP_put_PlayCount(This,newVal)	\
    (This)->lpVtbl -> put_PlayCount(This,newVal)

#define IWMP_get_ShowStatusBar(This,pVal)	\
    (This)->lpVtbl -> get_ShowStatusBar(This,pVal)

#define IWMP_put_ShowStatusBar(This,newVal)	\
    (This)->lpVtbl -> put_ShowStatusBar(This,newVal)

#define IWMP_get_ShowAudioControls(This,pVal)	\
    (This)->lpVtbl -> get_ShowAudioControls(This,pVal)

#define IWMP_put_ShowAudioControls(This,newVal)	\
    (This)->lpVtbl -> put_ShowAudioControls(This,newVal)

#define IWMP_get_ShowCaptioning(This,pVal)	\
    (This)->lpVtbl -> get_ShowCaptioning(This,pVal)

#define IWMP_put_ShowCaptioning(This,newVal)	\
    (This)->lpVtbl -> put_ShowCaptioning(This,newVal)

#define IWMP_get_ShowControls(This,pVal)	\
    (This)->lpVtbl -> get_ShowControls(This,pVal)

#define IWMP_put_ShowControls(This,newVal)	\
    (This)->lpVtbl -> put_ShowControls(This,newVal)

#define IWMP_get_ShowDisplay(This,pVal)	\
    (This)->lpVtbl -> get_ShowDisplay(This,pVal)

#define IWMP_put_ShowDisplay(This,newVal)	\
    (This)->lpVtbl -> put_ShowDisplay(This,newVal)

#define IWMP_get_ShowGotoBar(This,pVal)	\
    (This)->lpVtbl -> get_ShowGotoBar(This,pVal)

#define IWMP_put_ShowGotoBar(This,newVal)	\
    (This)->lpVtbl -> put_ShowGotoBar(This,newVal)

#define IWMP_get_ShowPositionControls(This,pVal)	\
    (This)->lpVtbl -> get_ShowPositionControls(This,pVal)

#define IWMP_put_ShowPositionControls(This,newVal)	\
    (This)->lpVtbl -> put_ShowPositionControls(This,newVal)

#define IWMP_get_ShowTracker(This,pVal)	\
    (This)->lpVtbl -> get_ShowTracker(This,pVal)

#define IWMP_put_ShowTracker(This,newVal)	\
    (This)->lpVtbl -> put_ShowTracker(This,newVal)

#define IWMP_Startup(This)	\
    (This)->lpVtbl -> Startup(This)

#define IWMP_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#define IWMP_get_Bandwidth(This,pVal)	\
    (This)->lpVtbl -> get_Bandwidth(This,pVal)

#define IWMP_get_BaseURL(This,pVal)	\
    (This)->lpVtbl -> get_BaseURL(This,pVal)

#define IWMP_put_BaseURL(This,pVal)	\
    (This)->lpVtbl -> put_BaseURL(This,pVal)

#define IWMP_get_BufferingCount(This,pVal)	\
    (This)->lpVtbl -> get_BufferingCount(This,pVal)

#define IWMP_get_BufferingProgress(This,pVal)	\
    (This)->lpVtbl -> get_BufferingProgress(This,pVal)

#define IWMP_get_BufferingTime(This,pVal)	\
    (This)->lpVtbl -> get_BufferingTime(This,pVal)

#define IWMP_get_CanSeek(This,pVal)	\
    (This)->lpVtbl -> get_CanSeek(This,pVal)

#define IWMP_get_CanSeekToMarkers(This,pVal)	\
    (This)->lpVtbl -> get_CanSeekToMarkers(This,pVal)

#define IWMP_get_ChannelDescription(This,pVal)	\
    (This)->lpVtbl -> get_ChannelDescription(This,pVal)

#define IWMP_get_ChannelName(This,pVal)	\
    (This)->lpVtbl -> get_ChannelName(This,pVal)

#define IWMP_get_ChannelURL(This,pVal)	\
    (This)->lpVtbl -> get_ChannelURL(This,pVal)

#define IWMP_get_ClientID(This,pVal)	\
    (This)->lpVtbl -> get_ClientID(This,pVal)

#define IWMP_get_ConnectionSpeed(This,pVal)	\
    (This)->lpVtbl -> get_ConnectionSpeed(This,pVal)

#define IWMP_get_ContactAddress(This,pVal)	\
    (This)->lpVtbl -> get_ContactAddress(This,pVal)

#define IWMP_get_ContactEmail(This,pVal)	\
    (This)->lpVtbl -> get_ContactEmail(This,pVal)

#define IWMP_get_ContactPhone(This,pVal)	\
    (This)->lpVtbl -> get_ContactPhone(This,pVal)

#define IWMP_get_CurrentMarker(This,pVal)	\
    (This)->lpVtbl -> get_CurrentMarker(This,pVal)

#define IWMP_put_CurrentMarker(This,newVal)	\
    (This)->lpVtbl -> put_CurrentMarker(This,newVal)

#define IWMP_get_CurrentPosition(This,pVal)	\
    (This)->lpVtbl -> get_CurrentPosition(This,pVal)

#define IWMP_put_CurrentPosition(This,newVal)	\
    (This)->lpVtbl -> put_CurrentPosition(This,newVal)

#define IWMP_get_DefaultFrame(This,pVal)	\
    (This)->lpVtbl -> get_DefaultFrame(This,pVal)

#define IWMP_put_DefaultFrame(This,newVal)	\
    (This)->lpVtbl -> put_DefaultFrame(This,newVal)

#define IWMP_get_Duration(This,pVal)	\
    (This)->lpVtbl -> get_Duration(This,pVal)

#define IWMP_get_EntryCount(This,pVal)	\
    (This)->lpVtbl -> get_EntryCount(This,pVal)

#define IWMP_get_ErrorCode(This,pVal)	\
    (This)->lpVtbl -> get_ErrorCode(This,pVal)

#define IWMP_get_ErrorDescription(This,pVal)	\
    (This)->lpVtbl -> get_ErrorDescription(This,pVal)

#define IWMP_get_HasError(This,pVal)	\
    (This)->lpVtbl -> get_HasError(This,pVal)

#define IWMP_get_HasMultipleItems(This,pVal)	\
    (This)->lpVtbl -> get_HasMultipleItems(This,pVal)

#define IWMP_get_ImageSourceHeight(This,pVal)	\
    (This)->lpVtbl -> get_ImageSourceHeight(This,pVal)

#define IWMP_get_ImageSourceWidth(This,pVal)	\
    (This)->lpVtbl -> get_ImageSourceWidth(This,pVal)

#define IWMP_get_InvokeURLs(This,pVal)	\
    (This)->lpVtbl -> get_InvokeURLs(This,pVal)

#define IWMP_put_InvokeURLs(This,newVal)	\
    (This)->lpVtbl -> put_InvokeURLs(This,newVal)

#define IWMP_get_IsBroadcast(This,pVal)	\
    (This)->lpVtbl -> get_IsBroadcast(This,pVal)

#define IWMP_get_IsDurationValid(This,pVal)	\
    (This)->lpVtbl -> get_IsDurationValid(This,pVal)

#define IWMP_get_LostPackets(This,pVal)	\
    (This)->lpVtbl -> get_LostPackets(This,pVal)

#define IWMP_get_MarkerCount(This,pVal)	\
    (This)->lpVtbl -> get_MarkerCount(This,pVal)

#define IWMP_get_OpenState(This,pVal)	\
    (This)->lpVtbl -> get_OpenState(This,pVal)

#define IWMP_get_PlayState(This,pVal)	\
    (This)->lpVtbl -> get_PlayState(This,pVal)

#define IWMP_get_PreviewMode(This,pVal)	\
    (This)->lpVtbl -> get_PreviewMode(This,pVal)

#define IWMP_put_PreviewMode(This,newVal)	\
    (This)->lpVtbl -> put_PreviewMode(This,newVal)

#define IWMP_get_ReadyState(This,pVal)	\
    (This)->lpVtbl -> get_ReadyState(This,pVal)

#define IWMP_get_ReceivedPackets(This,pVal)	\
    (This)->lpVtbl -> get_ReceivedPackets(This,pVal)

#define IWMP_get_ReceptionQuality(This,pVal)	\
    (This)->lpVtbl -> get_ReceptionQuality(This,pVal)

#define IWMP_get_RecoveredPackets(This,pVal)	\
    (This)->lpVtbl -> get_RecoveredPackets(This,pVal)

#define IWMP_get_SAMIFileName(This,pVal)	\
    (This)->lpVtbl -> get_SAMIFileName(This,pVal)

#define IWMP_put_SAMIFileName(This,newVal)	\
    (This)->lpVtbl -> put_SAMIFileName(This,newVal)

#define IWMP_get_SAMILang(This,pVal)	\
    (This)->lpVtbl -> get_SAMILang(This,pVal)

#define IWMP_put_SAMILang(This,newVal)	\
    (This)->lpVtbl -> put_SAMILang(This,newVal)

#define IWMP_get_SAMIStyle(This,pVal)	\
    (This)->lpVtbl -> get_SAMIStyle(This,pVal)

#define IWMP_put_SAMIStyle(This,newVal)	\
    (This)->lpVtbl -> put_SAMIStyle(This,newVal)

#define IWMP_get_SelectionEnd(This,pVal)	\
    (This)->lpVtbl -> get_SelectionEnd(This,pVal)

#define IWMP_put_SelectionEnd(This,newVal)	\
    (This)->lpVtbl -> put_SelectionEnd(This,newVal)

#define IWMP_get_SelectionStart(This,pVal)	\
    (This)->lpVtbl -> get_SelectionStart(This,pVal)

#define IWMP_put_SelectionStart(This,newVal)	\
    (This)->lpVtbl -> put_SelectionStart(This,newVal)

#define IWMP_get_SendErrorEvents(This,pVal)	\
    (This)->lpVtbl -> get_SendErrorEvents(This,pVal)

#define IWMP_put_SendErrorEvents(This,newVal)	\
    (This)->lpVtbl -> put_SendErrorEvents(This,newVal)

#define IWMP_get_SendKeyboardEvents(This,pVal)	\
    (This)->lpVtbl -> get_SendKeyboardEvents(This,pVal)

#define IWMP_put_SendKeyboardEvents(This,newVal)	\
    (This)->lpVtbl -> put_SendKeyboardEvents(This,newVal)

#define IWMP_get_SendMouseClickEvents(This,pVal)	\
    (This)->lpVtbl -> get_SendMouseClickEvents(This,pVal)

#define IWMP_put_SendMouseClickEvents(This,newVal)	\
    (This)->lpVtbl -> put_SendMouseClickEvents(This,newVal)

#define IWMP_get_SendMouseMoveEvents(This,pVal)	\
    (This)->lpVtbl -> get_SendMouseMoveEvents(This,pVal)

#define IWMP_put_SendMouseMoveEvents(This,newVal)	\
    (This)->lpVtbl -> put_SendMouseMoveEvents(This,newVal)

#define IWMP_get_SendOpenStateChangeEvents(This,pVal)	\
    (This)->lpVtbl -> get_SendOpenStateChangeEvents(This,pVal)

#define IWMP_put_SendOpenStateChangeEvents(This,newVal)	\
    (This)->lpVtbl -> put_SendOpenStateChangeEvents(This,newVal)

#define IWMP_get_SendPlayStateChangeEvents(This,pVal)	\
    (This)->lpVtbl -> get_SendPlayStateChangeEvents(This,pVal)

#define IWMP_put_SendPlayStateChangeEvents(This,newVal)	\
    (This)->lpVtbl -> put_SendPlayStateChangeEvents(This,newVal)

#define IWMP_get_SendWarningEvents(This,pVal)	\
    (This)->lpVtbl -> get_SendWarningEvents(This,pVal)

#define IWMP_put_SendWarningEvents(This,newVal)	\
    (This)->lpVtbl -> put_SendWarningEvents(This,newVal)

#define IWMP_get_SourceLink(This,pVal)	\
    (This)->lpVtbl -> get_SourceLink(This,pVal)

#define IWMP_AboutBox(This)	\
    (This)->lpVtbl -> AboutBox(This)

#define IWMP_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#define IWMP_GetCodecDescription(This,nCodec,pDescription)	\
    (This)->lpVtbl -> GetCodecDescription(This,nCodec,pDescription)

#define IWMP_GetCodecInstalled(This,pstrCodec,pIsInstalled)	\
    (This)->lpVtbl -> GetCodecInstalled(This,pstrCodec,pIsInstalled)

#define IWMP_GetCurrentEntry(This,pVal)	\
    (This)->lpVtbl -> GetCurrentEntry(This,pVal)

#define IWMP_GetMarkerName(This,nMarker,pMarkerName)	\
    (This)->lpVtbl -> GetMarkerName(This,nMarker,pMarkerName)

#define IWMP_GetMarkerTime(This,nMarker,pMarkerTime)	\
    (This)->lpVtbl -> GetMarkerTime(This,nMarker,pMarkerTime)

#define IWMP_GetMediaInfoString(This,MPMediaInfoType,pstrMediaInfo)	\
    (This)->lpVtbl -> GetMediaInfoString(This,MPMediaInfoType,pstrMediaInfo)

#define IWMP_Next(This)	\
    (This)->lpVtbl -> Next(This)

#define IWMP_Open(This,pstrClip)	\
    (This)->lpVtbl -> Open(This,pstrClip)

#define IWMP_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IWMP_Play(This)	\
    (This)->lpVtbl -> Play(This)

#define IWMP_Previous(This)	\
    (This)->lpVtbl -> Previous(This)

#define IWMP_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IWMP_get_Rate(This,pVal)	\
    (This)->lpVtbl -> get_Rate(This,pVal)

#define IWMP_put_Rate(This,newVal)	\
    (This)->lpVtbl -> put_Rate(This,newVal)

#define IWMP_get_DisplaySize(This,pVal)	\
    (This)->lpVtbl -> get_DisplaySize(This,pVal)

#define IWMP_put_DisplaySize(This,newVal)	\
    (This)->lpVtbl -> put_DisplaySize(This,newVal)

#define IWMP_get_SourceProtocol(This,pVal)	\
    (This)->lpVtbl -> get_SourceProtocol(This,pVal)

#define IWMP_get_ErrorCorrection(This,pVal)	\
    (This)->lpVtbl -> get_ErrorCorrection(This,pVal)

#define IWMP_FinalConstruct(This)	\
    (This)->lpVtbl -> FinalConstruct(This)

#define IWMP_get_AllowChangeDisplaySize(This,pVal)	\
    (This)->lpVtbl -> get_AllowChangeDisplaySize(This,pVal)

#define IWMP_put_AllowChangeDisplaySize(This,newVal)	\
    (This)->lpVtbl -> put_AllowChangeDisplaySize(This,newVal)

#define IWMP_get_AllowScan(This,pVal)	\
    (This)->lpVtbl -> get_AllowScan(This,pVal)

#define IWMP_put_AllowScan(This,newVal)	\
    (This)->lpVtbl -> put_AllowScan(This,newVal)

#define IWMP_get_AnimationAtStart(This,pVal)	\
    (This)->lpVtbl -> get_AnimationAtStart(This,pVal)

#define IWMP_put_AnimationAtStart(This,newVal)	\
    (This)->lpVtbl -> put_AnimationAtStart(This,newVal)

#define IWMP_get_AudioStream(This,pVal)	\
    (This)->lpVtbl -> get_AudioStream(This,pVal)

#define IWMP_put_AudioStream(This,newVal)	\
    (This)->lpVtbl -> put_AudioStream(This,newVal)

#define IWMP_get_AutoRewind(This,pVal)	\
    (This)->lpVtbl -> get_AutoRewind(This,pVal)

#define IWMP_put_AutoRewind(This,newVal)	\
    (This)->lpVtbl -> put_AutoRewind(This,newVal)

#define IWMP_get_Balance(This,pVal)	\
    (This)->lpVtbl -> get_Balance(This,pVal)

#define IWMP_put_Balance(This,newVal)	\
    (This)->lpVtbl -> put_Balance(This,newVal)

#define IWMP_get_CanPreview(This,pVal)	\
    (This)->lpVtbl -> get_CanPreview(This,pVal)

#define IWMP_get_CanScan(This,pVal)	\
    (This)->lpVtbl -> get_CanScan(This,pVal)

#define IWMP_get_CaptioningID(This,pVal)	\
    (This)->lpVtbl -> get_CaptioningID(This,pVal)

#define IWMP_get_ClickToPlay(This,pVal)	\
    (This)->lpVtbl -> get_ClickToPlay(This,pVal)

#define IWMP_put_ClickToPlay(This,newVal)	\
    (This)->lpVtbl -> put_ClickToPlay(This,newVal)

#define IWMP_get_CodecCount(This,pVal)	\
    (This)->lpVtbl -> get_CodecCount(This,pVal)

#define IWMP_get_CreationDate(This,pVal)	\
    (This)->lpVtbl -> get_CreationDate(This,pVal)

#define IWMP_get_CursorType(This,pVal)	\
    (This)->lpVtbl -> get_CursorType(This,pVal)

#define IWMP_put_CursorType(This,newVal)	\
    (This)->lpVtbl -> put_CursorType(This,newVal)

#define IWMP_get_DisplayBackColor(This,pVal)	\
    (This)->lpVtbl -> get_DisplayBackColor(This,pVal)

#define IWMP_put_DisplayBackColor(This,newVal)	\
    (This)->lpVtbl -> put_DisplayBackColor(This,newVal)

#define IWMP_get_DisplayForeColor(This,pVal)	\
    (This)->lpVtbl -> get_DisplayForeColor(This,pVal)

#define IWMP_put_DisplayForeColor(This,newVal)	\
    (This)->lpVtbl -> put_DisplayForeColor(This,newVal)

#define IWMP_get_DisplayMode(This,pVal)	\
    (This)->lpVtbl -> get_DisplayMode(This,pVal)

#define IWMP_put_DisplayMode(This,newVal)	\
    (This)->lpVtbl -> put_DisplayMode(This,newVal)

#define IWMP_get_EnableContextMenu(This,pVal)	\
    (This)->lpVtbl -> get_EnableContextMenu(This,pVal)

#define IWMP_put_EnableContextMenu(This,newVal)	\
    (This)->lpVtbl -> put_EnableContextMenu(This,newVal)

#define IWMP_get_EnableFullScreenControls(This,pVal)	\
    (This)->lpVtbl -> get_EnableFullScreenControls(This,pVal)

#define IWMP_put_EnableFullScreenControls(This,newVal)	\
    (This)->lpVtbl -> put_EnableFullScreenControls(This,newVal)

#define IWMP_get_EnablePositionControls(This,pVal)	\
    (This)->lpVtbl -> get_EnablePositionControls(This,pVal)

#define IWMP_put_EnablePositionControls(This,newVal)	\
    (This)->lpVtbl -> put_EnablePositionControls(This,newVal)

#define IWMP_get_EnableTracker(This,pVal)	\
    (This)->lpVtbl -> get_EnableTracker(This,pVal)

#define IWMP_put_EnableTracker(This,newVal)	\
    (This)->lpVtbl -> put_EnableTracker(This,newVal)

#define IWMP_get_Language(This,pVal)	\
    (This)->lpVtbl -> get_Language(This,pVal)

#define IWMP_get_StreamCount(This,pVal)	\
    (This)->lpVtbl -> get_StreamCount(This,pVal)

#define IWMP_get_TransparentAtStart(This,pVal)	\
    (This)->lpVtbl -> get_TransparentAtStart(This,pVal)

#define IWMP_put_TransparentAtStart(This,newVal)	\
    (This)->lpVtbl -> put_TransparentAtStart(This,newVal)

#define IWMP_get_VideoBorder3D(This,pVal)	\
    (This)->lpVtbl -> get_VideoBorder3D(This,pVal)

#define IWMP_put_VideoBorder3D(This,newVal)	\
    (This)->lpVtbl -> put_VideoBorder3D(This,newVal)

#define IWMP_get_VideoBorderColor(This,pVal)	\
    (This)->lpVtbl -> get_VideoBorderColor(This,pVal)

#define IWMP_put_VideoBorderColor(This,newVal)	\
    (This)->lpVtbl -> put_VideoBorderColor(This,newVal)

#define IWMP_get_VideoBorderWidth(This,pVal)	\
    (This)->lpVtbl -> get_VideoBorderWidth(This,pVal)

#define IWMP_put_VideoBorderWidth(This,newVal)	\
    (This)->lpVtbl -> put_VideoBorderWidth(This,newVal)

#define IWMP_FastForward(This)	\
    (This)->lpVtbl -> FastForward(This)

#define IWMP_FastReverse(This)	\
    (This)->lpVtbl -> FastReverse(This)

#define IWMP_GetCodecURL(This,pstrCodecURL)	\
    (This)->lpVtbl -> GetCodecURL(This,pstrCodecURL)

#define IWMP_GetMediaParameter(This,nParam,szParameterName,pstrParameterValue)	\
    (This)->lpVtbl -> GetMediaParameter(This,nParam,szParameterName,pstrParameterValue)

#define IWMP_GetMediaParameterName(This,nParam,nIndex,pstrParameterName)	\
    (This)->lpVtbl -> GetMediaParameterName(This,nParam,nIndex,pstrParameterName)

#define IWMP_GetMoreInfoURL(This,pstrMoreInfoURL)	\
    (This)->lpVtbl -> GetMoreInfoURL(This,pstrMoreInfoURL)

#define IWMP_GetStreamGroup(This,pstrStreamGroup)	\
    (This)->lpVtbl -> GetStreamGroup(This,pstrStreamGroup)

#define IWMP_GetStreamName(This,pstrStreamName)	\
    (This)->lpVtbl -> GetStreamName(This,pstrStreamName)

#define IWMP_GetStreamSelected(This,nStream,fIsSelected)	\
    (This)->lpVtbl -> GetStreamSelected(This,nStream,fIsSelected)

#define IWMP_IsSoundCardEnabled(This,fIsEnabled)	\
    (This)->lpVtbl -> IsSoundCardEnabled(This,fIsEnabled)

#define IWMP_SetCurrentEntry(This,nValue)	\
    (This)->lpVtbl -> SetCurrentEntry(This,nValue)

#define IWMP_ShowDialog(This,nValue)	\
    (This)->lpVtbl -> ShowDialog(This,nValue)

#define IWMP_StreamSelect(This,nSelect)	\
    (This)->lpVtbl -> StreamSelect(This,nSelect)

#define IWMP_OnWindowMessage(This,msg,wParam,lParam,plResult)	\
    (This)->lpVtbl -> OnWindowMessage(This,msg,wParam,lParam,plResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_AutoSize_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL vbool);


void __RPC_STUB IWMP_put_AutoSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_AutoSize_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbool);


void __RPC_STUB IWMP_get_AutoSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_BorderStyle_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long style);


void __RPC_STUB IWMP_put_BorderStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_BorderStyle_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pstyle);


void __RPC_STUB IWMP_get_BorderStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_Enabled_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL vbool);


void __RPC_STUB IWMP_put_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_Enabled_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbool);


void __RPC_STUB IWMP_get_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_FileName_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_FileName_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IWMP_put_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_Volume_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_Volume_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IWMP_put_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_Mute_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_Mute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_Mute_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_Mute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_AutoStart_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_AutoStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_AutoStart_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_AutoStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_PlayCount_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_PlayCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_PlayCount_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IWMP_put_PlayCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ShowStatusBar_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ShowStatusBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_ShowStatusBar_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_ShowStatusBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ShowAudioControls_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ShowAudioControls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_ShowAudioControls_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_ShowAudioControls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ShowCaptioning_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ShowCaptioning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_ShowCaptioning_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_ShowCaptioning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ShowControls_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ShowControls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_ShowControls_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_ShowControls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ShowDisplay_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ShowDisplay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_ShowDisplay_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_ShowDisplay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ShowGotoBar_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ShowGotoBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_ShowGotoBar_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_ShowGotoBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ShowPositionControls_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ShowPositionControls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_ShowPositionControls_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_ShowPositionControls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ShowTracker_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ShowTracker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_ShowTracker_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_ShowTracker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_Startup_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_Startup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_Shutdown_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_Shutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_Bandwidth_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_Bandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_BaseURL_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_BaseURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_BaseURL_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ BSTR pVal);


void __RPC_STUB IWMP_put_BaseURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_BufferingCount_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_BufferingCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_BufferingProgress_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_BufferingProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_BufferingTime_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_BufferingTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_CanSeek_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_CanSeek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_CanSeekToMarkers_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_CanSeekToMarkers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ChannelDescription_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ChannelDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ChannelName_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ChannelName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ChannelURL_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ChannelURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ClientID_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ClientID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ConnectionSpeed_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ConnectionSpeed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ContactAddress_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ContactAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ContactEmail_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ContactEmail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ContactPhone_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ContactPhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_CurrentMarker_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_CurrentMarker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_CurrentMarker_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IWMP_put_CurrentMarker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_CurrentPosition_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_CurrentPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_CurrentPosition_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IWMP_put_CurrentPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_DefaultFrame_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_DefaultFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_DefaultFrame_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IWMP_put_DefaultFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_Duration_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_Duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_EntryCount_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_EntryCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ErrorCode_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ErrorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ErrorDescription_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ErrorDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_HasError_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_HasError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_HasMultipleItems_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_HasMultipleItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ImageSourceHeight_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ImageSourceHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ImageSourceWidth_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ImageSourceWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_InvokeURLs_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_InvokeURLs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_InvokeURLs_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_InvokeURLs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_IsBroadcast_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_IsBroadcast_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_IsDurationValid_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_IsDurationValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_LostPackets_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_LostPackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_MarkerCount_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_MarkerCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_OpenState_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_OpenState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_PlayState_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_PlayState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_PreviewMode_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_PreviewMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_PreviewMode_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_PreviewMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ReadyState_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ReadyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ReceivedPackets_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ReceivedPackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ReceptionQuality_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ReceptionQuality_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_RecoveredPackets_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_RecoveredPackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SAMIFileName_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SAMIFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SAMIFileName_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IWMP_put_SAMIFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SAMILang_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SAMILang_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SAMILang_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IWMP_put_SAMILang_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SAMIStyle_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SAMIStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SAMIStyle_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IWMP_put_SAMIStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SelectionEnd_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SelectionEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SelectionEnd_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IWMP_put_SelectionEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SelectionStart_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SelectionStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SelectionStart_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IWMP_put_SelectionStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SendErrorEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SendErrorEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SendErrorEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_SendErrorEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SendKeyboardEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SendKeyboardEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SendKeyboardEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_SendKeyboardEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SendMouseClickEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SendMouseClickEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SendMouseClickEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_SendMouseClickEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SendMouseMoveEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SendMouseMoveEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SendMouseMoveEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_SendMouseMoveEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SendOpenStateChangeEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SendOpenStateChangeEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SendOpenStateChangeEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_SendOpenStateChangeEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SendPlayStateChangeEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SendPlayStateChangeEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SendPlayStateChangeEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_SendPlayStateChangeEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SendWarningEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SendWarningEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_SendWarningEvents_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_SendWarningEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SourceLink_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SourceLink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_AboutBox_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_AboutBox_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_Cancel_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetCodecDescription_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long nCodec,
    /* [retval][out] */ BSTR __RPC_FAR *pDescription);


void __RPC_STUB IWMP_GetCodecDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetCodecInstalled_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ BSTR __RPC_FAR *pstrCodec,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pIsInstalled);


void __RPC_STUB IWMP_GetCodecInstalled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetCurrentEntry_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_GetCurrentEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetMarkerName_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long nMarker,
    /* [retval][out] */ BSTR __RPC_FAR *pMarkerName);


void __RPC_STUB IWMP_GetMarkerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetMarkerTime_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long nMarker,
    /* [retval][out] */ double __RPC_FAR *pMarkerTime);


void __RPC_STUB IWMP_GetMarkerTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetMediaInfoString_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long MPMediaInfoType,
    /* [retval][out] */ BSTR __RPC_FAR *pstrMediaInfo);


void __RPC_STUB IWMP_GetMediaInfoString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_Next_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_Open_Proxy( 
    IWMP __RPC_FAR * This,
    BSTR pstrClip);


void __RPC_STUB IWMP_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_Pause_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_Play_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_Play_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_Previous_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_Previous_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_Stop_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_Rate_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_Rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_Rate_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IWMP_put_Rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_DisplaySize_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_DisplaySize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_DisplaySize_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IWMP_put_DisplaySize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_SourceProtocol_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_SourceProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ErrorCorrection_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ErrorCorrection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_FinalConstruct_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_FinalConstruct_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_AllowChangeDisplaySize_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_AllowChangeDisplaySize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_AllowChangeDisplaySize_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_AllowChangeDisplaySize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_AllowScan_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_AllowScan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_AllowScan_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_AllowScan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_AnimationAtStart_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_AnimationAtStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_AnimationAtStart_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_AnimationAtStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_AudioStream_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_AudioStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_AudioStream_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IWMP_put_AudioStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_AutoRewind_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_AutoRewind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_AutoRewind_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_AutoRewind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_Balance_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_Balance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_Balance_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IWMP_put_Balance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_CanPreview_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_CanPreview_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_CanScan_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_CanScan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_CaptioningID_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_CaptioningID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_ClickToPlay_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_ClickToPlay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_ClickToPlay_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_ClickToPlay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_CodecCount_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_CodecCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_CreationDate_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ DATE __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_CreationDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_CursorType_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_CursorType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_CursorType_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IWMP_put_CursorType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_DisplayBackColor_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VB_OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_DisplayBackColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_DisplayBackColor_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VB_OLE_COLOR newVal);


void __RPC_STUB IWMP_put_DisplayBackColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_DisplayForeColor_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VB_OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_DisplayForeColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_DisplayForeColor_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VB_OLE_COLOR newVal);


void __RPC_STUB IWMP_put_DisplayForeColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_DisplayMode_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_DisplayMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_DisplayMode_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IWMP_put_DisplayMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_EnableContextMenu_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_EnableContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_EnableContextMenu_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_EnableContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_EnableFullScreenControls_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_EnableFullScreenControls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_EnableFullScreenControls_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_EnableFullScreenControls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_EnablePositionControls_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_EnablePositionControls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_EnablePositionControls_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_EnablePositionControls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_EnableTracker_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_EnableTracker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_EnableTracker_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_EnableTracker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_Language_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_Language_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_StreamCount_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_StreamCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_TransparentAtStart_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_TransparentAtStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_TransparentAtStart_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_TransparentAtStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_VideoBorder3D_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_VideoBorder3D_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_VideoBorder3D_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IWMP_put_VideoBorder3D_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_VideoBorderColor_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_VideoBorderColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_VideoBorderColor_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IWMP_put_VideoBorderColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IWMP_get_VideoBorderWidth_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IWMP_get_VideoBorderWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IWMP_put_VideoBorderWidth_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IWMP_put_VideoBorderWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_FastForward_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_FastForward_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_FastReverse_Proxy( 
    IWMP __RPC_FAR * This);


void __RPC_STUB IWMP_FastReverse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetCodecURL_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pstrCodecURL);


void __RPC_STUB IWMP_GetCodecURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetMediaParameter_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long nParam,
    BSTR szParameterName,
    /* [retval][out] */ BSTR __RPC_FAR *pstrParameterValue);


void __RPC_STUB IWMP_GetMediaParameter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetMediaParameterName_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long nParam,
    long nIndex,
    /* [retval][out] */ BSTR __RPC_FAR *pstrParameterName);


void __RPC_STUB IWMP_GetMediaParameterName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetMoreInfoURL_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pstrMoreInfoURL);


void __RPC_STUB IWMP_GetMoreInfoURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetStreamGroup_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pstrStreamGroup);


void __RPC_STUB IWMP_GetStreamGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetStreamName_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pstrStreamName);


void __RPC_STUB IWMP_GetStreamName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_GetStreamSelected_Proxy( 
    IWMP __RPC_FAR * This,
    /* [in] */ long nStream,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *fIsSelected);


void __RPC_STUB IWMP_GetStreamSelected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_IsSoundCardEnabled_Proxy( 
    IWMP __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *fIsEnabled);


void __RPC_STUB IWMP_IsSoundCardEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_SetCurrentEntry_Proxy( 
    IWMP __RPC_FAR * This,
    long nValue);


void __RPC_STUB IWMP_SetCurrentEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_ShowDialog_Proxy( 
    IWMP __RPC_FAR * This,
    long nValue);


void __RPC_STUB IWMP_ShowDialog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_StreamSelect_Proxy( 
    IWMP __RPC_FAR * This,
    long nSelect);


void __RPC_STUB IWMP_StreamSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IWMP_OnWindowMessage_Proxy( 
    IWMP __RPC_FAR * This,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT __RPC_FAR *plResult);


void __RPC_STUB IWMP_OnWindowMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWMP_INTERFACE_DEFINED__ */



#ifndef __PLAYEROCXLib_LIBRARY_DEFINED__
#define __PLAYEROCXLib_LIBRARY_DEFINED__

/****************************************
 * Generated header for library: PLAYEROCXLib
 * at Wed Apr 10 11:24:15 2002
 * using MIDL 3.02.88
 ****************************************/
/* [helpstring][version][uuid] */ 



EXTERN_C const IID LIBID_PLAYEROCXLib;

#ifndef ___IWMPEvents_DISPINTERFACE_DEFINED__
#define ___IWMPEvents_DISPINTERFACE_DEFINED__

/****************************************
 * Generated header for dispinterface: _IWMPEvents
 * at Wed Apr 10 11:24:15 2002
 * using MIDL 3.02.88
 ****************************************/
/* [helpstring][uuid] */ 



EXTERN_C const IID DIID__IWMPEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface DECLSPEC_UUID("E17A7C5E-CB17-4379-A487-C8ABC76A6541")
    _IWMPEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IWMPEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _IWMPEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _IWMPEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _IWMPEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _IWMPEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _IWMPEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _IWMPEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _IWMPEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _IWMPEventsVtbl;

    interface _IWMPEvents
    {
        CONST_VTBL struct _IWMPEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IWMPEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IWMPEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IWMPEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IWMPEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IWMPEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IWMPEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IWMPEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IWMPEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_WMP;

#ifdef __cplusplus

class DECLSPEC_UUID("22D6F312-B0F6-11D0-94AB-0080C74C7E95")
WMP;
#endif
#endif /* __PLAYEROCXLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
