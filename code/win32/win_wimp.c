
#include "../renderer/tr_local.h"
#include "resource.h"
#include "glw_win.h"
#include "win_local.h"

#define	WINDOW_CLASS_NAME	"Quake 3: Arena"
static qboolean s_classRegistered = qfalse;

typedef enum {
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;

//
// function declaration
//
static rserr_t W_SetMode(const char* drivername, int mode, int colorbits, qboolean cdsFullscreen);
static void PrintCDSError(int value);
static qboolean W_CreateWindow(const char* drivername, int width, int height, int colorbits, qboolean cdsFullscreen);

/*
** W_StartDriverAndSetMode
*/
/*static*/ qboolean W_StartDriverAndSetMode(const char* drivername,
	int mode,
	int colorbits,
	qboolean cdsFullscreen)
{
	rserr_t err;

	err = W_SetMode(drivername, r_mode->integer, colorbits, cdsFullscreen);

	switch (err)
	{
	case RSERR_INVALID_FULLSCREEN:
		ri.Printf(PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n");
		return qfalse;
	case RSERR_INVALID_MODE:
		ri.Printf(PRINT_ALL, "...WARNING: could not set the given mode (%d)\n", mode);
		return qfalse;
	default:
		break;
	}
	return qtrue;
}

/*
** W_SetMode
*/
static rserr_t W_SetMode(const char* drivername,
	int mode,
	int colorbits,
	qboolean cdsFullscreen)
{
	HDC hDC;
	const char* win_fs[] = { "W", "FS" };
	int		cdsRet;
	DEVMODE dm;

	//
	// print out informational messages
	//
	ri.Printf(PRINT_ALL, "...setting mode %d:", mode);
	if (!R_GetModeInfo(&glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, mode))
	{
		ri.Printf(PRINT_ALL, " invalid mode\n");
		return RSERR_INVALID_MODE;
	}
	ri.Printf(PRINT_ALL, " %d %d %s\n", glConfig.vidWidth, glConfig.vidHeight, win_fs[cdsFullscreen]);

	//
	// check our desktop attributes
	//
	hDC = GetDC(GetDesktopWindow());
	glw_state.desktopBitsPixel = GetDeviceCaps(hDC, BITSPIXEL);
	glw_state.desktopWidth = GetDeviceCaps(hDC, HORZRES);
	glw_state.desktopHeight = GetDeviceCaps(hDC, VERTRES);
	ReleaseDC(GetDesktopWindow(), hDC);

	// do a CDS if needed
	if (cdsFullscreen)
	{
		memset(&dm, 0, sizeof(dm));

		dm.dmSize = sizeof(dm);

		dm.dmPelsWidth = glConfig.vidWidth;
		dm.dmPelsHeight = glConfig.vidHeight;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		if (r_displayRefresh->integer != 0)
		{
			dm.dmDisplayFrequency = r_displayRefresh->integer;
			dm.dmFields |= DM_DISPLAYFREQUENCY;
		}

		// try to change color depth if possible
		if (colorbits != 0)
		{
			if (glw_state.allowdisplaydepthchange)
			{
				dm.dmBitsPerPel = colorbits;
				dm.dmFields |= DM_BITSPERPEL;
				ri.Printf(PRINT_ALL, "...using colorsbits of %d\n", colorbits);
			}
			else
			{
				ri.Printf(PRINT_ALL, "WARNING:...changing depth not supported on Win95 < pre-OSR 2.x\n");
			}
		}
		else
		{
			ri.Printf(PRINT_ALL, "...using desktop display depth of %d\n", glw_state.desktopBitsPixel);
		}

		//
		// if we're already in fullscreen then just create the window
		//
		if (glw_state.cdsFullscreen)
		{
			ri.Printf(PRINT_ALL, "...already fullscreen, avoiding redundant CDS\n");

			if (!W_CreateWindow(drivername, glConfig.vidWidth, glConfig.vidHeight, colorbits, qtrue))
			{
				ri.Printf(PRINT_ALL, "...restoring display settings\n");
				ChangeDisplaySettings(0, 0);
				return RSERR_INVALID_MODE;
			}
		}
		//
		// need to call CDS
		//
		else
		{
			ri.Printf(PRINT_ALL, "...calling CDS: ");

			// try setting the exact mode requested, because some drivers don't report
			// the low res modes in EnumDisplaySettings, but still work
			if ((cdsRet = ChangeDisplaySettings(&dm, CDS_FULLSCREEN)) == DISP_CHANGE_SUCCESSFUL)
			{
				ri.Printf(PRINT_ALL, "ok\n");

				if (!W_CreateWindow(drivername, glConfig.vidWidth, glConfig.vidHeight, colorbits, qtrue))
				{
					ri.Printf(PRINT_ALL, "...restoring display settings\n");
					ChangeDisplaySettings(0, 0);
					return RSERR_INVALID_MODE;
				}

				glw_state.cdsFullscreen = qtrue;
			}
			else
			{
				//
				// the exact mode failed, so scan EnumDisplaySettings for the next largest mode
				//
				DEVMODE		devmode;
				int			modeNum;

				ri.Printf(PRINT_ALL, "failed, ");

				PrintCDSError(cdsRet);

				ri.Printf(PRINT_ALL, "...trying next higher resolution:");

				// we could do a better matching job here...
				for (modeNum = 0; ; modeNum++) {
					if (!EnumDisplaySettings(NULL, modeNum, &devmode)) {
						modeNum = -1;
						break;
					}
					if (devmode.dmPelsWidth >= glConfig.vidWidth
						&& devmode.dmPelsHeight >= glConfig.vidHeight
						&& devmode.dmBitsPerPel >= 15) {
						break;
					}
				}

				if (modeNum != -1 && (cdsRet = ChangeDisplaySettings(&devmode, CDS_FULLSCREEN)) == DISP_CHANGE_SUCCESSFUL)
				{
					ri.Printf(PRINT_ALL, " ok\n");
					if (!W_CreateWindow(drivername, glConfig.vidWidth, glConfig.vidHeight, colorbits, qtrue))
					{
						ri.Printf(PRINT_ALL, "...restoring display settings\n");
						ChangeDisplaySettings(0, 0);
						return RSERR_INVALID_MODE;
					}

					glw_state.cdsFullscreen = qtrue;
				}
				else
				{
					ri.Printf(PRINT_ALL, " failed, ");

					PrintCDSError(cdsRet);

					ri.Printf(PRINT_ALL, "...restoring display settings\n");
					ChangeDisplaySettings(0, 0);

					glw_state.cdsFullscreen = qfalse;
					glConfig.isFullscreen = qfalse;
					if (!W_CreateWindow(drivername, glConfig.vidWidth, glConfig.vidHeight, colorbits, qfalse))
					{
						return RSERR_INVALID_MODE;
					}
					return RSERR_INVALID_FULLSCREEN;
				}
			}
		}
	}
	else
	{
		if (glw_state.cdsFullscreen)
		{
			ChangeDisplaySettings(0, 0);
		}

		glw_state.cdsFullscreen = qfalse;
		if (!W_CreateWindow(drivername, glConfig.vidWidth, glConfig.vidHeight, colorbits, qfalse))
		{
			return RSERR_INVALID_MODE;
		}
	}

	//
	// success, now check display frequency, although this won't be valid on Voodoo(2)
	//
	memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);
	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
	{
		glConfig.displayFrequency = dm.dmDisplayFrequency;
	}

	// NOTE: this is overridden later on standalone 3Dfx drivers
	glConfig.isFullscreen = cdsFullscreen;

	return RSERR_OK;
}

/*
** VKW_CreateWindow
**
** Responsible for creating the Win32 window and initializing the OpenGL driver.
*/
#define	WINDOW_STYLE	(WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE)
static qboolean W_CreateWindow(const char* drivername, int width, int height, int colorbits, qboolean cdsFullscreen)
{
	RECT			r;
	cvar_t* vid_xpos, * vid_ypos;
	int				stylebits;
	int				x, y, w, h;
	int				exstyle;

	//
	// register the window class if necessary
	//
	if (!s_classRegistered)
	{
		WNDCLASS wc;

		memset(&wc, 0, sizeof(wc));

		wc.style = 0;
		wc.lpfnWndProc = (WNDPROC)glw_state.wndproc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = wv.hInstance;
		wc.hIcon = LoadIcon(wv.hInstance, MAKEINTRESOURCE(IDI_ICON1));
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(void*)COLOR_GRAYTEXT;
		wc.lpszMenuName = 0;
		wc.lpszClassName = WINDOW_CLASS_NAME;

		if (!RegisterClass(&wc))
		{
			ri.Error(ERR_FATAL, "GLW_CreateWindow: could not register window class");
		}
		s_classRegistered = qtrue;
		ri.Printf(PRINT_ALL, "...registered window class\n");
	}

	//
	// create the HWND if one does not already exist
	//
	if (!wv.hWnd)
	{
		//
		// compute width and height
		//
		r.left = 0;
		r.top = 0;
		r.right = width;
		r.bottom = height;

		if (cdsFullscreen)
		{
			exstyle = WS_EX_TOPMOST;
			stylebits = WS_POPUP | WS_VISIBLE | WS_SYSMENU;
		}
		else
		{
			exstyle = 0;
			stylebits = WINDOW_STYLE | WS_SYSMENU;
			AdjustWindowRect(&r, stylebits, FALSE);
		}

		w = r.right - r.left;
		h = r.bottom - r.top;

		if (cdsFullscreen)
		{
			x = 0;
			y = 0;
		}
		else
		{
			vid_xpos = ri.Cvar_Get("vid_xpos", "", 0);
			vid_ypos = ri.Cvar_Get("vid_ypos", "", 0);
			x = vid_xpos->integer;
			y = vid_ypos->integer;

			// adjust window coordinates if necessary 
			// so that the window is completely on screen
			if (x < 0)
				x = 0;
			if (y < 0)
				y = 0;

			if (w < glw_state.desktopWidth &&
				h < glw_state.desktopHeight)
			{
				if (x + w > glw_state.desktopWidth)
					x = (glw_state.desktopWidth - w);
				if (y + h > glw_state.desktopHeight)
					y = (glw_state.desktopHeight - h);
			}
		}

		wv.hWnd = CreateWindowEx(
			exstyle,
			WINDOW_CLASS_NAME,
			"Quake 3: Arena",
			stylebits,
			x, y, w, h,
			NULL,
			NULL,
			wv.hInstance,
			NULL);

		if (!wv.hWnd)
		{
			ri.Error(ERR_FATAL, "GLW_CreateWindow() - Couldn't create window");
		}

		ShowWindow(wv.hWnd, SW_SHOW);
		UpdateWindow(wv.hWnd);
		ri.Printf(PRINT_ALL, "...created window@%d,%d (%dx%d)\n", x, y, w, h);
	}
	else
	{
		ri.Printf(PRINT_ALL, "...window already present, CreateWindowEx skipped\n");
	}

	qboolean initDriver = qfalse;
	if (!Q_stricmp(drivername, OPENGL_DRIVER_NAME)) initDriver = GLW_InitDriver(drivername, colorbits);
	else if(!Q_stricmp(drivername, VULKAN_DRIVER_NAME)) initDriver = VKW_InitDriver(drivername, colorbits);

	if (!initDriver)
	{
		ShowWindow(wv.hWnd, SW_HIDE);
		DestroyWindow(wv.hWnd);
		wv.hWnd = NULL;

		return qfalse;
	}

	SetForegroundWindow(wv.hWnd);
	SetFocus(wv.hWnd);

	return qtrue;
}

static void PrintCDSError(int value)
{
	switch (value)
	{
	case DISP_CHANGE_RESTART:
		ri.Printf(PRINT_ALL, "restart required\n");
		break;
	case DISP_CHANGE_BADPARAM:
		ri.Printf(PRINT_ALL, "bad param\n");
		break;
	case DISP_CHANGE_BADFLAGS:
		ri.Printf(PRINT_ALL, "bad flags\n");
		break;
	case DISP_CHANGE_FAILED:
		ri.Printf(PRINT_ALL, "DISP_CHANGE_FAILED\n");
		break;
	case DISP_CHANGE_BADMODE:
		ri.Printf(PRINT_ALL, "bad mode\n");
		break;
	case DISP_CHANGE_NOTUPDATED:
		ri.Printf(PRINT_ALL, "not updated\n");
		break;
	default:
		ri.Printf(PRINT_ALL, "unknown error %d\n", value);
		break;
	}
}

