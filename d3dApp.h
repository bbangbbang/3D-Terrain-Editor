
#pragma once
#include <Windowsx.h>
#include <d3d11.h>
#include <iostream>
#include "d3dUtil.h"
#include "GameTimer.h"
class D3DApp
{	
protected:
	HINSTANCE	mhAppInst;
	HWND		mhMainWnd;
	HWND		mhDrawHwnd;
		
	int			mWindowWidth;
	int			mWindowHeight;

	int			mDrawWidth;
	int			mDrawHeight;

	ID3D11Device			*md3dDevice;
	ID3D11DeviceContext		*md3dImmediateContext;
	IDXGISwapChain			*mSwapChain;
	ID3D11Texture2D			*mDepthStencilBuffer;
	ID3D11RenderTargetView	*mRenderTargetView;
	ID3D11DepthStencilView	*mDepthStencilView;
	D3D11_VIEWPORT			 mViewport;

	GameTimer mTimer;

private:
	bool InitMainWindow();
	bool InitDirect3D();
	void ShowFramesPerSecond();

protected:

	std::wstring	mMainWndName;
	UINT			m4xMsaaQuality;
	bool			mEnable4xMsaa;
	bool			mPaused;
	bool			mResizing;
	



public:
	D3DApp(HINSTANCE hInst);
	virtual ~D3DApp();

	HINSTANCE	AppInst() ;
	HWND		MainWnd() ;
	float		AspectRatio() ;


	int Run();

	virtual bool				Init();
	virtual LRESULT CALLBACK	MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT CALLBACK	D3DDrawProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void				OnResize();
	virtual void				UpdateScene(float dt) = 0;
	virtual void				DrawScene() = 0;

	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }
};