/**
**    Hatchit Engine
**    Copyright(c) 2015 Third-Degree
**
**    GNU Lesser General Public License
**    This file may be used under the terms of the GNU Lesser
**    General Public License version 3 as published by the Free
**    Software Foundation and appearing in the file LICENSE.LGPLv3 included
**    in the packaging of this file. Please review the following information
**    to ensure the GNU Lesser General Public License requirements
**    will be met: https://www.gnu.org/licenses/lgpl.html
**
**/

#include <ht_d3d12renderer.h>
#include <ht_debug.h>
#include <wrl.h>

namespace Hatchit {

    namespace Graphics {

        namespace DirectX {

            D3D12Renderer::D3D12Renderer()
            {
                m_pipelineState = nullptr;
            }

            D3D12Renderer::~D3D12Renderer()
            {
                DirectX::ReleaseCOM(m_device);
                DirectX::ReleaseCOM(m_swapChain);
                DirectX::ReleaseCOM(m_renderTargetViewHeap);
                DirectX::ReleaseCOM(m_commandQueue);
                DirectX::ReleaseCOM(m_commandAllocator);
                DirectX::ReleaseCOM(m_commandList);
                DirectX::ReleaseCOM(m_pipelineState);
                DirectX::ReleaseCOM(m_fence);
                for (int i = 0; i < NUM_RENDER_TARGETS; i++)
                    DirectX::ReleaseCOM(m_renderTargets[i]);
            }

            bool D3D12Renderer::VInitialize(const RendererParams& params)
            {
                m_clearColor = params.clearColor;

                HRESULT hr = S_OK;

                /*
                * Initialize the D3D12 Debug Layer
                */
#ifdef _DEBUG
                Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
                {
                    debugController->EnableDebugLayer();
                }
#endif
                IDXGIFactory4* factory = nullptr;
                hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to create DXGIFactory.\n");
#endif
                    DirectX::ReleaseCOM(factory);
                    return false;
                }

                IDXGIAdapter1* hardwareAdapter = nullptr;
                hr = checkHardwareAdapter(factory, &hardwareAdapter);
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to find suitable Direct3D12 adapter.\n");
#endif
                    DirectX::ReleaseCOM(hardwareAdapter);
                    DirectX::ReleaseCOM(factory);
                    return false;
                }

                /*
                * Create the device
                */
                hr = D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to create Direct3D12 device.\n");
#endif
                    DirectX::ReleaseCOM(hardwareAdapter);
                    DirectX::ReleaseCOM(factory);
                    return false;
                }

                /**
                * Describe and create the command queue
                *
                * NOTE: 
                *  Specifying COMMAND_LIST_TYPE_DIRECT means
                *  this command buffer is only available for use with direct
                *  GPU calls.
                */
                D3D12_COMMAND_QUEUE_DESC queueDesc = {};
                queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

                hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to create Direct3D12 Command Queue.\n");
#endif
                    DirectX::ReleaseCOM(hardwareAdapter);
                    DirectX::ReleaseCOM(factory);
                    return false;
                }

                RECT r;
                GetClientRect((HWND)params.window, &r);
                UINT width = r.right - r.left;
                UINT height = r.bottom - r.top;

                DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
                swapChainDesc.BufferCount = NUM_RENDER_TARGETS; //double buffered
                swapChainDesc.BufferDesc.Width = width;
                swapChainDesc.BufferDesc.Height = height;
                swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                swapChainDesc.OutputWindow = (HWND)params.window;
                swapChainDesc.SampleDesc.Count = 1;
                swapChainDesc.Windowed = true;


                /**
                * NOTE:
                * New to Direct3D12, the swap chain must now be passed
                * the command queue rather than the device. As it will operate and flush
                * the command queue directly.
                */
                IDXGISwapChain* swapChain;
                hr = factory->CreateSwapChain(m_commandQueue, &swapChainDesc, &swapChain);
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to create swapChain.\n");
#endif
                    DirectX::ReleaseCOM(hardwareAdapter);
                    DirectX::ReleaseCOM(factory);
                    return false;
                }

                /*
                * Attempt to query a IDXGISwapChain3 interface from swap chain
                */
                hr = swapChain->QueryInterface(__uuidof(IDXGISwapChain3), reinterpret_cast<void**>(&m_swapChain));
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to query IDXGISwapChain3 interface.\n");
#endif
                    DirectX::ReleaseCOM(hardwareAdapter);
                    DirectX::ReleaseCOM(factory);
                    return false;
                }
                DirectX::ReleaseCOM(swapChain);
                m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

                /*
                * Create desciptor heaps
                */
                D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
                rtvHeapDesc.NumDescriptors = 2;
                rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_renderTargetViewHeap));
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to create render target view heap.\n");
#endif
                    DirectX::ReleaseCOM(hardwareAdapter);
                    DirectX::ReleaseCOM(factory);
                    return false;
                }
                m_renderTargetViewHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

                /*
                * Now we will create the resources for each frame
                */
                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
                for (uint32_t i = 0; i < NUM_RENDER_TARGETS; i++)
                {
                    hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
                    if (FAILED(hr))
                    {
#ifdef _DEBUG
                        Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to get render target buffer for resource creation.\n");
#endif
                        DirectX::ReleaseCOM(hardwareAdapter);
                        DirectX::ReleaseCOM(factory);
                        return false;
                    }

                    m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, rtvHandle);

                    rtvHandle.ptr += 1 * m_renderTargetViewHeapSize;
                }

                /*
                * Create a command allocator for managing memory for command list
                */
                hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to create command allocator for command list memory allocation.\n");
#endif
                    DirectX::ReleaseCOM(hardwareAdapter);
                    DirectX::ReleaseCOM(factory);
                    return false;
                }

                DirectX::ReleaseCOM(hardwareAdapter);
                DirectX::ReleaseCOM(factory);


                /*
                * Create the command list
                */
                hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr, IID_PPV_ARGS(&m_commandList));
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to create command list.\n");
#endif
                    return false;
                }

                DirectX::ThrowIfFailed(m_commandList->Close());

                /*
                * Create synchronization objects
                */
                hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    Core::DebugPrintF("D3D12Renderer::VInitialize(), Failed to create fence.\n");
#endif
                    return false;
                }
                m_fenceValue = 1;
                m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                if (!m_fenceEvent) 
                {
                    DirectX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
                    return false;
                }
                    
                
                return true;
            }

            void D3D12Renderer::VDeInitialize()
            {
                waitForFrame();

                CloseHandle(m_fenceEvent);
            }

            void D3D12Renderer::VSetClearColor(const Color& color)
            {
                m_clearColor = color;
            }

            void D3D12Renderer::VClearBuffer(ClearArgs args)
            {
                
            }

            void D3D12Renderer::VPresent()
            {
               
                DirectX::ThrowIfFailed(m_commandAllocator->Reset());

                DirectX::ThrowIfFailed(m_commandList->Reset(m_commandAllocator, m_pipelineState));

                m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_renderTargetViewHeapSize);

                /*
                * Record basic command
                */
                m_commandList->ClearRenderTargetView(rtvHandle, reinterpret_cast<float*>(&m_clearColor), 0, nullptr);

                /*
                * Indicate that we are using the back buffer to present
                */
                m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

                DirectX::ThrowIfFailed(m_commandList->Close());

                /*
                * Execute command list
                */
                ID3D12CommandList* ppCommandLists[] = { m_commandList };
                m_commandQueue->ExecuteCommandLists(1, ppCommandLists);

                DirectX::ThrowIfFailed(m_swapChain->Present(1, 0));

                waitForFrame();
            }

            void D3D12Renderer::VResizeBuffers(uint32_t width, uint32_t height)
            {

            }

            HRESULT D3D12Renderer::checkHardwareAdapter(IDXGIFactory2 * pFactory, IDXGIAdapter1 ** ppAdapter)
            {
                HRESULT hr = S_OK;

                IDXGIAdapter1* _adapter;

                *ppAdapter = nullptr;
            
                /*
                * Iterate over all available adapters, and select the first
                * adapter that supports Direct3D 12
                */
                for (uint32_t index = 0; index < DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(index, &_adapter); index++)
                {
                    DXGI_ADAPTER_DESC1 desc;
                    _adapter->GetDesc1(&desc);

                    /**
                    * We do not want to select the "Microsoft Basic Render Driver" adapter
                    * as this is a default render-only adapter with no outputs. WARP should
                    * be used instead for software device
                    */
                    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                        continue;

                    /**
                    * Check to see if the adapter (GPU) supports Direct3D 12
                    */
                    hr = D3D12CreateDevice(_adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
                    if (SUCCEEDED(hr))
                        break;
                }

                *ppAdapter = _adapter;

                return hr;
            }

            void D3D12Renderer::waitForFrame()
            {
                //Signal and increment fence value
                const uint64_t fence = m_fenceValue;
                DirectX::ThrowIfFailed(m_commandQueue->Signal(m_fence, fence));
                m_fenceValue++;

                //Wait until previous frame is finished
                if (m_fence->GetCompletedValue() < fence)
                {
                    DirectX::ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
                    WaitForSingleObject(m_fenceEvent, INFINITE);
                }

                m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
            }


        }

    }

}