#include <d3d9.h>


//释放Com对象宏
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) {if(p){(p)->Release();(p)=NULL;}}
#endif


wchar_t*g_pClassName=L"HelloDirect3D";//窗口类名
wchar_t*g_pWindowName=L" 简单Direct3D实例";
LPDIRECT3DDEVICE9 g_pd3dDevice=NULL;//Direct3D设备接口
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuf = NULL;//顶点缓存接口

//描述三角形顶点的顶点结构
struct CUSTOMVERTEX
{
	FLOAT _x ,_y ,_z ,_rhw;	//顶点的位置
	DWORD _color;//顶点的颜色
	CUSTOMVERTEX(FLOAT _x,FLOAT _y, FLOAT _z,FLOAT _rhw,DWORD color):_x(_x),_y(_y),_z(_z),_rhw(_rhw),_color(color){}
};

//顶点格式
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)


HRESULT InitDirect3D(HWND hWnd);//初始化Direct3D
void Direct3DRender();//渲染图形
void Direct3DCleanup();//清理Direct3D资源

//顶点缓存
void CreateVertexBuffer();
void DrawPrimitive();

//窗口过程函数
LRESULT CALLBACK WinMainProc(HWND,UINT,WPARAM,LPARAM);

//-----------------------------------------
//Name : WinMain();
//Desc : Windows应用程序入口函数
//----------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd )
{
	WNDCLASS wndclass;
	//设计窗口类
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

	//注册窗口
	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL,L"注册窗口失败",L"错误提示",NULL);
		return 1;
	}

	//创建窗口
	HWND hWnd=CreateWindow(g_pClassName,g_pWindowName,WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,500,500,NULL,NULL,hInstance,NULL);
	if (!hWnd)
	{
		MessageBox(NULL,L"创建窗口失败",L"错误提示",NULL);
		return 1;
	}

	//初始化 Direct3D
	InitDirect3D(hWnd);


	//显示窗口
	ShowWindow(hWnd,SW_SHOWNORMAL);
	
	//更新窗口
	UpdateWindow(hWnd);

	//处理消息
	MSG msg;
	ZeroMemory(&msg,sizeof(msg));
	while(msg.message!=WM_QUIT)
	{
		//从消息队列中取消息并删除队列中消息
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);//转换消息
			DispatchMessage(&msg);//分发消息
		}
		else
		{
			Direct3DRender();//绘制3D场景
		}
	}

	UnregisterClass(g_pClassName,hInstance);
	return (int)msg.wParam;
}


//窗口过程函数
LRESULT CALLBACK WinMainProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:	//客户区重h
		Direct3DRender();//渲染图形
		ValidateRect(hWnd,NULL);//更新客户区显示
		break;
	case WM_DESTROY://窗口销毁消息
		Direct3DCleanup();//清理Direct3D
		PostQuitMessage(0);//退出
		break;
	case WM_KEYDOWN:
		if (wParam==VK_ESCAPE)//ESC键
		{
			DestroyWindow(hWnd);//销毁窗口，并发送一条WM_DESTROY消息
		}
		break;
	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);
		break;
	}
	return 0;
}


//Direct3D初始化
HRESULT InitDirect3D(HWND hWnd)
{
	//创建IDirect3D接口
	LPDIRECT3D9 pD3D=NULL;//IDirect3D9接口
	pD3D=Direct3DCreate9(D3D_SDK_VERSION);//创建IDirect3D9接口对象
	//D3D_SDK_VERSION参数表示当前使用的DirectX SDK版本，用于确保应用程序所有包含的头文件在编译时能够与DirectX运行时的DLL相匹配

	if (pD3D==NULL)
	{
		return E_FAIL;
	}


	//获取硬件设备信息
	//IDirect3D9接口提供了GetDeviceCaps方法获取指定设备的性能参数，该方法将所取得的硬件设备信息保存到一个D3DCAPS9结构中
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




	//创建Direct3D设备接口
	//在创建IDirect3DDevice9接口对象之前，需要准备一个D3DPRESENT_PARAMETERS结构类型的参数，用于说明如何创建Direct3D设备
	/*
	typedef struct _D3DPRESENT_PARAMETERS_ {
	UINT 			BackBufferWidth;//后台缓冲区宽度
	UINT 			BackBufferHeight; //后台缓冲区高度
	D3DFORMAT   		BackBufferFormat;//...保存像素的格式
	UINT 			BackBufferCount;//..数量
	D3DMULTISAMPLE_TYPE  	MultiSampleType;//多重取样类型
	DWORD   			MultiSampleQuality;//多重取样质量
	D3DSWAPEFFECT  		SwapEffect;//后台缓冲区内容复制到前台缓冲区的方式
	HWND 			hDeviceWindow;//绘制图形的窗口句柄
	BOOL 			Windowed;
	BOOL 			EnableAutoDepthStencil;//Direct3D是否为应用程序自动管理内存深度
	D3DFORMAT   		AutoDepthStencilFormat;//方式
	DWORD   			Flags;//附加属性 通常为0
	UINT 			FullScreen_RefreshRateInHz;//全屏模式时指定屏幕刷新率
	UINT 			PresentationInterval;//指定平面的翻转模式，在窗口模式下 其取值为0
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

	//在填充D3DPRESENT_PARAMETERS结构后，可以调用IDirect3D9接口的CreateDeivce方法创建IDirect3DDevice9接口对象
	/*
	HRESULT IDirect3DDevice9::CreateDevice( 
	UINT     Adapter, //表示将创建的IDirect3DDevice9接口对象所代表的显卡类型
	D3DDEVTYPE  DeviceType, 
	HWND   hFocusWindow, 
	DWORD  BehaviorFlags, //Direct3D设备进行3D运算的方式
	D3DPRESENT_PARAMETERS  *pPresentationParameters,
	IDirect3DDevice9  **ppReturnedDeviceInterface 
	);
	*/
	//IDirect3D9接口的CreateDevice函数返回一个HRESULT类型的返回值，可以通过SUCCESSED和FALIED宏判断该函数的执行结果。如果CreateDevice函数执行成功，SUCCESSED宏将返回TRUE，而FAILED宏则返回FALSE。
	pD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,vp,&d3dpp,&g_pd3dDevice);

	pD3D->Release();//释放Direct3D接口


	//创建顶点缓存
	CreateVertexBuffer();
	return S_OK;

}


//---------------------------------------
//Name : Direct3DRender()
//Desc : 绘制3D场景
//---------------------------------------
void Direct3DRender()
{
	/*
	Direct3D的绘制过程就是：绘制→显示→绘制→显示。
	但是，每当开始绘制图形之前，都需要通过IDirect3DDevice9接口的Clear方法将后台缓存中的内容进行清空，并设置表面的填充颜色等

	HRESULT IDirect3DDevice9::Clear( 
	DWORD   Count,
	const D3DRECT *pRects, //指定对表面指定的矩形区域进行清除操作，数组中包含的矩形的数量由Count参数指定
	DWORD    Flags, //指定了需要清除的表面，该参数可以取值于D3DCLEAR_TARGET、D3DCLEAR_ZBUFFER和D3DCLEAR_STENCIL，分别表示对后台缓存、深度缓存和模板缓存进行清除
	D3DCOLOR Color, //用于指定在清除缓存内容后设置的背景色，可以通过D3DCOLOR_XRGB宏将RGB值转换给该参数
	float        Z, //指定当清除缓存内容后设置深度缓存的值
	DWORD    Stencil 
	); 

	*/
	g_pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(45,50,170),1.0f,0);//蓝色

	//开始绘制
	g_pd3dDevice->BeginScene();

	/*图形绘制的实际过程*/
	DrawPrimitive();

	//结束绘制
	g_pd3dDevice->EndScene();

	//翻转
	g_pd3dDevice->Present(NULL,NULL,NULL,NULL);
}








//-----------------------------------------------
//Name : Direct3DCleanup()
//Desc : 清理Direct3D , 并释放COM接口
//----------------------------------------------
void Direct3DCleanup()
{
	SAFE_RELEASE(g_pd3dDevice);
}


//-------------------------------------------
//Name : CreateVertexBuffer()
//Desc : 创建图形顶点缓存
//-------------------------------------------

void CreateVertexBuffer()
{
	//顶点缓存（Vertex Buffer）是Direct3D用来保存顶点数据的内存空间，可以位于系统内存和图形卡的显存中。
	//当Direct3D绘制图形时，将根据这些顶点结构创建一个三角形列表来描述物体的形状和轮廓
	/*
	在Direct3D中，顶点缓存由IDirect3DVertexBuffer9接口对象表示。在使用顶点缓存之前，需要定义描述顶点的顶点结构，然后可以通过IDirect3DDevice9接口的CreateVertexBuffer方法创建顶点缓存

	HRESULT IDirect3DDevice9::CreateVertexBuffer(
	UINT       Length,
	DWORD    Usage, //指定使用缓存的一些附加属性
	DWORD    FVF, //将要存储在顶点缓存中的灵活顶点格式
	D3DPOOL   Pool, //一个D3DPOOL枚举类型，用于指定存储顶点缓存或索引缓冲的内存位置，在默认情况下时位于显存中
	IDirect3DVertexBuffer9**  ppVertexBuffer, //IDirect3DVertexBuffer9接口类型的变量的指针，分别用于接收创建的顶点缓存和索引缓存的指针
	HANDLE*   pSharedHandle
	);

	*/
	//创建顶点缓存
	g_pd3dDevice->CreateVertexBuffer(6*sizeof(CUSTOMVERTEX),0,D3DFVF_CUSTOMVERTEX,D3DPOOL_DEFAULT,&g_pVertexBuf,NULL);

	//填充顶点数据
	CUSTOMVERTEX*pVertices=NULL;

	/*
	在取得IDirect3DVertexBuffer9接口对象的指针后，就可以通过这该接口的Lock方法获取指向顶点缓存的存储区的指针，并通过该指针访问缓存中的数据

	HRESULT IDirect3DVertexBuffer9::Lock(
	UINT     OffsetToLock, //在存储区中加锁的起始位置
	UINT     SizeToLock, //以字节为单位的加锁的存储区大小，值为0表示整个缓存存储区
	BYTE  **ppbData, //用于接收被锁定的存储区起始位置的指针，该参数指向的存储区的读写方式由Flags参数指定
	DWORD  Flags
	);

	*/

	//锁定缓存区
	g_pVertexBuf->Lock(0,0,(void**)&pVertices,0);
	pVertices[0]=CUSTOMVERTEX(220.0f, 120.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255,0,0));//V0
	pVertices[1]=CUSTOMVERTEX(420.0f, 120.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0,255,0));//V1
	pVertices[2]=CUSTOMVERTEX(220.0f, 320.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255,255,0));//V2

	pVertices[3]=CUSTOMVERTEX(420.0f, 120.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0,255,0));//V1
	pVertices[4]=CUSTOMVERTEX(420.0f, 320.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0,0,255));//V3
	pVertices[5]=CUSTOMVERTEX(220.0f, 320.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255,255,0));//V2

	//当使用Lock方法对应缓存区进行加锁并完成数据的访问后，需要调用IDirect3DVertexBuffer9接口的Unlock方法对缓存进行解锁
	g_pVertexBuf->Unlock();
}

//-------------------------------------------
//Name : DrawPrimitive()
//Desc :	绘制顶点缓存中的图形
//-------------------------------------------
void DrawPrimitive()
{
	//需要通过IDirect3DDevice9接口的SetStreamSource方法将包含几何体信息的顶点缓存与渲染流水线相关联
	/*
	HRESULT IDirect3DDevice9::SetStreamSource(
	UINT  StreamNumber, //用于指定与该顶点缓存建立连接的数据流
	IDirect3DVertexBuffer9 *pStreamData, //包含顶点数据的顶点缓存指针
	UINT  OffsetInBytes, //在数据流中以字节为单位的偏移量
	UINT  Stride //在顶点缓冲中存储的每个顶点结构的大小，可以通过sizeof运算符计算顶点结构的实际大小
	);

	*/
	//渲染正方形
	g_pd3dDevice->SetStreamSource(0,g_pVertexBuf,0,sizeof(CUSTOMVERTEX));

	//调用IDirect3DDevice9接口的SetFVF方法指定顶点格式
	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	//将物体模型的顶点填充到顶点缓存，并设置数据流输入源和顶点格式后，可以调用IDirect3DDevice9接口的DrawPrimitive方法根据顶点缓存中的顶点绘制模型。
	/*
	HRESULT IDirect3DDevice9::DrawPrimitive(
	D3DPRIMITIVETYPE  PrimitiveType,
	UINT  StartVertex,
	UINT  PrimitiveCount
	);

	*/
	g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,0,2);
}
