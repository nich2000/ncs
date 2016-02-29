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
#include "socket.h"
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
int ws_server_init (ws_server_t *server);
int ws_server_start(ws_server_t *server, sock_port_t port);
int ws_server_work (ws_server_t *server);
int ws_server_stop (ws_server_t *server);
int ws_server_pause(ws_server_t *server);
//==============================================================================
void *ws_server_worker(void *arg);
//==============================================================================
int ws_accept(void *sender, SOCKET socket);
//==============================================================================
void *ws_recv_worker(void *arg);
void *ws_send_worker(void *arg);
//==============================================================================
int ws_hand_shake(char *request, char *response, int *size);
//==============================================================================
int ws_make_frame(WSFrame_t frame_type, unsigned char* msg, int msg_length, unsigned char* buffer, int buffer_size);
WSFrame_t ws_get_frame(unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length);
//==============================================================================
int         _ws_server_id = 0;
ws_server_t _ws_server;
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
int ws_server_init(ws_server_t *server)
{
  custom_worker_init(&server->custom_server.custom_worker);

  server->custom_server.custom_worker.id   = _ws_server_id++;
  server->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  server->custom_server.custom_worker.mode = SOCK_MODE_WS_SERVER;

  server->custom_server.on_accept          = &ws_accept;

  server->hand_shake                       = SOCK_FALSE;
}
//==============================================================================
int ws_server_start(ws_server_t *server, sock_port_t port)
{
  ws_server_init(server);

  server->custom_server.custom_worker.port  = port;
  server->custom_server.custom_worker.state = SOCK_STATE_START;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  return pthread_create(&server->custom_server.work_thread, &tmp_attr, ws_server_worker, (void*)server);
}
//==============================================================================
int ws_server_work(ws_server_t *server)
{
}
//==============================================================================
int ws_server_stop(ws_server_t *server)
{
  server->custom_server.custom_worker.state = SOCK_STATE_STOP;
}
//==============================================================================
int ws_server_pause(ws_server_t *server)
{
  server->custom_server.custom_worker.state = SOCK_STATE_PAUSE;
}
//==============================================================================
int ws_server_status()
{
  sock_print_custom_worker_info(&_ws_server.custom_server.custom_worker, "ws_server");
}
//==============================================================================
void *ws_server_worker(void *arg)
{
  log_add("BEGIN ws_server_worker", LOG_DEBUG);

  ws_server_t *tmp_server = (ws_server_t*)arg;

  custom_server_start(&tmp_server->custom_server.custom_worker);
  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  log_add("END ws_server_worker", LOG_DEBUG);
}
//==============================================================================
int ws_accept(void *sender, SOCKET socket)
{
  char tmp[1024];
  sprintf(tmp, "ws_accept, socket: %d", socket);
  log_add(tmp, LOG_DEBUG);

  SOCKET *s = malloc(sizeof(SOCKET));
  memcpy(s, &socket, sizeof(SOCKET));

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(NULL, &tmp_attr, ws_recv_worker, (void*)s);
//  pthread_create(NULL, &tmp_attr, ws_send_worker, (void*)s);
}
//==============================================================================
void *ws_recv_worker(void *arg)
{
  SOCKET tmp_sock = *(SOCKET*)arg;
  free(arg);

  char tmp[1024];
  sprintf(tmp, "BEGIN ws_recv_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  char request[2048];
  char response[1024*1024];
  int  size = 0;

  while(1)
  {
    if(sock_recv(tmp_sock, request, &size))
    {
      ws_hand_shake(request, response, &size);

      sock_send(tmp_sock, response, size);

      return NULL;
    }
  }

  sprintf(tmp, "END ws_recv_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  return NULL;
}
//==============================================================================
void *ws_send_worker(void *arg)
{
  SOCKET tmp_sock = *(SOCKET*)arg;
  free(arg);

  char tmp[1024];
  sprintf(tmp, "BEGIN ws_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  while(1)
  {
    sleep(1);
  }

  sprintf(tmp, "END ws_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  return NULL;
}
//==============================================================================
int ws_hand_shake(char *request, char *response, int *size)
{
  char *tmp_request;

  char tmp_accept_key[64];

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

      // вычислить бинарный хеш SHA-1 полученной строки
      char tmp_sha1[20];
      memset(tmp_sha1, 0, 20);
      sha1(tmp_sha1, token_value, strlen(token_value) * 8);

      // закодировать хеш в Base64
      base64enc(tmp_accept_key, tmp_sha1, 20);

      break;
    };
    tmp_request = strtok(NULL, "\r\n");
  }

  strcpy(response, "HTTP/1.1 101 Switching Protocols\r\n");
  strcat(response, "Upgrade: websocket\r\n");
  strcat(response, "Connection: Upgrade\r\n");
  char tmp[128];
  sprintf(tmp, "Sec-WebSocket-Accept: %s\r\n\r\n", tmp_accept_key);
  strcat(response, tmp);

  *size = strlen(response);

  return 0;
}
//==============================================================================
int ws_make_frame(WSFrame_t frame_type, unsigned char* msg, int msg_length, unsigned char* buffer, int buffer_size)
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
WSFrame_t ws_get_frame(unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length)
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
