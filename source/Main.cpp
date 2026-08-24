/*
	Dear ImGui wrapper for debugmenu by aap

	This aims to provide an alternative for debugmenu that is a little more modern while being compatible with existing plugins.
*/



#include "plugin.h"
#include <d3d9.h>
#ifndef GTASA
#include "rwd3d9/rwd3d9.h"
#endif
#include "gui/dx9hook.h"
#include "gui/imgui/imgui.h"
#include "debugmenu.h"
#include <utility>

using namespace plugin;

// One saved original per call site. Reading the original from a single site and installing
// that at every other site hands whatever hooked THAT site the job of updating the pad at
// call sites it never patched - and if its hook is not a plain pass-through, every one of
// those sites breaks. SkyUI.asi hooks 0x53BEE6 in SA with a one-shot initialiser that
// returns without calling CPad::UpdatePads once it has run, which is exactly that case.
static void (*CPadUpdateCall[16])();

static bool DebugMenuHandledPad()
{
	if (!TheMenu.m_bIsActive)
		return false;

	// Clear mouse data so the camera won't move when closing the debug menu
	CPad* pad = CPad::GetPad(0);
	CPad::UpdatePads();
	CPad::NewMouseControllerState.x = 0.0f;
	CPad::NewMouseControllerState.y = 0.0f;
	pad->NewMouseControllerState.x = 0.0f;
	pad->NewMouseControllerState.y = 0.0f;
#ifdef GTA3
	pad->ClearMouseHistory();
#else
	CPad::ClearMouseHistory();
#endif

	pad->NewState.DPadUp = 0;
	pad->OldState.DPadUp = 0;
	pad->NewState.DPadDown = 0;
	pad->OldState.DPadDown = 0;
	pad->DisablePlayerControls = true;
	return true;
}

template<size_t I>
void __cdecl CPad__UpdatePadsHook()
{
	if (!DebugMenuHandledPad())
		CPadUpdateCall[I]();
}

template<size_t... I>
static void HookPadCalls(const uintptr_t *sites, std::index_sequence<I...>)
{
	((CPadUpdateCall[I] = reinterpret_cast<void(*)()>(injector::ReadRelativeOffset(sites[I] + 1).as_int()),
	  injector::MakeCALL(sites[I], CPad__UpdatePadsHook<I>)), ...);
}

template<size_t N>
static void HookPadCalls(const uintptr_t (&sites)[N])
{
	static_assert(N <= sizeof(CPadUpdateCall) / sizeof(CPadUpdateCall[0]), "grow CPadUpdateCall");
	HookPadCalls(sites, std::make_index_sequence<N>{});
}

#ifdef GTASA
// SA func (Missing from plugin-sdk, so we add it ourselves)
void __cdecl RsMouseSetPos(RwV2d* pos) {
	Call<0x6194A0, RwV2d*>(pos);
}

void __cdecl RsMouseSetPosHook(RwV2d* pos) // this is so game doesnt keep centering mouse pointer
{
	if (TheMenu.m_bIsActive)
		return;
	else
		RsMouseSetPos(pos);
}
#else
BOOL __stdcall SetCursorPosHook(int x, int y) // this is so game doesnt keep centering mouse pointer
{
	if (TheMenu.m_bIsActive)
		return false;
	else
		return SetCursorPos(x, y);
}
#endif

class debugmenuimgui
{
public:
	debugmenuimgui()
	{
		auto h = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(DX9Hook_Thread), 0, NULL, 0);

		if (!(h == nullptr)) CloseHandle(h);

#ifdef GTA3
		static const uintptr_t padCalls[] = {
			0x48C850, 0x48AE15, 0x48DE2F, 0x48E717, 0x582AA9, 0x582C3C, 0x592C0C
		};
		HookPadCalls(padCalls);

		patch::SetPointer(0x61D4E4, SetCursorPosHook);
#elif GTAVC
		// update pads in cgame proccess, and everywhere else it is called from
		static const uintptr_t padCalls[] = {
			0x4A4412, 0x490476, 0x4A5C7E, 0x4A669F, 0x4AB0A0, 0x54460C,
			0x5FFFD9, 0x60018F, 0x61D9F4, 0x61DBB6, 0x61DD47
		};
		HookPadCalls(padCalls);

		injector::MakeCALL(0x602115, SetCursorPosHook);
#elif GTASA
		// update pads in cgame proccess, and everywhere else it is called from
		static const uintptr_t padCalls[] = {
			0x53BEE6, 0x53E78B, 0x57C607, 0x57D7C5, 0x731540, 0x748B17
		};
		HookPadCalls(padCalls);

		injector::MakeCALL(0x53E9F1, RsMouseSetPosHook);
#endif

		Events::initGameEvent.after += []
			{
				TheMenu.m_bCanBeActivated = true;
			};

		// we use ermaccer's hooks except endscene one as skygfx radiosity makes menu look awkward
#ifdef GTA3
		CdeclEvent<AddressList<0x48E6DF, H_CALL>, PRIORITY_AFTER, ArgPickNone, void(void)> CCreditsRenderEvent;
		CCreditsRenderEvent.after += []() {
#elif GTAVC
		CdeclEvent<AddressList<0x4A613D, H_CALL>, PRIORITY_AFTER, ArgPickNone, void(void)> CFontRenderEvent;
		CFontRenderEvent += []() {
#elif GTASA
		CdeclEvent<AddressList<0x53EBB1, H_CALL>, PRIORITY_AFTER, ArgPickNone, void(void)> CFontRenderEvent;
		CFontRenderEvent.after += [] {
#endif
			GUIImplementationDX9::OnEndScene((LPDIRECT3DDEVICE9)RwD3D9GetCurrentD3DDevice());
			};
		};
			} debugmenuimguiPlugin;
