#include "Graphics.h"
#include <DirectXMath.h>
#include <vector>
#include "imgui/imgui_impl_dx11.h"

// use this way to link libraries
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")       // needed to compile shaders

// less typing while not losing too much clarity
namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

Graphics::Graphics(HWND hwnd, unsigned int width, unsigned int height)
	:
	width(width),
	height(height)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hwnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	);

	 wrl::ComPtr<ID3D11Resource> pBackBuffer = nullptr;
	 pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer);
	 pDevice->CreateRenderTargetView(
		 pBackBuffer.Get(),
		 nullptr,
		 &pTarget
	 );

	 // create z-buffer
	 D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	 dsDesc.DepthEnable = TRUE;
	 dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	 dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	 wrl::ComPtr<ID3D11DepthStencilState> pDSState;
	 pDevice->CreateDepthStencilState(&dsDesc, &pDSState);

	 // bind depth state
	 pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

	 // create depth stensil texture
	 wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	 D3D11_TEXTURE2D_DESC descDepth = {};
	 descDepth.Width = width;
	 descDepth.Height = height;
	 descDepth.MipLevels = 1u;
	 descDepth.ArraySize = 1u;
	 descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	 descDepth.SampleDesc.Count = 1u;
	 descDepth.SampleDesc.Quality = 0u;
	 descDepth.Usage = D3D11_USAGE_DEFAULT;
	 descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	 pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

	 // create view of depth stensil texture
	 D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	 descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	 descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	 descDSV.Texture2D.MipSlice = 0u;
	 pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDSV);

	 // bind depth stensil to pipeline
	 pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());

	 // configure viewport
	 D3D11_VIEWPORT vp;
	 vp.Width = float(width);
	 vp.Height = float(height);
	 vp.MinDepth = 0.0f;
	 vp.MaxDepth = 1.0f;
	 vp.TopLeftX = 0.0f;
	 vp.TopLeftY = 0.0f;
	 pContext->RSSetViewports(1u, &vp);

	 // init imgui
	 ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());
}

void Graphics::EndFrame()
{
	// swaps the front and back buffer  
	// _____    ____
	// | 1 | -> |  |
	// |0_2| <- |__|
	pSwap->Present(0u, 0u);
}

void Graphics::TestDraw(int x, int y, Gui& gui)
{
	// static variables for testing only
	static float offsetX = 0.0f;
	static float offsetY = 0.0f;
	static float triangleColor[4] = { 1.0f, 0.55f, 0.60f, 1.00f };

	gui.AddText("some random text");

	// clear and set background color
	FLOAT color[4] = { 0,1.0f,159.0f/255.0f,1.0f };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
	// clear z-buffer
	pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

	struct Vertex
	{
		struct
		{
			float x;
			float y;
		}pos;
	};

	// vertices being sent to gpu to be drawn
	Vertex vertices[] =
	{
		{-0.5f,-0.5f},
		{ 0.0f, 0.5f},
		{ 0.5f,-0.5f}
	};

	for (auto& v : vertices)
	{
		v.pos.x += offsetX;
		v.pos.y += offsetY;
	}
	// adding gui sliders
	gui.AddSlider("offset x", offsetX, -1.0f, 1.0f);
	gui.AddSlider("offset y", offsetY, -1.0f, 1.0f);

	// need to make sure indices are CLOCKWISE WINDING or gpu will cull it
	//    1
	//  0   2
	const unsigned short indices[]
	{
		0,1,2
	};
	
	// create vertex buffer and bind to pipeline
	wrl::ComPtr<ID3D11Buffer> pVertexBuffer = nullptr;
	D3D11_SUBRESOURCE_DATA srd;
	srd.pSysMem = vertices;

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(vertices);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.StructureByteStride = sizeof(Vertex);

	pDevice->CreateBuffer(&bd, &srd, &pVertexBuffer);
	UINT stride = sizeof(Vertex);
	UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);

	// create index buffer and bind to pipeline 
	wrl::ComPtr<ID3D11Buffer> pIndexBuffer = nullptr;
	srd.pSysMem = indices;

	bd.ByteWidth = sizeof(indices);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.StructureByteStride = sizeof(unsigned short);

	pDevice->CreateBuffer(&bd, &srd, &pIndexBuffer);
	stride = sizeof(unsigned short);
	offset = 0u;
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, offset);

	// blob for shaders
	wrl::ComPtr<ID3DBlob> pBlob;

	// create vertex shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	D3DReadFileToBlob(L"VertexShaderBasic.cso", &pBlob);
	pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);
	// bind vertex shader
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);

	// input (vertex) layout
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};
	// input layout wants blob from creation of vertex shader so it needs to be done first
	pDevice->CreateInputLayout(ied, (UINT)std::size(ied), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout);
	pContext->IASetInputLayout(pInputLayout.Get());

	// create pixel shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	D3DReadFileToBlob(L"PixelShaderBasic.cso", &pBlob);
	pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
	// bind pixel shader
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);
	
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());

	// sending 4 floats to the pixel shader
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;

	D3D11_BUFFER_DESC cbd;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = 4*sizeof(float);
	cbd.StructureByteStride = 0u;

	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = triangleColor;
	pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer);

	// bind 4 floats to pixel shader
	pContext->PSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());

	// variable color panel
	gui.AddColor4("<-(click) Triangle Color", triangleColor);

	// set primitive topology
	pContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw object
	pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u);
	
}