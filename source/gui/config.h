#pragma once
#include <Windows.h>
#include <string>

/*
	Minimal INI-style config for debugmenu-imgui.

	File lives next to debugmenu.dll as "debugmenu.ini". If it doesn't exist,
	it's created with defaults on first run. Kept dependency-free (no external
	INI library) since we only need a handful of flat key=value pairs.

	Example file:
		[DebugMenu]
		ToggleKey=M
		ToggleModifierCtrl=1
		ToggleModifierShift=0
		ToggleModifierAlt=0
		FontScale=1.0
		SaveWindowPosition=1
*/
class DebugMenuConfig {
public:
	// Toggle hotkey. Modifier flags are independent so combos like Ctrl+Shift+M work.
	unsigned char m_toggleKeyVK;   // virtual-key code, e.g. 'M'
	bool m_toggleModCtrl;
	bool m_toggleModShift;
	bool m_toggleModAlt;

	float m_fontScale;
	bool m_bSaveWindowPosition;

	static DebugMenuConfig& Get();

	void Load();
	void Save() const; // writes current values back out (used to materialize defaults on first run)

	// Returns true if the given WM_KEYDOWN wParam + current modifier state matches the configured toggle combo.
	bool IsToggleCombo(WPARAM vkCode) const;

	// Human-readable form for UI/tooltip purposes, e.g. "Ctrl+M".
	std::string ToDisplayString() const;

private:
	DebugMenuConfig();

	std::wstring GetIniPath() const;
};
