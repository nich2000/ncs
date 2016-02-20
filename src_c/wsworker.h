#ifndef WSWORKER_H
#define WSWORKER_H

#include "sha1.h"
#include "base64.h"

#include "protocol.h"

// http://learn.javascript.ru/websockets#описание-фрейма

/*
* GET /chat HTTP/1.1
* Host: example.com:8000
* Upgrade: websocket
* Connection: Upgrade
* Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
* Sec-WebSocket-Version: 13
*/

/*
* HTTP/1.1 101 Switching Protocols
* Upgrade: websocket
* Connection: Upgrade
* Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
*/

int ws_handle_buffer(pack_buffer buffer);

#endif //WSWORKER_H
