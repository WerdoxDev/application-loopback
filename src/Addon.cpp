#include <napi.h>
#include "LoopbackCaptureWrap.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
   return LoopbackCaptureWrap::Init(env, exports);
}

NODE_API_MODULE(loopback_capture_addon, InitAll)
