#include <string.h>

#include "wsworker.h"
#include "log.h"

#include "sha1.h"
#include "base64.h"

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

int ws_hand_shake(char *request, char *response)
{
  char *tmp_request;

  tmp_request = strtok(request, "\r\n");
  while(tmp_request != NULL)
  {
    if(strstr(tmp_request, "Sec-WebSocket-Key") != NULL)
    {
      // Sec-WebSocket-Key: F1LbMpxlqYsjyna4cuCXlg==
      char token[128];
      char token_value[128];
      memset(token_value, 0, 128);
      sscanf(tmp_request, "%s %s", token, token_value);

      // взять строковое значение из заголовка Sec-WebSocket-Key
      // и объединить со строкой 258EAFA5-E914-47DA-95CA-C5AB0DC85B11
      strcat(token_value, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
      log_add(token_value, LOG_DEBUG);

      // вычислить бинарный хеш SHA-1 полученной строки
      char tmp_sha1[20];
      memset(tmp_sha1, 0, 20);
      sha1(tmp_sha1, token_value, strlen(token_value) * 8);
      log_add(tmp_sha1, LOG_DEBUG);

      // закодировать хеш в Base64
      base64enc(response, tmp_sha1, 20);
      log_add(response, LOG_DEBUG);

      break;
    };
    tmp_request = strtok(NULL, "\r\n");
  }

  return 0;
}

int ws_handle_buffer(pack_buffer buffer)
{
  log_add(buffer, LOG_INFO);

  return 0;
}
