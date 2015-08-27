avboost
=======

some templates that's very very useful during development.

avboost 是一些在开发 avplayer 的项目过程中编写的极其有用的方便的小模板工具库的集合. 是对 Boost 库的一个补充.

包括但不限于


*    base64 编解码
*    cfunction 封装 boost::funcion 为 C 接口的回调.
*    async\_coro\_queue 协程使用的异步列队.
*    async\_foreach 协程并发的 for\_each 版本
*    async\_dir\_walk 利用协程并发的 for\_each 版本实现的异步文件夹遍历.
*    acceptor\_server 一个简单的 accepter 用于接受TCP连接.
*    avloop 为 asio 添加 idle 回调. 当 io\_service 没事情干的时候调用回调.
*    timedcall 简单的 asio  deadline\_timer 封装. 用于延时执行一个任务.
*    hash 来自 boost.sandbox 的 哈希编码, 支持 SHA1 MD5
*    multihandler 一个回调封装, 用于 指定的回调发生N次后调用真正的回调.
*    json\_create\_escapes\_utf8.hpp 在使用 boost 的 json 编码器的时候, 包含这个头文件, 可以让 boost 的 json 编码器输出未经过转码的utf8文本
*    urlencode.hpp 用于 url 的编码, 将非允许字符使用 % 编码.
*    consolestr.hpp 用于将UTF-8字符串转码为本地控制台的字体编码以输出不乱码. 因 linux 和 windows 的控制台编码不一样而引入
*    avproxy.hpp asio 的helper， 用于一行代码完成 dns 解析+主机连接+代理协商
  

avproxy
=======

asio based async proxy connector

avproxy is used to make async connections with or without proxyes  in one single line of code.

avproxy 用于在一行代码之内异步发起通过代理和不通过代理的TCP链接

avproxy support nested proxy aka proxy behind proxy.

avproxy 支持代理嵌套，能以“代理套代理”的方式发起链接。

avproxy is as simple as one line call to avproxy::async\_proxy\_connect.

avproxy 的使用非常简单，调用　avproxy::async\_proxy\_connect 即可。

async\_proxyconnect 需要的 proxychain 参数可以使用 avproxy::autoproxychain 自动从环境变量构建




