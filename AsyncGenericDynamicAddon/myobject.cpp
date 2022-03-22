// myobject.cpp

#include "myobject.h"

#include <napi.h>

#include <msclr/marshal_cppstd.h> // Required for marshalling strings between managed and unmanaged

namespace MyAddon
{

  using namespace System;
  using namespace System::Dynamic;
  using namespace System::Reflection;
  using namespace System::Text::Json;

#pragma managed

  static Object^ Deserialize(std::string jsonString)
  {
    // Convert std::string to System::String
    String^ jsonStringAsSystemString = gcnew String(jsonString.c_str());

    // Deserialize object and return
    ExpandoObject^ obj = JsonSerializer::Deserialize<ExpandoObject^>(jsonStringAsSystemString, gcnew JsonSerializerOptions());
    return obj;
  }

  static std::string Serialize(Object^ obj)
  {
    // Serialize object to JSON
    String^ jsonStringIn = JsonSerializer::Serialize<Object^>(obj, gcnew System::Text::Json::JsonSerializerOptions());

    // Convert System:String to std:string
    return msclr::interop::marshal_as<std::string>(jsonStringIn);
  }

  static std::string InvokeMethod(std::string assemblyPath, std::string typeName, std::string methodName) {  // Must be std:string arguments to cross managed/unmanaged

    // Reference to assembly
    System::String^ myAssemblyPathAsSystemString = gcnew String(assemblyPath.c_str());
    Assembly^ myAssembly = Assembly::LoadFrom(myAssemblyPathAsSystemString);

    // Referenc to type
    System::String^ myTypeNameAsSystemString = gcnew String(typeName.c_str());
    Type^ myType = myAssembly->GetType(myTypeNameAsSystemString);

    // Reference to function
    System::String^ myMethodNameNameAsSystemString = gcnew String(methodName.c_str());

    //Invoke function
    array<System::Object^>^ myArgs = gcnew array<System::Object^>(0);
    Object^ myObj = myType->InvokeMember(myMethodNameNameAsSystemString, BindingFlags::InvokeMethod, nullptr, myType, myArgs);

    return Serialize(myObj);
  }



  static std::string InvokeMethod(std::string assemblyPath, std::string typeName, std::string methodName, std::string payload) {  // Must be std:string argumnets to cross managed/unmanaged

    try {
      // Reference to assembly
      System::String^ myAssemblyPathAsSystemString = gcnew String(assemblyPath.c_str());
      Assembly^ myAssembly = Assembly::LoadFrom(myAssemblyPathAsSystemString);

      // Referenc to type
      System::String^ myTypeNameAsSystemString = gcnew String(typeName.c_str());
      Type^ myType = myAssembly->GetType(myTypeNameAsSystemString);

      // Reference to function
      System::String^ myMethodNameNameAsSystemString = gcnew String(methodName.c_str());

      //Invoke function
      array<System::Object^>^ myArgs = gcnew array<System::Object^>(1);
      myArgs[0] = Deserialize(payload);
      Object^ myObj = myType->InvokeMember(myMethodNameNameAsSystemString, BindingFlags::InvokeMethod, nullptr, myType, myArgs);

      return Serialize(myObj);
    }
    catch (Exception^ ex) {
      const std::string myExMsg = msclr::interop::marshal_as<std::string>(ex->Message);
      throw InvokeException(myExMsg);
    }
  }

#pragma unmanaged

  class Worker : public Napi::AsyncWorker {
  public:
    Worker(std::string assemblyPath, std::string typeName, std::string methodName,
      std::string payload, Napi::Function& callback)
      : assemblyPath(assemblyPath), typeName(typeName), methodName(methodName), payload(payload), Napi::AsyncWorker(callback), myMethod() {}
    ~Worker() {}

    // Executed inside the worker-thread.
    // It is not safe to access JS engine data structure
    // here, so everything we need for input and output
    // should go on `this`.
    // It's absolutely essential that the Execute method makes NO Node-API calls. This means that the Execute method has
    // no access to any input values passed by the JavaScript code.
    void Execute() {
      try {
        myMethod = InvokeMethod(this->assemblyPath, this->typeName, this->methodName, this->payload);
      }
      catch (InvokeException ex) {
        SetError(ex.what());
      }
    }

    // Executed when the async work is complete
    // this function will be run inside the main event loop
    // so it is safe to use JS engine data again
    void OnOK() {

      const std::string resultStringified = myMethod;

      // Convert std::string to Napi::String
      Napi::Env myEnv = this->Env();
      Napi::String resultStringifiedAsNapiString = Napi::String::New(myEnv, resultStringified);

      // Get references to global function
      Napi::Object json = myEnv.Global().Get("JSON").As<Napi::Object>();
      Napi::Function parse = json.Get("parse").As<Napi::Function>();

      // Parse string and return object
      Napi::Value myError = myEnv.Undefined();
      Napi::Object myResult = parse.Call(json, { resultStringifiedAsNapiString }).As<Napi::Object>();

      // Return object through callback
      Callback().Call({ myError, myResult });
    }

    void OnError(const Napi::Error& error) {

      HandleError(error.Message());

    };

    void HandleError(std::string msg) {

      Napi::Env myEnv = this->Env();

      Napi::Object myError = Napi::Object::New(myEnv);
      myError.Set(Napi::String::New(myEnv, "msg"), Napi::String::New(myEnv, msg));
      Napi::Value myResult = myEnv.Undefined();

      // Return object through callback
      Callback().Call({ myError, myResult });
    }

  private:
    std::string assemblyPath;
    std::string typeName;
    std::string methodName;
    std::string payload;
    std::string myMethod;
  };

  Napi::Object MyObject::Init(Napi::Env env, Napi::Object exports) {

    Napi::Function func = DefineClass(
      env, "MyObject", { InstanceMethod("run", &MyObject::Run),
        InstanceMethod("runAsync", &MyObject::RunAsync)
      });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);

    exports.Set("MyObject", func);
    return exports;
  }

  MyObject::MyObject(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MyObject>(info) {

    Napi::Object myConfigObj = info[0].As<Napi::Object>();

    this->assemblyPath = myConfigObj.Get("assembly").As<Napi::String>().Utf8Value();
    this->typeName = myConfigObj.Get("typeName").As<Napi::String>().Utf8Value();
    this->methodName = myConfigObj.Get("methodName").As<Napi::String>().Utf8Value();

  };

  Napi::Object MyObject::NewInstance(Napi::Env env, Napi::Value arg) {

    Napi::EscapableHandleScope scope(env);
    Napi::Object obj = env.GetInstanceData<Napi::FunctionReference>()->New({ arg });
    return scope.Escape(napi_value(obj)).ToObject();

  }

  Napi::Value MyObject::Run(const Napi::CallbackInfo& info) {

    Napi::Env env = info.Env();

    std::string myAssemblyPath = this->assemblyPath;
    std::string myTypeName = this->typeName;
    std::string myMethodName = this->methodName;

    Napi::Object myPayload = info[0].As<Napi::Object>();

    // Get references to global function
    Napi::Object json = env.Global().Get("JSON").As<Napi::Object>();
    Napi::Function parse = json.Get("parse").As<Napi::Function>();
    Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

    // Parse payload
    Napi::String myStringifiedPayload = stringify.Call(json, { myPayload }).As<Napi::String>();

    // Convert Napi::String to std:string
    std::string myStringifiedPayloadAsStdString = myStringifiedPayload.Utf8Value();

    std::string myResult = InvokeMethod(myAssemblyPath, myTypeName, myMethodName, myStringifiedPayloadAsStdString);

    return Napi::String::New(env, myResult);
  }

  Napi::Value MyObject::RunAsync(const Napi::CallbackInfo& info) {

    Napi::Env env = info.Env();

    std::string myAssemblyPath = this->assemblyPath;
    std::string myTypeName = this->typeName;
    std::string myMethodName = this->methodName;

    Napi::Object myPayload = info[0].As<Napi::Object>();
    Napi::Function myCallback = info[1].As<Napi::Function>();

    // Get references to global function
    Napi::Object json = env.Global().Get("JSON").As<Napi::Object>();
    Napi::Function parse = json.Get("parse").As<Napi::Function>();
    Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

    // Parse payload
    Napi::String myStringifiedPayload = stringify.Call(json, { myPayload }).As<Napi::String>();

    // Convert Napi::String to std:string
    std::string myStringifiedPayloadAsStdString = myStringifiedPayload.Utf8Value();

    // Create woker and queue
    Worker* myWorker = new Worker(myAssemblyPath, myTypeName, myMethodName, myStringifiedPayloadAsStdString, myCallback);
    myWorker->Queue();

    return info.Env().Undefined();
  }

}