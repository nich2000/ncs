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
//      log_add(token_value, LOG_DEBUG);

      // вычислить бинарный хеш SHA-1 полученной строки
      char tmp_sha1[20];
      memset(tmp_sha1, 0, 20);
      sha1(tmp_sha1, token_value, strlen(token_value) * 8);
//      log_add(tmp_sha1, LOG_DEBUG);

      // закодировать хеш в Base64
      base64enc(response, tmp_sha1, 20);
//      log_add(response, LOG_DEBUG);

      break;
    };
    tmp_request = strtok(NULL, "\r\n");
  }

  return 0;
}

int ws_handle_buffer(pack_buffer buffer)
{
  char tmp[1024];
  int tmp_size;

//  bytes_to_hex(buffer, strlen(buffer), tmp);
//  log_add(tmp, LOG_INFO);

  ws_get_frame(buffer, strlen(buffer), tmp, 1024, &tmp_size);
  log_add(tmp, LOG_INFO);

  return 0;
}

int ws_make_frame(WebSocketFrameType frame_type, unsigned char* msg, int msg_length, unsigned char* buffer, int buffer_size)
{
  int pos = 0;
  int size = msg_length;
  buffer[pos++] = (unsigned char)frame_type; // text frame

  if(size <= 125)
  {
    buffer[pos++] = size;
  }
  else if(size <= 65535)
  {
    buffer[pos++] = 126; //16 bit length follows

    buffer[pos++] = (size >> 8) & 0xFF; // leftmost first
    buffer[pos++] = size & 0xFF;
  }
  else
  { // >2^16-1 (65535)
    buffer[pos++] = 127; //64 bit length follows

    // write 8 bytes length (significant first)

    // since msg_length is int it can be no longer than 4 bytes = 2^32-1
    // padd zeroes for the first 4 bytes
    for(int i=3; i>=0; i--)
    {
      buffer[pos++] = 0;
    }
    // write the actual 32bit msg_length in the next 4 bytes
    for(int i=3; i>=0; i--)
    {
      buffer[pos++] = ((size >> 8*i) & 0xFF);
    }
  }
  memcpy((void*)(buffer+pos), msg, size);
  return (size+pos);
}

WebSocketFrameType ws_get_frame(unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length)
{
  //printf("getTextFrame()\n");
  if(in_length < 3)
    return INCOMPLETE_FRAME;

  unsigned char msg_opcode = (in_buffer[0]     ) & 0x0F;
  unsigned char msg_fin    = (in_buffer[0] >> 7) & 0x01;
  unsigned char msg_masked = (in_buffer[1] >> 7) & 0x01;

  // *** message decoding

  int payload_length = 0;
  int pos = 2;
  int length_field = in_buffer[1] & (~0x80);
  unsigned int mask = 0;

  //printf("IN:"); for(int i=0; i<20; i++) printf("%02x ",buffer[i]); printf("\n");

  if(length_field <= 125)
  {
    payload_length = length_field;
  }
  else if(length_field == 126)
  { //msglen is 16bit!
    payload_length = in_buffer[2] + (in_buffer[3]<<8);
    pos += 2;
  }
  else if(length_field == 127)
  { //msglen is 64bit!
    payload_length = in_buffer[2] + (in_buffer[3]<<8);
    pos += 8;
  }
  //printf("PAYLOAD_LEN: %08x\n", payload_length);
  if(in_length < payload_length+pos)
  {
    return INCOMPLETE_FRAME;
  }

  if(msg_masked)
  {
    mask = *((unsigned int*)(in_buffer+pos));
    //printf("MASK: %08x\n", mask);
    pos += 4;

    // unmask data:
    unsigned char* c = in_buffer+pos;
    for(int i=0; i<payload_length; i++)
    {
      c[i] = c[i] ^ ((unsigned char*)(&mask))[i%4];
    }
  }

  if(payload_length > out_size)
  {
    //TODO: if output buffer is too small -- ERROR or resize(free and allocate bigger one) the buffer ?
  }

  memcpy((void*)out_buffer, (void*)(in_buffer+pos), payload_length);
  out_buffer[payload_length] = 0;
  *out_length = payload_length+1;

  //printf("TEXT: %s\n", out_buffer);

  if(msg_opcode == 0x0)
    return (msg_fin)?TEXT_FRAME:INCOMPLETE_TEXT_FRAME; // continuation frame ?
  if(msg_opcode == 0x1)
    return (msg_fin)?TEXT_FRAME:INCOMPLETE_TEXT_FRAME;
  if(msg_opcode == 0x2)
    return (msg_fin)?BINARY_FRAME:INCOMPLETE_BINARY_FRAME;
  if(msg_opcode == 0x9)
    return PING_FRAME;
  if(msg_opcode == 0xA)
    return PONG_FRAME;

  return ERROR_FRAME;
}
