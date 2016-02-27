//==============================================================================
//==============================================================================
#include <string.h>

#include "wsworker.h"
#include "log.h"
#include "sha1.h"
#include "base64.h"
#include "protocol.h"
#include "utils.h"
#include "socket_utils.h"
//==============================================================================
// http://learn.javascript.ru/websockets#описание-фрейма
//==============================================================================
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
//==============================================================================
int ws_server_init (ws_worker_t *worker);
int ws_server_start(ws_worker_t *worker, sock_port_t port);
int ws_server_work (ws_worker_t *worker);
int ws_server_stop (ws_worker_t *worker);
int ws_server_pause(ws_worker_t *worker);
//==============================================================================
int ws_accept(SOCKET socket);
void *ws_server_worker(void *arg);
//==============================================================================
int ws_hand_shake   (char *request, char *response);
int ws_handle_buffer(pack_buffer buffer);
//==============================================================================
int ws_make_frame(WebSocketFrameType frame_type, unsigned char* msg, int msg_length, unsigned char* buffer, int buffer_size);
WebSocketFrameType ws_get_frame(unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length);
//==============================================================================
int         _ws_server_id = 0;
ws_worker_t _ws_server;
//==============================================================================
int ws_server(sock_state_t state, sock_port_t port)
{
  sock_print_server_header(SOCK_MODE_WS_SERVER, port);

  switch(state)
  {
    case SOCK_STATE_NONE:
    {
      break;
    }
    case SOCK_STATE_START:
    {
      ws_server_start(&_ws_server, port);
      break;
    }
    case SOCK_STATE_STOP:
    {
      ws_server_stop(&_ws_server);
      break;
    }
    case SOCK_STATE_PAUSE:
    {
      ws_server_pause(&_ws_server);
      break;
    }
    default:;
  };

  return SOCK_OK;
}
//==============================================================================
int ws_server_init(ws_worker_t *worker)
{
  custom_worker_init(&worker->custom_server.custom_worker);

  worker->custom_server.custom_worker.id   = _ws_server_id++;
  worker->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  worker->custom_server.custom_worker.mode = SOCK_MODE_WS_SERVER;
  worker->custom_server.on_accept          = &ws_accept;
}
//==============================================================================
int ws_server_start(ws_worker_t *worker, sock_port_t port)
{
  ws_server_init(worker);

  worker->custom_server.custom_worker.port  = port;
  worker->custom_server.custom_worker.state = SOCK_STATE_START;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  return pthread_create(&worker->custom_server.custom_worker.work_thread, &tmp_attr, ws_server_worker, (void*)worker);
}
//==============================================================================
int ws_server_work(ws_worker_t *worker)
{

}
//==============================================================================
int ws_server_stop(ws_worker_t *worker)
{
  worker->custom_server.custom_worker.state = SOCK_STATE_STOP;
}
//==============================================================================
int ws_server_pause(ws_worker_t *worker)
{
  worker->custom_server.custom_worker.state = SOCK_STATE_PAUSE;
}
//==============================================================================
int ws_server_status()
{
  print_custom_worker_info(&_ws_server.custom_server.custom_worker, "ws_server");
}
//==============================================================================
void *ws_server_worker(void *arg)
{
  log_add("BEGIN ws_server_worker", LOG_DEBUG);

  ws_worker_t *tmp_server = (ws_worker_t*)arg;

  custom_server_start(&tmp_server->custom_server.custom_worker);
//  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  log_add("END ws_server_worker", LOG_DEBUG);
}
//==============================================================================
int ws_accept(SOCKET socket)
{

}
//==============================================================================
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
//==============================================================================
int ws_handle_buffer(pack_buffer buffer)
{
//  if(worker->handshake)
//  {
//    char tmp[1024];
//    int tmp_size;
//    ws_get_frame(buffer, strlen(buffer), tmp, 1024, &tmp_size);
//    log_add(tmp, LOG_INFO);
//  }
//  else
//  {
//    char *tmp_message = (char*)malloc(10000);
//    ws_hand_shake((char*)buffer, tmp_message);

//    worker->is_locked++;

//    worker->out_message = (char*)malloc(10000);

//    strcpy(worker->out_message, "HTTP/1.1 101 Switching Protocols\r\n");
//    strcat(worker->out_message, "Upgrade: websocket\r\n");
//    strcat(worker->out_message, "Connection: Upgrade\r\n");
//    char tmp[10240];
//    sprintf(tmp, "Sec-WebSocket-Accept: %s\r\n\r\n", tmp_message);
//    strcat(worker->out_message, tmp);

//    worker->out_message[strlen(worker->out_message)+1] = '\0';

//    worker->is_locked--;

//    // TODO утечка
////        free(tmp_message);
//  };

  return 0;
}
//==============================================================================
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
//==============================================================================
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
//==============================================================================
