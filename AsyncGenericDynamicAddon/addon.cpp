// addon.cpp

#include <napi.h>

#include "addon.h"
#include "myobject.h"

namespace MyAddon
{

#pragma managed

  //

#pragma unmanaged


  Napi::Object CreateFuncObject(const Napi::CallbackInfo& info) {

    Napi::Env myEnv = info.Env();

    Napi::Object myObj = MyObject::NewInstance(myEnv, info[0].As<Napi::Object>());

    return myObj;
  }

  Napi::Object InitAll(Napi::Env env, Napi::Object exports) {

    MyObject::Init(env, exports); // Must init object

    exports.Set(Napi::String::New(env, "func"), Napi::Function::New(env, CreateFuncObject));

    return exports;

  }

  NODE_API_MODULE(hello, InitAll)

}