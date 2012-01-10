#ifndef STB_SHELL_H
#define STB_SHELL_H

/* The control points are both held by the shell */
#include "MSCP_ControlPoint_Wrapper.h"
#include "AVRCP_ControlPoint.h"




/*
 * Call this to setup the shell.  Note that this is the call that
 * will start the control points, and the thread will be relinquished quickly.
 *
 * This will not, however, start the shell.  To start the shell, call STBS_Run.
 */
void STBS_Init(void *chain);

/*
 * The main shell event loop.  Note that this will not relinquish the thread
 * until the shell exits, symbolizing that the user desires to quit.
 */
void STBS_Run(void);

/*
 * Shut things down.
 */
void STBS_Uninit();

#endif