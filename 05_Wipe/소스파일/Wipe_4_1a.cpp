﻿//------------------------------------------------------------
// Wipe_4_1a.cpp
// 원형 화면 전환 (경계 블렌드) 
// 
//------------------------------------------------------------

#include <D3D11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>

#define PI					3.14159265f			// 원주율
#define VIEW_WIDTH			640					// 화면 너비 
#define VIEW_HEIGHT			480					// 화면 높이 
#define GRAD_WIDTH			100					// 그라데이션 너비 
#define DIVIDE_NUM			50					// 분할수 
#define MIN_R				10.0f				// 최소반경 


// 텍스처 구조체 
struct TEX_PICTURE {
	ID3D11ShaderResourceView	*pSRViewTexture;
	D3D11_TEXTURE2D_DESC		tdDesc;
	int							nWidth, nHeight;
};


ID3D11DeviceContext		*g_pImmediateContext;	// 디바이스 컨텍스트 
ID3D11BlendState		*g_pbsAlphaBlend;


int FlushDrawingPictures( void );				// 그림 그리기 대기 행렬 플러시 
int DrawPicture( float x, float y, TEX_PICTURE *pTexPic );	// 그림 그리기 
int Draw2DPolygon( float x1, float y1, float u1, float v1,
				   float x2, float y2, float u2, float v2,
				   float x3, float y3, float u3, float v3,
				   TEX_PICTURE *pTexPic );		// 2D폴리곤 그리기 
int Draw2DPolygonWithColor( float x1, float y1, float u1, float v1, int nColor1,
							float x2, float y2, float u2, float v2, int nColor2,
							float x3, float y3, float u3, float v3, int nColor3,
							TEX_PICTURE *pTexPic );	// 2D폴리곤 그리기(색)



float	fBoundary_r1, fBoundary_r2, fBoundary_r3;
float	fBoundary_v;

TEX_PICTURE				g_tPic1, g_tPic2;



int InitChangingPictures( void )						// 처음 한 번만 호출된다 
{
	fBoundary_r1 = -GRAD_WIDTH + MIN_R;					// r1 초기화 
	fBoundary_r2 = fBoundary_r1 + GRAD_WIDTH;			// r2 초기화 
	fBoundary_r3 = sqrtf( VIEW_WIDTH * VIEW_WIDTH + VIEW_HEIGHT * VIEW_HEIGHT ) / 2.0f;	// r3 초기화 
	fBoundary_v = 5.0f;								// 경계 속도

	return 0;
}


int DrawChangingPictures( void )						// 매 프레임 호출된다 
{
	int					i;
	float				fAngle1, fAngle2;
	float				fAngleDelta;
	float				xt[6], yt[6];					// 위쪽 직선상의 점 
	int					nCenterColor;

	fBoundary_r1 += fBoundary_v;						// 경계선을 움직인다 
	if ( fBoundary_r1 < -GRAD_WIDTH + MIN_R ) {			// 최소원 
		fBoundary_r1 = -GRAD_WIDTH + MIN_R;
		fBoundary_v = -fBoundary_v;
	}
	if ( fBoundary_r1 > fBoundary_r3 ) {				// 최대원 
		fBoundary_r1 = fBoundary_r3;
		fBoundary_v = -fBoundary_v;
	}
	fBoundary_r2 =  fBoundary_r1 + GRAD_WIDTH;
	fAngleDelta = 2.0f * PI / DIVIDE_NUM;
	fAngle1 = 0.0f;
	fAngle2 = fAngleDelta;
	if ( fBoundary_r1 > 1.0f ) {
		for ( i = 0; i < DIVIDE_NUM; i++ ) {
			xt[0] = VIEW_WIDTH  / 2.0f + fBoundary_r1 * cosf( fAngle1 );
			yt[0] = VIEW_HEIGHT / 2.0f + fBoundary_r1 * sinf( fAngle1 );
			xt[1] = VIEW_WIDTH  / 2.0f + fBoundary_r1 * cosf( fAngle2 );
			yt[1] = VIEW_HEIGHT / 2.0f + fBoundary_r1 * sinf( fAngle2 );
			xt[2] = VIEW_WIDTH  / 2.0f + fBoundary_r2 * cosf( fAngle1 );
			yt[2] = VIEW_HEIGHT / 2.0f + fBoundary_r2 * sinf( fAngle1 );
			xt[3] = VIEW_WIDTH  / 2.0f + fBoundary_r2 * cosf( fAngle2 );
			yt[3] = VIEW_HEIGHT / 2.0f + fBoundary_r2 * sinf( fAngle2 );
			xt[4] = VIEW_WIDTH  / 2.0f + fBoundary_r3 * cosf( fAngle1 );
			yt[4] = VIEW_HEIGHT / 2.0f + fBoundary_r3 * sinf( fAngle1 );
			xt[5] = VIEW_WIDTH  / 2.0f + fBoundary_r3 * cosf( fAngle2 );
			yt[5] = VIEW_HEIGHT / 2.0f + fBoundary_r3 * sinf( fAngle2 );
			FlushDrawingPictures();
			g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
			Draw2DPolygon( VIEW_WIDTH  / 2.0f, VIEW_HEIGHT / 2.0f, 0.5f, 0.5f,
						   xt[1], yt[1], xt[1] / VIEW_WIDTH, yt[1] / VIEW_HEIGHT,
						   xt[0], yt[0], xt[0] / VIEW_WIDTH, yt[0] / VIEW_HEIGHT,
						   &g_tPic1 );
			Draw2DPolygon( xt[0], yt[0], xt[0] / VIEW_WIDTH, yt[0] / VIEW_HEIGHT,
						   xt[5], yt[5], xt[5] / VIEW_WIDTH, yt[5] / VIEW_HEIGHT,
						   xt[4], yt[4], xt[4] / VIEW_WIDTH, yt[4] / VIEW_HEIGHT,
						   &g_tPic2 );
			Draw2DPolygon( xt[5], yt[5], xt[5] / VIEW_WIDTH, yt[5] / VIEW_HEIGHT,
						   xt[0], yt[0], xt[0] / VIEW_WIDTH, yt[0] / VIEW_HEIGHT,
						   xt[1], yt[1], xt[1] / VIEW_WIDTH, yt[1] / VIEW_HEIGHT,
						   &g_tPic2 );
			FlushDrawingPictures();
			g_pImmediateContext->OMSetBlendState( g_pbsAlphaBlend, NULL, 0xFFFFFFFF );
			Draw2DPolygonWithColor( xt[0], yt[0], xt[0] / VIEW_WIDTH, yt[0] / VIEW_HEIGHT, 0xffffffff,
									xt[3], yt[3], xt[3] / VIEW_WIDTH, yt[3] / VIEW_HEIGHT, 0x00ffffff,
									xt[2], yt[2], xt[2] / VIEW_WIDTH, yt[2] / VIEW_HEIGHT, 0x00ffffff,
						   &g_tPic1 );
			Draw2DPolygonWithColor( xt[3], yt[3], xt[3] / VIEW_WIDTH, yt[3] / VIEW_HEIGHT, 0x00ffffff,
									xt[0], yt[0], xt[0] / VIEW_WIDTH, yt[0] / VIEW_HEIGHT, 0xffffffff,
									xt[1], yt[1], xt[1] / VIEW_WIDTH, yt[1] / VIEW_HEIGHT, 0xffffffff,
						   &g_tPic1 );
			fAngle1 += fAngleDelta;
			fAngle2 += fAngleDelta;
		}
	}
	else {
		FlushDrawingPictures();
		g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
		DrawPicture( 0.0f, 0.0f, &g_tPic2 );
		for ( i = 0; i < DIVIDE_NUM; i++ ) {
			xt[2] = VIEW_WIDTH  / 2.0f + fBoundary_r2 * cosf( fAngle1 );
			yt[2] = VIEW_HEIGHT / 2.0f + fBoundary_r2 * sinf( fAngle1 );
			xt[3] = VIEW_WIDTH  / 2.0f + fBoundary_r2 * cosf( fAngle2 );
			yt[3] = VIEW_HEIGHT / 2.0f + fBoundary_r2 * sinf( fAngle2 );
			FlushDrawingPictures();
			g_pImmediateContext->OMSetBlendState( g_pbsAlphaBlend, NULL, 0xFFFFFFFF );
			nCenterColor = ( ( int )( fBoundary_r2 * 255 / GRAD_WIDTH ) << 24 ) + 0xffffff;
			Draw2DPolygonWithColor( VIEW_WIDTH  / 2.0f, VIEW_HEIGHT / 2.0f, 0.5f, 0.5f,    nCenterColor,
									xt[3], yt[3], xt[3] / VIEW_WIDTH, yt[3] / VIEW_HEIGHT, 0x00ffffff,
									xt[2], yt[2], xt[2] / VIEW_WIDTH, yt[2] / VIEW_HEIGHT, 0x00ffffff,
									&g_tPic1 );
			fAngle1 += fAngleDelta;
			fAngle2 += fAngleDelta;
		}
	}

	return 0;
}


//------------------------------------------------------------
// 이하 DirectX에 의한 표시 프로그램


#include <stdio.h>
#include <windows.h>
#include <tchar.h>							// Unicode, 멀티바이트 문자 관련  


#define MAX_BUFFER_VERTEX				10000	// 최대 버퍼 정점수 


// 링크 라이브러리 
#pragma comment( lib, "d3d11.lib" )   			// D3D11 라이브러리
#pragma comment( lib, "d3dx11.lib" )


// 세이프 릴리스 매크로 
#ifndef SAFE_RELEASE
#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release(); ( p )=NULL; } }
#endif


// 정점 구조체 
struct CUSTOMVERTEX {
    XMFLOAT4	v4Pos;
    XMFLOAT4	v4Color;
	XMFLOAT2	v2UV;
};

// 셰이더 상수 구조체 
struct CBNeverChanges
{
    XMMATRIX mView;
};


// 글로벌 변수  
UINT  g_nClientWidth;							// 그리기 영역 너비 
UINT  g_nClientHeight;							// 그리기 영역 높이 

HWND        g_hWnd;         					// 윈도 핸들 


ID3D11Device			*g_pd3dDevice;			// 디바이스 
IDXGISwapChain		*g_pSwapChain;			// DXGI 스왑체인 
ID3D11RasterizerState	*g_pRS;					// 래스터라이저 
ID3D11RenderTargetView	*g_pRTV;					// 렌더링 타깃 
D3D_FEATURE_LEVEL      g_FeatureLevel;			// 피처 레빌 

ID3D11Buffer			*g_pD3D11VertexBuffer;
ID3D11VertexShader	*g_pVertexShader;
ID3D11PixelShader		*g_pPixelShader;
ID3D11InputLayout		*g_pInputLayout;
ID3D11SamplerState	*g_pSamplerState;

ID3D11Buffer			*g_pCBNeverChanges = NULL;

// 그리기 정점 버퍼  
CUSTOMVERTEX g_cvVertices[MAX_BUFFER_VERTEX];
int							g_nVertexNum = 0;
ID3D11ShaderResourceView	*g_pNowTexture = NULL;


// Direct3D 초기화 
HRESULT InitD3D( void )
{
    HRESULT hr = S_OK;
	D3D_FEATURE_LEVEL  FeatureLevelsRequested[6] = { D3D_FEATURE_LEVEL_11_0,
													 D3D_FEATURE_LEVEL_10_1,
													 D3D_FEATURE_LEVEL_10_0,
													 D3D_FEATURE_LEVEL_9_3,
													 D3D_FEATURE_LEVEL_9_2,
													 D3D_FEATURE_LEVEL_9_1 };
	UINT               numLevelsRequested = 6;
	D3D_FEATURE_LEVEL  FeatureLevelsSupported;

	// 디바이스 작성 
	hr = D3D11CreateDevice( NULL,
					D3D_DRIVER_TYPE_HARDWARE, 
					NULL, 
					0,
					FeatureLevelsRequested, 
					numLevelsRequested,
					D3D11_SDK_VERSION, 
					&g_pd3dDevice,
					&FeatureLevelsSupported,
					&g_pImmediateContext );
	if( FAILED ( hr ) ) {
		return hr;
	}

	// 팩토리 취득 
	IDXGIDevice * pDXGIDevice;
	hr = g_pd3dDevice->QueryInterface( __uuidof( IDXGIDevice ), ( void ** )&pDXGIDevice );
	IDXGIAdapter * pDXGIAdapter;
	hr = pDXGIDevice->GetParent( __uuidof( IDXGIAdapter ), ( void ** )&pDXGIAdapter );
	IDXGIFactory * pIDXGIFactory;
	pDXGIAdapter->GetParent( __uuidof( IDXGIFactory ), ( void ** )&pIDXGIFactory);

	// 스왑체인 작성 
    DXGI_SWAP_CHAIN_DESC	sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = g_nClientWidth;
	sd.BufferDesc.Height = g_nClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	hr = pIDXGIFactory->CreateSwapChain( g_pd3dDevice, &sd, &g_pSwapChain );

	pDXGIDevice->Release();
	pDXGIAdapter->Release();
	pIDXGIFactory->Release();

	if( FAILED ( hr ) ) {
		return hr;
	}

    // 렌더링 타깃 생성 
    ID3D11Texture2D			*pBackBuffer = NULL;
    D3D11_TEXTURE2D_DESC BackBufferSurfaceDesc;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't get backbuffer." ), _T( "Error" ), MB_OK );
        return hr;
    }
    pBackBuffer->GetDesc( &BackBufferSurfaceDesc );
    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRTV );
    SAFE_RELEASE( pBackBuffer );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create render target view." ), _T( "Error" ), MB_OK );
        return hr;
    }

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRTV, NULL );

    // 래스터라이저 설정 
    D3D11_RASTERIZER_DESC drd;
	ZeroMemory( &drd, sizeof( drd ) );
	drd.FillMode				= D3D11_FILL_SOLID;
	drd.CullMode				= D3D11_CULL_NONE;
	drd.FrontCounterClockwise	= FALSE;
	drd.DepthClipEnable			= TRUE;
    hr = g_pd3dDevice->CreateRasterizerState( &drd, &g_pRS );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create rasterizer state." ), _T( "Error" ), MB_OK );
        return hr;
    }
    g_pImmediateContext->RSSetState( g_pRS );

    // 뷰포트 설정 
    D3D11_VIEWPORT vp;
    vp.Width    = ( FLOAT )g_nClientWidth;
    vp.Height   = ( FLOAT )g_nClientHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    return S_OK;
}


// 프로그래머블 셰이더 작성  
HRESULT MakeShaders( void )
{
    HRESULT hr;
    ID3DBlob* pVertexShaderBuffer = NULL;
    ID3DBlob* pPixelShaderBuffer = NULL;
    ID3DBlob* pError = NULL;

    DWORD dwShaderFlags = 0;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
    // 컴파일 
    hr = D3DX11CompileFromFile( _T( "Basic_2D.fx" ), NULL, NULL, "VS", "vs_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pVertexShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't open Basic_2D.fx" ), _T( "Error" ), MB_OK );
        SAFE_RELEASE( pError );
        return hr;
    }
    hr = D3DX11CompileFromFile( _T( "Basic_2D.fx" ), NULL, NULL, "PS", "ps_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pPixelShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pError );
        return hr;
    }
    SAFE_RELEASE( pError );
    
    // VertexShader 작성 
    hr = g_pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
										   pVertexShaderBuffer->GetBufferSize(),
										   NULL, &g_pVertexShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }
    // PixelShader 작성 
    hr = g_pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
										  pPixelShaderBuffer->GetBufferSize(),
										  NULL, &g_pPixelShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }

    // 입력 버퍼의 입력 형식 
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );
	// 입력 버퍼의 입력 형식 작성  
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements,
										  pVertexShaderBuffer->GetBufferPointer(),
										  pVertexShaderBuffer->GetBufferSize(),
										  &g_pInputLayout );
    SAFE_RELEASE( pVertexShaderBuffer );
    SAFE_RELEASE( pPixelShaderBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // 셰이더 상수 버퍼 작성 
    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof( bd ) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( CBNeverChanges );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;

	// 변환행렬  
    CBNeverChanges	cbNeverChanges;
	XMMATRIX		mScreen;
    mScreen = XMMatrixIdentity();
	mScreen._11 =  2.0f / g_nClientWidth;
	mScreen._22 = -2.0f / g_nClientHeight;
	mScreen._41 = -1.0f;
	mScreen._42 =  1.0f;
	cbNeverChanges.mView = XMMatrixTranspose( mScreen );
	g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

    return S_OK;
}


// 텍스처 로드 
int LoadTexture( TCHAR *szFileName, TEX_PICTURE *pTexPic, int nWidth, int nHeight,
				 int nTexWidth, int nTexHeight )
{
    HRESULT						hr;
	D3DX11_IMAGE_LOAD_INFO		liLoadInfo;
	ID3D11Texture2D				*pTexture;

	ZeroMemory( &liLoadInfo, sizeof( D3DX11_IMAGE_LOAD_INFO ) );
	liLoadInfo.Width = nTexWidth;
	liLoadInfo.Height = nTexHeight;
	liLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	liLoadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, szFileName, &liLoadInfo,
												 NULL, &( pTexPic->pSRViewTexture ), NULL );
    if ( FAILED( hr ) ) {
        return hr;
    }
	pTexPic->pSRViewTexture->GetResource( ( ID3D11Resource ** )&( pTexture ) );
	pTexture->GetDesc( &( pTexPic->tdDesc ) );
	pTexture->Release();

	pTexPic->nWidth = nWidth;
	pTexPic->nHeight = nHeight;

	return S_OK;
}


// 그리기 모드 오브젝트 초기화 
int InitDrawModes( void )
{
    HRESULT				hr;

	// 블렌드 스테이트 
    D3D11_BLEND_DESC BlendDesc;
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;
    BlendDesc.RenderTarget[0].BlendEnable           = TRUE;
    BlendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    BlendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = g_pd3dDevice->CreateBlendState( &BlendDesc, &g_pbsAlphaBlend );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // 샘플러 
    D3D11_SAMPLER_DESC samDesc;
    ZeroMemory( &samDesc, sizeof( samDesc ) );
    samDesc.Filter          = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressV        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressW        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.ComparisonFunc  = D3D11_COMPARISON_ALWAYS;
    samDesc.MaxLOD          = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &samDesc, &g_pSamplerState );
    if ( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}


// 지오메트리 초기화 
HRESULT InitGeometry( void )
{
    HRESULT hr = S_OK;

    // 정점 버퍼 작성 
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth            = sizeof( CUSTOMVERTEX ) * MAX_BUFFER_VERTEX;
    BufferDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags            = 0;

    D3D11_SUBRESOURCE_DATA SubResourceData;
    SubResourceData.pSysMem             = g_cvVertices;
    SubResourceData.SysMemPitch         = 0;
    SubResourceData.SysMemSlicePitch    = 0;
    hr = g_pd3dDevice->CreateBuffer( &BufferDesc, &SubResourceData, &g_pD3D11VertexBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

	// 텍스처 작성 
	g_tPic1.pSRViewTexture =  NULL;
	g_tPic2.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "17.bmp" ), &g_tPic1, 640, 480, 1024, 512 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 17.bmp" ), _T( "Error" ), MB_OK );
       return hr;
    }
	hr = LoadTexture( _T( "18.BMP" ), &g_tPic2, 640, 480, 1024, 512 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 18.BMP" ), _T( "Error" ), MB_OK );
        return hr;
    }

	return S_OK;
}


// 종료처리  
int Cleanup( void )
{
    SAFE_RELEASE( g_tPic1.pSRViewTexture );
    SAFE_RELEASE( g_tPic2.pSRViewTexture );
    SAFE_RELEASE( g_pD3D11VertexBuffer );

    SAFE_RELEASE( g_pSamplerState );
    SAFE_RELEASE( g_pbsAlphaBlend );
    SAFE_RELEASE( g_pInputLayout );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pCBNeverChanges );

    SAFE_RELEASE( g_pRS );									// 래스터라이저 

	// 스테이터스 클리어 
	if ( g_pImmediateContext ) {
		g_pImmediateContext->ClearState();
		g_pImmediateContext->Flush();
	}

    SAFE_RELEASE( g_pRTV );								// 렌더링 타깃 

    // 스왑체인
    if ( g_pSwapChain != NULL ) {
        g_pSwapChain->SetFullscreenState( FALSE, 0 );
    }
    SAFE_RELEASE( g_pSwapChain );

    SAFE_RELEASE( g_pImmediateContext );					// 디바이스 컨텍스트
    SAFE_RELEASE( g_pd3dDevice );							// 디바이스

	return 0;
}


// 윈도 프로시저  
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


// 그림 그리기 대기행렬 플러시  
int FlushDrawingPictures( void )
{
	HRESULT			hr;

	if ( ( g_nVertexNum > 0 ) && g_pNowTexture ) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		hr = g_pImmediateContext->Map( g_pD3D11VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		if ( SUCCEEDED( hr ) ) {
			CopyMemory( mappedResource.pData, &( g_cvVertices[0] ), sizeof( CUSTOMVERTEX ) * g_nVertexNum );
			g_pImmediateContext->Unmap( g_pD3D11VertexBuffer, 0 );
		}
		g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pNowTexture );
		g_pImmediateContext->Draw( g_nVertexNum, 0 );
	}
	g_nVertexNum = 0;
	g_pNowTexture = NULL;

	return 0;
}


// 그림 그리기  
int DrawPicture( float x, float y, TEX_PICTURE *pTexPic )
{
	if ( g_nVertexNum > ( MAX_BUFFER_VERTEX - 6 ) ) return -1;	// 정점이 버퍼에서 넘치면 그리지 않고  
	// 텍스처가 바뀌면 대기 행렬 플러시 
	if ( ( pTexPic->pSRViewTexture != g_pNowTexture ) && g_pNowTexture ) {
		FlushDrawingPictures();
	}

	// 정점 세트  
	g_cvVertices[g_nVertexNum + 0].v4Pos   = XMFLOAT4( x,                         y,                    0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v2UV    = XMFLOAT2( 0.0f, 0.0f );
	g_cvVertices[g_nVertexNum + 1].v4Pos   = XMFLOAT4( x,                         y + pTexPic->nHeight, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v2UV    = XMFLOAT2( 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v4Pos   = XMFLOAT4( x + pTexPic->nWidth,       y,                    0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v2UV    = XMFLOAT2( 1.0f, 0.0f );
	g_cvVertices[g_nVertexNum + 3] = g_cvVertices[g_nVertexNum + 1];
	g_cvVertices[g_nVertexNum + 4].v4Pos   = XMFLOAT4( x + pTexPic->nWidth,       y + pTexPic->nHeight, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 4].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 4].v2UV    = XMFLOAT2( 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 5] = g_cvVertices[g_nVertexNum + 2];
	g_nVertexNum += 6;
	g_pNowTexture = pTexPic->pSRViewTexture;

	return 0;
}


// 2D 폴리곤 그리기 
int Draw2DPolygon( float x1, float y1, float u1, float v1,
				   float x2, float y2, float u2, float v2,
				   float x3, float y3, float u3, float v3,
				   TEX_PICTURE *pTexPic )
{
	if ( g_nVertexNum > ( MAX_BUFFER_VERTEX - 3 ) ) return -1;	// 정점이 버퍼에서 넘치면 그리지 않고  

	// 텍스처가 바뀌면 대기 행렬 플러시
	if ( ( pTexPic->pSRViewTexture != g_pNowTexture ) && g_pNowTexture ) {
		FlushDrawingPictures();
	}

	// 정점 세트 
	g_cvVertices[g_nVertexNum + 0].v4Pos   = XMFLOAT4( x1, y1, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v2UV    = XMFLOAT2( u1, v1 );
	g_cvVertices[g_nVertexNum + 1].v4Pos   = XMFLOAT4( x2, y2, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v2UV    = XMFLOAT2( u2, v2 );
	g_cvVertices[g_nVertexNum + 2].v4Pos   = XMFLOAT4( x3, y3, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v4Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v2UV    = XMFLOAT2( u3, v3 );
	g_nVertexNum += 3;
	g_pNowTexture = pTexPic->pSRViewTexture;

	return 0;
}


// 2D 폴리곤 그리기(색) 
int Draw2DPolygonWithColor( float x1, float y1, float u1, float v1, int nColor1,
							float x2, float y2, float u2, float v2, int nColor2,
							float x3, float y3, float u3, float v3, int nColor3,
							TEX_PICTURE *pTexPic )
{
	if ( g_nVertexNum > ( MAX_BUFFER_VERTEX - 3 ) ) return -1;	//  정점이 버퍼에서 넘치면 그리지 않고

	// 텍스처가 바뀌면 대기 행렬 플러시
	if ( ( pTexPic->pSRViewTexture != g_pNowTexture ) && g_pNowTexture ) {
		FlushDrawingPictures();
	}

	// 정점 세트 
	g_cvVertices[g_nVertexNum + 0].v4Pos   = XMFLOAT4( x1, y1, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 0].v4Color = XMFLOAT4( ( float )( ( nColor1 >> 16 ) & 0xff ) / 255.0f,
													   ( float )( ( nColor1 >>  8 ) & 0xff ) / 255.0f,
													   ( float )(   nColor1         & 0xff ) / 255.0f,
													   ( float )( ( nColor1 >> 24 ) & 0xff ) / 255.0f );
	g_cvVertices[g_nVertexNum + 0].v2UV    = XMFLOAT2( u1, v1 );
	g_cvVertices[g_nVertexNum + 1].v4Pos   = XMFLOAT4( x2, y2, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v4Color = XMFLOAT4( ( float )( ( nColor2 >> 16 ) & 0xff ) / 255.0f,
													   ( float )( ( nColor2 >>  8 ) & 0xff ) / 255.0f,
													   ( float )(   nColor2         & 0xff ) / 255.0f,
													   ( float )( ( nColor2 >> 24 ) & 0xff ) / 255.0f );
	g_cvVertices[g_nVertexNum + 1].v2UV    = XMFLOAT2( u2, v2 );
	g_cvVertices[g_nVertexNum + 2].v4Pos   = XMFLOAT4( x3, y3, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v4Color = XMFLOAT4( ( float )( ( nColor3 >> 16 ) & 0xff ) / 255.0f,
													   ( float )( ( nColor3 >>  8 ) & 0xff ) / 255.0f,
													   ( float )(   nColor3         & 0xff ) / 255.0f,
													   ( float )( ( nColor3 >> 24 ) & 0xff ) / 255.0f );
	g_cvVertices[g_nVertexNum + 2].v2UV    = XMFLOAT2( u3, v3 );
	g_nVertexNum += 3;
	g_pNowTexture = pTexPic->pSRViewTexture;

	return 0;
}


// 렌더링  
HRESULT Render( void )
{
    // 화면 클리어 
	XMFLOAT4	v4Color = XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f );
    g_pImmediateContext->ClearRenderTargetView( g_pRTV, ( float * )&v4Color );

    // 샘플러 래스터라이저 세트 
    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerState );
    g_pImmediateContext->RSSetState( g_pRS );
    
    // 화면 설정  
    UINT nStrides = sizeof( CUSTOMVERTEX );
    UINT nOffsets = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pD3D11VertexBuffer, &nStrides, &nOffsets );
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    g_pImmediateContext->IASetInputLayout( g_pInputLayout );

    // 셰이더 설정 
    g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );
    g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );

    // 그리기 
    g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
	DrawChangingPictures();

    // 표시 
	FlushDrawingPictures();

    return S_OK;
}


// 엔트리 포인트 
int WINAPI _tWinMain( HINSTANCE hInst, HINSTANCE, LPTSTR, int )
{
	LARGE_INTEGER			nNowTime, nLastTime;		// 현재와 한 프레임 전의 시각 
	LARGE_INTEGER			nTimeFreq;					// 시간단위 

    // 화면 크기 
    g_nClientWidth  = VIEW_WIDTH;						// 너비 
    g_nClientHeight = VIEW_HEIGHT;						// 높이  

	// Register the window class
    WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
                      _T( "D3D Sample" ), NULL };
    RegisterClassEx( &wc );

	RECT rcRect;
	SetRect( &rcRect, 0, 0, g_nClientWidth, g_nClientHeight );
	AdjustWindowRect( &rcRect, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( _T( "D3D Sample" ), _T( "Wipe_4_1a" ),
						   WS_OVERLAPPEDWINDOW, 100, 20, rcRect.right - rcRect.left, rcRect.bottom - rcRect.top,
						   GetDesktopWindow(), NULL, wc.hInstance, NULL );

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D() ) && SUCCEEDED( MakeShaders() ) )
    {
        // Create the shaders
        if( SUCCEEDED( InitDrawModes() ) )
        {
			if ( SUCCEEDED( InitGeometry() ) ) {				// 지오메트리 초기화  

				// Show the window
				ShowWindow( g_hWnd, SW_SHOWDEFAULT );
				UpdateWindow( g_hWnd );

				InitChangingPictures();							// 캐릭터 초기화
				
				QueryPerformanceFrequency( &nTimeFreq );			//  시간단위 
				QueryPerformanceCounter( &nLastTime );			// 1프레임 전 시각 초기화  

				// Enter the message loop
				MSG msg;
				ZeroMemory( &msg, sizeof( msg ) );
				while( msg.message != WM_QUIT )
				{
					Render();
//					DrawChangingPictures();
					do {
						if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
						{
							TranslateMessage( &msg );
							DispatchMessage( &msg );
						}
						QueryPerformanceCounter( &nNowTime );
					} while( ( ( nNowTime.QuadPart - nLastTime.QuadPart ) < ( nTimeFreq.QuadPart / 90 ) ) &&
							 ( msg.message != WM_QUIT ) );
					while( ( ( nNowTime.QuadPart - nLastTime.QuadPart ) < ( nTimeFreq.QuadPart / 60 ) ) &&
						   ( msg.message != WM_QUIT ) )
					{
						QueryPerformanceCounter( &nNowTime );
					}
					nLastTime = nNowTime;
					g_pSwapChain->Present( 0, 0 );					// 표시 
				}
			}
        }
    }

    // Clean up everything and exit the app
    Cleanup();
    UnregisterClass( _T( "D3D Sample" ), wc.hInstance );
    return 0;
}
