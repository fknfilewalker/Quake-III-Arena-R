/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// win_input.c -- win32 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.

#include "../client/client.h"
#include "win_local.h"


typedef struct {
	int			oldButtonState;

	qboolean	mouseActive;
	qboolean	mouseInitialized;
  qboolean  mouseStartupDelayed; // delay mouse init to try DI again when we have a window
} WinMouseVars_t;

static WinMouseVars_t s_wmv;

static int	window_center_x, window_center_y;

//
// Joystick definitions
//
#define	JOY_MAX_AXES		6				// X, Y, Z, R, U, V

typedef struct {
	qboolean	avail;
	int			id;			// joystick number
	JOYCAPS		jc;

	int			oldbuttonstate;
	int			oldpovstate;

	JOYINFOEX	ji;
} joystickInfo_t;

static	joystickInfo_t	joy;



cvar_t	*in_midi;
cvar_t	*in_midiport;
cvar_t	*in_midichannel;
cvar_t	*in_mididevice;

cvar_t	*in_mouse;
cvar_t  *in_logitechbug;
cvar_t	*in_joystick;
cvar_t	*in_joyBallScale;
cvar_t	*in_debugJoystick;
cvar_t	*joy_threshold;

qboolean	in_appactive;

/*
============================================================

WIN32 MOUSE CONTROL

============================================================
*/

/*
================
IN_InitWin32Mouse
================
*/
void IN_InitWin32Mouse( void ) 
{
}

/*
================
IN_ShutdownWin32Mouse
================
*/
void IN_ShutdownWin32Mouse( void ) {
}

/*
================
IN_ActivateWin32Mouse
================
*/
void IN_ActivateWin32Mouse( void ) {
	int			width, height;
	RECT		window_rect;

	width = GetSystemMetrics (SM_CXSCREEN);
	height = GetSystemMetrics (SM_CYSCREEN);

	GetWindowRect ( wv.hWnd, &window_rect);
	if (window_rect.left < 0)
		window_rect.left = 0;
	if (window_rect.top < 0)
		window_rect.top = 0;
	if (window_rect.right >= width)
		window_rect.right = width-1;
	if (window_rect.bottom >= height-1)
		window_rect.bottom = height-1;
	window_center_x = (window_rect.right + window_rect.left)/2;
	window_center_y = (window_rect.top + window_rect.bottom)/2;

	SetCursorPos (window_center_x, window_center_y);

	SetCapture ( wv.hWnd );
	ClipCursor (&window_rect);
	while (ShowCursor (FALSE) >= 0)
		;
}

/*
================
IN_DeactivateWin32Mouse
================
*/
void IN_DeactivateWin32Mouse( void ) 
{
	ClipCursor (NULL);
	ReleaseCapture ();
	while (ShowCursor (TRUE) < 0)
		;
}

/*
================
IN_Win32Mouse
================
*/
void IN_Win32Mouse( int *mx, int *my ) {
	POINT		current_pos;

	// find mouse movement
	GetCursorPos (&current_pos);

	// force the mouse to the center, so there's room to move
	SetCursorPos (window_center_x, window_center_y);

	*mx = current_pos.x - window_center_x;
	*my = current_pos.y - window_center_y;
}


/*
============================================================

DIRECT INPUT MOUSE CONTROL

============================================================
*/

#undef DEFINE_GUID

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUID(GUID_SysMouse,   0x6F1D2B60,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_XAxis,   0xA36D02E0,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_YAxis,   0xA36D02E1,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_ZAxis,   0xA36D02E2,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);


#define DINPUT_BUFFERSIZE           16
#define iDirectInputCreate(a,b,c,d)	pDirectInputCreate(a,b,c,d)

HRESULT (WINAPI *pDirectInputCreate)(HINSTANCE hinst, DWORD dwVersion,
	LPDIRECTINPUT * lplpDirectInput, LPUNKNOWN punkOuter);

static HINSTANCE hInstDI;

typedef struct MYDATA {
	LONG  lX;                   // X axis goes here
	LONG  lY;                   // Y axis goes here
	LONG  lZ;                   // Z axis goes here
	BYTE  bButtonA;             // One button goes here
	BYTE  bButtonB;             // Another button goes here
	BYTE  bButtonC;             // Another button goes here
	BYTE  bButtonD;             // Another button goes here
} MYDATA;

static DIOBJECTDATAFORMAT rgodf[] = {
  { &GUID_XAxis,    FIELD_OFFSET(MYDATA, lX),       DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { &GUID_YAxis,    FIELD_OFFSET(MYDATA, lY),       DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { &GUID_ZAxis,    FIELD_OFFSET(MYDATA, lZ),       0x80000000 | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonA), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonB), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonC), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonD), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
};

#define NUM_OBJECTS (sizeof(rgodf) / sizeof(rgodf[0]))

// NOTE TTimo: would be easier using c_dfDIMouse or c_dfDIMouse2 
static DIDATAFORMAT	df = {
	sizeof(DIDATAFORMAT),       // this structure
	sizeof(DIOBJECTDATAFORMAT), // size of object data format
	DIDF_RELAXIS,               // absolute axis coordinates
	sizeof(MYDATA),             // device data size
	NUM_OBJECTS,                // number of objects
	rgodf,                      // and here they are
};

static LPDIRECTINPUT		g_pdi;
static LPDIRECTINPUTDEVICE	g_pMouse;

void IN_DIMouse( int *mx, int *my );

/*
========================
IN_InitDIMouse
========================
*/
qboolean IN_InitDIMouse( void ) {
    HRESULT		hr;
	int			x, y;
	DIPROPDWORD	dipdw = {
		{
			sizeof(DIPROPDWORD),        // diph.dwSize
			sizeof(DIPROPHEADER),       // diph.dwHeaderSize
			0,                          // diph.dwObj
			DIPH_DEVICE,                // diph.dwHow
		},
		DINPUT_BUFFERSIZE,              // dwData
	};

	Com_Printf( "Initializing DirectInput...\n");

	if (!hInstDI) {
		hInstDI = LoadLibrary("dinput.dll");
		
		if (hInstDI == NULL) {
			Com_Printf ("Couldn't load dinput.dll\n");
			return qfalse;
		}
	}

	if (!pDirectInputCreate) {
		pDirectInputCreate = (long (__stdcall *)(void *,unsigned long ,struct IDirectInputA ** ,struct IUnknown *))
			GetProcAddress(hInstDI,"DirectInputCreateA");

		if (!pDirectInputCreate) {
			Com_Printf ("Couldn't get DI proc addr\n");
			return qfalse;
		}
	}

	// register with DirectInput and get an IDirectInput to play with.
	hr = iDirectInputCreate( wv.hInstance, DIRECTINPUT_VERSION, &g_pdi, NULL);

	if (FAILED(hr)) {
		Com_Printf ("iDirectInputCreate failed\n");
		return qfalse;
	}

	// obtain an interface to the system mouse device.
	hr = IDirectInput_CreateDevice(g_pdi, &GUID_SysMouse, &g_pMouse, NULL);

	if (FAILED(hr)) {
		Com_Printf ("Couldn't open DI mouse device\n");
		return qfalse;
	}

	// set the data format to "mouse format".
	hr = IDirectInputDevice_SetDataFormat(g_pMouse, &df);

	if (FAILED(hr)) 	{
		Com_Printf ("Couldn't set DI mouse format\n");
		return qfalse;
	}

	// set the cooperativity level.
	hr = IDirectInputDevice_SetCooperativeLevel(g_pMouse, wv.hWnd,
			DISCL_EXCLUSIVE | DISCL_FOREGROUND);

	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=50
	if (FAILED(hr)) {
		Com_Printf ("Couldn't set DI coop level\n");
		return qfalse;
	}


	// set the buffer size to DINPUT_BUFFERSIZE elements.
	// the buffer size is a DWORD property associated with the device
	hr = IDirectInputDevice_SetProperty(g_pMouse, DIPROP_BUFFERSIZE, &dipdw.diph);

	if (FAILED(hr)) {
		Com_Printf ("Couldn't set DI buffersize\n");
		return qfalse;
	}

	// clear any pending samples
	IN_DIMouse( &x, &y );
	IN_DIMouse( &x, &y );

	Com_Printf( "DirectInput initialized.\n");
	return qtrue;
}

/*
==========================
IN_ShutdownDIMouse
==========================
*/
void IN_ShutdownDIMouse( void ) {
    if (g_pMouse) {
		IDirectInputDevice_Release(g_pMouse);
		g_pMouse = NULL;
	}

    if (g_pdi) {
		IDirectInput_Release(g_pdi);
		g_pdi = NULL;
	}
}

/*
==========================
IN_ActivateDIMouse
==========================
*/
void IN_ActivateDIMouse( void ) {
	HRESULT		hr;

	if (!g_pMouse) {
		return;
	}

	// we may fail to reacquire if the window has been recreated
	hr = IDirectInputDevice_Acquire( g_pMouse );
	if (FAILED(hr)) {
		if ( !IN_InitDIMouse() ) {
			Com_Printf ("Falling back to Win32 mouse support...\n");
			Cvar_Set( "in_mouse", "-1" );
		}
	}
}

/*
==========================
IN_DeactivateDIMouse
==========================
*/
void IN_DeactivateDIMouse( void ) {
	if (!g_pMouse) {
		return;
	}
	IDirectInputDevice_Unacquire( g_pMouse );
}


/*
===================
IN_DIMouse
===================
*/
void IN_DIMouse( int *mx, int *my ) {
	DIDEVICEOBJECTDATA	od;
	DIMOUSESTATE		state;
	DWORD				dwElements;
	HRESULT				hr;
  int value;
	static float		oldSysTime;

	if ( !g_pMouse ) {
		return;
	}

	// fetch new events
	for (;;)
	{
		dwElements = 1;

		hr = IDirectInputDevice_GetDeviceData(g_pMouse,
				sizeof(DIDEVICEOBJECTDATA), &od, &dwElements, 0);
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED)) {
			IDirectInputDevice_Acquire(g_pMouse);
			return;
		}

		/* Unable to read data or no data available */
		if ( FAILED(hr) ) {
			break;
		}

		if ( dwElements == 0 ) {
			break;
		}

		switch (od.dwOfs) {
		case DIMOFS_BUTTON0:
			if (od.dwData & 0x80)
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MOUSE1, qtrue, 0, NULL );
			else
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MOUSE1, qfalse, 0, NULL );
			break;

		case DIMOFS_BUTTON1:
			if (od.dwData & 0x80)
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MOUSE2, qtrue, 0, NULL );
			else
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MOUSE2, qfalse, 0, NULL );
			break;
			
		case DIMOFS_BUTTON2:
			if (od.dwData & 0x80)
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MOUSE3, qtrue, 0, NULL );
			else
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MOUSE3, qfalse, 0, NULL );
			break;

		case DIMOFS_BUTTON3:
			if (od.dwData & 0x80)
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MOUSE4, qtrue, 0, NULL );
			else
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MOUSE4, qfalse, 0, NULL );
			break;      
    // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=50
		case DIMOFS_Z:
			value = od.dwData;
			if (value == 0) {

			} else if (value < 0) {
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL );
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL );
			} else {
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MWHEELUP, qtrue, 0, NULL );
				Sys_QueEvent( od.dwTimeStamp, SE_KEY, K_MWHEELUP, qfalse, 0, NULL );
			}
			break;
		}
	}

	// read the raw delta counter and ignore
	// the individual sample time / values
	hr = IDirectInputDevice_GetDeviceState(g_pMouse,
			sizeof(DIDEVICEOBJECTDATA), &state);
	if ( FAILED(hr) ) {
		*mx = *my = 0;
		return;
	}
	*mx = state.lX;
	*my = state.lY;
}

/*
============================================================

  MOUSE CONTROL

============================================================
*/

/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
void IN_ActivateMouse( void ) 
{
	if (!s_wmv.mouseInitialized ) {
		return;
	}
	if ( !in_mouse->integer ) 
	{
		s_wmv.mouseActive = qfalse;
		return;
	}
	if ( s_wmv.mouseActive ) 
	{
		return;
	}

	s_wmv.mouseActive = qtrue;

	if ( in_mouse->integer != -1 ) {
		IN_ActivateDIMouse();
	}
	IN_ActivateWin32Mouse();
}


/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
void IN_DeactivateMouse( void ) {
	if (!s_wmv.mouseInitialized ) {
		return;
	}
	if (!s_wmv.mouseActive ) {
		return;
	}
	s_wmv.mouseActive = qfalse;

	IN_DeactivateDIMouse();
	IN_DeactivateWin32Mouse();
}



/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse( void ) 
{
	s_wmv.mouseInitialized = qfalse;
  s_wmv.mouseStartupDelayed = qfalse;

	if ( in_mouse->integer == 0 ) {
		Com_Printf ("Mouse control not active.\n");
		return;
	}

	// nt4.0 direct input is screwed up
	if ( ( wv.osversion.dwPlatformId == VER_PLATFORM_WIN32_NT ) &&
		 ( wv.osversion.dwMajorVersion == 4 ) )
	{
		Com_Printf ("Disallowing DirectInput on NT 4.0\n");
		Cvar_Set( "in_mouse", "-1" );
	}

	if ( in_mouse->integer == -1 ) {
		Com_Printf ("Skipping check for DirectInput\n");
	} else {
    if (!wv.hWnd)
    {
      Com_Printf ("No window for DirectInput mouse init, delaying\n");
      s_wmv.mouseStartupDelayed = qtrue;
      return;
    }
		if ( IN_InitDIMouse() ) {
	    s_wmv.mouseInitialized = qtrue;
			return;
		}
		Com_Printf ("Falling back to Win32 mouse support...\n");
	}
	s_wmv.mouseInitialized = qtrue;
	IN_InitWin32Mouse();
}

/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent (int mstate)
{
	int		i;

	if ( !s_wmv.mouseInitialized )
		return;

// perform button actions
	for  (i = 0 ; i < 3 ; i++ )
	{
		if ( (mstate & (1<<i)) &&
			!(s_wmv.oldButtonState & (1<<i)) )
		{
			Sys_QueEvent( wv.sysMsgTime, SE_KEY, K_MOUSE1 + i, qtrue, 0, NULL );
		}

		if ( !(mstate & (1<<i)) &&
			(s_wmv.oldButtonState & (1<<i)) )
		{
			Sys_QueEvent( wv.sysMsgTime, SE_KEY, K_MOUSE1 + i, qfalse, 0, NULL );
		}
	}	

	s_wmv.oldButtonState = mstate;
}


/*
===========
IN_MouseMove
===========
*/
void IN_MouseMove ( void ) {
	int		mx, my;

	if ( g_pMouse ) {
		IN_DIMouse( &mx, &my );
	} else {
		IN_Win32Mouse( &mx, &my );
	}

	if ( !mx && !my ) {
		return;
	}

	Sys_QueEvent( 0, SE_MOUSE, mx, my, 0, NULL );
}


/*
=========================================================================

=========================================================================
*/

/*
===========
IN_Startup
===========
*/
void IN_Startup( void ) {
	Com_Printf ("\n------- Input Initialization -------\n");
	IN_StartupMouse ();
	Com_Printf ("------------------------------------\n");

	in_mouse->modified = qfalse;
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown( void ) {
	IN_DeactivateMouse();
	IN_ShutdownDIMouse();
	Cmd_RemoveCommand("midiinfo" );
}


/*
===========
IN_Init
===========
*/
void IN_Init( void ) {
	// mouse variables
  in_mouse				= Cvar_Get ("in_mouse",					"1",		CVAR_ARCHIVE|CVAR_LATCH);
	in_logitechbug  = Cvar_Get ("in_logitechbug", "0", CVAR_ARCHIVE);

	IN_Startup();
}


/*
===========
IN_Activate

Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void IN_Activate (qboolean active) {
	in_appactive = active;

	if ( !active )
	{
		IN_DeactivateMouse();
	}
}


/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
void IN_Frame (void) {

	if ( !s_wmv.mouseInitialized ) {
    if (s_wmv.mouseStartupDelayed && wv.hWnd)
		{
			Com_Printf("Proceeding with delayed mouse init\n");
      IN_StartupMouse();
			s_wmv.mouseStartupDelayed = qfalse;
		}
		return;
	}

	if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {
		// temporarily deactivate if not in the game and
		// running on the desktop
		// voodoo always counts as full screen
		if (Cvar_VariableValue ("r_fullscreen") == 0
			/*&& strcmp( Cvar_VariableString("r_glDriver"), _3DFX_DRIVER_NAME)*/ )	{
			IN_DeactivateMouse ();
			return;
		}
	}

	if ( !in_appactive ) {
		IN_DeactivateMouse ();
		return;
	}

	IN_ActivateMouse();

	// post events to the system que
	IN_MouseMove();

}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void) 
{
	s_wmv.oldButtonState = 0;
}

