#ifndef _MEDIA_PLAYER_VERSIONS_METHODS_H
#define _MEDIA_PLAYER_VERSIONS_METHODS_H

#include "MicroMediaRenderer.h"

#ifdef _WIN32_WCE
#elif WIN32
	#include "mediaplayer1.h"
#endif




/******************************************************************************************
 *	BEGIN: Message map and MPInvoke defines
 *
 *	MPINV:	Indicates invocation on windows media player.
 *	MPEVT:	Indicates handling of windows media player event.
 *	RLINV:	Indicates invocation on renderer state logic.
 ******************************************************************************************/
#define WM_MPINVOKE (WM_USER + 1)

#define MPINV_SET_VOLUME_MASTER		0x0100
#define MPINV_SET_VOLUME_LEFT		0x0101
#define MPINV_SET_VOLUME_RIGHT		0x0102
#define MPINV_SET_MUTE				0x0200

#define MPINV_SET_URI				0x0300
#define MPINV_SET_CURRENT_POSITION	0x0400
#define MPINV_SET_STATE				0x0500

#define MPEVT_STATE_CHANGE			0x0600

#define RLINV_INV_SET_MEDIA_URI		0x1000
#define RLINV_NEXT_PREVIOUS			0x1100
#define RLINV_SET_TRACK_INDEX		0x1200
#define RLINV_SET_STATE				0x1300
#define RLINV_SET_PLAYMODE			0x1400
/******************************************************************************************
 *	END: Message map and MPInvoke defines
 ******************************************************************************************/




/******************************************************************************************
 *	BEGIN: App variables
 *			Variables needed so app can coordinate activities of 
 *			MediaPlayer, RendererLogic modules, and UPnP-related modules.
 ******************************************************************************************/

/* State coordination: last known state play/stop/pause command */
extern enum MR_Enum_States SC_LastCommand;

/* State coordination: if nonzero, file/URI was set and is still pending */
extern int SC_PendingSetFileName;								

/* State coordination: desired volume-levels for each master/left/right channels */
extern int   SC_VolumeMaster;
extern int   SC_VolumeLeft;
extern int   SC_VolumeRight;

/*
 *	Thread chain for UPnP stack, playlist parsing, and renderer logic.
 */
extern void* The_RendererChain;

/*
 *	Token object for MediaRenderer Microstack.
 *	(eg, MicroMediaRenderer.h-defined object).
 */
extern void* The_UpnpStack;

/*
 *	Token object for Renderer logic.
 *	(eg, RendererStateLogic.h-defined object).
 */
extern void* The_RendererLogic;

/*
 *	Token object for processing playlists.
 *	(eg, HttpPlaylistParser.h-defined object).
 */
extern void* The_PlaylistProcessor;

/*
 *	Main window for app.
 */
extern HWND MainWindow;

/*
 *	The windows media player object
 */
#ifdef _WIN32_WCE
#elif WIN32
	extern CMediaplayer1* The_MediaPlayer;
#endif

/******************************************************************************************
 *	END: App variables
 ******************************************************************************************/




/******************************************************************************************
 *	BEGIN: Function declarations
 ******************************************************************************************/
#define PM_Pause()	PostMessage(WM_MPINVOKE, MPINV_SET_STATE, MR_State_Paused)
#define PM_Play()	PostMessage(WM_MPINVOKE, MPINV_SET_STATE, MR_State_Playing)
#define PM_Stop()	PostMessage(WM_MPINVOKE, MPINV_SET_STATE, MR_State_Stopped)
#define PM_OnStateChange(x)	PostMessage(WM_MPINVOKE, MPEVT_STATE_CHANGE, (LPARAM) x)

#define PM2_Pause()	PostMessage(MainWindow, WM_MPINVOKE, MPINV_SET_STATE, MR_State_Paused)
#define PM2_Play()	PostMessage(MainWindow, WM_MPINVOKE, MPINV_SET_STATE, MR_State_Playing)
#define PM2_Stop()	PostMessage(MainWindow, WM_MPINVOKE, MPINV_SET_STATE, MR_State_Stopped)
#define PM2_OnStateChange(x)	PostMessage(MainWindow, WM_MPINVOKE, MPEVT_STATE_CHANGE, (LPARAM) x)


void SinkQuery_CurrentPosition(int* seconds, int* absSeconds, int* count, int* absCount);
void SinkRequest_MediaUriChange(const char* url);
void SinkRequest_MuteChange(enum MR_Enum_AudioChannels Channel,int Mute);
void SinkRequest_NextPrevious(int trackdelta);
void SinkRequest_PlayModeChange(MR_Enum_PlayModes playmode);
void SinkRequest_Seek(enum MR_Enum_SeekModes seekMode, int seekPosition);
void SinkRequest_StateChange(enum MR_Enum_States state, enum MR_Enum_PlaySpeeds playSpeed);
void SinkRequest_VolumeChange(enum MR_Enum_AudioChannels Channel,unsigned short Volume);

void HandleMediaPlayer_SetVolume();
void HandleMediaPlayer_SetState(enum MR_Enum_States desiredState);
void HandleMediaPlayer_OnError();
void HandleMediaPlayer_OnStateChange(long newState);

void IM_FindTargetUri(void *rslObj, const char *mediaUri, int targetIndex, int wrapAround);
int IM_IsCodecBusy(void *rslObj);

void IM_Pause(void *rslObj);
void IM_Play(void *rslObj);
void IM_Stop(void *rslObj);
void IM_StreamThis(void *rslObj, const char *trackUri);

void IM_OnResult_FindTargetUri (void *parserObject, int wrapAround, const char* playlistUri, int itemIndex, void *userObject, /*OUT*/ int actualIndex, /*OUT - MUST COPY THIS*/ const char *targetUri, /*OUT*/int duration, /*OUT - MUST COPY THIS*/ const char* comment);
void IM_OnResult_PlaylistUriExists (void* parserObject, const char* playlistUri, void* userObject, int uriExists);
void IM_OnResult_UpdateItemCount (void *parserObject, const char* playlistUri, void* userObject, /*OUT*/int maxIndexNotKnown, /*OUT*/int itemCount);

int IM_ValidateMediaUri(void *rslObj, const char *mediaUri);

LRESULT MainSwitch (WPARAM wp, LPARAM lp);
void Init_TheMediaPlayer(HWND mainWindow, void* mediaPlayer);
void Init_TheRendererVariables(void* chain, const char* friendlyname, const char* udn, const char* serialNo);
void Uninit_TheRendererVariables();
/******************************************************************************************
 *	END: Function declarations
 ******************************************************************************************/

#endif
