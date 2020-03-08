#pragma once
#include <windows.h>
#include <vector>
#include <Shlobj.h>
// Tooltip: small comments when mouseover
class TOOLTIP_REPO
{
public:

	/// <summary>
	/// Retrieves the current instance of the Tooltip Repo
	/// </summary>
	static TOOLTIP_REPO& Instance() //https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
	{
		static TOOLTIP_REPO instance; // Guaranteed to be destroyed.
							          // Instantiated on first use.
		return instance;
	}

	TOOLTIP_REPO(TOOLTIP_REPO const&) = delete;
	void operator=(TOOLTIP_REPO const&) = delete;

	/// <summary>
	/// Enables or disables tooltips for the whole application
	/// </summary>
	/// <param name="activate">TRUE enables tooltips, FALSE disables them</param>
	void ActivateTooltips(BOOL activate);

	/// <summary>
	/// Create a tooltip for a control that can manage it, eg buttons, trackbars, static and more
	/// <para>For Static controls make sure to add the SS_NOTIFY style</para>
	/// </summary>
	/// <param name="hWnd">The control that will show the tooltip</param>
	/// <param name="messageID">Resource String's ID</param>
	/// <returns>The hwnd of the tooltip control</returns>
	HWND CreateToolTip(HWND hWnd, WORD messageID);
	
	/// <summary>
	/// A hack to create a tooltip for the RECT of a window
	/// <para>Use only if CreateToolTip() doesn't work</para>
	/// </summary>
	HWND CreateToolTipForRect(HWND hwndParent, WORD messageID);
	
	//· Create a tooltip for a tool(control) that cant handle tooltips by itself, inside a parent window
	/// <summary>
	/// A hack to create a tooltip for a control that can't handle them
	/// <para>Basically adds it to the parent and assigns a RECT the size of your control</para>
	/// <para>If anything is resized the position of the tooltip will change</para>
	/// <para>Use only if CreateToolTip() doesn't work</para>
	/// </summary>
	HWND CreateToolTipForRect(HWND hwndParent, HWND hwndTool, WORD messageID);

	//TODO(fran): I think the hInstance should be obtained from each hwnd
	/// <summary>
	/// Set the hInstance that will be used by every tooltip to retrieve their corresponding text string
	/// </summary>
	/// <returns>The previuos hInstance</returns>
	HINSTANCE SetHInstance(HINSTANCE hInst);

private:
	TOOLTIP_REPO();
	~TOOLTIP_REPO();

	HINSTANCE hInstance=NULL;//TODO(fran): should we ask for the instance to each control? for now we dont really need it

	BOOL active = FALSE;
	std::vector<HWND> tooltips;

	void AddTooltip(HWND tooltip);

};

