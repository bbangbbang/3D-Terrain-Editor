
#include "d3dApp.h"
#include "Effects.h"
#include "InputLayouts.h"
#include "Terrain.h"
#include "MathHelper.h"
#include "RenderStates.h"
#include "Camera.h"
#include "Collider.h"
#include "BrushShape.h"
#include "Light.h"
#include "resource.h"
#include <algorithm>

#include <CommCtrl.h>

#include <map>
using namespace std;

class Driver : public D3DApp
{
public:
	Driver(HINSTANCE);
	~Driver();
	
	void	OnResize();
	void	UpdateScene(float dt);
	void	DrawScene();
	bool	Init();


	LRESULT CALLBACK RightChildProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT CALLBACK D3DDrawProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	enum MODE {
		HEIGHT_EDIT, TEXTURE_EDIT, IMPORT_EXPORT
	} mCurrentMode;

	enum CONTROL_ID 
	{
		BTN_UP, BTN_DOWN, BTN_FLAT, LISTBOX_TEXTURE, BTN_TEXTURE, RADIO_FILL, RADIO_ERASE, BTN_ADDTEXTURE, BTN_DELTEXTURE, BTN_SMOOTH,
		BTN_IMPORT_NORMAL, BTN_IMPORT_HEIGHT, BTN_IMPORT_BLEND, BTN_EXPORT_NORMAL, BTN_EXPORT_HEIGHT, BTN_EXPORT_BLEND,
		RADIO_BASE, RADIO_TEX1, RADIO_TEX2, RADIO_TEX3, RADIO_TEX4, CHECKBOX_SLOD, CHECKBOX_WIREFRAME, CHECKBOX_SMOOTH
	};

	enum EDIT_TYPE {
		UP, DOWN, FLAT, SMOOTH
	} mEditType;

	int mColVertexCount;
	int mRowVertexCount;

	float mTerrainWidth;
	float mTerrainHeight;
	float mTerrainDepth;
	int mMaxRateOfChange;
	int mdRateOfChange;
	int mRateOfChange;

	int mMaxBrushSize;
	int mdBrushSize;

	vector<wstring> mLayerTexture;
	map<std::wstring,std::wstring> mTextureList;


	ID3D11ShaderResourceView* mLayerMapSRV;
	ID3D11ShaderResourceView* mBlendMapSRV;
	ID3D11Texture2D* mBlendTex;

	int mTexTransparency;
	HWND mhTab;
	HWND mhBaseRadioBtn, mhTex1RadioBtn, mhTex2RadioBtn, mhTex3RadioBtn, mhTex4RadioBtn, mhTexFillBtn, mhTexEraseBtn;
	HWND mhAddTexture, mhDelTexture, mhImportBlendMap, mhImportNormalMap, mhImportHeightMap, mhExportBlendMap, mhExportNormalMap, mhExportHeightMap;
	HWND mhRateOfChangeStatic, mhRadiusOfBrushStatic, mhImportStatic, mhExportStatic;
	HWND mhRightChildhWnd; 
	HWND mhRateOfChangeScroll, mhRadiusOfBrushScroll;
	HWND mhUpBtn, mhDownBtn, mhFlatBtn, mhTextureBtn, mhSmoothBtn;
	HWND mhTextureList;
	HWND mhSLODCheckBox, mhWireframeCheckBox, mhSmoothCheckBox;
	
	int mTabControlWidth;
	int mTabControlHeight;

	DirectionalLight	mDirLight;
	Material			mMat;

	Collider::Ray mPickRay;
	Collider::Frustum mFrustum;

	Camera mCam;

	Terrain		*mTerrain;
	BrushShape	mBrushShape;

	XMFLOAT4X4	mTerrainWorld;
	
	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
	
	bool mbSLOD, mbWireFrame, mbSmoothCheck;

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	

	void MoveBrushShape(int x, int y);

	void InitRightChildWindow();
	void InitBlendMap();
	void ExportNormalMap(TCHAR* filePath);
	void ImportBlendMap(TCHAR* filePath);
	void ExportBlendMap(TCHAR* filePath);
	void ImportHeightMap(TCHAR* filePath);
	void ExportHeightMap(TCHAR* filePath);
};

INT_PTR CALLBACK SettingDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK HeightMapImportDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK HeightMapExportDlgProc(HWND, UINT, WPARAM, LPARAM);

void ErrorHandling(char*);
int gWidth, gHeight, gDepth, gM, gCell = 32;
int gMinWidth = 10, gMaxWidth = 10000,
	gMinDepth = 10, gMaxDepth = 10000,
	gMinHeight = 10, gMaxHeight = 10000,
	gMinCell = 8, gMaxCell = 32,
	gMinM = 33, gMaxM = 2049;

bool gbFlipVertical, gbSignAbility = true;
int gHeigtMapIOBit;


static Driver *driver;

LRESULT CALLBACK TabControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	return driver->RightChildProc(hWnd, msg, wParam, lParam);
}




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{


	Driver theApp(hInstance);

#ifdef _DEBUG || DEBUG
	AllocConsole();                 // Allocate console window
	freopen("CONOUT$", "a", stderr); // Redirect stderr to console
	freopen("CONOUT$", "a", stdout); // Redirect stdout also
	freopen("CONIN$", "r", stdin);
#endif
	if (!theApp.Init())
		return 0;
	
	return 	theApp.Run();

}

Driver::Driver(HINSTANCE hInst)
	: D3DApp(hInst), mTheta(1.5f*MathHelper::PI), mPhi(0.25f*MathHelper::PI), mRadius(5.0f), mTabControlWidth(300), mTabControlHeight(mWindowHeight)
	, mRateOfChange(1), mLayerMapSRV(0), mBlendMapSRV(0), mLayerTexture(5), mColVertexCount(0), mRowVertexCount(0), mCurrentMode(HEIGHT_EDIT), mTexTransparency(0)
	, mTerrainWidth(0), mTerrainHeight(0), mMaxBrushSize(0), mdBrushSize(0), mMaxRateOfChange(10), mdRateOfChange(2), mEditType(UP), mTerrainDepth(0), mbSLOD(false), mbWireFrame(false)
	, mbSmoothCheck(true)
{
	driver = this;
	mLastMousePos.x = mLastMousePos.y = 0;
	
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mTerrainWorld, I);

	mTerrain = new Terrain();
}

Driver::~Driver()
{
	ReleaseCOM(mLayerMapSRV);
	ReleaseCOM(mBlendMapSRV);
	InputLayouts::Destroy();
	Effects::Destroy();
	RenderStates::Destroy();
}

void Driver::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(45 / 180.0f * MathHelper::PI, AspectRatio(), 0.01f, 100000.0f);


}

bool Driver::Init()
{
	InitRightChildWindow();
	if (!D3DApp::Init())
		return false;

	Effects::Init(md3dDevice);
	InputLayouts::Init(md3dDevice);
	RenderStates::Init(md3dDevice);


	int ret = DialogBox(AppInst(), MAKEINTRESOURCE(IDD_SETTING_DIALOG), mhMainWnd, SettingDlgProc);
	if (!ret) {
		SendMessageA(mhMainWnd, WM_CLOSE, 0, 0);
		return false;
	}
	mTerrainWidth = gWidth;
	mTerrainDepth = gDepth;
	mTerrainHeight = gHeight;

	mColVertexCount = mRowVertexCount = gM;
	mMaxBrushSize = mTerrainWidth * mTerrainDepth / max(mTerrainWidth, mTerrainDepth) / 10.0f;
	mdBrushSize = max(1, mMaxBrushSize / 10.0f);

	SetScrollRange(mhRadiusOfBrushScroll, SB_CTL, 0, mMaxBrushSize, TRUE);
	SetScrollPos(mhRadiusOfBrushScroll, SB_CTL, mMaxBrushSize, TRUE);

	mTerrain->Init(mTerrainWidth, mTerrainHeight,mTerrainDepth, mColVertexCount, mRowVertexCount, gCell, md3dDevice, md3dImmediateContext);
	mBrushShape.InitCircle(md3dDevice, XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), mMaxBrushSize, 10.0f / 180 * MathHelper::PI);
	mBrushShape.EnableTerrainHeight(mTerrain);
	mTexTransparency = mRateOfChange / (float)mMaxRateOfChange * 255;

	InitBlendMap();

	

	mDirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);


	XMVECTOR pos = XMVectorSet(0.0f, 300.0f, -100.0f, 1.0f);
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);


	mCam.SetLookAt(pos, target, up);

	return true;
}

void Driver::InitBlendMap()
{

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = mColVertexCount;
	desc.Height = mRowVertexCount;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;


	md3dDevice->CreateTexture2D(&desc, 0, &mBlendTex);

	D3D11_MAPPED_SUBRESOURCE data;

	md3dImmediateContext->Map(mBlendTex, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &data);

	UCHAR* pTexel = (UCHAR*)data.pData;
	for (UINT z = 0; z < desc.Height; z++)
	{
		for (UINT x = 0; x < desc.Width; x++)
		{
			pTexel[z*data.RowPitch + x * 4 + 0] = 0; // R
			pTexel[z*data.RowPitch + x * 4 + 1] = 0; // G
			pTexel[z*data.RowPitch + x * 4 + 2] = 0; // B
			pTexel[z*data.RowPitch + x * 4 + 3] = 0; // A
		}
	}

	md3dImmediateContext->Unmap(mBlendTex, 0);

	md3dDevice->CreateShaderResourceView(mBlendTex, 0, &mBlendMapSRV);
}
void Driver::DrawScene()
{

	ID3D11RenderTargetView* renderTargets[1] = { mRenderTargetView };
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);


	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	mCam.UpdateViewMatrix();
	mFrustum = Collider::ComputeFrustumFromViewProj(mCam.GetViewMatrix()*mCam.GetProjMatrix());
	
	XMMATRIX world = XMLoadFloat4x4(&mTerrainWorld);
	XMMATRIX view = mCam.GetViewMatrix();
	XMMATRIX proj = mCam.GetProjMatrix();

	XMMATRIX worldViewProj = world * view * proj;


	Effects::DisplacementMapFX->SetWorld(world);
	Effects::DisplacementMapFX->SetWorldViewProj(worldViewProj);
	ID3DX11EffectTechnique *tech = Effects::DisplacementMapFX->BasicTech;

	if(mbWireFrame)
		md3dImmediateContext->RSSetState(RenderStates::WireFrameRS);




	Effects::DisplacementMapFX->SetDirLight(mDirLight);
	Effects::DisplacementMapFX->SetEysPosW(mCam.GetPositionF());
	Effects::DisplacementMapFX->SetMaterial(mMat);
	Effects::DisplacementMapFX->SetLayerMapArray(mLayerMapSRV);
	Effects::DisplacementMapFX->SetBlendMap(mBlendMapSRV);
	mTerrain->Draw(md3dImmediateContext, tech, mFrustum,mCam.GetPosition(), mbSLOD);
	md3dImmediateContext->RSSetState(0);



	Effects::ColorFX->SetWorld(world);
	Effects::ColorFX->SetWorldViewProj(worldViewProj);
	tech = Effects::ColorFX->BasicTech;
	md3dImmediateContext->RSSetState(RenderStates::WireFrameRS);


	mBrushShape.DrawCircle(md3dImmediateContext, tech);

	md3dImmediateContext->RSSetState(0);

	HR(mSwapChain->Present(0, 0));

}

void Driver::UpdateScene(float dt)
{
	
}

void Driver::OnMouseDown(WPARAM btnState, int x, int y)
{
	vector<int> v;
	
	
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	
	if (btnState & MK_LBUTTON)
	{
		
		Collider::Sphere s(mBrushShape.GetCenter(), mBrushShape.GetRadius());
		if (mCurrentMode == HEIGHT_EDIT) {
			if (mEditType == SMOOTH)
			{
				mTerrain->AddHeightInShape(s, 0, 0, true);
				return;
			}
			
			if (mRateOfChange == 0) return;
			int d = abs(mRateOfChange);
			bool bFlat = false;
			if (mEditType == DOWN) d *= -1;
			else if (mEditType == FLAT) bFlat = true;



			mTerrain->AddHeightInShape(s, d, bFlat, mbSmoothCheck);
		}
		else if (mCurrentMode == TEXTURE_EDIT)
		{
			int n = 0;
			UCHAR color[4] = { 0, };

			if (SendMessage(mhTex1RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) { color[0] = mTexTransparency; n = 0; }
			else if (SendMessage(mhTex2RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) { color[1] = mTexTransparency; n = 1; }
			else if (SendMessage(mhTex3RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) { color[2] = mTexTransparency; n = 2; }
			else if (SendMessage(mhTex4RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) { color[3] = mTexTransparency; n = 3; }
			else return;

			//if (mLayerTexture[n + 1] == L"") return;
			vector<int> vIndices;
			mTerrain->FindVertexIndexInShape(s, vIndices);

			D3D11_MAPPED_SUBRESOURCE data;
			md3dImmediateContext->Map(mBlendTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
			UCHAR* pTexel = (UCHAR*)data.pData;

			for (UINT i = 0; i < vIndices.size(); i++)
			{

				int z, x;
				z = vIndices[i] / mColVertexCount;
				x = vIndices[i] % mColVertexCount;


				if (n == 0)pTexel[z*data.RowPitch + x * 4 + 0] = color[0]; // R
				else if (n == 1)pTexel[z*data.RowPitch + x * 4 + 1] = color[1]; // G
				else if (n == 2)pTexel[z*data.RowPitch + x * 4 + 2] = color[2]; // B
				else if (n == 3)pTexel[z*data.RowPitch + x * 4 + 3] = color[3]; // A
			}


			md3dImmediateContext->Unmap(mBlendTex, 0);

		}
	}

	else if (btnState & MK_RBUTTON)
	{

	}

	else if (btnState & MK_MBUTTON)
	{


	}
	SetCapture(mhDrawHwnd);
}

void Driver::OnMouseMove(WPARAM btnState, int x, int y)
{
	MoveBrushShape(x, y);
	static int lastTickCount = GetTickCount();
	static int curTickCount;
	curTickCount = GetTickCount();
	if (btnState & MK_LBUTTON)
	{
		if (curTickCount - lastTickCount >= 30) {
			Collider::Sphere s(mBrushShape.GetCenter(), mBrushShape.GetRadius());
			if (mCurrentMode == HEIGHT_EDIT) {
				if (mEditType == SMOOTH)
				{
					mTerrain->AddHeightInShape(s, 0, 0, true);
					return;
				}
				if (mRateOfChange == 0) return;
				int st = GetTickCount();

				int d = abs(mRateOfChange);


				bool bFlat = false;
				if (mEditType == DOWN) d *= -1;
				else if (mEditType == FLAT) bFlat = true;



				mTerrain->AddHeightInShape(s, d, bFlat, mbSmoothCheck);
				int et = GetTickCount();

				int t = et - st;
			}

			else if (mCurrentMode == TEXTURE_EDIT)
			{
				int n = 0;
				UCHAR color[4] = { 0, };
 
				if (SendMessage(mhTex1RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) { color[0] = mTexTransparency; n = 0; }
				else if (SendMessage(mhTex2RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) { color[1] = mTexTransparency; n = 1; }
				else if (SendMessage(mhTex3RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) { color[2] = mTexTransparency; n = 2; }
				else if (SendMessage(mhTex4RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) { color[3] = mTexTransparency; n = 3; }
				else return;
				//if (mLayerTexture[n+1] == L"") return;
				
				vector<int> vIndices;
				mTerrain->FindVertexIndexInShape(s, vIndices);

				D3D11_MAPPED_SUBRESOURCE data;
				md3dImmediateContext->Map(mBlendTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
				UCHAR* pTexel = (UCHAR*)data.pData;


				for (UINT i = 0; i < vIndices.size(); i++)
				{
					int z, x;
					z = vIndices[i] / mColVertexCount;
					x = vIndices[i] % mColVertexCount;


					if (n == 0)pTexel[z*data.RowPitch + x * 4 + 0] = color[0]; // R
					else if (n == 1)pTexel[z*data.RowPitch + x * 4 + 1] = color[1]; // G
					else if (n == 2)pTexel[z*data.RowPitch + x * 4 + 2] = color[2]; // B
					else if (n == 3)pTexel[z*data.RowPitch + x * 4 + 3] = color[3]; // A
					
				}



				md3dImmediateContext->Unmap(mBlendTex, 0);


			}
			lastTickCount = curTickCount;
		}
	}
	
	else if (btnState & MK_RBUTTON)
	{

		float dx = 0.1f*(x - mLastMousePos.x) / 180.0f * MathHelper::PI;
		float dy = 0.1f*(y - mLastMousePos.y) / 180.0f * MathHelper::PI;
		mCam.Yaw(dx);
		mCam.Pitch(dy);

		
	}

	else if (btnState & MK_MBUTTON)
	{


		float dx = (x - mLastMousePos.x) * 0.1f;
		float dy = (y - mLastMousePos.y) * 0.1f;

		mCam.MoveUp(dy);
		mCam.MoveRight(-dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void Driver::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Driver::MoveBrushShape(int x, int y)
{
	XMMATRIX P = mCam.GetProjMatrix();

	mPickRay = Collider::ComputePickRayFromScreen(x, y, mDrawWidth, mDrawHeight, P);

	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(mCam.GetViewMatrix()), mCam.GetViewMatrix());


	XMVECTOR origin = XMLoadFloat3(&mPickRay.Origin);
	XMVECTOR dir = XMLoadFloat3(&mPickRay.Dir);
	origin = XMVector3TransformCoord(origin, invView);
	dir = XMVector3Normalize(XMVector3TransformNormal(dir, invView));

	XMVECTOR v;

	if (mTerrain->IsIntersectVertexWithRay(origin, dir, v))
	{

		mBrushShape.SetCenter(v);
	


		
		//printf("x : %f, y : %f, z : %f \n", vv.x, vv.y, vv.z);
	}
}



void Driver::InitRightChildWindow()
{
	WNDCLASS wndClass;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hInstance = mhAppInst;
	wndClass.lpszMenuName = 0;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndClass.lpfnWndProc = TabControlProc;
	wndClass.lpszClassName = L"RightChild";
	RegisterClass(&wndClass);

}


void Driver::ExportNormalMap(TCHAR* filePath)
{
	ID3D11Texture2D* normalTex;
	normalTex = mTerrain->GetNormalMap();
	D3DX11SaveTextureToFileW(md3dImmediateContext, normalTex, D3DX11_IFF_DDS, filePath);
}

void Driver::ImportBlendMap(TCHAR* filePath)
{

	D3DX11_IMAGE_LOAD_INFO desc;
	ID3D11Texture2D* srcTex;
	desc.Width = D3DX11_FROM_FILE;
	desc.Height = D3DX11_FROM_FILE;
	desc.Depth = D3DX11_FROM_FILE;
	desc.FirstMipLevel = 0;
	desc.MipLevels = D3DX11_FROM_FILE;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	desc.MiscFlags = 0;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.MipFilter = 0;
	desc.pSrcInfo = 0;

	D3DX11_IMAGE_INFO info;

	D3DX11CreateTextureFromFile(md3dDevice, filePath, &desc, 0, (ID3D11Resource**)&srcTex, 0);
	D3D11_TEXTURE2D_DESC srcDesc;
	srcTex->GetDesc(&srcDesc);
	

	D3D11_MAPPED_SUBRESOURCE src, dst;
	md3dImmediateContext->Map(srcTex, 0, D3D11_MAP_READ, 0, &src);
		md3dImmediateContext->Map(mBlendTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &dst);

		UCHAR* pSrc = (UCHAR*)src.pData;
		UCHAR* pDst = (UCHAR*)dst.pData;

		
		for (UINT z = 0; z < mRowVertexCount; z++)
		{
			for (UINT x = 0; x < mColVertexCount; x++)
			{

				pDst[z*mRowVertexCount*4 + x * 4 + 0] = pSrc[z*src.RowPitch + x * 4 + 0]; // R
				pDst[z*mRowVertexCount*4 + x * 4 + 1] = pSrc[z*src.RowPitch + x * 4 + 1]; // G
				pDst[z*mRowVertexCount*4 + x * 4 + 2] = pSrc[z*src.RowPitch + x * 4 + 2]; // B
				pDst[z*mRowVertexCount*4 + x * 4 + 3] = pSrc[z*src.RowPitch + x * 4 + 3]; // A
			}
		}

		md3dImmediateContext->Unmap(mBlendTex,0);
	md3dImmediateContext->Unmap(srcTex, 0);

	ReleaseCOM(srcTex);
}

void Driver::ExportBlendMap(TCHAR* filePath)
{

	D3DX11SaveTextureToFileW(md3dImmediateContext, mBlendTex, D3DX11_IFF_DDS, filePath);
}


void Driver::ImportHeightMap(TCHAR* filePath)
{
	int ret = DialogBox(AppInst(), MAKEINTRESOURCE(ID_HEIGHT_IMPORT), mhMainWnd, HeightMapImportDlgProc);
	if (!ret) {
		return;
	}
	mTerrain->LoadHeightMap(filePath,gWidth,gDepth,gHeight,gM, gHeigtMapIOBit,gbFlipVertical, gbSignAbility);
}

void Driver::ExportHeightMap(TCHAR* filePath)
{
	int ret = DialogBox(AppInst(), MAKEINTRESOURCE(IDD_HEIGHTMAP_EXPORT), mhMainWnd, HeightMapExportDlgProc);
	if (!ret) {
		return;
	}

	mTerrain->SaveHeightMapTo8bitRaw(filePath, gHeigtMapIOBit);
}


LRESULT CALLBACK Driver::RightChildProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	wstring str2;
	str2.resize(256);
	TCITEM tie;
	int val, i, n;
	OPENFILENAME OFN;
	TCHAR fileName[MAX_PATH] = L"";
	switch (msg)
	{
	case WM_CREATE:
		mhTab = CreateWindow(WC_TABCONTROL, L"",
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, 0, 0, hWnd, 0, mhAppInst, 0);
		tie.mask = TCIF_TEXT;
		tie.pszText = L"Terrain";
		TabCtrl_InsertItem(mhTab, 0, &tie);
		tie.pszText = L"Texture";
		TabCtrl_InsertItem(mhTab, 1, &tie);
		tie.pszText = L"Import / Export";
		TabCtrl_InsertItem(mhTab, 2, &tie);
	
		

		mhRateOfChangeStatic = CreateWindow(L"static", L"증감률", WS_CHILD | WS_VISIBLE, 10, 260, 100, 15, hWnd, (HMENU)-1, mhAppInst, 0);
		mhRateOfChangeScroll = CreateWindow(L"scrollbar", 0, WS_CHILD | WS_VISIBLE | SBS_HORZ
			, 10, 280, 200, 20, hWnd, 0, mhAppInst, 0);
		SetScrollRange(mhRateOfChangeScroll, SB_CTL, 0, mMaxRateOfChange, TRUE);
		SetScrollPos(mhRateOfChangeScroll, SB_CTL, (int)mRateOfChange, TRUE);

		//HWND , mDelTexture, mImportBlendMap, mImportNormalMap, mImportHeightMap, mExportBlendMap, mExportNormalMap, mExportHeightMap;

		mhRadiusOfBrushStatic = CreateWindow(L"static", L"브러쉬 크기", WS_CHILD | WS_VISIBLE, 10, 310, 100, 15, hWnd, (HMENU)-1, mhAppInst, 0);
		mhRadiusOfBrushScroll = CreateWindow(L"scrollbar", 0, WS_CHILD | WS_VISIBLE | SBS_HORZ
			, 10, 330, 200, 20, hWnd, 0, mhAppInst, 0);

		mhUpBtn = CreateWindow(L"button", L"UP", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			5, 60, 90, 30, hWnd, (HMENU)BTN_UP ,mhAppInst,0);
		mhDownBtn = CreateWindow(L"button", L"DOWN", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			105, 60, 90, 30, hWnd, (HMENU)BTN_DOWN, mhAppInst, 0);
		mhFlatBtn = CreateWindow(L"button", L"FLAT", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			205, 60, 90, 30, hWnd, (HMENU)BTN_FLAT, mhAppInst, 0);
		mhSmoothBtn = CreateWindow(L"button", L"Smooth", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			5, 105, 90, 30, hWnd, (HMENU)BTN_SMOOTH, mhAppInst, 0);

		mhSLODCheckBox = CreateWindow(L"button", L"SLOD Ability", WS_CHILD | WS_VISIBLE |
			BS_AUTOCHECKBOX, 11, 370, 120,20, hWnd, (HMENU)CHECKBOX_SLOD, mhAppInst, NULL);
		mhWireframeCheckBox = CreateWindow(L"button", L"WireFrame", WS_CHILD | WS_VISIBLE |
			BS_AUTOCHECKBOX, 11, 400, 100, 20, hWnd, (HMENU)CHECKBOX_WIREFRAME, mhAppInst, NULL);
		mhSmoothCheckBox = CreateWindow(L"button", L"Smooth Ability", WS_CHILD | WS_VISIBLE |
			BS_AUTOCHECKBOX, 11, 430, 130, 20, hWnd, (HMENU)CHECKBOX_SMOOTH, mhAppInst, NULL);

		SendMessage(mhSmoothCheckBox, BM_SETCHECK, BST_CHECKED, 0);

		mhTextureList = CreateWindow(L"listbox", NULL, WS_CHILD | WS_BORDER , 100, 40, 120, 150, hWnd, (HMENU)LISTBOX_TEXTURE, mhAppInst, 0);
		SendMessage(mhTextureList, LB_SETCOLUMNWIDTH, 100, 0);

	
		mhBaseRadioBtn = CreateWindow(L"button", L"Base", WS_CHILD |  BS_AUTORADIOBUTTON |WS_GROUP, 10, 40, 60, 30, hWnd, (HMENU)RADIO_BASE, mhAppInst, 0);
		mhTex1RadioBtn = CreateWindow(L"button", L"Tex 1", WS_CHILD |  BS_AUTORADIOBUTTON  , 10, 70, 60, 30, hWnd, (HMENU)RADIO_TEX1, mhAppInst, 0);
		mhTex2RadioBtn = CreateWindow(L"button", L"Tex 2", WS_CHILD |  BS_AUTORADIOBUTTON  , 10, 100, 60, 30, hWnd, (HMENU)RADIO_TEX2, mhAppInst, 0);
		mhTex3RadioBtn = CreateWindow(L"button", L"Tex 3", WS_CHILD |  BS_AUTORADIOBUTTON  , 10, 130, 60, 30, hWnd, (HMENU)RADIO_TEX3, mhAppInst, 0);
		mhTex4RadioBtn = CreateWindow(L"button", L"Tex 4", WS_CHILD |  BS_AUTORADIOBUTTON  , 10, 160, 60, 30, hWnd, (HMENU)RADIO_TEX4, mhAppInst, 0);
		CheckRadioButton(hWnd, RADIO_BASE, RADIO_TEX4, RADIO_BASE);


		mhAddTexture = CreateWindow(L"button", L"추가", WS_CHILD | BS_PUSHBUTTON,
			75, 200, 70, 30, hWnd, (HMENU)BTN_ADDTEXTURE, mhAppInst, 0);

		mhDelTexture = CreateWindow(L"button", L"제거", WS_CHILD | BS_PUSHBUTTON,
			150, 200, 70, 30, hWnd, (HMENU)BTN_DELTEXTURE, mhAppInst, 0);

		mhTextureBtn = CreateWindow(L"button", L"적용", WS_CHILD | BS_PUSHBUTTON,
			225, 200, 70, 30, hWnd, (HMENU)BTN_TEXTURE, mhAppInst, 0);

		mhTexFillBtn = CreateWindow(L"button", L"칠하기", WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
			10, 200, 90, 30, hWnd, (HMENU)RADIO_FILL, mhAppInst, 0);
		mhTexEraseBtn = CreateWindow(L"button", L"지우기", WS_CHILD | BS_AUTORADIOBUTTON,
			110, 200, 90, 30, hWnd, (HMENU)RADIO_ERASE, mhAppInst, 0);
		CheckRadioButton(hWnd, RADIO_FILL, RADIO_ERASE, RADIO_FILL);



		mhImportStatic= CreateWindow(L"static", L"Import", WS_CHILD , 55, 70, 100, 30, hWnd, (HMENU)-1, mhAppInst, 0);
		mhExportStatic= CreateWindow(L"static", L"Export", WS_CHILD , 185, 70, 100, 30, hWnd, (HMENU)-1, mhAppInst, 0);


		mhImportBlendMap = CreateWindow(L"button", L"BlendMap", WS_CHILD | BS_PUSHBUTTON,
			30, 100, 100, 30, hWnd, (HMENU)BTN_IMPORT_BLEND, mhAppInst, 0);
		mhImportHeightMap = CreateWindow(L"button", L"HeightMap", WS_CHILD | BS_PUSHBUTTON,
			30, 150, 100, 30, hWnd, (HMENU)BTN_IMPORT_HEIGHT, mhAppInst, 0);
		mhImportNormalMap = CreateWindow(L"button", L"NormalMap", WS_CHILD | BS_PUSHBUTTON,
			30, 200, 100, 30, hWnd, (HMENU)BTN_IMPORT_NORMAL, mhAppInst, 0);
		mhExportBlendMap = CreateWindow(L"button", L"BlendMap", WS_CHILD | BS_PUSHBUTTON,
			160, 100, 100, 30, hWnd, (HMENU)BTN_EXPORT_BLEND, mhAppInst, 0);
		mhExportHeightMap = CreateWindow(L"button", L"HeightMap", WS_CHILD | BS_PUSHBUTTON,
			160, 150, 100, 30, hWnd, (HMENU)BTN_EXPORT_HEIGHT, mhAppInst, 0);
		mhExportNormalMap = CreateWindow(L"button", L"NormalMap", WS_CHILD | BS_PUSHBUTTON,
			160, 200, 100, 30, hWnd, (HMENU)BTN_EXPORT_NORMAL, mhAppInst, 0);

		return 0;

	case WM_HSCROLL:
		if ((HWND)lParam == mhRateOfChangeScroll) {
			switch (LOWORD(wParam))
			{
			case SB_LINELEFT:
				mRateOfChange = max(0, mRateOfChange - mdRateOfChange);
				break;
			case SB_LINERIGHT:
				mRateOfChange = min(mMaxRateOfChange, mRateOfChange + mdRateOfChange);
				break;
			case SB_PAGELEFT:
				mRateOfChange = max(0, mRateOfChange - mdRateOfChange * 2);
				break;
			case SB_PAGERIGHT:
				mRateOfChange = min(mMaxRateOfChange, mRateOfChange + mdRateOfChange * 2);
				break;

			case SB_THUMBTRACK:
				mRateOfChange = HIWORD(wParam);
				break;

			}
			SetScrollPos((HWND)lParam, SB_CTL, mRateOfChange, TRUE);
			mTexTransparency = mRateOfChange / (float)mMaxRateOfChange * 255;
			
			
		}
		else if ((HWND)lParam == mhRadiusOfBrushScroll)
		{

			val = mBrushShape.GetRadius();
			switch (LOWORD(wParam))
			{
			case SB_LINELEFT:
				val = max(0, val - mdBrushSize);
				break;
			case SB_LINERIGHT:
				val = min(mMaxBrushSize, val + mdBrushSize);
				break;
			case SB_PAGELEFT:
				val = max(0, val - mdBrushSize*2);
				break;
			case SB_PAGERIGHT:
				val = min(mMaxBrushSize, val + mdBrushSize*2);
				break;

			case SB_THUMBTRACK:
				val = HIWORD(wParam);
				break;

			}
			SetScrollPos((HWND)lParam, SB_CTL, val, TRUE);

			mBrushShape.SetRadius(val);
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			
		case BTN_UP:
			
			mEditType = UP;
			break;

		case BTN_DOWN:
			mEditType = DOWN;
			break;

		case BTN_FLAT:
			mEditType = FLAT;
			break;

		case BTN_SMOOTH:
			mEditType = SMOOTH;
			break;

		case CHECKBOX_SLOD:
			mbSLOD = !mbSLOD;
			break;


		case CHECKBOX_SMOOTH:
			mbSmoothCheck = !mbSmoothCheck;
			break;

		case CHECKBOX_WIREFRAME:
			mbWireFrame = !mbWireFrame;
			break;

		case BTN_IMPORT_BLEND:
			memset(&OFN, 0, sizeof(OFN));
			OFN.lStructSize = sizeof(OFN);
			OFN.hwndOwner = hWnd;
			OFN.lpstrFilter = L"Texture file(*.dds)\0*.dds\0";
			OFN.lpstrFile = fileName;
			OFN.nMaxFile = MAX_PATH;
			if (GetOpenFileName(&OFN) != 0)
			{
				
				ImportBlendMap(fileName);
			}
			break;
		
		case BTN_EXPORT_BLEND:
			lstrcpy(fileName, L"*.dds");
			memset(&OFN, 0, sizeof(OFN));
			OFN.lStructSize = sizeof(OFN);
			OFN.hwndOwner = hWnd;
			OFN.lpstrFilter = L"DDS file(*.dds)\0*.dds\0";
			OFN.lpstrFile = fileName;
			OFN.nMaxFile = MAX_PATH;
			OFN.lpstrDefExt = L".dds";
			OFN.Flags = OFN_NOCHANGEDIR;
			if (GetSaveFileName(&OFN)) {

				ExportBlendMap(fileName);
			}

			break;

		case BTN_EXPORT_NORMAL:
			lstrcpy(fileName, L"*.dds");
			memset(&OFN, 0, sizeof(OFN));
			OFN.lStructSize = sizeof(OFN);
			OFN.hwndOwner = hWnd;
			OFN.lpstrFilter = L"DDS file(*.dds)\0*.dds\0";
			OFN.lpstrFile = fileName;
			OFN.nMaxFile = MAX_PATH;
			OFN.lpstrDefExt = L".dds";
			OFN.Flags = OFN_NOCHANGEDIR;
			if (GetSaveFileName(&OFN)) {

				ExportNormalMap(fileName);
			}

			break;

		case BTN_IMPORT_HEIGHT:
			memset(&OFN, 0, sizeof(OFN));
			OFN.lStructSize = sizeof(OFN);
			OFN.hwndOwner = hWnd;
			OFN.lpstrFilter = L"Raw file(*.RAW)\0*.RAW\0";
			OFN.lpstrFile = fileName;
			OFN.nMaxFile = MAX_PATH;
			if (GetOpenFileName(&OFN) != 0)
			{

				ImportHeightMap(fileName);
			}
			break;

		case BTN_EXPORT_HEIGHT:
			lstrcpy(fileName, L"*.raw");
			memset(&OFN, 0, sizeof(OFN));
			OFN.lStructSize = sizeof(OFN);
			OFN.hwndOwner = hWnd;
			OFN.lpstrFilter = L"RAW file(*.RAW)\0*.RAW\0";
			OFN.lpstrFile = fileName;
			OFN.nMaxFile = MAX_PATH;
			OFN.lpstrDefExt = L".raw";
			OFN.Flags = OFN_NOCHANGEDIR;
			if (GetSaveFileName(&OFN)) {

				ExportHeightMap(fileName);
			}
			break;

		case BTN_ADDTEXTURE:
			memset(&OFN, 0, sizeof(OFN));
			OFN.lStructSize = sizeof(OFN);
			OFN.hwndOwner = hWnd;
			OFN.lpstrFilter = L"Texture file(*.dds)\0*.dds\0";
			OFN.lpstrFile = fileName;
			OFN.nMaxFile = MAX_PATH;
			if (GetOpenFileName(&OFN) != 0)
			{
				UINT i = 0;

				for (i = lstrlen(fileName); fileName[i] != L'\\'; i--);

				map<wstring, wstring>::iterator it;
				it = mTextureList.find(&fileName[i+1]);

				// 중복 추가 여부 확인
				if (it == mTextureList.end()) {

					SendMessage(mhTextureList, LB_ADDSTRING, 0, (LPARAM)&fileName[i + 1]);
					mTextureList.insert(pair<wstring, wstring>(&fileName[i + 1], fileName));
				}
			}
			break;


		case BTN_DELTEXTURE:

			n = SendMessage(mhTextureList, LB_GETCURSEL, 0, 0);
			SendMessage(mhTextureList, LB_GETTEXT, n, (LPARAM)str2.c_str());
			if (n == -1) break;
			SendMessage(mhTextureList, LB_DELETESTRING, n, 0);
			
			for (int i = 0; i < mLayerTexture.size(); i++)
			{ 
				if (mLayerTexture[i] == mTextureList[str2.c_str()]) 
					mLayerTexture[i] = L"";

				
			}
			mTextureList.erase(str2.c_str());
			ReleaseCOM(mLayerMapSRV);
			mLayerMapSRV = D3DUtil::CreateTexture2DArraySRV(md3dDevice, md3dImmediateContext, mLayerTexture, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 0);

			break;

	
		case BTN_TEXTURE:
			n = SendMessage(mhTextureList, LB_GETCURSEL, 0, 0);
			if (n == -1) break;
			SendMessage(mhTextureList, LB_GETTEXT, n, (LPARAM)str2.c_str());
			
			map<wstring, wstring>::iterator it;
			it = mTextureList.find(str2.c_str());

			wstring name = it->second;
			if (SendMessage(mhBaseRadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) mLayerTexture[0] = name;
			else if (SendMessage(mhTex1RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) mLayerTexture[1] = name;
			else if (SendMessage(mhTex2RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) mLayerTexture[2] = name;
			else if (SendMessage(mhTex3RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) mLayerTexture[3] = name;
			else if (SendMessage(mhTex4RadioBtn, BM_GETCHECK, 0, 0) == BST_CHECKED) mLayerTexture[4] = name;


			ReleaseCOM(mLayerMapSRV);
			mLayerMapSRV = D3DUtil::CreateTexture2DArraySRV(md3dDevice, md3dImmediateContext, mLayerTexture, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 0);

			break;

		}

		
		return 0;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case TCN_SELCHANGE:
			int i = TabCtrl_GetCurSel(mhTab);
			if (i == 0)
			{
				mCurrentMode = HEIGHT_EDIT;
				ShowWindow(mhTextureList, SW_HIDE);
				ShowWindow(mhUpBtn, SW_SHOW	);
				ShowWindow(mhDownBtn, SW_SHOW);
				ShowWindow(mhFlatBtn, SW_SHOW);
				ShowWindow(mhSmoothBtn, SW_SHOW);
				ShowWindow(mhSLODCheckBox, SW_SHOW);
				ShowWindow(mhWireframeCheckBox, SW_SHOW);
				ShowWindow(mhSmoothCheckBox, SW_SHOW);
				ShowWindow(mhRadiusOfBrushScroll, SW_SHOW);
				ShowWindow(mhRateOfChangeScroll, SW_SHOW);
				ShowWindow(mhRateOfChangeStatic, SW_SHOW);
				ShowWindow(mhRadiusOfBrushStatic, SW_SHOW);
				ShowWindow(mhBaseRadioBtn, SW_HIDE);
				ShowWindow(mhTex1RadioBtn, SW_HIDE);
				ShowWindow(mhTex2RadioBtn, SW_HIDE);
				ShowWindow(mhTex3RadioBtn, SW_HIDE);
				ShowWindow(mhTex4RadioBtn, SW_HIDE);
				ShowWindow(mhTextureBtn, SW_HIDE);
				ShowWindow(mhTexFillBtn, SW_HIDE);
				ShowWindow(mhAddTexture, SW_HIDE);
				ShowWindow(mhDelTexture, SW_HIDE);
				ShowWindow(mhImportBlendMap, SW_HIDE);
				ShowWindow(mhImportHeightMap, SW_HIDE);
				ShowWindow(mhImportNormalMap, SW_HIDE);
				ShowWindow(mhExportBlendMap, SW_HIDE);
				ShowWindow(mhExportHeightMap, SW_HIDE);
				ShowWindow(mhExportNormalMap, SW_HIDE);
				ShowWindow(mhImportStatic, SW_HIDE);
				ShowWindow(mhExportStatic, SW_HIDE);


			}
			else if (i == 1)
			{
				mCurrentMode = TEXTURE_EDIT;
				ShowWindow(mhTextureList, SW_SHOW);
				ShowWindow(mhUpBtn, SW_HIDE);
				ShowWindow(mhDownBtn, SW_HIDE);
				ShowWindow(mhFlatBtn, SW_HIDE);
				ShowWindow(mhSmoothBtn, SW_HIDE);
				ShowWindow(mhRadiusOfBrushScroll, SW_SHOW);
				ShowWindow(mhRateOfChangeScroll, SW_SHOW);
				ShowWindow(mhRateOfChangeStatic, SW_SHOW);
				ShowWindow(mhRadiusOfBrushStatic, SW_SHOW);
				ShowWindow(mhBaseRadioBtn, SW_SHOW);
				ShowWindow(mhTex1RadioBtn, SW_SHOW);
				ShowWindow(mhTex2RadioBtn, SW_SHOW);
				ShowWindow(mhTex3RadioBtn, SW_SHOW);
				ShowWindow(mhTex4RadioBtn, SW_SHOW);
				ShowWindow(mhTextureBtn, SW_SHOW);
				ShowWindow(mhTexFillBtn, SW_HIDE);
				ShowWindow(mhTexEraseBtn, SW_HIDE);
				ShowWindow(mhAddTexture, SW_SHOW);
				ShowWindow(mhDelTexture, SW_SHOW);
				ShowWindow(mhImportBlendMap, SW_HIDE);
				ShowWindow(mhImportHeightMap, SW_HIDE);
				ShowWindow(mhImportNormalMap, SW_HIDE);
				ShowWindow(mhExportBlendMap, SW_HIDE);
				ShowWindow(mhExportHeightMap, SW_HIDE);
				ShowWindow(mhExportNormalMap, SW_HIDE);	
				ShowWindow(mhImportStatic, SW_HIDE);
				ShowWindow(mhExportStatic, SW_HIDE);
				ShowWindow(mhSLODCheckBox, SW_HIDE);
				ShowWindow(mhWireframeCheckBox, SW_HIDE);
				ShowWindow(mhSmoothCheckBox, SW_HIDE);
			}

			else if (i == 2)
			{
				mCurrentMode = IMPORT_EXPORT;
				ShowWindow(mhTextureList, SW_HIDE);
				ShowWindow(mhUpBtn, SW_HIDE);
				ShowWindow(mhDownBtn, SW_HIDE);
				ShowWindow(mhFlatBtn, SW_HIDE);
				ShowWindow(mhSmoothBtn, SW_HIDE);
				ShowWindow(mhRadiusOfBrushScroll, SW_HIDE);
				ShowWindow(mhRateOfChangeScroll, SW_HIDE);
				ShowWindow(mhRateOfChangeStatic, SW_HIDE);
				ShowWindow(mhRadiusOfBrushStatic, SW_HIDE);
				ShowWindow(mhBaseRadioBtn, SW_HIDE);
				ShowWindow(mhTex1RadioBtn, SW_HIDE);
				ShowWindow(mhTex2RadioBtn, SW_HIDE);
				ShowWindow(mhTex3RadioBtn, SW_HIDE);
				ShowWindow(mhTex4RadioBtn, SW_HIDE);
				ShowWindow(mhTextureBtn, SW_HIDE);
				ShowWindow(mhTexFillBtn, SW_HIDE);
				ShowWindow(mhTexEraseBtn, SW_HIDE);

				ShowWindow(mhAddTexture, SW_HIDE);
				ShowWindow(mhDelTexture, SW_HIDE);
				ShowWindow(mhImportBlendMap, SW_SHOW);
				ShowWindow(mhImportHeightMap, SW_SHOW);
				ShowWindow(mhImportNormalMap, SW_HIDE);
				ShowWindow(mhExportBlendMap, SW_SHOW);
				ShowWindow(mhExportHeightMap, SW_SHOW);
				ShowWindow(mhExportNormalMap, SW_SHOW);
				ShowWindow(mhImportStatic, SW_SHOW);
				ShowWindow(mhExportStatic, SW_SHOW);
				ShowWindow(mhSLODCheckBox, SW_HIDE);
				ShowWindow(mhWireframeCheckBox, SW_HIDE);
				ShowWindow(mhSmoothCheckBox, SW_HIDE);
			}
			
			InvalidateRect(0,  0, TRUE);
			break;
		}
		return 0;

	case WM_SIZE:

		InvalidateRect(0, 0, TRUE);
		MoveWindow(mhTab, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		
		return 0;


		return 0;


	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}



LRESULT Driver::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int ret;
	RECT rect;
	int cx, cy, x, y;
	switch (msg)
	{

	case WM_CREATE:
		GetWindowRect(hWnd, &rect);

		cx = GetSystemMetrics(SM_CXSCREEN);
		cy = GetSystemMetrics(SM_CYSCREEN);
		x = cx / 2 - rect.right / 2;
		y = cy / 2 - rect.bottom / 2;

		MoveWindow(hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, TRUE);


		mPaused = false;
		mhDrawHwnd = CreateWindow(L"DrawWindow", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, 0, mhAppInst, 0);
		mhRightChildhWnd = CreateWindow(L"RightChild", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, 0, mhAppInst, 0);


		return 0;
	
	case WM_SIZE:
		mWindowWidth = LOWORD(lParam);
		mWindowHeight = HIWORD(lParam);


		mDrawWidth = mWindowWidth - mTabControlWidth;
		mDrawHeight = mWindowHeight;
		
		MoveWindow(mhDrawHwnd, 0, 0, mDrawWidth, mDrawHeight, TRUE);
		mTabControlHeight = mDrawHeight;

		MoveWindow(mhRightChildhWnd, mDrawWidth, 0, mTabControlWidth, mTabControlHeight, TRUE);

		return 0;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 500;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 500;
		return 0;

	case WM_MOVE:
		InvalidateRect(0, 0, FALSE);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}



LRESULT Driver::D3DDrawProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	int ret;
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


	case WM_MOUSEWHEEL:
		mCam.Walk((SHORT)HIWORD(wParam)/10.0);

		return 0;

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}



INT_PTR CALLBACK SettingDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int cx, cy, x, y;
	int w, d, h, m, cell;
	RECT rect;
	switch (iMessage)
	{

	case WM_INITDIALOG: 
		GetWindowRect(hDlg, &rect);

		cx = GetSystemMetrics(SM_CXSCREEN);
		cy = GetSystemMetrics(SM_CYSCREEN);
		x = cx / 2 - rect.right / 2;
		y = cy / 2 - rect.bottom / 2;

		MoveWindow(hDlg, x, y, rect.right-rect.left, rect.bottom-rect.top, TRUE);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_BUTTON_OK:
			BOOL ret;
			gWidth = GetDlgItemInt(hDlg, IDC_WIDTH, &ret, false);
			gHeight = GetDlgItemInt(hDlg, IDC_HEIGHT, &ret, false);
			gDepth = GetDlgItemInt(hDlg, IDC_DEPTH, &ret, false);
			gM = GetDlgItemInt(hDlg, IDC_M, &ret, false);
			//gCell = GetDlgItemInt(hDlg, IDC_CELL, &ret, false);
			if (!gWidth | !gHeight | !gDepth | !gM) {
				ErrorHandling("Input Data");
				break;
			}

			EndDialog(hDlg, 1);
			return TRUE;


		case IDC_WIDTH:
			switch(HIWORD(wParam)) {
			case EN_KILLFOCUS:
				w = GetDlgItemInt(hDlg, IDC_WIDTH, &ret, false);
				w = MathHelper::Clamp(w, gMinWidth, gMaxWidth);
				SetDlgItemInt(hDlg, IDC_WIDTH, w, false);
				
				break;
			}
			break;
		case IDC_DEPTH:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				d = GetDlgItemInt(hDlg, IDC_DEPTH, &ret, false);
				d = MathHelper::Clamp(d, gMinDepth, gMaxDepth);
				SetDlgItemInt(hDlg, IDC_DEPTH, d, false);

				break;
			}
			break;
		case IDC_HEIGHT:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				h = GetDlgItemInt(hDlg, IDC_HEIGHT, &ret, false);
				h = MathHelper::Clamp(h, gMinHeight, gMaxHeight);
				SetDlgItemInt(hDlg, IDC_HEIGHT, h, false);

				break;
			}
			break;
		case IDC_M:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				m = GetDlgItemInt(hDlg, IDC_M, &ret, false);
				m = MathHelper::Clamp(m, gMinM, gMaxM);
				
				
				if (!MathHelper::CheckPowerOfTwo(m)) {
					m = MathHelper::CalNumToPowerOfTwo(m);

				}


				m += 1;
				SetDlgItemInt(hDlg, IDC_M, m, false);

				break;
			}
			break;
		case IDC_CELL:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				cell = GetDlgItemInt(hDlg, IDC_CELL, &ret, false);
				cell = MathHelper::Clamp(cell, gMinCell, gMaxCell);
				if (!MathHelper::CheckPowerOfTwo(cell)) {
					cell = MathHelper::CalNumToPowerOfTwo(cell);

				}

				SetDlgItemInt(hDlg, IDC_CELL, cell, false);

				break;
			}
			break;
		}

		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK HeightMapImportDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int cx, cy, x, y;
	int w, d, h, m, cell;
	RECT rect;
	static HWND hCombo, hSignRadio;
	static char format[][10] = { "8 bit", "16 bit" };
	switch (iMessage)
	{

	case WM_INITDIALOG:
		GetWindowRect(hDlg, &rect);
		hCombo = GetDlgItem(hDlg, IDC_FORMAT);
		hSignRadio = GetDlgItem(hDlg, IDC_SIGN);
		SendMessage(hSignRadio, BM_SETCHECK, BST_CHECKED, 0);
		SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)format[0]);
		SendMessageA(hCombo, CB_ADDSTRING, 1, (LPARAM)format[1]);
		SendMessageA(hCombo, CB_SETCURSEL, 0, 0);

		cx = GetSystemMetrics(SM_CXSCREEN);
		cy = GetSystemMetrics(SM_CYSCREEN);
		x = cx / 2 - rect.right / 2;
		y = cy / 2 - rect.bottom / 2;

		MoveWindow(hDlg, x, y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_BUTTON_OK:

			BOOL ret;

			gWidth = GetDlgItemInt(hDlg, IDC_WIDTH, &ret, false);
			gHeight = GetDlgItemInt(hDlg, IDC_HEIGHT, &ret, false);
			gDepth = GetDlgItemInt(hDlg, IDC_DEPTH, &ret, false);
			gM = GetDlgItemInt(hDlg, IDC_M, &ret, false);


			if (!SendMessageA(hCombo, CB_GETCURSEL, 0, 0))gHeigtMapIOBit = 16;
			else gHeigtMapIOBit = 8;
			
			EndDialog(hDlg, 1);
			return TRUE;

		case IDC_FLIP:
			gbFlipVertical = !gbFlipVertical;

			break;

		case IDC_SIGN:
			gbSignAbility = !gbSignAbility;
			break;

		case IDC_WIDTH:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				w = GetDlgItemInt(hDlg, IDC_WIDTH, &ret, false);
				w = MathHelper::Clamp(w, gMinWidth, gMaxWidth);
				SetDlgItemInt(hDlg, IDC_WIDTH, w, false);

				break;
			}
			break;
		case IDC_DEPTH:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				d = GetDlgItemInt(hDlg, IDC_DEPTH, &ret, false);
				d = MathHelper::Clamp(d, gMinDepth, gMaxDepth);
				SetDlgItemInt(hDlg, IDC_DEPTH, d, false);

				break;
			}
			break;
		case IDC_HEIGHT:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				h = GetDlgItemInt(hDlg, IDC_HEIGHT, &ret, false);
				h = MathHelper::Clamp(h, gMinHeight, gMaxHeight);
				SetDlgItemInt(hDlg, IDC_HEIGHT, h, false);

				break;
			}
			break;
		case IDC_M:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				m = GetDlgItemInt(hDlg, IDC_M, &ret, false);
				m = MathHelper::Clamp(m, gMinM, gMaxM);

				if (!MathHelper::CheckPowerOfTwo(m)) {
					m = MathHelper::CalNumToPowerOfTwo(m);

				}

				m += 1;
				SetDlgItemInt(hDlg, IDC_M, m, false);

				break;
			}
			break;
	
		}

		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return FALSE;
}


INT_PTR CALLBACK HeightMapExportDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int cx, cy, x, y;
	int w, d, h, m, cell;
	RECT rect;
	static HWND hCombo, hSignRadio;
	static char format[][10] = { "8 bit", "16 bit" };
	switch (iMessage)
	{

	case WM_INITDIALOG:
		GetWindowRect(hDlg, &rect);
		hCombo = GetDlgItem(hDlg, IDC_FORMAT);


		SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)format[0]);
		SendMessageA(hCombo, CB_ADDSTRING, 1, (LPARAM)format[1]);
		SendMessageA(hCombo, CB_SETCURSEL, 0, 0);

		cx = GetSystemMetrics(SM_CXSCREEN);
		cy = GetSystemMetrics(SM_CYSCREEN);
		x = cx / 2 - rect.right / 2;
		y = cy / 2 - rect.bottom / 2;

		MoveWindow(hDlg, x, y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_BUTTON_OK:

			BOOL ret;
			gWidth = GetDlgItemInt(hDlg, IDC_WIDTH, &ret, false);
			gHeight = GetDlgItemInt(hDlg, IDC_HEIGHT, &ret, false);
			gDepth = GetDlgItemInt(hDlg, IDC_DEPTH, &ret, false);
			gM = GetDlgItemInt(hDlg, IDC_M, &ret, false);


			if (!SendMessageA(hCombo, CB_GETCURSEL, 0, 0))gHeigtMapIOBit = 16;
			else gHeigtMapIOBit = 8;

			EndDialog(hDlg, 1);
			return TRUE;

			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return FALSE;
}

void ErrorHandling(char* err_msg)
{
	MessageBoxA(0, err_msg, "Error", MB_OK);
}
