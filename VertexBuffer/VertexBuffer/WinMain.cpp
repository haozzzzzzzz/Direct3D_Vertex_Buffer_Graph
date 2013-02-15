#include <d3d9.h>


//�ͷ�Com�����
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) {if(p){(p)->Release();(p)=NULL;}}
#endif


wchar_t*g_pClassName=L"HelloDirect3D";//��������
wchar_t*g_pWindowName=L" ��Direct3Dʵ��";
LPDIRECT3DDEVICE9 g_pd3dDevice=NULL;//Direct3D�豸�ӿ�
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuf = NULL;//���㻺��ӿ�

//���������ζ���Ķ���ṹ
struct CUSTOMVERTEX
{
	FLOAT _x ,_y ,_z ,_rhw;	//�����λ��
	DWORD _color;//�������ɫ
	CUSTOMVERTEX(FLOAT _x,FLOAT _y, FLOAT _z,FLOAT _rhw,DWORD color):_x(_x),_y(_y),_z(_z),_rhw(_rhw),_color(color){}
};

//�����ʽ
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)


HRESULT InitDirect3D(HWND hWnd);//��ʼ��Direct3D
void Direct3DRender();//��Ⱦͼ��
void Direct3DCleanup();//����Direct3D��Դ

//���㻺��
void CreateVertexBuffer();
void DrawPrimitive();

//���ڹ��̺���
LRESULT CALLBACK WinMainProc(HWND,UINT,WPARAM,LPARAM);

//-----------------------------------------
//Name : WinMain();
//Desc : WindowsӦ�ó�����ں���
//----------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd )
{
	WNDCLASS wndclass;
	//��ƴ�����
	wndclass.cbClsExtra=0;
	wndclass.cbWndExtra=0;
	wndclass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hInstance=hInstance;
	wndclass.lpfnWndProc=WinMainProc;
	wndclass.lpszClassName=g_pClassName;
	wndclass.lpszMenuName=NULL;
	wndclass.style=CS_HREDRAW|CS_VREDRAW;

	//ע�ᴰ��
	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL,L"ע�ᴰ��ʧ��",L"������ʾ",NULL);
		return 1;
	}

	//��������
	HWND hWnd=CreateWindow(g_pClassName,g_pWindowName,WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,500,500,NULL,NULL,hInstance,NULL);
	if (!hWnd)
	{
		MessageBox(NULL,L"��������ʧ��",L"������ʾ",NULL);
		return 1;
	}

	//��ʼ�� Direct3D
	InitDirect3D(hWnd);


	//��ʾ����
	ShowWindow(hWnd,SW_SHOWNORMAL);
	
	//���´���
	UpdateWindow(hWnd);

	//������Ϣ
	MSG msg;
	ZeroMemory(&msg,sizeof(msg));
	while(msg.message!=WM_QUIT)
	{
		//����Ϣ������ȡ��Ϣ��ɾ����������Ϣ
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);//ת����Ϣ
			DispatchMessage(&msg);//�ַ���Ϣ
		}
		else
		{
			Direct3DRender();//����3D����
		}
	}

	UnregisterClass(g_pClassName,hInstance);
	return (int)msg.wParam;
}


//���ڹ��̺���
LRESULT CALLBACK WinMainProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:	//�ͻ�����h
		Direct3DRender();//��Ⱦͼ��
		ValidateRect(hWnd,NULL);//���¿ͻ�����ʾ
		break;
	case WM_DESTROY://����������Ϣ
		Direct3DCleanup();//����Direct3D
		PostQuitMessage(0);//�˳�
		break;
	case WM_KEYDOWN:
		if (wParam==VK_ESCAPE)//ESC��
		{
			DestroyWindow(hWnd);//���ٴ��ڣ�������һ��WM_DESTROY��Ϣ
		}
		break;
	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);
		break;
	}
	return 0;
}


//Direct3D��ʼ��
HRESULT InitDirect3D(HWND hWnd)
{
	//����IDirect3D�ӿ�
	LPDIRECT3D9 pD3D=NULL;//IDirect3D9�ӿ�
	pD3D=Direct3DCreate9(D3D_SDK_VERSION);//����IDirect3D9�ӿڶ���
	//D3D_SDK_VERSION������ʾ��ǰʹ�õ�DirectX SDK�汾������ȷ��Ӧ�ó������а�����ͷ�ļ��ڱ���ʱ�ܹ���DirectX����ʱ��DLL��ƥ��

	if (pD3D==NULL)
	{
		return E_FAIL;
	}


	//��ȡӲ���豸��Ϣ
	//IDirect3D9�ӿ��ṩ��GetDeviceCaps������ȡָ���豸�����ܲ������÷�������ȡ�õ�Ӳ���豸��Ϣ���浽һ��D3DCAPS9�ṹ��
	/*
	HRESULT IDirect3D9::GetDeviceCaps( 
	UINT  Adapter, 
	D3DDEVTYPE  DeviceType, 
	D3DCAPS9  *pCaps 
	);
	*/
	D3DCAPS9 caps;
	int vp=0;
	pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,&caps);
	if (caps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		vp=D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
		vp=D3DCREATE_SOFTWARE_VERTEXPROCESSING;




	//����Direct3D�豸�ӿ�
	//�ڴ���IDirect3DDevice9�ӿڶ���֮ǰ����Ҫ׼��һ��D3DPRESENT_PARAMETERS�ṹ���͵Ĳ���������˵����δ���Direct3D�豸
	/*
	typedef struct _D3DPRESENT_PARAMETERS_ {
	UINT 			BackBufferWidth;//��̨���������
	UINT 			BackBufferHeight; //��̨�������߶�
	D3DFORMAT   		BackBufferFormat;//...�������صĸ�ʽ
	UINT 			BackBufferCount;//..����
	D3DMULTISAMPLE_TYPE  	MultiSampleType;//����ȡ������
	DWORD   			MultiSampleQuality;//����ȡ������
	D3DSWAPEFFECT  		SwapEffect;//��̨���������ݸ��Ƶ�ǰ̨�������ķ�ʽ
	HWND 			hDeviceWindow;//����ͼ�εĴ��ھ��
	BOOL 			Windowed;
	BOOL 			EnableAutoDepthStencil;//Direct3D�Ƿ�ΪӦ�ó����Զ������ڴ����
	D3DFORMAT   		AutoDepthStencilFormat;//��ʽ
	DWORD   			Flags;//�������� ͨ��Ϊ0
	UINT 			FullScreen_RefreshRateInHz;//ȫ��ģʽʱָ����Ļˢ����
	UINT 			PresentationInterval;//ָ��ƽ��ķ�תģʽ���ڴ���ģʽ�� ��ȡֵΪ0
	} D3DPRESENT_PARAMETERS;
	*/
	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory(&d3dpp, sizeof(d3dpp));


	d3dpp.BackBufferWidth=640;
	d3dpp.BackBufferHeight=480;
	d3dpp.BackBufferFormat=D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount=1;
	d3dpp.MultiSampleType=D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality=0;
	d3dpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow=hWnd;
	d3dpp.Windowed=TRUE;
	d3dpp.EnableAutoDepthStencil=TRUE;
	d3dpp.AutoDepthStencilFormat=D3DFMT_D24S8;
	d3dpp.Flags=0;
	d3dpp.FullScreen_RefreshRateInHz=D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;

	//�����D3DPRESENT_PARAMETERS�ṹ�󣬿��Ե���IDirect3D9�ӿڵ�CreateDeivce��������IDirect3DDevice9�ӿڶ���
	/*
	HRESULT IDirect3DDevice9::CreateDevice( 
	UINT     Adapter, //��ʾ��������IDirect3DDevice9�ӿڶ�����������Կ�����
	D3DDEVTYPE  DeviceType, 
	HWND   hFocusWindow, 
	DWORD  BehaviorFlags, //Direct3D�豸����3D����ķ�ʽ
	D3DPRESENT_PARAMETERS  *pPresentationParameters,
	IDirect3DDevice9  **ppReturnedDeviceInterface 
	);
	*/
	//IDirect3D9�ӿڵ�CreateDevice��������һ��HRESULT���͵ķ���ֵ������ͨ��SUCCESSED��FALIED���жϸú�����ִ�н�������CreateDevice����ִ�гɹ���SUCCESSED�꽫����TRUE����FAILED���򷵻�FALSE��
	pD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,vp,&d3dpp,&g_pd3dDevice);

	pD3D->Release();//�ͷ�Direct3D�ӿ�


	//�������㻺��
	CreateVertexBuffer();
	return S_OK;

}


//---------------------------------------
//Name : Direct3DRender()
//Desc : ����3D����
//---------------------------------------
void Direct3DRender()
{
	/*
	Direct3D�Ļ��ƹ��̾��ǣ����ơ���ʾ�����ơ���ʾ��
	���ǣ�ÿ����ʼ����ͼ��֮ǰ������Ҫͨ��IDirect3DDevice9�ӿڵ�Clear��������̨�����е����ݽ�����գ������ñ���������ɫ��

	HRESULT IDirect3DDevice9::Clear( 
	DWORD   Count,
	const D3DRECT *pRects, //ָ���Ա���ָ���ľ������������������������а����ľ��ε�������Count����ָ��
	DWORD    Flags, //ָ������Ҫ����ı��棬�ò�������ȡֵ��D3DCLEAR_TARGET��D3DCLEAR_ZBUFFER��D3DCLEAR_STENCIL���ֱ��ʾ�Ժ�̨���桢��Ȼ����ģ�建��������
	D3DCOLOR Color, //����ָ��������������ݺ����õı���ɫ������ͨ��D3DCOLOR_XRGB�꽫RGBֵת�����ò���
	float        Z, //ָ��������������ݺ�������Ȼ����ֵ
	DWORD    Stencil 
	); 

	*/
	g_pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(45,50,170),1.0f,0);//��ɫ

	//��ʼ����
	g_pd3dDevice->BeginScene();

	/*ͼ�λ��Ƶ�ʵ�ʹ���*/
	DrawPrimitive();

	//��������
	g_pd3dDevice->EndScene();

	//��ת
	g_pd3dDevice->Present(NULL,NULL,NULL,NULL);
}








//-----------------------------------------------
//Name : Direct3DCleanup()
//Desc : ����Direct3D , ���ͷ�COM�ӿ�
//----------------------------------------------
void Direct3DCleanup()
{
	SAFE_RELEASE(g_pd3dDevice);
}


//-------------------------------------------
//Name : CreateVertexBuffer()
//Desc : ����ͼ�ζ��㻺��
//-------------------------------------------

void CreateVertexBuffer()
{
	//���㻺�棨Vertex Buffer����Direct3D�������涥�����ݵ��ڴ�ռ䣬����λ��ϵͳ�ڴ��ͼ�ο����Դ��С�
	//��Direct3D����ͼ��ʱ����������Щ����ṹ����һ���������б��������������״������
	/*
	��Direct3D�У����㻺����IDirect3DVertexBuffer9�ӿڶ����ʾ����ʹ�ö��㻺��֮ǰ����Ҫ������������Ķ���ṹ��Ȼ�����ͨ��IDirect3DDevice9�ӿڵ�CreateVertexBuffer�����������㻺��

	HRESULT IDirect3DDevice9::CreateVertexBuffer(
	UINT       Length,
	DWORD    Usage, //ָ��ʹ�û����һЩ��������
	DWORD    FVF, //��Ҫ�洢�ڶ��㻺���е������ʽ
	D3DPOOL   Pool, //һ��D3DPOOLö�����ͣ�����ָ���洢���㻺�������������ڴ�λ�ã���Ĭ�������ʱλ���Դ���
	IDirect3DVertexBuffer9**  ppVertexBuffer, //IDirect3DVertexBuffer9�ӿ����͵ı�����ָ�룬�ֱ����ڽ��մ����Ķ��㻺������������ָ��
	HANDLE*   pSharedHandle
	);

	*/
	//�������㻺��
	g_pd3dDevice->CreateVertexBuffer(6*sizeof(CUSTOMVERTEX),0,D3DFVF_CUSTOMVERTEX,D3DPOOL_DEFAULT,&g_pVertexBuf,NULL);

	//��䶥������
	CUSTOMVERTEX*pVertices=NULL;

	/*
	��ȡ��IDirect3DVertexBuffer9�ӿڶ����ָ��󣬾Ϳ���ͨ����ýӿڵ�Lock������ȡָ�򶥵㻺��Ĵ洢����ָ�룬��ͨ����ָ����ʻ����е�����

	HRESULT IDirect3DVertexBuffer9::Lock(
	UINT     OffsetToLock, //�ڴ洢���м�������ʼλ��
	UINT     SizeToLock, //���ֽ�Ϊ��λ�ļ����Ĵ洢����С��ֵΪ0��ʾ��������洢��
	BYTE  **ppbData, //���ڽ��ձ������Ĵ洢����ʼλ�õ�ָ�룬�ò���ָ��Ĵ洢���Ķ�д��ʽ��Flags����ָ��
	DWORD  Flags
	);

	*/

	//����������
	g_pVertexBuf->Lock(0,0,(void**)&pVertices,0);
	pVertices[0]=CUSTOMVERTEX(220.0f, 120.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255,0,0));//V0
	pVertices[1]=CUSTOMVERTEX(420.0f, 120.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0,255,0));//V1
	pVertices[2]=CUSTOMVERTEX(220.0f, 320.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255,255,0));//V2

	pVertices[3]=CUSTOMVERTEX(420.0f, 120.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0,255,0));//V1
	pVertices[4]=CUSTOMVERTEX(420.0f, 320.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0,0,255));//V3
	pVertices[5]=CUSTOMVERTEX(220.0f, 320.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255,255,0));//V2

	//��ʹ��Lock������Ӧ���������м�����������ݵķ��ʺ���Ҫ����IDirect3DVertexBuffer9�ӿڵ�Unlock�����Ի�����н���
	g_pVertexBuf->Unlock();
}

//-------------------------------------------
//Name : DrawPrimitive()
//Desc :	���ƶ��㻺���е�ͼ��
//-------------------------------------------
void DrawPrimitive()
{
	//��Ҫͨ��IDirect3DDevice9�ӿڵ�SetStreamSource������������������Ϣ�Ķ��㻺������Ⱦ��ˮ�������
	/*
	HRESULT IDirect3DDevice9::SetStreamSource(
	UINT  StreamNumber, //����ָ����ö��㻺�潨�����ӵ�������
	IDirect3DVertexBuffer9 *pStreamData, //�����������ݵĶ��㻺��ָ��
	UINT  OffsetInBytes, //�������������ֽ�Ϊ��λ��ƫ����
	UINT  Stride //�ڶ��㻺���д洢��ÿ������ṹ�Ĵ�С������ͨ��sizeof��������㶥��ṹ��ʵ�ʴ�С
	);

	*/
	//��Ⱦ������
	g_pd3dDevice->SetStreamSource(0,g_pVertexBuf,0,sizeof(CUSTOMVERTEX));

	//����IDirect3DDevice9�ӿڵ�SetFVF����ָ�������ʽ
	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	//������ģ�͵Ķ�����䵽���㻺�棬����������������Դ�Ͷ����ʽ�󣬿��Ե���IDirect3DDevice9�ӿڵ�DrawPrimitive�������ݶ��㻺���еĶ������ģ�͡�
	/*
	HRESULT IDirect3DDevice9::DrawPrimitive(
	D3DPRIMITIVETYPE  PrimitiveType,
	UINT  StartVertex,
	UINT  PrimitiveCount
	);

	*/
	g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,0,2);
}
