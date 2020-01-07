#pragma once
#include <windows.h>
#include <vector>
#include <Shlobj.h>
// Tooltip: small comments when mouseover
class TOOLTIP_REPO
{
public:

	static TOOLTIP_REPO& Instance() //https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
	{
		static TOOLTIP_REPO instance; // Guaranteed to be destroyed.
							          // Instantiated on first use.
		return instance;
	}

	TOOLTIP_REPO(TOOLTIP_REPO const&) = delete;
	void operator=(TOOLTIP_REPO const&) = delete;

	void ActivateTooltips(BOOL activate);

	//· Create a tooltip for a control that can manage it, eg buttons
	//· If the tooltip was created successfully then it is added to the repo
	HWND CreateToolTip(int toolID, HWND hDlg, WORD messageID);
	
	//· Create a tooltip for the entire window of a control that can manage it
	//· If the tooltip was created successfully then it is added to the repo
	HWND CreateToolTipForRect(HWND hwndParent, WORD messageID);
	
	//· Create a tooltip for a tool(control) that cant handle tooltips by itself, inside a parent window
	//· If the tooltip was created successfully then it is added to the repo
	HWND CreateToolTipForRect(HWND hwndParent, HWND hwndTool, WORD messageID);

	HINSTANCE SetHInstance(HINSTANCE hInst);
private:
	TOOLTIP_REPO();
	~TOOLTIP_REPO();

	HINSTANCE hInstance=NULL;//TODO(fran): should we ask for the instance to each control? for now we dont really need it

	BOOL active = FALSE;
	std::vector<HWND> tooltips;

	void AddTooltip(HWND tooltip);

};

