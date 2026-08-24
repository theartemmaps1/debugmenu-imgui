#include "config.h"
#include "log.h"
#include <cstdlib>
#include <cwctype>

static const wchar_t* kIniName = L"debugmenu.ini";
static const wchar_t* kSection = L"DebugMenu";

DebugMenuConfig::DebugMenuConfig()
	: m_toggleKeyVK('M')
	, m_toggleModCtrl(true)
	, m_toggleModShift(false)
	, m_toggleModAlt(false)
	, m_fontScale(1.0f)
	, m_bSaveWindowPosition(true)
{
}

DebugMenuConfig& DebugMenuConfig::Get()
{
	static DebugMenuConfig instance;
	return instance;
}

std::wstring DebugMenuConfig::GetIniPath() const
{
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);

	wchar_t* end = wcsrchr(path, L'\\');
	if (end)
		end[1] = 0x00;

	std::wstring result(path);
	result += kIniName;
	return result;
}

// Very small helper: single uppercase letter/digit -> VK code. Falls back to 'M' if unparseable.
static unsigned char ParseKeyName(const wchar_t* name)
{
	if (!name || !name[0])
		return 'M';

	wchar_t c = towupper(name[0]);
	if ((c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9'))
		return (unsigned char)c;

	// Function keys F1-F12
	if ((c == L'F') && name[1])
	{
		int n = _wtoi(name + 1);
		if (n >= 1 && n <= 12)
			return (unsigned char)(VK_F1 + (n - 1));
	}

	return 'M';
}

static std::wstring KeyToName(unsigned char vk)
{
	if (vk >= VK_F1 && vk <= VK_F12)
	{
		wchar_t buf[4];
		swprintf(buf, 4, L"F%d", (vk - VK_F1) + 1);
		return buf;
	}
	wchar_t buf[2] = { (wchar_t)vk, 0 };
	return buf;
}

void DebugMenuConfig::Load()
{
	std::wstring iniPath = GetIniPath();

	// If missing, write defaults so the file is discoverable/editable right away.
	DWORD attrs = GetFileAttributesW(iniPath.c_str());
	if (attrs == INVALID_FILE_ATTRIBUTES)
	{
		Save();
		eLog::Message(__FUNCTION__, "INFO: No config found, wrote defaults to debugmenu.ini");
		return;
	}

	wchar_t buf[32];

	GetPrivateProfileStringW(kSection, L"ToggleKey", L"M", buf, 32, iniPath.c_str());
	m_toggleKeyVK = ParseKeyName(buf);

	m_toggleModCtrl = GetPrivateProfileIntW(kSection, L"ToggleModifierCtrl", 1, iniPath.c_str()) != 0;
	m_toggleModShift = GetPrivateProfileIntW(kSection, L"ToggleModifierShift", 0, iniPath.c_str()) != 0;
	m_toggleModAlt = GetPrivateProfileIntW(kSection, L"ToggleModifierAlt", 0, iniPath.c_str()) != 0;

	GetPrivateProfileStringW(kSection, L"FontScale", L"1.0", buf, 32, iniPath.c_str());
	m_fontScale = (float)_wtof(buf);
	if (m_fontScale <= 0.1f || m_fontScale > 8.0f) // sanity clamp against a bad/garbage value in the file
		m_fontScale = 1.0f;

	m_bSaveWindowPosition = GetPrivateProfileIntW(kSection, L"SaveWindowPosition", 1, iniPath.c_str()) != 0;

	eLog::Message(__FUNCTION__, "INFO: Loaded config - ToggleKey=%s FontScale=%.2f",
		ToDisplayString().c_str(), m_fontScale);
}

void DebugMenuConfig::Save() const
{
	std::wstring iniPath = GetIniPath();

	std::wstring keyName = KeyToName(m_toggleKeyVK);
	WritePrivateProfileStringW(kSection, L"ToggleKey", keyName.c_str(), iniPath.c_str());
	WritePrivateProfileStringW(kSection, L"ToggleModifierCtrl", m_toggleModCtrl ? L"1" : L"0", iniPath.c_str());
	WritePrivateProfileStringW(kSection, L"ToggleModifierShift", m_toggleModShift ? L"1" : L"0", iniPath.c_str());
	WritePrivateProfileStringW(kSection, L"ToggleModifierAlt", m_toggleModAlt ? L"1" : L"0", iniPath.c_str());

	wchar_t scaleBuf[32];
	swprintf(scaleBuf, 32, L"%.2f", m_fontScale);
	WritePrivateProfileStringW(kSection, L"FontScale", scaleBuf, iniPath.c_str());

	WritePrivateProfileStringW(kSection, L"SaveWindowPosition", m_bSaveWindowPosition ? L"1" : L"0", iniPath.c_str());
}

bool DebugMenuConfig::IsToggleCombo(WPARAM vkCode) const
{
	if (vkCode != m_toggleKeyVK)
		return false;

	bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	bool altDown = (GetKeyState(VK_MENU) & 0x8000) != 0;

	return ctrlDown == m_toggleModCtrl && shiftDown == m_toggleModShift && altDown == m_toggleModAlt;
}

std::string DebugMenuConfig::ToDisplayString() const
{
	std::string result;
	if (m_toggleModCtrl) result += "Ctrl+";
	if (m_toggleModShift) result += "Shift+";
	if (m_toggleModAlt) result += "Alt+";

	std::wstring keyName = KeyToName(m_toggleKeyVK);
	result += std::string(keyName.begin(), keyName.end());
	return result;
}
