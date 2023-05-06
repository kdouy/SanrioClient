#include "includes.h"
#include <string>
#include "../ImGui DirectX 11 Kiero Hook/jimmy/jimmy.h"
#include "../ImGui DirectX 11 Kiero Hook/sdk/game/world/world.h"
#include "imp/autoclicker.h"
#include "functions/functions.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

bool uiVisible = true;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_KEYUP && wParam == VK_INSERT)
		uiVisible = !uiVisible;

	ImGuiIO& io = ImGui::GetIO();

	if (uiVisible)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
			return true;
		}

		switch (uMsg) {
		case WM_LBUTTONDOWN:
			io.MouseDown[1] = !io.MouseDown[0];
			return 0;
		case WM_RBUTTONDOWN:
			io.MouseDown[1] = !io.MouseDown[1];
			return 0;
		case WM_MBUTTONDOWN:
			io.MouseDown[2] = !io.MouseDown[2];
			return 0;
		case WM_MOUSEWHEEL:
			return 0;
		case WM_MOUSEMOVE:
			io.MousePos.x = (signed short)(lParam);
			io.MousePos.y = (signed short)(lParam >> 16);
			return 0;
		}
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void themeInitilization()
{
	auto& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 16.5f, NULL, io.Fonts->GetGlyphRangesJapanese());

	ImGuiStyle* style = &ImGui::GetStyle();

	// style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	// style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
}

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	themeInitilization();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

DWORD jvmBase = (DWORD)GetModuleHandleA("jvm.dll");

bool autoClicker = false;
int clickCPS = 12;

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	mc = std::make_unique<c_minecraft>();
	localPlayer = std::make_unique<c_player>();
	world = std::make_unique<c_world>();

	if (uiVisible)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 300));
		ImGui::Begin("Sanrio", &uiVisible);

		if (ImGui::BeginTabBar(""))
		{
			if (ImGui::BeginTabItem("Client"))
			{
				ImGui::Checkbox("Auto Clicker", &autoClicker);
				ImGui::SliderInt("CPS", &clickCPS, 1, 30);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Distance"))
			{
				if (world->isMultiplayer() && world->getPlayerCount() > 1)
				{
					ImGui::Text("Player In Radius:");
					int playerInteger = 0;
					for (const auto& player : world->getPlayers())
					{
						playerInteger += 1;
						Vec3 clientPos = { localPlayer->getX(), localPlayer->getY(), localPlayer->getZ() };
						Vec3 playerPos = { player->getX(), player->getY(), player->getZ() };
						std::string coordinateString = "Player" + std::to_string(playerInteger) + ": " + std::to_string((int)functions::GetDistance3D(clientPos, playerPos)) + "m away";
						ImGui::Text(coordinateString.c_str());
					}
				}
				else
				{
					ImGui::Text("No Player In Radius");
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Debug"))
			{
				if (world->isMultiplayer() && world->getPlayerCount() > 1)
				{
					ImGui::Text("Multiplayer");
				}
				else if (localPlayer->getPlayerObj() != NULL)
				{
					ImGui::Text("Singleplayer");
				}
				else
				{
					ImGui::Text("Main Menu");
				}

				std::string coordinateString = "Client Coordinate: " + std::to_string((int)localPlayer->getX()) + ", " + std::to_string((int)localPlayer->getY()) + ", " + std::to_string((int)localPlayer->getZ());
				ImGui::Text(coordinateString.c_str());
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

HWND getMinecraftHWND()
{
	HWND mc = FindWindowA("LWJGL", "");
	return mc;
}

void autoClickFunc();

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)&oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		jmy_init(GetCurrentProcess(), (void*)jvmBase);

		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);

		std::thread autoClickThread(autoClickFunc);
		autoClickThread.detach();

		break;
	}
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}

void autoClickFunc()
{
	while (true)
	{
		if (GetKeyState(WM_LBUTTONDOWN) && autoClicker)
		{
			HWND mcHWND = getMinecraftHWND();
			PostMessage(mcHWND, WM_LBUTTONDOWN, 1, 0);
			Sleep(clickCPS);
			PostMessage(mcHWND, WM_LBUTTONUP, 0, 0);
		}
		Sleep(1);
	}
}