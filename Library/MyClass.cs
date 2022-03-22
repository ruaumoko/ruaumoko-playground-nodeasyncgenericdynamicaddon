// myclass.cs

using System.Threading;

namespace MyLibrary
{
  public class MyClass
  {

    public static object MyLongRunningMethod()
    {
        Thread.Sleep(10000);
        return new { number = 1000 };
    }

    public static object MyLongRunningMethod(dynamic payload)

    {
        Thread.Sleep(10000);
        return payload;   
    }

  }
}