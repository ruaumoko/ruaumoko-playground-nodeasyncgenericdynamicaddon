// myobject.h

#include <napi.h>

namespace MyAddon
{

  class MyObject : public Napi::ObjectWrap<MyObject> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::Object NewInstance(Napi::Env env, Napi::Value arg);
    MyObject(const Napi::CallbackInfo& info);

  private:
    Napi::Value Run(const Napi::CallbackInfo& info);
    Napi::Value RunAsync(const Napi::CallbackInfo& info);

    std::string assemblyPath;
    std::string typeName;
    std::string methodName;

  };

  class InvokeException : public std::exception {
  public:
    InvokeException(std::string const& message) : msg_(message) { }
    virtual char const* what() const noexcept { return msg_.c_str(); }

  private:
    std::string msg_;
  };

}
