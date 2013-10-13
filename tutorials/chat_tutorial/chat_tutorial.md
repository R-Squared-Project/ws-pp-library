Chat Tutorial
=============

Goals of this tutorial:
- Impliment a realtime websocket chat server


Server
- Nicknames
- Channels
- Subprotocol
- Origin restrictions
- HTTP statistics page






Hi all,

I am working on some tutorials for WebSocket++ and was wondering if anyone here has specific features or concepts that they feel would be particularly useful to have covered? Anything that you wish there was a tutorial for now or wished there had been when you were first setting up your WebSocket++ application?

Best,

Peter







WebSocket++ is a header only C++ library that implements RFC6455 The WebSocket
Protocol. It allows integrating WebSocket client and server functionality into
C++ programs. It uses interchangeable network transport modules including one
based on C++ iostreams and one based on Boost Asio.

Major Features
==============
* Full support for RFC6455
* Partial support for Hixie 76 / Hybi 00, 07-17 draft specs (server only)
* Message/event based interface
* Supports secure WebSockets (TLS), IPv6, and explicit proxies.
* Flexible dependency management (C++11 Standard Library or Boost)
* Interchangeable network transport modules (iostream and Boost Asio)
* Portable/cross platform (Posix/Windows, 32/64bit, Intel/ARM/PPC)
* Thread-safe

Get Involved
============

[![Build Status](https://travis-ci.org/zaphoyd/websocketpp.png)](https://travis-ci.org/zaphoyd/websocketpp)

**Project Website**
http://www.zaphoyd.com/websocketpp/

**User Manual**
http://www.zaphoyd.com/websocketpp/manual/

**GitHub Repository**
https://github.com/zaphoyd/websocketpp/

**Announcements Mailing List**
http://groups.google.com/group/websocketpp-announcements/

**IRC Channel**
 #websocketpp (freenode)

**Discussion / Development / Support Mailing List / Forum**
http://groups.google.com/group/websocketpp/

Author
======
Peter Thorson - websocketpp@zaphoyd.com