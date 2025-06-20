#include <thread>
#include <iostream>
#include <windows.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <audioclientactivationparams.h>
#include <wrl/client.h> // For ComPtr
#include <mfapi.h>
#include <wrl/implements.h>

using namespace Microsoft::WRL;

class LoopbackCapture
{
private:
    class AudioInterfaceActivateHandler : public RuntimeClass<
        RuntimeClassFlags<ClassicCom>,
        FtmBase,
        IActivateAudioInterfaceCompletionHandler>
    {
    public:
        HRESULT STDMETHODCALLTYPE ActivateCompleted(IActivateAudioInterfaceAsyncOperation* operation)
        {
            HRESULT hr = S_OK;
            HRESULT activateResult = E_FAIL;
            IUnknown* punkAudioInterface = nullptr;

            std::cout << "ACTIVATED" << std::endl;

            // Get the activation results
            hr = operation->GetActivateResult(&activateResult, &punkAudioInterface);
            if (FAILED(hr) || FAILED(activateResult) || !punkAudioInterface)
            {
                return E_FAIL;
            }

            // Get the audio client
            hr = punkAudioInterface->QueryInterface(IID_PPV_ARGS(&m_audioClient));
            if (FAILED(hr))
            {
                punkAudioInterface->Release();
                return hr;
            }

            // Store the audio client in the parent class
            if (m_parent)
            {
                m_parent->m_audioClient = m_audioClient;
                m_parent->OnAudioClientReady();
            }

            punkAudioInterface->Release();

            std::cout << "ALL GOOD" << std::endl;
            return S_OK;
        }

        void SetParent(LoopbackCapture* parent)
        {
            m_parent = parent;
        }

    private:
        LoopbackCapture* m_parent = nullptr;
        ComPtr<IAudioClient> m_audioClient;
    };

    ComPtr<AudioInterfaceActivateHandler> m_activateHandler;

public:
    ComPtr<IAudioCaptureClient> m_captureClient;
    HANDLE m_captureReadyEvent;
    HANDLE m_captureEvent;
    HANDLE m_stopEvent;

    UINT32 bufferFrameCount;

    HANDLE m_captureThread;
    std::atomic<bool> m_capturing = false;
    ComPtr<IAudioClient> m_audioClient;

    LoopbackCapture()
    {
        std::cout << "HERE?";
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        std::cout << "HERE2?";
        m_captureReadyEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        m_stopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        m_captureEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        std::cout << "HERE3?";

        m_activateHandler = Make<AudioInterfaceActivateHandler>();
        m_activateHandler->SetParent(this);
    }

    ~LoopbackCapture()
    {
        CloseHandle(m_captureReadyEvent);
        CloseHandle(m_stopEvent);
        CloseHandle(m_captureEvent);

        CoUninitialize();
    }

    HRESULT Initialize(DWORD processId, bool includeProcessTree)
    {
        std::cout << "Here6?";
        HRESULT hr;

        // hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);

        AUDIOCLIENT_ACTIVATION_PARAMS audioclientActivationParams = {};
        audioclientActivationParams.ActivationType = AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
        audioclientActivationParams.ProcessLoopbackParams.ProcessLoopbackMode = includeProcessTree ? PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE : PROCESS_LOOPBACK_MODE_EXCLUDE_TARGET_PROCESS_TREE;
        audioclientActivationParams.ProcessLoopbackParams.TargetProcessId = processId;

        PROPVARIANT activateParams = {};
        activateParams.vt = VT_BLOB;
        activateParams.blob.cbSize = sizeof(audioclientActivationParams);
        activateParams.blob.pBlobData = (BYTE*)&audioclientActivationParams;

        IActivateAudioInterfaceAsyncOperation* asyncOp = nullptr;
        hr = ActivateAudioInterfaceAsync(VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK, __uuidof(IAudioClient), &activateParams, m_activateHandler.Get(), &asyncOp);

        if (FAILED(hr))
        {
            // callback->Release();
            std::cout << "Here100?" << hr;
            return hr;
        }

        std::cout << "HELLLLOOO??" << hr;

        return hr;
    }

    HRESULT OnAudioClientReady()
    {
        HRESULT hr;
        // m_AudioClient = callback->GetAudioClient();
        // callback->Release();

        if (!m_audioClient)
        {
            std::cout << "Here10?";
            return E_FAIL;
        }

        WAVEFORMATEX captureFormat{};
        captureFormat.wFormatTag = WAVE_FORMAT_PCM;
        captureFormat.nChannels = 2;
        captureFormat.nSamplesPerSec = 44100;
        captureFormat.wBitsPerSample = 16;
        captureFormat.nBlockAlign = captureFormat.nChannels * captureFormat.wBitsPerSample / 8;
        captureFormat.nAvgBytesPerSec = captureFormat.nSamplesPerSec * captureFormat.nBlockAlign;

        hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            200000,
            AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,
            &captureFormat,
            nullptr);
        std::cout << "Here11?" << hr;

        hr = m_audioClient->GetBufferSize(&bufferFrameCount);
        if (FAILED(hr))
        {
            std::cout << "Here16?" << hr;
            return hr;
        }

        // CoTaskMemFree(&captureFormat);
        std::cout << "Here12?" << hr;
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_audioClient->GetService(IID_PPV_ARGS(&m_captureClient));
        if (FAILED(hr))
        {
            return hr;
        }

        // m_captureEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        // if (!m_captureEvent)
        // {
        //    return HRESULT_FROM_WIN32(GetLastError());
        // }

        // hr = m_audioClient->SetEventHandle(m_captureEvent);

        std::cout << "Here13?" << hr;

        std::cout << "Starting..." << std::endl;
        hr = m_audioClient->Start();
        if (FAILED(hr))
        {
            return hr;
        }

        std::cout << "Started loopback capture..." << std::endl;

        m_capturing = true;
        m_captureThread = CreateThread(NULL, 0, LoopbackCapture::CaptureThreadProc, this, 0, NULL);

        return hr;
    }

    HRESULT StopCapture()
    {
        if (!m_capturing)
        {
            return S_OK;
        }

        m_capturing = false;
        if (m_captureThread != NULL)
        {
            CloseHandle(m_captureThread);
            m_captureThread = NULL;
            ;
        }

        HRESULT hr = m_audioClient->Stop();
        CloseHandle(m_captureEvent);
        m_captureEvent = nullptr;
        return hr;
    }

    static DWORD WINAPI CaptureThreadProc(LPVOID lpParameter)
    {
        LoopbackCapture* loopbackCapture = static_cast<LoopbackCapture*>(lpParameter);
        loopbackCapture->CaptureThread();
        return 0;
    }

    void CaptureThread()
    {
        std::cout << "HELLO?" << m_capturing;
        // try
        // {
        //    // Your capture logic here
        // }
        // catch (const std::exception &e)
        // {
        //    // Handle exception (e.g., log it)
        //    std::cerr << "Exception in CaptureThread: " << e.what() << std::endl;
        // }
        // catch (...)
        // {
        //    // Handle unknown exceptions
        //    std::cerr << "Unknown exception in CaptureThread" << std::endl;
        // }
        // if (m_CaptureEvent == INVALID_HANDLE_VALUE)
        // {
        //    std::cerr << "m_CaptureEvent is invalid: " << GetLastError() << std::endl;
        //    return;
        // }

        while (m_capturing)
        {
            Sleep(10);
            // DWORD waitResult = WaitForSingleObject(m_CaptureEvent, 1000);
            // std::cout << waitResult << " " << m_CaptureEvent << std::endl;
            // if (waitResult != WAIT_OBJECT_0)
            // {
            //    if (waitResult == WAIT_FAILED)
            //    {
            //       std::cerr << "WaitForSingleObject failed: " << GetLastError() << std::endl;
            //    }
            //    continue;
            // }
            std::cout << "HI" << std::endl;

            UINT32 packetLength = 0;
            HRESULT hr = m_captureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr))
            {
                continue;
            }

            // while (packetLength != 0)
            // {
            //    BYTE *pData = nullptr;
            //    UINT32 numFrames = 0;
            //    DWORD flags;

            //    hr = m_CaptureClient->GetBuffer(&pData, &numFrames, &flags, nullptr, nullptr);
            //    if (SUCCEEDED(hr))
            //    {
            //       std::cout << "DATA:" << " " << static_cast<int>(pData[0]) << std::endl;
            //       // JS
            //       // m_Tsfn.BlockingCall([Data, FramesAvailable](Napi::Env env, Napi::Function jsCallback)
            //       //                     {
            //       //    auto buffer = Napi::Buffer<BYTE>::Copy(env, Data, FramesAvailable);
            //       //    jsCallback.Call({buffer}); });
            //       m_CaptureClient->ReleaseBuffer(numFrames);
            //    }

            //    hr = m_CaptureClient->GetNextPacketSize(&packetLength);
            //    if (FAILED(hr))
            //       break;
            // }

            // m_Tsfn.Release();
        }
    }
};
