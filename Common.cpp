#include "Common.h"

#include "utils.cpp"
//#include <Windowsx.h> //GET_X_LPARAM, GET_Y_LPARAM //I created my own

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif // !GET_X_LPARAM

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif // !GET_Y_LPARAM


BOOL GetMyClientRect(HWND hwnd, const CUSTOM_FRAME& frame, RECT* rc) {
	BOOL res = GetWindowRect(hwnd, rc);//INFO: could use getclientrect, though values will be wrong until the hwnd destroys its non client area
	if (res) {
		rc->right = RECTWIDTH((*rc)) - frame.right_border;
		rc->bottom = RECTHEIGHT((*rc)) - frame.bottom_border;
		rc->left = frame.left_border;
		rc->top = frame.caption_height;
	}
	return res;
}

HFONT CreateMyFont(LONG height) { //TODO(fran): first of all the whole enumfontfamilies should be done ONLY ONCE, we should store the one font we're gonna use (static variable?) and then just use that
	LOGFONTW lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfQuality = CLEARTYPE_QUALITY;
	//lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfHeight = height > 0 ? -height : height;

	static uint8_t index = (uint8_t)-1;

	//Possible fonts, in order of importance
	const WCHAR* requested_fontname[] = { TEXT("Segoe UI"), TEXT("Arial Unicode MS"), TEXT("Microsoft YaHei"), TEXT("Microsoft YaHei UI")
									, TEXT("Microsoft JhengHei"), TEXT("Microsoft JhengHei UI") };

	/// Decides which font FaceName is appropiate for the current system
	//GetFontFaceName 
	//Font guidelines: https://docs.microsoft.com/en-us/windows/win32/uxguide/vis-fonts
	//Stock fonts: https://docs.microsoft.com/en-us/windows/win32/gdi/using-a-stock-font-to-draw-text

	//TODO(fran): can we take the best codepage from each font and create our own? (look at font linking & font fallback)

	//We looked at 2195 fonts, this is what's left
	//Options:
	//Segoe UI (good txt, jp ok) (10 codepages) (supported on most versions of windows)
	//-Arial Unicode MS (good text; good jp) (33 codepages) (doesnt come with windows)
	//-Microsoft YaHei or UI (look the same,good txt,good jp) (6 codepages) (supported on most versions of windows)
	//-Microsoft JhengHei or UI (look the same,good txt,ok jp) (3 codepages) (supported on most versions of windows)
	if (index == (uint8_t)-1) {
		HDC hdc = GetDC(GetDesktopWindow()); //You can use any hdc, but not NULL
		struct _EFFE { const WCHAR* font; bool found=false; } _effe; 
		uint8_t i = 0;
		for (; i < ARRAYSIZE(requested_fontname); i++) {
			_effe.font= requested_fontname[i];
			wcsncpy_s(lf.lfFaceName, requested_fontname[i], ARRAYSIZE(lf.lfFaceName) - 1); //try to make the search faster and shorter, EnumFontFamiliesEx checks for lfFaceName and if valid only iterates over fonts of that family
			EnumFontFamiliesEx(hdc, &lf
				, [](const LOGFONT *lpelfe, const TEXTMETRIC * /*lpntme*/, DWORD /*FontType*/, LPARAM lParam)->int { 
						if (!wcscmp(((_EFFE*)lParam)->font, lpelfe->lfFaceName)) {
							((_EFFE*)lParam)->found = true; 
							return FALSE; 
						} 
						return TRUE;
					}
			, (LPARAM)&_effe, NULL);
			if (_effe.found) break;
		}
		if (_effe.found) index = i;
		else index = (uint8_t)-2;
	}

	//INFO: by default if I dont set faceName it uses "Modern", looks good but it lacks some charsets
	if(index==(uint8_t)-2)
		wcsncpy_s(lf.lfFaceName, L"", 1);
	else 
		wcsncpy_s(lf.lfFaceName, requested_fontname[index], ARRAYSIZE(lf.lfFaceName)-1);
	
	return CreateFontIndirectW(&lf);
}

LRESULT PaintCaption(HWND hWnd, CUSTOM_FRAME frame) {
	HDC         hdc;
	PAINTSTRUCT ps;
	RECT        rect;

	hdc = BeginPaint(hWnd, &ps);

	GetWindowRect(hWnd, &rect);
	//INFO: after the first wm_paint call getclientrect and getwindowrect return the same value which is the one returned by getwidowrect
	//GetWindowRect(hwnd, &rect);
	//HGDIOBJ original = NULL;

	//original = SelectObject(hdc, GetStockObject(DC_PEN));
	//SetDCPenColor(hdc, RGB(0, 255, 0));
	//SelectObject(hdc, CreateSolidBrush(RGB(255, 0, 0)));

	//Draw top caption frame
	RECT fill_rc = { frame.left_border, 0, RECTWIDTH(rect) - frame.right_border, frame.caption_height };
	FillRect(hdc, &fill_rc, frame.caption_bk_brush);

#if 0 //Assign a "transparent" color to the borders
	HBRUSH transp_br = CreateSolidBrush(RGB(255, 0, 255));
	RECT transp_rc_left = { 0,0,FRAME.left_border ,RECTHEIGHT(rect) };
	FillRect(hdc, &transp_rc_left, transp_br);
	RECT transp_rc_right = { RECTWIDTH(rect) - FRAME.right_border,0,RECTWIDTH(rect),RECTHEIGHT(rect) };
	FillRect(hdc, &transp_rc_right, transp_br);
	RECT transp_rc_bottom = { 0,RECTHEIGHT(rect) - FRAME.bottom_border,RECTWIDTH(rect),RECTHEIGHT(rect) };
	FillRect(hdc, &transp_rc_bottom, transp_br);
	DeleteObject(transp_br);
#endif
	//TODO: try SetWindowRgn to make invisible the borders, documentation says what is outside isnt displayed!

	//SelectObject(hdc, original);
	COLORREF prev_text_col = SetTextColor(hdc, frame.caption_text_color);
	LOGBRUSH bk_brush_info;
	GetObject(frame.caption_bk_brush, sizeof(bk_brush_info), &bk_brush_info);
	COLORREF prev_bk_col = SetBkColor(hdc, bk_brush_info.lbColor);

	//Draw logo icon
	SIZE logo_size;
	//TODO(fran): better to use getsystemmetric?
	logo_size.cx = (LONG)(frame.caption_height*.6f);
	logo_size.cy = logo_size.cx;
	HICON logo_icon = (HICON)LoadImage((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(frame.logo_icon), IMAGE_ICON
		, logo_size.cx, logo_size.cy, LR_SHARED);

	int logo_x_offset = GetSystemMetrics(SM_CXSMICON);
	if (logo_icon) {
		DrawIconEx(hdc, logo_x_offset, (frame.caption_height - logo_size.cy) / 2,
			logo_icon, logo_size.cx, logo_size.cy, 0, NULL, DI_NORMAL);

		DestroyIcon(logo_icon);
	}

	if (frame.caption_font)
	{
		HFONT prev_font = (HFONT)SelectObject(hdc, (HGDIOBJ)frame.caption_font);

		WCHAR title[200];
		SendMessage(hWnd, WM_GETTEXT, 200, (LPARAM)title);
		RECT text_rc = { logo_x_offset * 2 + logo_size.cx,0,min(200,RECTWIDTH(rect) - frame.right_border - frame.left_border),frame.caption_height };
		DrawText(hdc, title, -1, &text_rc, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
		SetTextColor(hdc, prev_text_col);
		SetBkColor(hdc, prev_bk_col);
		SelectObject(hdc, (HGDIOBJ)prev_font);
		EndPaint(hWnd, &ps);
	}
	return 0;
}

LRESULT HitTestNCA(HWND hWnd, WPARAM /*wParam*/, LPARAM lParam,CUSTOM_FRAME frame)
{
	// Get the point coordinates for the hit test.
	POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	// Get the window rectangle.
	RECT rcWindow;
	GetWindowRect(hWnd, &rcWindow);

	// Get the frame rectangle, adjusted for the style without a caption.
	RECT rcFrame = { 0 }; //TODO(fran): check if we should replace this with FRAME------------------------------------------------------------- also might be useful for retrieving default caption height
	AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

	// Determine if the hit test is for resizing. Default middle (1,1).
	USHORT uRow = 1;
	USHORT uCol = 1;
	bool fOnResizeBorder = false;

	// Determine if the point is at the top or bottom of the window.
	if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + frame.caption_height)
	{
		fOnResizeBorder = (ptMouse.y < (rcWindow.top - rcFrame.top));
		uRow = 0;
	}
	else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - frame.bottom_border)
	{
		uRow = 2;
	}

	// Determine if the point is at the left or right of the window.
	if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + frame.left_border)
	{
		uCol = 0; // left side
	}
	else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - frame.right_border)
	{
		uCol = 2; // right side
	}

	// Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
	LRESULT hitTests[3][3] =
	{
		{ HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCAPTION,    HTTOPRIGHT },
		{ HTLEFT,       HTNOWHERE,     HTRIGHT },
		{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
	};

	return hitTests[uRow][uCol];
}