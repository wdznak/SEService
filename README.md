# SEService

*SEServise* is a library that allows user to connect to web socket streams of different cryptocurrency exchanges.
Connections can be specified in the file "connections.json". File can be updated while program is running. Call to the "availableConnections()" will return updated list.

Every exchange has its own thread. You can subscribe to multiple channels within one exchange(thread). When web socket receives a message it will pass it to the user by the provided callback function. If a connection is interrupted *SEService* will try to reconnect 4 times and on failure error callback function will be called with appropriate error code.

###### Code example
```C++
// Both "errorQueue" and "messageQueue" should be thread safe
SEConnection connectionService{ [&](int connectionId, std::string message, ErrorCode errorCode) {
  errorQueue.push(connectionId, message, errorCode);
} };
   
int uniqueId = connectionService.connect(exchangeName, connectionId, [](int id, std::string message){
  messageQueue.push(id, message);
});

...
// Later 
connectionService.close(uniqueId, exchangeName);
// Or connectionService.cloaseAll() to close all active connections

```

###### Build (only Windows)
SEService requires *[uWebSockets v0.14](https://github.com/uNetworking/uWebSockets/tree/v0.14)*,  *[RapidJSON ](http://rapidjson.org/)* and *[C++ Requests: Curl for People](https://github.com/whoshuu/cpr)*. The easiest way to provide those libs is to install them via *[VCPKG](https://github.com/Microsoft/vcpkg)*.
