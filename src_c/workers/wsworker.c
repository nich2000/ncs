//==============================================================================
//==============================================================================
#include <stdarg.h>
#include <string.h>

#include "wsworker.h"
#include "cmdworker.h"
#include "ncs_log.h"
#include "sha1.h"
#include "base64.h"
#include "protocol_types.h"
#include "protocol.h"
#include "utils.h"
#include "socket_utils.h"
#include "socket.h"
#include "jansson.h"
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
int ws_server_resume(ws_server_t *server);
//==============================================================================
void *ws_server_worker(void *arg);
//==============================================================================
custom_remote_client_t *_ws_server_remote_clients_next(ws_server_t *ws_server);
//==============================================================================
void *ws_recv_worker(void *arg);
void *ws_send_worker(void *arg);
//==============================================================================
int ws_accept(void *sender, SOCKET socket, sock_host_t host);
int ws_new_data(void *sender, void *data);
int ws_disconnect(void *sender);
int ws_error(void *sender, error_t *error);
int ws_recv(void *sender, unsigned char *buffer, int size);
int ws_send(void *sender);
//==============================================================================
int packet_to_json(pack_packet_t *packet, pack_buffer_t buffer, pack_size_t *size);
int json_to_packet(pack_packet_t *packet, pack_buffer_t buffer, pack_size_t *size);
//==============================================================================
int ws_hand_shake(char *request, char *response, int *size);
//==============================================================================
int       ws_set_frame(WSFrame_t frame_type, unsigned char* msg, int msg_length, unsigned char* buffer, int buffer_size);
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
    case STATE_NONE:
    {
      break;
    }
    case STATE_START:
    {
      ws_server_start(&_ws_server, port);
      break;
    }
    case STATE_STOP:
    {
      ws_server_stop(&_ws_server);
      break;
    }
    case STATE_PAUSE:
    {
      ws_server_pause(&_ws_server);
      break;
    }
    case STATE_RESUME:
    {
      ws_server_resume(&_ws_server);
      break;
    }
    default:;
  }

  return ERROR_NONE;
}
//==============================================================================
int ws_server_init(ws_server_t *server)
{
  custom_server_init(&server->custom_server);

  custom_remote_clients_list_init(&server->custom_remote_clients_list);

//  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
//  {
//    custom_remote_client_t *tmp_client = &server->custom_remote_clients_list.items[i];
//    tmp_client->protocol.on_new_in_data  = ws_new_data;
//    tmp_client->protocol.on_new_out_data = ws_new_data;
//  }

  server->custom_server.custom_worker.id   = _ws_server_id++;
  server->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  server->custom_server.custom_worker.mode = SOCK_MODE_WS_SERVER;
  server->custom_server.on_accept          = &ws_accept;

  return ERROR_NONE;
}
//==============================================================================
int ws_server_start(ws_server_t *server, sock_port_t port)
{
  ws_server_init(server);

  server->custom_server.custom_worker.port  = port;
  server->custom_server.custom_worker.state = STATE_STARTING;
  if(server->custom_server.custom_worker.on_state != NULL)
    server->custom_server.custom_worker.on_state(server, STATE_STARTING);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&server->custom_server.work_thread, &tmp_attr, ws_server_worker, (void*)server);

  return ERROR_NONE;
}
//==============================================================================
int ws_server_work(ws_server_t *server)
{
  return ERROR_NONE;
}
//==============================================================================
int ws_server_stop(ws_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_STOPPING;
  if(server->custom_server.custom_worker.on_state != NULL)
    server->custom_server.custom_worker.on_state(server, STATE_STOPPING);

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(server->custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
      server->custom_remote_clients_list.items[i].custom_worker.state = STATE_STOPPING;

  return ERROR_NONE;
}
//==============================================================================
int ws_server_pause(ws_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_PAUSING;
  if(server->custom_server.custom_worker.on_state != NULL)
    server->custom_server.custom_worker.on_state(server, STATE_PAUSING);

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(server->custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
      server->custom_remote_clients_list.items[i].custom_worker.state = STATE_PAUSING;

  return ERROR_NONE;
}
//==============================================================================
int ws_server_resume(ws_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_RESUMING;
  if(server->custom_server.custom_worker.on_state != NULL)
    server->custom_server.custom_worker.on_state(server, STATE_RESUMING);

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(server->custom_remote_clients_list.items[i].custom_worker.state == STATE_PAUSE)
      server->custom_remote_clients_list.items[i].custom_worker.state = STATE_RESUMING;

  return ERROR_NONE;
}
//==============================================================================
int ws_server_status()
{
  clr_scr();

  print_custom_worker_info(&_ws_server.custom_server.custom_worker , "ws_server");

  print_custom_remote_clients_list_info(&_ws_server.custom_remote_clients_list, "ws_server");

  return ERROR_NONE;
}
//==============================================================================
void *ws_server_worker(void *arg)
{
  log_add("[BEGIN] ws_server_worker", LOG_DEBUG);

  ws_server_t *tmp_server = (ws_server_t*)arg;

  custom_server_start(&tmp_server->custom_server.custom_worker);
  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  log_add("[END] ws_server_worker", LOG_DEBUG);

  return NULL;
}
//==============================================================================
custom_remote_client_t *_ws_server_remote_clients_next(ws_server_t *ws_server)
{
  custom_remote_client_t *tmp_client = _custom_server_remote_clients_next(&ws_server->custom_remote_clients_list);

  if(tmp_client != NULL)
  {
    custom_remote_client_init(tmp_client);

    tmp_client->protocol.on_new_in_data  = ws_new_data;

    tmp_client->custom_worker.id    = ws_server->custom_remote_clients_list.next_id++;
    tmp_client->custom_worker.type  = SOCK_TYPE_REMOTE_CLIENT;
    tmp_client->custom_worker.mode  = ws_server->custom_server.custom_worker.mode;
    tmp_client->custom_worker.port  = ws_server->custom_server.custom_worker.port;

    tmp_client->on_disconnect       = ws_disconnect;
    tmp_client->on_error            = ws_error;
    tmp_client->on_recv             = ws_recv;
    tmp_client->on_send             = ws_send;
  }

  return tmp_client;
}
//==============================================================================
int ws_accept(void *sender, SOCKET socket, sock_host_t host)
{
  custom_remote_client_t *tmp_client = _ws_server_remote_clients_next(&_ws_server);

  if(tmp_client == 0)
  {
    log_add_fmt(LOG_ERROR_CRITICAL,
            "ws_accept, no available clients, socket: %d, host: %s",
            socket, host);
    return ERROR_NORMAL;
  }

  tmp_client->custom_worker.state = STATE_STARTING;
  if(tmp_client->custom_worker.on_state != NULL)
    tmp_client->custom_worker.on_state(tmp_client, STATE_STARTING);

  memcpy(&tmp_client->custom_worker.sock, &socket, sizeof(SOCKET));
  memcpy(tmp_client->custom_worker.host, host,   SOCK_HOST_SIZE);

  log_add_fmt(LOG_DEBUG,
              "ws_accept, socket: %d, host: %s, port: %d",
              tmp_client->custom_worker.sock, tmp_client->custom_worker.host, tmp_client->custom_worker.port);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&tmp_client->recv_thread, &tmp_attr, ws_recv_worker, (void*)tmp_client);
  pthread_create(&tmp_client->send_thread, &tmp_attr, ws_send_worker, (void*)tmp_client);

  return ERROR_NONE;
}
//==============================================================================
int ws_server_send_clients()
{
  if(_ws_server.custom_server.custom_worker.state != STATE_START)
    return ERROR_NORMAL;

  pack_packet_t tmp_packet;

  cmd_server_list(&tmp_packet);

  ws_server_send_pack(&tmp_packet);

  return ERROR_NONE;
}
//==============================================================================
void *ws_recv_worker(void *arg)
{
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  log_add_fmt(LOG_DEBUG, "[BEGIN] ws_recv_worker, socket: %d", tmp_sock);

  tmp_client->custom_worker.state = STATE_START;
  if(tmp_client->custom_worker.on_state != NULL)
    tmp_client->custom_worker.on_state(tmp_client, STATE_START);

  char *request  = (char*)malloc(2048);
  char *response = (char*)malloc(1024*1024);
  int  size      = 0;

  while(tmp_client->custom_worker.state == STATE_START)
  {
    if(sock_recv(tmp_sock, request, &size) == ERROR_NONE)
    {
      if(tmp_client->hand_shake == FALSE)
      {
        ws_hand_shake(request, response, &size);

        if(sock_send(tmp_sock, response, size) == ERROR_NONE)
        {
          tmp_client->hand_shake = TRUE;
          log_add_fmt(LOG_DEBUG, "handshake success, socket: %d", tmp_sock);

          ws_server_send_clients();
        }
      }
      else
      {
        int tmp_size;
        unsigned char tmp_buffer[1024];
        ws_get_frame((unsigned char*)request, strlen(request), tmp_buffer, 1024, &tmp_size);
        log_add_fmt(LOG_INFO, "ws_recv_worker, %s", tmp_buffer);

        cmd_derver_activate_all(STATE_STOP);
        cmd_server_activate(atoi((char*)tmp_buffer), STATE_START);
      }
    }

    usleep(1000);
  }

  free(request);
  free(response);

  tmp_client->custom_worker.state = STATE_STOP;
  if(tmp_client->custom_worker.on_state != NULL)
    tmp_client->custom_worker.on_state(tmp_client, STATE_STOP);

  log_add_fmt(LOG_DEBUG, "[END] ws_recv_worker, socket: %d", tmp_sock);

  return NULL;
}
//==============================================================================
void *ws_send_worker(void *arg)
{
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  log_add_fmt(LOG_DEBUG, "[BEGIN] ws_send_worker, socket: %d", tmp_sock);

  tmp_client->custom_worker.state = STATE_START;
  if(tmp_client->custom_worker.on_state != NULL)
    tmp_client->custom_worker.on_state(tmp_client, STATE_START);

  ws_server_send_clients();

  int tmp_errors = 0;

  while(tmp_client->custom_worker.state == STATE_START)
  {
    if(tmp_client->hand_shake == TRUE)
    {
      if(!tmp_client->custom_worker.is_locked)
        if((tmp_client->out_message != NULL) && (tmp_client->out_message_size != 0))
        {
          if(sock_send(tmp_sock, tmp_client->out_message, tmp_client->out_message_size) >= ERROR_NORMAL)
            tmp_errors++;

          free(tmp_client->out_message);
          tmp_client->out_message = NULL;
          tmp_client->out_message_size = 0;

          if((tmp_errors > SOCK_ERRORS_COUNT) || (tmp_client->custom_worker.state == STATE_STOP))
            break;
        }
    }

    usleep(1000);
  }

  tmp_client->custom_worker.state = STATE_STOP;
  if(tmp_client->custom_worker.on_state != NULL)
    tmp_client->custom_worker.on_state(tmp_client, STATE_STOP);

  log_add_fmt(LOG_DEBUG, "[END] ws_send_worker, socket: %d", tmp_sock);

  return NULL;
}
//==============================================================================
int ws_new_data(void *sender, void *data)
{
  return ERROR_NONE;
}
//==============================================================================
int ws_disconnect(void *sender)
{
  log_add("ws_disconnect, disconnected from server", LOG_INFO);

  return ERROR_NONE;
}
//==============================================================================
int ws_error(void *sender, error_t *error)
{
  log_add_fmt(LOG_INFO, "ws_error, message: %s", error->message);

  return ERROR_NONE;
}
//==============================================================================
int ws_recv(void *sender, unsigned char *buffer, int size)
{
  return ERROR_NONE;
}
//==============================================================================
int ws_send(void *sender)
{
  return ERROR_NONE;
}
//==============================================================================
/*
  [
    ["key", "value"],
    ["key", "value"],
    ["key", "value"]
  ]
*/
//==============================================================================
//https://jansson.readthedocs.org/en/2.7/apiref.html
int packet_to_json(pack_packet_t *packet, pack_buffer_t buffer, pack_size_t *size)
{
  json_t *tmp_words = json_array();

  for(int i = 0; i < packet->words_count; i++)
  {
    pack_key_t tmp_key;
    strcpy((char*)tmp_key, (char*)packet->words[i].key);

    pack_value_t tmp_value;
    pack_word_as_string(&packet->words[i], tmp_value);

    json_t *tmp_word = json_array();

    json_t *tmp_json_key   = json_string((char*)tmp_key);
    json_t *tmp_json_value = json_string((char*)tmp_value);

    json_array_append_new(tmp_word, tmp_json_key);
    json_array_append_new(tmp_word, tmp_json_value);

    json_array_append_new(tmp_words, tmp_word);
  }

  char *tmp_json = json_dumps(tmp_words, JSON_ENCODE_ANY);
  strcpy((char*)buffer, tmp_json);
  *size = strlen((char*)buffer);
  free(tmp_json);

  size_t tmp_index_word;
  json_t *tmp_word;
  json_array_foreach(tmp_words, tmp_index_word, tmp_word)
  {
//    size_t tmp_index;
//    json_t *tmp_value;
//    json_array_foreach(tmp_word, tmp_index, tmp_value)
//    {
//      json_decref(tmp_value);
//    }
    json_array_clear(tmp_word);
  }
  json_array_clear(tmp_words);
  json_decref(tmp_words);

  return ERROR_NONE;
}
//==============================================================================
int json_to_packet(pack_packet_t *packet, pack_buffer_t buffer, pack_size_t *size)
{
  return ERROR_NONE;
}
//==============================================================================
int ws_server_send_pack(pack_packet_t *pack)
{
  if(_ws_server.custom_server.custom_worker.state == STATE_START)
  {
    for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    {
      custom_remote_client_t *tmp_client = &_ws_server.custom_remote_clients_list.items[i];

      if(tmp_client->custom_worker.state == STATE_START)
      {
        tmp_client->custom_worker.is_locked = TRUE;
        if(tmp_client->custom_worker.on_lock != 0)
          tmp_client->custom_worker.on_lock(tmp_client, TRUE);

        pack_buffer_t json_buffer;
        pack_size_t   json_size = 0;
        packet_to_json(pack, json_buffer, &json_size);
//        log_add_fmt(LOG_DEBUG, "json:\n%s", json_buffer);

        pack_buffer_t tmp_buffer;
        pack_size_t   tmp_size = 0;
        tmp_size = ws_set_frame(TEXT_FRAME, json_buffer, json_size, tmp_buffer, PACK_BUFFER_SIZE);

        tmp_client->out_message_size = tmp_size;
        tmp_client->out_message = (char*)malloc(tmp_size);
        memcpy(tmp_client->out_message, tmp_buffer, tmp_client->out_message_size);

        tmp_client->custom_worker.is_locked = FALSE;
        if(tmp_client->custom_worker.on_lock != 0)
          tmp_client->custom_worker.on_lock(tmp_client, FALSE);
      }
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int ws_server_send_cmd(int argc, ...)
{
  if(_ws_server.custom_server.custom_worker.state == STATE_START)
  {
    for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    {
      custom_remote_client_t *tmp_client = &_ws_server.custom_remote_clients_list.items[i];

      if(tmp_client->custom_worker.state == STATE_START)
      {
        tmp_client->custom_worker.is_locked = TRUE;
        if(tmp_client->custom_worker.on_lock != 0)
          tmp_client->custom_worker.on_lock(tmp_client, TRUE);

        pack_packet_t tmp_pack;
        pack_init(&tmp_pack);

        va_list tmp_params;
        va_start(tmp_params, argc);
        unsigned char *tmp_cmd = va_arg(tmp_params, unsigned char*);
        pack_add_as_string(&tmp_pack, (unsigned char*)PACK_CMD_KEY, tmp_cmd);

        for(int i = 1; i < argc; i++)
        {
          unsigned char *tmp_param = va_arg(tmp_params, unsigned char*);
          pack_add_as_string(&tmp_pack, (unsigned char*)PACK_PARAM_KEY, tmp_param);
        }
        va_end(tmp_params);

        pack_buffer_t json_buffer;
        pack_size_t   json_size = 0;
        packet_to_json(&tmp_pack, json_buffer, &json_size);

        pack_buffer_t tmp_buffer;
        pack_size_t   tmp_size = 0;
        tmp_size = ws_set_frame(TEXT_FRAME, json_buffer, json_size, tmp_buffer, PACK_BUFFER_SIZE);

        tmp_client->out_message_size = tmp_size;
        tmp_client->out_message = (char*)malloc(tmp_size);
        memcpy(tmp_client->out_message, tmp_buffer, tmp_client->out_message_size);

        tmp_client->custom_worker.is_locked = FALSE;
        if(tmp_client->custom_worker.on_lock != 0)
          tmp_client->custom_worker.on_lock(tmp_client, FALSE);
      }
    }
  }

  return ERROR_NONE;
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
    }
    tmp_request = strtok(NULL, "\r\n");
  }

  strcpy(response, "HTTP/1.1 101 Switching Protocols\r\n");
  strcat(response, "Upgrade: websocket\r\n");
  strcat(response, "Connection: Upgrade\r\n");
  char tmp[128];
  sprintf(tmp, "Sec-WebSocket-Accept: %s\r\n\r\n", tmp_accept_key);
  strcat(response, tmp);

  *size = strlen(response);

  return ERROR_NONE;
}
//==============================================================================
int ws_set_frame(WSFrame_t frame_type, unsigned char* msg, int msg_length, unsigned char* buffer, int buffer_size)
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
