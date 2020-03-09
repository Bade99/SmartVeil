#include "ControlProcedures.h"



ControlProcedures::ControlProcedures()
{
}


ControlProcedures::~ControlProcedures()
{
	if (this->BackgroundPush) DeleteObject(this->BackgroundPush);
	if (this->BackgroundMouseover) DeleteObject(this->BackgroundMouseover);
	
	if (this->Background) DeleteObject(this->Background);
	if (this->Highlight) DeleteObject(this->Highlight);
	if (this->CaptionBackground) DeleteObject(this->CaptionBackground);
}

LRESULT CALLBACK ControlProcedures::ButtonProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	static BOOL MouseOver = false;
	static HWND CurrentMouseOverButton;

	ControlProcedures* pthis = (ControlProcedures*)dwRefData;

	switch (Msg) {
	case WM_MOUSEHOVER:
	{
		//force button to repaint and specify hover or not
		if (MouseOver && CurrentMouseOverButton == hWnd) break;
		MouseOver = true;
		CurrentMouseOverButton = hWnd;
		InvalidateRect(hWnd, 0, 1);
		return DefSubclassProc(hWnd, Msg, wParam, lParam);
	}
	case WM_MOUSELEAVE:
	{
		MouseOver = false;
		CurrentMouseOverButton = NULL;
		InvalidateRect(hWnd, 0, 1);
		return DefSubclassProc(hWnd, Msg, wParam, lParam);
	}
	case WM_MOUSEMOVE:
	{
		//TODO(fran): We are tracking the mouse every single time it moves, kind of suspect solution
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1;
		tme.hwndTrack = hWnd;

		TrackMouseEvent(&tme);
		return DefSubclassProc(hWnd, Msg, wParam, lParam);
	}
	case WM_PAINT://TODO(fran): I think we should handle erasebkgnd cause there some weird problems happen, we are patching them by calling update every time we change the control from the outside
	{
		PAINTSTRUCT ps; //TODO(fran): we arent using the rectangle from the ps, I think we should for performance
		HDC hdc = BeginPaint(hWnd, &ps);

		RECT rc; GetClientRect(hWnd, &rc);
		//int controlID = GetDlgCtrlID(hWnd);

		WORD ButtonState = SendMessageW(hWnd, BM_GETSTATE, 0, 0);
		if (ButtonState & BST_PUSHED) {
			LOGBRUSH lb;
			GetObject(pthis->BackgroundPush, sizeof(LOGBRUSH), &lb);
			SetBkColor(hdc, lb.lbColor);
			FillRect(hdc, &rc, pthis->BackgroundPush);
		}
		else if (MouseOver && CurrentMouseOverButton == hWnd) {
			LOGBRUSH lb;
			GetObject(pthis->BackgroundMouseover, sizeof(LOGBRUSH), &lb);
			SetBkColor(hdc, lb.lbColor);
			FillRect(hdc, &rc, pthis->BackgroundMouseover);
		}
		else {
			LOGBRUSH lb;
			GetObject(pthis->Background, sizeof(LOGBRUSH), &lb);
			SetBkColor(hdc, lb.lbColor);
			FillRect(hdc, &rc, pthis->Background);// (HBRUSH)GetStockObject(BLACK_BRUSH));
		}

		//if I do 4 calls to fillrect I could create a border
		//HBRUSH BorderColor;
#if 0
		if (MouseOver) BorderColor = ButtonBorderOver;
		else BorderColor = (HBRUSH)GetStockObject(WHITE_BRUSH);
#else 
		//BorderColor = pthis->Highlight;// (HBRUSH)GetStockObject(WHITE_BRUSH);
#endif
		int borderSize = max(1, RECTHEIGHT(rc)*.06f);
						// = max(1,GetSystemMetrics(SM_CXSIZEFRAME)-1); //TODO(fran): this doesnt look as good, we need a combination between RECTHEIGHT and GetSystemMetrics

#if 1


		HPEN pen = CreatePen(PS_SOLID, borderSize, ColorFromBrush(pthis->Highlight)); //para el borde

		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, (HBRUSH)GetStockObject(HOLLOW_BRUSH));//para lo de adentro
		HPEN oldpen = (HPEN)SelectObject(hdc, pen);

		//Border
		Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

		SelectObject(hdc, oldbrush);
		SelectObject(hdc, oldpen);
		DeleteObject(pen);
#else
		RECT borderLeft;
		borderLeft.left = rc.left;
		borderLeft.top = rc.top;
		borderLeft.right = borderSize;
		borderLeft.bottom = rc.bottom - borderSize;
		FillRect(hdc, &borderLeft, BorderColor);
		RECT borderRight;
		borderRight.left = rc.right - borderSize;
		borderRight.top = rc.top;
		borderRight.right = rc.right;
		borderRight.bottom = rc.bottom;
		FillRect(hdc, &borderRight, BorderColor);
		RECT borderTop;
		borderTop.left = rc.left;
		borderTop.top = rc.top;
		borderTop.right = rc.right;
		borderTop.bottom = borderSize;
		FillRect(hdc, &borderTop, BorderColor);
		RECT borderBottom;
		borderBottom.left = rc.left;
		borderBottom.top = rc.bottom - borderSize;
		borderBottom.right = rc.right;
		borderBottom.bottom = rc.bottom;
		FillRect(hdc, &borderBottom, BorderColor);
#endif

		DWORD style = GetWindowLongPtr(hWnd, GWL_STYLE);
		if (style & BS_ICON) {//Here will go buttons that have only an icon
			//IMPORTANT INFO: I can create a png (with transparency), then go to a page and export multiple icos in one file,
			// create an ico resource, then delete the file that visual studio creates, change the name that 
			// the resource references to my file
			//HICON icon = (HICON)SendMessageW(hWnd, BM_GETIMAGE, IMAGE_ICON, 0);
			
			//SUPER TODO(fran): there is something wrong here, the icon is not being correctly centered :c

			//SIZE icon_size; 
			DirectX::XMFLOAT2 icon_size; //trying to reduce precision errors
			icon_size.x = RECTWIDTH(rc)*.75f +1; //TODO(fran): should use the smaller of the two axis
			//TODO(fran): why does this +1 -1 work perfectly and aligns the icon correctly in every resolution???
			icon_size.y = icon_size.x;

			//LONG icon_id = GetWindowLongPtr(hWnd, GWL_USERDATA);
			LONG icon_id = SendMessage(hWnd, BM_GETIMAGE, IMAGE_ICON, 0);
			//TODO: should probably use inflaterect

			//INFO: everybody MUST be on the same process for this hinstance to work
			HICON icon = (HICON)LoadImage((HINSTANCE)GetWindowLongPtr(hWnd, GWL_HINSTANCE), MAKEINTRESOURCE(icon_id), IMAGE_ICON, icon_size.x, icon_size.y, LR_SHARED);

			if (icon) {
				//TODO(fran): icon position isnt perfectly centered, probably due to precision error + even versus odd rect and icon sizes

#if 1
				DrawIconEx(hdc, .5f*(((float)RECTWIDTH(rc)) - (icon_size.x-1.f)), .5f*(((float)RECTHEIGHT(rc)) - (icon_size.y-1.f)),
					icon, icon_size.x, icon_size.y, 0, NULL, DI_NORMAL);//INFO: changing that NULL gives the option of "flicker free" icon drawing, if I need
				//INFO: using DI_MASK the button could have an interesting look
#else
				RECT icon_rc = rc;
				icon_rc.left += borderSize;
				icon_rc.top += borderSize;
				icon_rc.right -= borderSize;
				icon_rc.bottom -= borderSize;

				DrawIconEx(hdc, icon_rc.left + RECTWIDTH(icon_rc) / 2 - icon_size.x / 2, icon_rc.top + RECTHEIGHT(icon_rc) / 2 - icon_size.y / 2, icon, icon_size.x, icon_size.y,
					0, NULL, DI_NORMAL);
#endif

				DestroyIcon(icon);
			}
		}
		else { //Here will go buttons that have only text
			HFONT font = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
			if (font) {//if font == NULL then it is using system font(default I assume)
				(HFONT)SelectObject(hdc, (HGDIOBJ)(HFONT)font);
			}
			SetTextColor(hdc, ColorFromBrush(pthis->Highlight));
			WCHAR Text[40];
			int len = SendMessage(hWnd, WM_GETTEXT,
				ARRAYSIZE(Text), (LPARAM)Text);

#if 1
			TEXTMETRIC tm;
			GetTextMetrics(hdc, &tm);
			// Calculate vertical position for the item string so that it will be vertically centered
			int yPos = (rc.bottom + rc.top - tm.tmHeight) / 2;

			SetTextAlign(hdc, TA_CENTER);
			//int xPos = ((rc.right - rc.left) - tm.tmAveCharWidth*len)/2; //not good enough
			int xPos = (rc.right - rc.left) / 2;
			TextOut(hdc, xPos, yPos, Text, len);
#else
			//INFO(fran): interestin look, if I can get the text to render entirely 
			SetTextAlign(hdc, TA_CENTER);
			TextOut(hdc, (rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2, Text, len);
#endif
		}
		EndPaint(hWnd, &ps);

		return 0;
	}
	default:
		return DefSubclassProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK ControlProcedures::ComboProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	static BOOL MouseOverCombo = FALSE;
	static HWND CurrentMouseOverCombo;

	ControlProcedures* pthis = (ControlProcedures*)dwRefData;

	switch (Msg)
	{
	case WM_MOUSEHOVER:
	{
		//force button to repaint and specify hover or not
		if (MouseOverCombo && CurrentMouseOverCombo == hWnd) break;
		MouseOverCombo = true;
		CurrentMouseOverCombo = hWnd;
		InvalidateRect(hWnd, 0, 1);
		break;
	}
	case WM_MOUSELEAVE:
	{
		MouseOverCombo = false;
		CurrentMouseOverCombo = NULL;
		InvalidateRect(hWnd, 0, 1);
		break;
	}
	case WM_MOUSEMOVE:
	{
		//TODO(fran): We are tracking the mouse every single time it moves, kind of suspect solution
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1;
		tme.hwndTrack = hWnd;

		TrackMouseEvent(&tme);
		break;
	}
	//case CBN_DROPDOWN://lets us now that the list is about to be show, therefore the user clicked us
	case WM_PAINT:
	{
		DWORD style = GetWindowLongPtr(hWnd, GWL_STYLE);
		if (!(style & CBS_DROPDOWNLIST))
			break;

		RECT rc;
		GetClientRect(hWnd, &rc);

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		BOOL ButtonState = SendMessageW(hWnd, CB_GETDROPPEDSTATE, 0, 0);
		if (ButtonState) {
			SetBkColor(hdc, ColorFromBrush(pthis->BackgroundPush));
			FillRect(hdc, &rc, pthis->BackgroundPush);
		}
		else if (MouseOverCombo && CurrentMouseOverCombo == hWnd) {
			SetBkColor(hdc, ColorFromBrush(pthis->BackgroundMouseover));
			FillRect(hdc, &rc, pthis->BackgroundMouseover);
		}
		else {
			SetBkColor(hdc, ColorFromBrush(pthis->Background));
			FillRect(hdc, &rc, pthis->Background);
		}

		RECT client_rec;
		GetClientRect(hWnd, &client_rec);

		//TODO
		HPEN pen = CreatePen(PS_SOLID, max(1, (RECTHEIGHT(client_rec))*.01f), ColorFromBrush(pthis->Highlight)); //For the border

		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, (HBRUSH)GetStockObject(HOLLOW_BRUSH));//For the insides
		HPEN oldpen = (HPEN)SelectObject(hdc, pen);

		SelectObject(hdc, (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0));
		//SetBkColor(hdc, bkcolor);
		SetTextColor(hdc, ColorFromBrush(pthis->Highlight));

		//Border
		Rectangle(hdc, 0, 0, rc.right, rc.bottom);

		/*
		if (GetFocus() == hWnd)
		{
			//INFO: with this we know when the control has been pressed
			RECT temp = rc;
			InflateRect(&temp, -2, -2);
			DrawFocusRect(hdc, &temp);
		}
		*/
		int DISTANCE_TO_SIDE = 5;

		int index = SendMessage(hWnd, CB_GETCURSEL, 0, 0);
		if (index >= 0)
		{
			int buflen = SendMessage(hWnd, CB_GETLBTEXTLEN, index, 0);
			TCHAR *buf = new TCHAR[(buflen + 1)];
			SendMessage(hWnd, CB_GETLBTEXT, index, (LPARAM)buf);
			rc.left += DISTANCE_TO_SIDE;
			DrawText(hdc, buf, -1, &rc, DT_EDITCONTROL | DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			delete[]buf;
		}
		RECT r;
		SIZE icon_size;
		GetClientRect(hWnd, &r);
		icon_size.cy = (r.bottom - r.top)*.6f;
		icon_size.cx = icon_size.cy;

		HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWL_HINSTANCE); 
		//LONG icon_id = GetWindowLongPtr(hWnd, GWL_USERDATA);
		//TODO(fran): we could set the icon value on control creation, use GWL_USERDATA
		HICON combo_dropdown_icon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(pthis->ComboIconID), IMAGE_ICON, icon_size.cx, icon_size.cy, LR_SHARED);

		if (combo_dropdown_icon) {
			DrawIconEx(hdc, r.right - r.left - icon_size.cx - DISTANCE_TO_SIDE, ((r.bottom - r.top) - icon_size.cy) / 2,
				combo_dropdown_icon, icon_size.cx, icon_size.cy, 0, NULL, DI_NORMAL);//INFO: changing that NULL gives the option of "flicker free" icon drawing, if I need
			DestroyIcon(combo_dropdown_icon);
		}

		SelectObject(hdc, oldpen);
		SelectObject(hdc, oldbrush);
		//DeleteObject(brush);
		DeleteObject(pen);

		EndPaint(hWnd, &ps);
		return 0;
	}

	//case WM_NCDESTROY:
	//{
	//	RemoveWindowSubclass(hWnd, this->ComboProc, uIdSubclass);//TODO(fran): is this necessary
	//	break;
	//}

	}

	return DefSubclassProc(hWnd, Msg, wParam, lParam);
}

LRESULT CALLBACK ControlProcedures::CheckboxProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	static BOOL MouseOverCheck = FALSE;
	static HWND CurrentMouseOverCheck = NULL;

	ControlProcedures* pthis = (ControlProcedures*)dwRefData;

	switch (Msg)
	{
	case WM_MOUSEHOVER:
	{
		//force button to repaint and specify hover or not
		if (MouseOverCheck && CurrentMouseOverCheck == hWnd) break;
		MouseOverCheck = true;
		CurrentMouseOverCheck = hWnd;
		InvalidateRect(hWnd, 0, 1);
		break;
	}
	case WM_MOUSELEAVE:
	{
		MouseOverCheck = false;
		CurrentMouseOverCheck = NULL;
		InvalidateRect(hWnd, 0, 1);
		break;
	}
	case WM_MOUSEMOVE:
	{
		//TODO(fran): We are tracking the mouse every single time it moves, kind of suspect solution
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1;
		tme.hwndTrack = hWnd;

		TrackMouseEvent(&tme);
		break;
	}
	case WM_PAINT:
	{
		DWORD style = GetWindowLongPtr(hWnd, GWL_STYLE);
		if (!(style & BS_AUTOCHECKBOX))
			break;

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		RECT modified_rc;
		GetClientRect(hWnd, &modified_rc);
		int height = modified_rc.bottom - modified_rc.top;

		int CheckboxWidth = GetSystemMetrics(SM_CXMENUCHECK); //TODO(fran): use this in other places too!, were I have constants

		modified_rc.right = CheckboxWidth;
		modified_rc.top = (height - CheckboxWidth) / 2;
		modified_rc.bottom = modified_rc.top + CheckboxWidth;

		WORD ButtonState = SendMessageW(hWnd, BM_GETSTATE, 0, 0);
		if (ButtonState & BST_PUSHED) {
			SetBkColor(hdc, ColorFromBrush(pthis->BackgroundPush));
			FillRect(hdc, &modified_rc, pthis->BackgroundPush);
		}
		else if (MouseOverCheck && CurrentMouseOverCheck == hWnd) {
			SetBkColor(hdc, ColorFromBrush(pthis->BackgroundMouseover));
			FillRect(hdc, &modified_rc, pthis->BackgroundMouseover);
		}
		else {
			SetBkColor(hdc, ColorFromBrush(pthis->Background));
			FillRect(hdc, &modified_rc, pthis->Background);
		}

		RECT client_rec;
		GetClientRect(hWnd, &client_rec);

		//TODO
		HPEN pen = CreatePen(PS_SOLID, max(1, (RECTHEIGHT(client_rec))*.01f), ColorFromBrush(pthis->Highlight)); //para el borde

		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, (HBRUSH)GetStockObject(HOLLOW_BRUSH));//para lo de adentro
		HPEN oldpen = (HPEN)SelectObject(hdc, pen);

		SelectObject(hdc, (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0));

		SetTextColor(hdc, ColorFromBrush(pthis->Highlight));

		//Border
		Rectangle(hdc, modified_rc.left, modified_rc.top, modified_rc.right, modified_rc.bottom);

		RECT icon_rc = modified_rc;

		//TODO
		int deflation = -max(1, CheckboxWidth * .2f);
		InflateRect(&icon_rc, deflation, deflation);
		HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWL_HINSTANCE); 
		//LONG icon_id = GetWindowLongPtr(hWnd, GWL_USERDATA); 
		HICON checkbox_tick_icon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(pthis->CheckboxIconID), IMAGE_ICON, RECTWIDTH(icon_rc), RECTHEIGHT(icon_rc), LR_SHARED);

		//Draw tick mark if checked
		if (ButtonState & BST_CHECKED && checkbox_tick_icon) { //TODO(fran): make an icon that is easier to see

			DrawIconEx(hdc, icon_rc.left, icon_rc.top,
				checkbox_tick_icon, RECTWIDTH(icon_rc), RECTHEIGHT(icon_rc), 0, NULL, DI_NORMAL);
		}
		if (checkbox_tick_icon) {
			DestroyIcon(checkbox_tick_icon);
		}

		WCHAR Text[150];
		int len = SendMessage(hWnd, WM_GETTEXT,	ARRAYSIZE(Text), (LPARAM)Text);

		TEXTMETRIC tm;
		GetTextMetrics(hdc, &tm);
		// Calculate vertical position for the item string so that it will be vertically centered
		RECT unmodified_rc;
		GetClientRect(hWnd, &unmodified_rc);
		int yPos = (unmodified_rc.bottom + unmodified_rc.top - tm.tmHeight) / 2;

		int xPos = modified_rc.right*1.5f;
		LOGBRUSH background_lb;
		GetObject(pthis->Background, sizeof(LOGBRUSH), &background_lb);

		SetBkColor(hdc, background_lb.lbColor);
		TextOut(hdc, xPos, yPos, Text, len);

		SelectObject(hdc, oldpen);
		SelectObject(hdc, oldbrush);
		//DeleteObject(brush);
		DeleteObject(pen);
		EndPaint(hWnd, &ps);
		return 0;
	}
	//case WM_NCDESTROY:
	//{
	//	RemoveWindowSubclass(hWnd, this->CheckboxProc, uIdSubclass);
	//	break;
	//}

	}

	return DefSubclassProc(hWnd, Msg, wParam, lParam);
}

LRESULT CALLBACK ControlProcedures::CaptionButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	static BOOL MouseOver = false;
	static HWND CurrentMouseOverButton;

	ControlProcedures* pthis = (ControlProcedures*)dwRefData;

	switch (message) {
	case WM_MOUSEHOVER:
	{
		//force button to repaint and specify hover or not
		if (MouseOver && CurrentMouseOverButton == hwnd) break;
		MouseOver = true;
		CurrentMouseOverButton = hwnd;
		InvalidateRect(hwnd, 0, 1);
		break;
	}
	case WM_MOUSELEAVE:
	{
		MouseOver = false;
		CurrentMouseOverButton = NULL;
		InvalidateRect(hwnd, 0, 1);
		break;
	}
	case WM_MOUSEMOVE:
	{
		//TODO(fran): We are tracking the mouse every single time it moves, kind of suspect solution
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1;
		tme.hwndTrack = hwnd;

		TrackMouseEvent(&tme);
		break;
	}
	case WM_ERASEBKGND:
	{
		return 1;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		RECT rc; GetClientRect(hwnd, &rc);
		//int controlID = GetDlgCtrlID(hWnd);

		WORD ButtonState = SendMessageW(hwnd, BM_GETSTATE, 0, 0);
		if (ButtonState & BST_PUSHED) {
			SetBkColor(hdc, ColorFromBrush(pthis->BackgroundPush));
			FillRect(hdc, &rc, pthis->BackgroundPush);
		}
		else if (MouseOver && CurrentMouseOverButton == hwnd) {
			SetBkColor(hdc, ColorFromBrush(pthis->BackgroundMouseover));
			FillRect(hdc, &rc, pthis->BackgroundMouseover);
		}
		else {
			SetBkColor(hdc, ColorFromBrush(pthis->CaptionBackground));
			FillRect(hdc, &rc, pthis->CaptionBackground);
		}

		//POINT cross[4];//INFO: nice test, unfortunately no antialiasing
		//cross[0] = {RECTWIDTH(rc)/2,RECTHEIGHT(rc)};//pto de abajo
		//cross[1] = {rc.left,RECTHEIGHT(rc)/2};//pto de medio izq
		//cross[2] = { RECTWIDTH(rc) / 2 ,rc.top};//pto de arriba
		//cross[3] = { RECTWIDTH(rc) ,RECTHEIGHT(rc) /2};//pto de medio der
		//HRGN cross_rgn = CreatePolygonRgn(cross, 4, ALTERNATE);//WINDING
		//SelectClipRgn(hdc, cross_rgn);//INFO: coordinates MUST be in device units
		//FillRect(hdc, &rc, BorderColor);
		//SelectClipRgn(hdc, NULL);

		//LONG icon_id = GetWindowLongPtr(hwnd, GWL_USERDATA);
		DWORD style = GetWindowLongPtr(hwnd, GWL_STYLE);

		LONG icon_id = NULL;

		//This windows should always have an icon so no need to test with BS_ICON
		if (style & WS_MINIMIZEBOX) icon_id = pthis->CaptionMinimizeIconID; //Is a minimize button
		else if (style & WS_MAXIMIZEBOX)icon_id = pthis->CaptionCloseIconID; //Is a maximize button

		if (icon_id) {
			SIZE icon_size;
			icon_size.cx = RECTHEIGHT(rc)*.55f;
			icon_size.cy = icon_size.cx;

			HICON icon = (HICON)LoadImage((HINSTANCE)GetWindowLongPtr(hwnd, GWL_HINSTANCE), MAKEINTRESOURCE(icon_id), IMAGE_ICON, icon_size.cx, icon_size.cy, LR_SHARED);

			if (icon) {
				DrawIconEx(hdc, .5f*(RECTWIDTH(rc) - icon_size.cx), .5f*(RECTHEIGHT(rc) - icon_size.cy),
					icon, icon_size.cx, icon_size.cy, 0, NULL, DI_NORMAL);
				DestroyIcon(icon);
			}
		}

		EndPaint(hwnd, &ps);

		return 0;
	}
	default:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK ControlProcedures::SecretButtonProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	switch (Msg) {
	case WM_ERASEBKGND: return 1;
	case WM_PAINT:
	{
		ControlProcedures* pthis = (ControlProcedures*)dwRefData;
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		LOGBRUSH lb;
		GetObject(pthis->Background, sizeof(LOGBRUSH), &lb);
		SetBkColor(hdc, lb.lbColor);
		SetTextColor(hdc, lb.lbColor);
		RECT rc;
		GetClientRect(hWnd, &rc);
		FillRect(hdc, &rc, pthis->Background);
		EndPaint(hWnd, &ps);
		return 0;
	}
	default: return DefSubclassProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

//TODO(fran): if the user wants to write his/her own key again after say, testing how the control works, he/she wont be able
// to do it, as this control detects already registerd hotkeys and bans them, even your own
//SUPER TODO(fran): Man I hate this control, let me write what I want, later when I want to apply you can decide if you let me or not

LRESULT CALLBACK ControlProcedures::HotkeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {

	//TODO(fran): if we put multiple hotkey controls this has to change
	static BOOL accepted_hotkey = FALSE;
	static BOOL new_valid_hotkey = FALSE;//Right after we have a new valid hotkey paint it another color

	ControlProcedures* pthis = (ControlProcedures*)dwRefData;

	switch (Msg) {
	case WM_ERASEBKGND: { return 1; }//INFO: erasebkgnd only erases the client area, so it is ok, then wm_paint goes over it
	case SCV_HOTKEY_RESET_TEXT: { //TODO(fran): botched!
		//INFO: receives the modifiers in the normal format, so a conversion to the one used by the control is needed
		SendMessage(hWnd,
			HKM_SETHOTKEY,
			MAKEWORD((UINT)wParam, HotkeyModifiersToHotkeyControlModifiers((UINT)lParam)),
			0);
		break;
	}
	case SCV_HOTKEY_REG_SUCCESS:
	{
		accepted_hotkey = TRUE;
		new_valid_hotkey = TRUE;
		RECT r; GetClientRect(hWnd, &r);
		InvalidateRect(hWnd, &r, FALSE);
		SetTimer(hWnd, 1, 5000, NULL);
		break;
	}
	case WM_TIMER: {
		KillTimer(hWnd, 1);
		new_valid_hotkey = FALSE;
		RECT r; GetClientRect(hWnd, &r);
		InvalidateRect(hWnd, &r, FALSE);
		break;
	}
	case SCV_HOTKEY_REG_FAILED:
	{
		//If registration failed then the value we currently have is incorrect, therefore paint it red
		accepted_hotkey = FALSE;
		RECT r; GetClientRect(hWnd, &r);
		InvalidateRect(hWnd, &r, FALSE);
		break;
	}
	case WM_NCPAINT:
	{
		//Thank god for this person
		//https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/a407591a-4b1e-4adc-ab0b-3c8b3aec3153/the-evil-wmncpaint?forum=windowsuidevelopment

		//Get only the frame area to paint
#define DCX_USESTYLE 0x00010000

		HDC hotframedc = GetDCEx(hWnd, 0, DCX_WINDOW | DCX_USESTYLE);

		RECT RC, RW;
		GetClientRect(hWnd, &RC);
		GetWindowRect(hWnd, &RW);//INFO: this function returns the value in screen coordinates, it is NOT the position inside the parent hwnd
		SIZE P2; P2.cx = RW.left; P2.cy = RW.top;
		MapWindowPoints(0, hWnd, (LPPOINT)&RW, 2);
		OffsetRect(&RC, -RW.left, -RW.top);
		OffsetRect(&RW, -RW.left, -RW.top);
		HRGN TempRgn = nullptr;
#if 0
		if (wParam == 1 || wParam == 0) {
			// 1 means entire area (undocumented)
			ExcludeClipRect(hotframedc, RC.left, RC.top, RC.right, RC.bottom);
			TempRgn = 0;
		}
		else {
			TempRgn = CreateRectRgn(RC.left + P2.cx, RC.top + P2.cy, RC.right + P2.cx, RC.bottom + P2.cy);
			if (CombineRgn(TempRgn, (HRGN)wParam, TempRgn, RGN_DIFF) == NULLREGION) // nothing to paint, you can take a shortcut
				OffsetRgn(TempRgn, -P2.cx, -P2.cy);

			ExtSelectClipRgn(hotframedc, TempRgn, RGN_AND);
		}
#endif
		// paint your borders here
		//For Rectangle: border -> pen ; fill -> brush
		HBRUSH old_brush = (HBRUSH)SelectObject(hotframedc, (HBRUSH)GetStockObject(HOLLOW_BRUSH));

		RECT client_rec;
		GetClientRect(hWnd, &client_rec);

		//TODO
		HPEN pen = CreatePen(PS_SOLID, max(1, (RECTHEIGHT(client_rec))*.01f), ColorFromBrush(pthis->Highlight)); //para el borde

		HPEN old_pen = (HPEN)SelectObject(hotframedc, pen);
		Rectangle(hotframedc, RW.left, RW.top, RW.right, RW.bottom);
		//FillRect(hotframedc, &RW, (HBRUSH)GetStockObject(GRAY_BRUSH));

		if (old_brush) SelectObject(hotframedc, old_brush);
		if (old_pen) SelectObject(hotframedc, old_pen);

		ReleaseDC(hWnd, hotframedc);
		if (TempRgn != 0) DeleteObject(TempRgn);

		if(pen) DeleteObject(pen);

		// Paint into this DC 
		//RECT rect;
		//GetWindowRect(hWnd, &rect);
		//FillRect(hotframedc, &rect, (HBRUSH)GetStockObject(GRAY_BRUSH));
		//ReleaseDC(hWnd, hotframedc);

		return 0;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC hotdc = BeginPaint(hWnd, &paint);
		
		SetBkColor(hotdc, ColorFromBrush(pthis->Background));
		if (new_valid_hotkey) {
			SetTextColor(hotdc, RGB(0, 255, 0));//TODO(fran): I leave this and the red hardcoded for now
		}
		else if (accepted_hotkey) {
			SetTextColor(hotdc, ColorFromBrush(pthis->Highlight));
		}
		else {
			SetTextColor(hotdc, RGB(255, 0, 0));
		}
		HFONT font = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
		HFONT old_font = nullptr;
		if (font) {//if font == NULL then it is using system font(default I assume)
			old_font = (HFONT)SelectObject(hotdc, (HGDIOBJ)(HFONT)font);
		}

		FillRect(hotdc, &paint.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));

		WORD hotkey = SendMessage(hWnd, HKM_GETHOTKEY, 0, 0);
		BYTE vk = LOBYTE(hotkey);
		BYTE mods = HIBYTE(hotkey);

		std::wstring HotkeyString = ControlHotkeyString(mods, vk);

		TEXTMETRIC tm;
		GetTextMetrics(hotdc, &tm);
		// Calculate vertical position for the item string so that it will be vertically centered
		RECT rec;
		GetClientRect(hWnd, &rec);
		int yPos = (rec.bottom + rec.top - tm.tmHeight) / 2;

		TextOutW(hotdc, 0, yPos, HotkeyString.c_str(), HotkeyString.length());

		SIZE text_size;
		GetTextExtentPoint32W(hotdc, HotkeyString.c_str(), HotkeyString.length(), &text_size);
		SetCaretPos(text_size.cx, yPos);

		if (font) {
			SelectObject(hotdc, (HGDIOBJ)(HFONT)old_font);
		}

		ReleaseDC(hWnd, hotdc);
		EndPaint(hWnd, &paint);
		return 0;
	}
	default: return DefSubclassProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}