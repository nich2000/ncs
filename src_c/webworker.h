#ifndef WEBWORKER_H
#define WEBWORKER_H

#include "protocol.h"

/*
* GET /index.html HTTP/1.0 \r\n
* Host: www.paulgriffiths.net \r\n
* User-Agent: Lynx/2.8.1rel.2 libwww-FM/2.14 \r\n
* Accept-Encoding: gzip, compress \r\n
* Accept-Language: en \r\n\r\n
*/

/*
* HTTP/1.0 200 OK \r\n
* Server: PGWebServ v0.1 \r\n
* Content-Type: text/html \r\n\r\n
* <DATA> \r\n
*/

int web_handle_buffer(pack_buffer buffer);

#endif //WEBWORKER_H
