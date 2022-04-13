#include <Windows.h>
//#include <D3DCommon.h>
//#include <dxgi.h>
//#include <d3d11.h>

#include "../detourxs-master/detourxs.h"
#include "skse64_common/Utilities.h"
#include "skse64_common/skse_version.h"

//#define SAFE_RELEASE(x) if((x)) {x->Release(); x = nullptr;}
//
//
//static IDXGISwapChain* tempSwapChain = nullptr;
//static ID3D11Device* tempDevice = nullptr;
//static ID3D11DeviceContext* tempContext = nullptr;
//
////Real Device Pointers
//static ID3D11Device* g_device = nullptr;
//static ID3D11DeviceContext* g_context = nullptr;
//static IDXGISwapChain* g_swapChain = nullptr;
//
////SwapChain Vtable Adressss
//static DWORD_PTR* swapChainVtable = nullptr;
//static DWORD_PTR* contextVTable = nullptr;
//
//
//DWORD_PTR Hook(DWORD_PTR *VMT, int index, DWORD_PTR ourFunc, DWORD_PTR *oldFunc) {
//	/* We return the original function address for them */
//	DWORD_PTR returnAddy = VMT[index];
//	*oldFunc = returnAddy;
//
//	/* Allow us to edit the spot */
//	DWORD oldProtect;
//	VirtualProtect((LPVOID)(VMT + index), 8, PAGE_EXECUTE_READWRITE, &oldProtect);
//
//	/* Set the address as our function */
//	DWORD_PTR * funcAddy = VMT + index;
//	*funcAddy = ourFunc;
//
//	/* Return it to the original protection */
//	VirtualProtect((LPVOID)(VMT + index), 4, oldProtect, &oldProtect);
//
//	return returnAddy;
//}
//
//typedef void(__stdcall *ClearStateHook) (ID3D11DeviceContext* This);
//static ClearStateHook oClearStateHook = NULL;
//void  __stdcall hookClearState(ID3D11DeviceContext* This)
//{
//	static int foo = 60 * 5;
//	foo++;
//	if (foo > 60 * 5) {
//		foo = 0;
//		logger.error("hookClearState\n");
//	}
//
//	//scaleTest();
//
//	return oClearStateHook(This);
//}
//
//typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
//static D3D11PresentHook oPresent = NULL;
//
////Present Function
//HRESULT __stdcall hookD3D11Present(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
//{
//	return oPresent(This, SyncInterval, Flags);
//}
//
//void ReleaseCreatedDevice()
//{
//	SAFE_RELEASE(tempContext);
//	SAFE_RELEASE(tempDevice);
//	SAFE_RELEASE(tempSwapChain);
//}
//
//
//DetourXS clearHookDetour;
//typedef HRESULT(__stdcall *D3D11CreateDeviceAndSwapChainHook) (
//	_In_opt_        IDXGIAdapter         *pAdapter,
//	D3D_DRIVER_TYPE      DriverType,
//	HMODULE              Software,
//	UINT                 Flags,
//	_In_opt_  const D3D_FEATURE_LEVEL    *pFeatureLevels,
//	UINT                 FeatureLevels,
//	UINT                 SDKVersion,
//	_In_opt_  const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
//	_Out_opt_       IDXGISwapChain       **ppSwapChain,
//	_Out_opt_       ID3D11Device         **ppDevice,
//	_Out_opt_       D3D_FEATURE_LEVEL    *pFeatureLevel,
//	_Out_opt_       ID3D11DeviceContext  **ppImmediateContext
//	);
//static D3D11CreateDeviceAndSwapChainHook oCreateHook = NULL;
//
//
//
//HRESULT __stdcall D3D11CreateDeviceAndSwapChain2(
//	_In_opt_        IDXGIAdapter         *pAdapter,
//	D3D_DRIVER_TYPE      DriverType,
//	HMODULE              Software,
//	UINT                 Flags,
//	_In_opt_  const D3D_FEATURE_LEVEL    *pFeatureLevels,
//	UINT                 FeatureLevels,
//	UINT                 SDKVersion,
//	_In_opt_  const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
//	_Out_opt_       IDXGISwapChain       **ppSwapChain,
//	_Out_opt_       ID3D11Device         **ppDevice,
//	_Out_opt_       D3D_FEATURE_LEVEL    *pFeatureLevel,
//	_Out_opt_       ID3D11DeviceContext  **ppImmediateContext
//) {
//	logger.error("Create Device and SwapChain called\n");
//	HRESULT res = oCreateHook(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels
//			, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
//
//	logger.error("Hooking ClearState\n");
//	contextVTable = (DWORD_PTR*)*ppImmediateContext;
//	contextVTable = (DWORD_PTR*)contextVTable[0];
//	Hook(contextVTable, 110, (DWORD_PTR)hookClearState, (UINT_PTR *)&oClearStateHook);
//	logger.error("ClearState Hooked\n");
//	return res;
//}
//
//
//
//
//DWORD __stdcall HookCreateFn(LPVOID)
//{
//	HWND hWnd = GetForegroundWindow();
//	if (hWnd == nullptr)
//		return false;
//	logger.error("hook CreateDevice\n");
//	clearHookDetour.Create((LPVOID)D3D11CreateDeviceAndSwapChain, D3D11CreateDeviceAndSwapChain2, &(LPVOID)oCreateHook);
//	//oCreateHook = (D3D11CreateDeviceAndSwapChainHook)clearHookDetour.GetTrampoline();
//	logger.error("Hook Complete\n");
//
//	return 0;
//}
//
//



//OLD METHOD

typedef UINT64 (__cdecl *renderHook) (void* This, UINT64 arg);

renderHook orender = nullptr;

void updateActors();
UINT64 __cdecl Render(void *This, UINT64 arg)
{
	updateActors();
	return orender(This, arg);
}

#ifdef RUNTIME_VR_VERSION_1_4_15

RelocPtr <void*> ProcessTasks_HookTarget_Enter(0x005BAB10); //For VR 1.4.15

#elif RUNTIME_VERSION_1_6_353
RelocPtr <void*> ProcessTasks_HookTarget_Enter(0x005DACE0); //For SSE 1.6.353 and up
#elif RUNTIME_VERSION_1_6_342

RelocPtr <void*> ProcessTasks_HookTarget_Enter(0x005DAE80); //For SSE 1.6.323 and up


#elif RUNTIME_VERSION_1_6_323

RelocPtr <void*> ProcessTasks_HookTarget_Enter(0x005D9CC0); //For SSE 1.6.323 and up

#elif RUNTIME_VERSION_1_6_318

RelocPtr <void*> ProcessTasks_HookTarget_Enter(0x005D9F50); //For SSE 1.6.318 and up

#elif RUNTIME_VERSION_1_5_97 || RUNTIME_VERSION_1_5_80 || RUNTIME_VERSION_1_5_73

RelocPtr <void*> ProcessTasks_HookTarget_Enter(0x005B2FF0); //For SSE 1.5.73 and up

#elif RUNTIME_VERSION_1_5_62 || RUNTIME_VERSION_1_5_53 || RUNTIME_VERSION_1_5_50

RelocPtr <void*> ProcessTasks_HookTarget_Enter(0x005B31E0); //For SSE 1.5.50 - 1.5.62

#elif RUNTIME_VERSION_1_5_39

RelocPtr <void*> ProcessTasks_HookTarget_Enter(0x005B34A0);  //For SSE 1.5.39

#elif RUNTIME_VERSION_1_5_23

RelocPtr <void*> ProcessTasks_HookTarget_Enter(0x005B2EF0); //For SSE 1.5.23

#endif

DetourXS renderDetour;

void DoHookOLD() 
{
	_MESSAGE("Attempting Game Hook with old");
	renderDetour.Create(ProcessTasks_HookTarget_Enter.GetPtr(), Render, &(LPVOID)orender);
}


//-----------------------------------------------------------------------------------------------
//New Method

typedef void(*_BSTaskPool_ProcessTaskQueue) (void* thisPtr);

_BSTaskPool_ProcessTaskQueue orig_BSTaskPool_ProcessTaskQueue = nullptr;

#ifdef RUNTIME_VR_VERSION_1_4_15

RelocPtr <_BSTaskPool_ProcessTaskQueue> BSTaskPool_ProcessTaskQueue(0x005CA100); //For VR 1.4.15

#elif RUNTIME_VERSION_1_6_318

RelocPtr <_BSTaskPool_ProcessTaskQueue> BSTaskPool_ProcessTaskQueue(0x005E8790); //For SSE 1.5.73 and up

#elif RUNTIME_VERSION_1_5_97 || RUNTIME_VERSION_1_5_80 || RUNTIME_VERSION_1_5_73

RelocPtr <_BSTaskPool_ProcessTaskQueue> BSTaskPool_ProcessTaskQueue(0x005C1BE0); //For SSE 1.5.73 and up

#elif RUNTIME_VERSION_1_5_62 || RUNTIME_VERSION_1_5_53 || RUNTIME_VERSION_1_5_50

RelocPtr <_BSTaskPool_ProcessTaskQueue> BSTaskPool_ProcessTaskQueue(0x005C1DD0); //For SSE 1.5.50 - 1.5.62

#elif RUNTIME_VERSION_1_5_39

RelocPtr <_BSTaskPool_ProcessTaskQueue> BSTaskPool_ProcessTaskQueue(0x005C2090); //For SSE 1.5.39

#elif RUNTIME_VERSION_1_5_23

RelocPtr <_BSTaskPool_ProcessTaskQueue> BSTaskPool_ProcessTaskQueue(0x005C1A20); //For SSE 1.5.23

#endif



void hk_BSTaskPool_ProcessTaskQueue(void* thisPtr)
{
	orig_BSTaskPool_ProcessTaskQueue(thisPtr);
	updateActors();
}

void DoHookNEW() 
{
	_MESSAGE("Attempting Game Hook with new");

	renderDetour.Create(BSTaskPool_ProcessTaskQueue.GetPtr(), hk_BSTaskPool_ProcessTaskQueue, &(LPVOID)orig_BSTaskPool_ProcessTaskQueue);
}