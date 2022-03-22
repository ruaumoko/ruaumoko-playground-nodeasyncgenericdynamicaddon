var interop = require('bindings')('asyncgenericdynamicaddon');

//const readline = require('readline').createInterface({
//  input: process.stdin,
//  output: process.stdout
//})

//console.log("Pause to attach to process by asking a question");
//readline.question(`What's your name?`, name => {
//  console.log(`Hi ${name}!`)
//  readline.close()

//  var myFunc = interop.func({
//    assembly: "C:/My Files/Playground/NodeAsyncGenericDynamicAddon/Library/bin/x64/Debug/net5.0/library.dll",
//    typeName: "MyLibrary.MyClass",
//    methodName: "MyLongRunningMethod"
//  });

//  console.log("Calling function async");
//  myFunc.runAsync({ id: 1, numbers: [1.0, 2.0, 3.0] }, (error, result) => {
//    console.log("Async result ->");
//    console.log(error);
//    console.log(result);
//  });

//  //console.log("Calling function sync");
//  //var mySyncResult = myFunc.run({ id: 1, numbers: [1.0, 2.0, 3.0] });
//  //console.log("Sync result ->");
//  //console.log(mySyncResult);

//  console.log("Waiting ...")
//  process.stdin.resume();
//})


var myFunc = interop.func({
  assembly: "C:/My Files/Playground/NodeAsyncGenericDynamicAddon/Library/bin/x64/Debug/net5.0/library1.dll",
  typeName: "MyLibrary.MyClass",
  methodName: "MyLongRunningMethod"
});

console.log("Calling function async");
myFunc.runAsync({ id: 1, numbers: [1.0, 2.0, 3.0] }, (error, result) => {
  console.log("Async result ->");
  console.log(error);
  console.log(result);
});

console.log("Waiting ...")
process.stdin.resume();
