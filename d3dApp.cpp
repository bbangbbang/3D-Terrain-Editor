#include "d3dApp.h"
#include <sstream>
static D3DApp* gd3dApp;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	return gd3dApp->MsgProc(hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK DrawProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	return gd3dApp->D3DDrawProc(hWnd, iMessage, wParam, lParam);
}


D3DApp::D3DApp(HINSTANCE hInst)
:	mhAppInst(hInst), mWindowWidth(1280), mWindowHeight(720), mDrawWidth(mWindowWidth), mDrawHeight(mWindowHeight),
	mMainWndName(L"3D Terrain Editor"), md3dDevice(0), mSwapChain(0), mDepthStencilBuffer(0), mEnable4xMsaa(TRUE),
	md3dImmediateContext(0), mRenderTargetView(0), mDepthStencilView(0),mhDrawHwnd(0)

{
	ZeroMemory(&mViewport, sizeof(D3D11_VIEWPORT));
	gd3dApp = this;
}

void D3DApp::ShowFramesPerSecond(){

	static float dTime = 0;
	static int frames = 0;

	frames++;
	dTime += mTimer.DeltaTime();

	if (dTime >= 1.0f)
	{

		float fps = frames;
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs << mMainWndName << L"        "
			<< L"FPS: " << fps << L"        "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(mhMainWnd, outs.str().c_str());
		
		frames = 0;
		dTime = 0;
	}


}


D3DApp::~D3DApp()
{
	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mDepthStencilBuffer);
	ReleaseCOM(mSwapChain);
	ReleaseCOM(md3dImmediateContext);
	ReleaseCOM(md3dDevice);
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS WndClass;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = mhAppInst;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.lpszClassName = L"D3DApp";
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	
	
	if (!RegisterClass(&WndClass))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	WndClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	WndClass.lpfnWndProc = DrawProc;
	WndClass.lpszClassName = L"DrawWindow";
	if (!RegisterClass(&WndClass))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, mWindowWidth, mWindowHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"D3DApp", mMainWndName.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, (HMENU)NULL, mhAppInst, NULL);



	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

bool D3DApp::InitDirect3D()
{
	UINT DeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE
		, 0, 0, 0, 0, D3D11_SDK_VERSION, &md3dDevice, &featureLevel, &md3dImmediateContext);

	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 is not supported.", 0, 0);
		return false;
	}

	md3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);
	
	DXGI_SWAP_CHAIN_DESC sd;

	if (mEnable4xMsaa)                       
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Width = mDrawWidth;
	sd.BufferDesc.Height = mDrawHeight;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = 0;
	sd.OutputWindow = mhDrawHwnd;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Windowed = true;	

	IDXGIDevice* dxgiDevice = 0;
	md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

	IDXGIAdapter* dxgiAdapter = 0;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);

	IDXGIFactory* dxgiFactory = 0;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

	dxgiFactory->CreateSwapChain(md3dDevice, &sd, &mSwapChain);

	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	OnResize();

	return true;
}

HINSTANCE D3DApp::AppInst() {	

	return mhAppInst; 
}

HWND D3DApp::MainWnd() { 

	return mhMainWnd;

}
float D3DApp::AspectRatio() {
	
	return static_cast<float>(mDrawWidth) / mDrawHeight;
}


bool D3DApp::Init()
{

	if (!InitMainWindow())
		return false;

	if (!InitDirect3D())
		return false;

	return true;
}

int D3DApp::Run()
{
	MSG msg = { 0, };

	mTimer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		else
		{

			mTimer.Tick();
			if (!mPaused)
			{
				//ShowFramesPerSecond();
				UpdateScene(mTimer.DeltaTime());
				DrawScene();
			}
		}
	}


	return msg.wParam;
}




void D3DApp::OnResize()
{

	if (md3dDevice == NULL) return;

	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilBuffer);

	

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	
	if (mEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Width = mDrawWidth;
	depthStencilDesc.Height = mDrawHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	
	md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer);
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView);

	mSwapChain->ResizeBuffers(1, mDrawWidth, mDrawHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	ID3D11Texture2D* backBuffer;
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView);
	ReleaseCOM(backBuffer);



	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	mViewport.Height = static_cast<float>(mDrawHeight);
	mViewport.MaxDepth = 1.0f;
	mViewport.MinDepth = 0.0f;
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = static_cast<float>(mDrawWidth);

	md3dImmediateContext->RSSetViewports(1, &mViewport);
}

LRESULT D3DApp::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{

	case WM_CREATE:
		mPaused = false;
		mhDrawHwnd = CreateWindow(L"DrawWindow", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, 0, mhAppInst, 0);

		return 0;
	
	case WM_SIZE:
		mWindowWidth = LOWORD(lParam);
		mWindowHeight = HIWORD(lParam);

		RECT crt;
		GetClientRect(hWnd, &crt);

		mDrawWidth = mWindowWidth;
		mDrawHeight = mWindowHeight;
		MoveWindow(mhDrawHwnd, 0, 0, mDrawWidth, mDrawHeight, TRUE);

		return 0;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT D3DApp::D3DDrawProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{


	switch (msg)
	{

	case WM_CREATE:

		return 0;

	
	case WM_ENTERSIZEMOVE:
		mTimer.Stop();
		mResizing = true;

		return 0;

	case WM_EXITSIZEMOVE:

		mResizing = false;
		OnResize();
		mTimer.Start();

		return 0;
	case WM_SIZE:

		if (wParam == SIZE_MAXIMIZED) {
			OnResize();
		}

		else if (wParam == SIZE_RESTORED) {

			if (mResizing)
			{
			}

			else
			{
				OnResize();
			}

		}

		return 0;

	

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}