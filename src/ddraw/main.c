#include <stdio.h>
#include <windows.h>
#include <ddraw.h>
#include "gfx.h"
#include "timer.h"
#include "game.h"

#define WCNAME	"eradicatewin"

static LRESULT CALLBACK handle_msg(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam);
static int regwinclass(HINSTANCE hinst);

void msgbox(const char *msg);

HWND win;
int fullscreen = 1;

int have_joy;
unsigned int joy_bnstate, joy_bndiff, joy_bnpress;

static unsigned int num_pressed;
static unsigned char keystate[256];

static unsigned long start_time;
static unsigned int modkeys;

static int win_width, win_height;
static int quit;


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprev, char *cmdline, int showcmd)
{
	int vmidx;
	char *fake_argv[] = {"game.exe", 0};
	unsigned int wstyle;

#ifndef NDEBUG
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif

	if(regwinclass(hinst) == -1) {
		return 1;
	}

	wstyle = fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
	if(!(win = CreateWindowEx(0, WCNAME, "eradicate", wstyle, 10, 10, 320, 240, 0, 0, hinst, 0))) {
		msgbox("failed to create window");
		return 1;
	}
	ShowWindow(win, 1);

	if(au_init() == -1) {
		return 1;
	}

	if(init_video() == -1) {
		return 1;
	}

	if((vmidx = match_video_mode(640, 480, 16)) == -1) {
		return 1;
	}
	if(!set_video_mode(vmidx, 1)) {
		msgbox("failed to set requested video mode 640x480 16bpp");
		return 1;
	}

	time_msec = 0;
	if(init(1, fake_argv) == -1) {
		return 1;
	}
	reset_timer();

	for(;;) {
		MSG msg;

		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(quit) goto end;
		}

		inp_update();

		time_msec = get_msec();
		draw();
	}

end:
	cleanup_video();
	if(win) {
		DestroyWindow(win);
	}
	UnregisterClass(WCNAME, hinst);
	return 0;
}

void game_quit(void)
{
	quit = 1;
}

/* joystick */
int joy_detect(void)
{
	return 0;
}

void joy_update(void)
{
}

int kb_isdown(int key)
{
	switch(key) {
	case KB_ANY:
		return num_pressed;

	case KB_ALT:
		return keystate[KB_LALT] + keystate[KB_RALT];

	case KB_CTRL:
		return keystate[KB_LCTRL] + keystate[KB_RCTRL];
	}

	if(isalpha(key)) {
		key = tolower(key);
	}
	return keystate[key];
}

/* timer */
void init_timer(int res_hz)
{
}

void reset_timer(void)
{
	start_time = timeGetTime();
}

unsigned long get_msec(void)
{
	return timeGetTime() - start_time;
}

static int translate_vkey(int vkey)
{
	switch(vkey) {
	case VK_PRIOR: return KB_PGUP;
	case VK_NEXT: return KB_PGDN;
	case VK_END: return KB_END;
	case VK_HOME: return KB_HOME;
	case VK_LEFT: return KB_LEFT;
	case VK_UP: return KB_UP;
	case VK_RIGHT: return KB_RIGHT;
	case VK_DOWN: return KB_DOWN;
	default:
		break;
	}

	if(vkey >= 'A' && vkey <= 'Z') {
		vkey += 32;
	} else if(vkey >= VK_F1 && vkey <= VK_F12) {
		vkey -= VK_F1 + KB_F1;
	}

	return vkey;
}

static LRESULT CALLBACK handle_msg(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam)
{
	int x, y, key;

	switch(msg) {
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		quit = 1;
		break;

	case WM_PAINT:
		ValidateRect(win, 0);
		break;

	case WM_SIZE:
		x = lparam & 0xffff;
		y = lparam >> 16;
		if(x != win_width && y != win_height) {
			win_width = x;
			win_height = y;
		}
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		key = translate_vkey(wparam);
		if(key < 256) {
			keystate[key] = 1;
		}
		game_key(key, 1);
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		key = translate_vkey(wparam);
		if(key < 256) {
			keystate[key] = 0;
		}
		game_key(key, 0);
		break;

	case WM_SYSCOMMAND:
		wparam &= 0xfff0;
		if(wparam == SC_KEYMENU || wparam == SC_SCREENSAVE || wparam == SC_MONITORPOWER) {
			return 0;
		}
	default:
		return DefWindowProc(win, msg, wparam, lparam);
	}

	return 0;
}

static int regwinclass(HINSTANCE hinst)
{
	WNDCLASSEX wc = {0};

	wc.cbSize = sizeof wc;
	wc.lpszClassName = WCNAME;
	wc.hInstance = hinst;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = handle_msg;
	wc.hIcon = wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if(!RegisterClassEx(&wc)) {
		msgbox("Failed to register window class");
		return -1;
	}
	return 0;
}

void msgbox(const char *msg)
{
	MessageBox(0, msg, "error", MB_OK);
}
