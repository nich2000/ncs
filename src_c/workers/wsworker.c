//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: wsworker.c
 */
//==============================================================================
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "jansson.h"
#include "sha1.h"
#include "base64.h"

#include "wsworker.h"

#include "cmdworker.h"
#include "ncs_log.h"
#include "protocol_types.h"
#include "protocol.h"
#include "utils.h"
#include "socket_utils.h"
#include "socket.h"
#include "exec.h"
//==============================================================================
// http://learn.javascript.ru/websockets#описание-фрейма
//==============================================================================
/**
* GET /chat HTTP/1.1
* Host: example.com:8000
* Upgrade: websocket
* Connection: Upgrade
* Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
* Sec-WebSocket-Version: 13
 */
/**
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
custom_remote_client_t *_ws_remote_clients_next (ws_server_t *ws_server);
int                     _ws_remote_clients_count(ws_server_t *ws_server);
//==============================================================================
void *ws_recv_worker(void *arg);
void *ws_send_worker(void *arg);
//==============================================================================
int on_ws_accept    (void *sender, SOCKET socket, sock_host_t host);
int on_ws_new_data  (void *sender, void *data);
int on_ws_disconnect(void *sender);
int on_ws_error     (void *sender, ncs_error_t *error);
int on_ws_recv      (void *sender, char *buffer, int size);
int on_ws_send      (void *sender);
//==============================================================================
int packet_to_json_str(pack_packet_t *pack, char *buffer, int *size);
//==============================================================================
int json_str_to_packet(pack_packet_t *pack, char *buffer, int *size);
//==============================================================================
int ws_hand_shake(char *request, char *response, int *size);
//==============================================================================
int       ws_set_frame(WSFrame_t frame_type, unsigned char* msg, int msg_length, unsigned char* buffer, int buffer_size);
WSFrame_t ws_get_frame(unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length);
//==============================================================================
static ws_server_t _ws_server;
//==============================================================================
int ws_refresh_rate = 1000;
//==============================================================================
extern char *pack_struct_keys[];
extern char *pack_struct_captions[];
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
  custom_server_init(STATIC_WS_SERVER_ID, &server->custom_server);

  custom_remote_clients_init(&server->custom_remote_clients_list);

  strcpy((char*)server->custom_server.custom_worker.session_id, STATIC_WS_SERVER_NAME);
  server->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  server->custom_server.custom_worker.mode = SOCK_MODE_WS_SERVER;

  server->custom_server.on_accept          = &on_ws_accept;

  return ERROR_NONE;
}
//==============================================================================
int ws_server_start(ws_server_t *server, sock_port_t port)
{
  if(server->custom_server.custom_worker.state == STATE_START)
    return make_last_error(ERROR_NORMAL, errno, "ws_server_start, server already started");

  ws_server_init(server);

  server->custom_server.custom_worker.port  = port;
  server->custom_server.custom_worker.state = STATE_STARTING;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_DETACHED);

  int res = pthread_create(&server->custom_server.work_thread, &tmp_attr, ws_server_worker, (void*)server);
  if(res != 0)
    return make_last_error_fmt(ERROR_CRITICAL, errno, "ws_server_start, pthread_create result(%d)",
                               res);

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

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(server->custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
      server->custom_remote_clients_list.items[i].custom_worker.state = STATE_STOPPING;

  return ERROR_NONE;
}
//==============================================================================
int ws_server_pause(ws_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_PAUSING;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(server->custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
      server->custom_remote_clients_list.items[i].custom_worker.state = STATE_PAUSING;

  return ERROR_NONE;
}
//==============================================================================
int ws_server_resume(ws_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_RESUMING;

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
  ws_server_t *tmp_server = (ws_server_t*)arg;

  log_add_fmt(LOG_DEBUG, "[WS] [BEGIN] ws_server_worker, server id: %d",
              tmp_server->custom_server.custom_worker.id);

  if(custom_server_start(&tmp_server->custom_server.custom_worker) >= ERROR_NORMAL)
  {
    log_add_fmt(LOG_ERROR_CRITICAL, "[WS] ws_server_worker,\nmessage: %s",
                last_error()->message);
    goto exit;
  }

  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  exit:
  log_add_fmt(LOG_DEBUG, "[WS] [END] ws_server_worker, server id: %d",
              tmp_server->custom_server.custom_worker.id);
  return NULL;
}
//==============================================================================
custom_remote_client_t *_ws_remote_clients_next(ws_server_t *ws_server)
{
  custom_remote_client_t *tmp_client = _custom_remote_clients_next(&ws_server->custom_remote_clients_list);

  if(tmp_client != NULL)
  {
    custom_remote_client_init(ID_GEN_NEW, tmp_client);

    tmp_client->protocol.on_new_in_data  = on_ws_new_data;

    tmp_client->custom_worker.type  = SOCK_TYPE_REMOTE_CLIENT;
    tmp_client->custom_worker.mode  = ws_server->custom_server.custom_worker.mode;
    tmp_client->custom_worker.port  = ws_server->custom_server.custom_worker.port;

    tmp_client->on_disconnect       = on_ws_disconnect;
    tmp_client->on_error            = on_ws_error;
    tmp_client->on_recv             = on_ws_recv;
    tmp_client->on_send             = on_ws_send;
  }

  return tmp_client;
}
//==============================================================================
int _ws_remote_clients_count(ws_server_t *ws_server)
{
  return _custom_remote_clients_count_con(&ws_server->custom_remote_clients_list);
}
//==============================================================================
int on_ws_accept(void *sender, SOCKET socket, sock_host_t host)
{
  custom_server_t *tmp_server = (custom_server_t*)sender;
  custom_remote_client_t *tmp_client = _ws_remote_clients_next(&_ws_server);

  if(tmp_client == 0)
    return make_last_error_fmt(LOG_ERROR_CRITICAL,
                               errno,
                               "ws_accept, server id: %d, no available clients, socket: %d, host: %s",
                               tmp_server->custom_worker.id,
                               socket, host);

  time_t rawtime;
  time (&rawtime);
  tmp_client->connect_state = CONNECTED;
  tmp_client->connect_time = rawtime;

  tmp_client->custom_worker.state = STATE_STARTING;

  memcpy(&tmp_client->custom_worker.sock, &socket, sizeof(SOCKET));
  memcpy(tmp_client->custom_worker.host, host,   SOCK_HOST_SIZE);

  log_add_fmt(LOG_DEBUG, "[WS] ws_accept, server id: %d, socket: %d, host: %s, port: %d",
              tmp_server->custom_worker.id,
              tmp_client->custom_worker.sock, tmp_client->custom_worker.host, tmp_client->custom_worker.port);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_DETACHED);

  int res = pthread_create(&tmp_client->recv_thread, &tmp_attr, ws_recv_worker, (void*)tmp_client);
  if(res != 0)
    return make_last_error_fmt(ERROR_CRITICAL, errno, "on_ws_accept, pthread_create result(%d)",
                               res);

  res = pthread_create(&tmp_client->send_thread, &tmp_attr, ws_send_worker, (void*)tmp_client);
  if(res != 0)
    return make_last_error_fmt(ERROR_CRITICAL, errno, "on_ws_accept, pthread_create result(%d)",
                               res);

  return ERROR_NONE;
}
//==============================================================================
void *ws_recv_worker(void *arg)
{
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  log_add_fmt(LOG_DEBUG, "[WS] [BEGIN] ws_recv_worker, worker id: %d",
              tmp_client->custom_worker.id);

  tmp_client->custom_worker.state = STATE_START;

  char *request   = (char*)malloc(SOCK_WS_BUFFER_SIZE);
  char *response  = (char*)malloc(SOCK_WS_BUFFER_SIZE * SOCK_WS_BUFFER_SIZE);
  int  tmp_size   = 0;
  int  tmp_errors = 0;

  while(tmp_client->custom_worker.state == STATE_START)
  {
    int res = sock_recv(tmp_sock, request, &tmp_size);

    if(res == ERROR_NONE)
    {
      if(tmp_size == 0)
        goto next;

      if(tmp_client->hand_shake == FALSE)
      {
        if(ws_hand_shake(request, response, &tmp_size) == ERROR_NONE)
        {
          if(sock_send(tmp_sock, response, tmp_size) == ERROR_NONE)
          {
            tmp_client->hand_shake = TRUE;
            log_add_fmt(LOG_DEBUG, "[WS] ws_recv_worker, handshake success, worker id: %d",
                        tmp_client->custom_worker.id);
          }
          else
          {
            errno = 1;
            make_last_error_fmt(ERROR_NORMAL, errno, "[WS] ws_recv_worker, sock_send, errno: %d,\n" \
                                "message: %s",
                                errno, last_error()->message);
            goto errorhandler;
          }
        }
        else
        {
          make_last_error_fmt(ERROR_NORMAL, errno, "[WS] ws_recv_worker, ws_hand_shake, errno: %d,\n" \
                              "message: %s",
                              errno, last_error()->message);
          goto errorhandler;
        }
      }
      else
      {
        time_t rawtime;
        time (&rawtime);
        tmp_client->recv_time = rawtime;

        int tmp_size;
        unsigned char tmp_buffer[SOCK_WS_BUFFER_SIZE];
        ws_get_frame((unsigned char*)request, strlen(request), tmp_buffer, SOCK_WS_BUFFER_SIZE, &tmp_size);

        log_add_fmt(LOG_INFO, "[WS] ws_recv_worker,\nmessage: %s",
                    tmp_buffer);

        pack_packet_t tmp_pack;
        if(json_str_to_packet(&tmp_pack, (char*)tmp_buffer, &tmp_size) == ERROR_NONE)
        {
          if(handle_command_pack(tmp_client, &tmp_pack) >= ERROR_NORMAL)
          {
            make_last_error_fmt(ERROR_NORMAL, errno, "[WS] ws_recv_worker, handle_command_pack, errno: %d,\n" \
                                "message: %s",
                                errno, last_error()->message);
            goto errorhandler;
          }
        }
        else
        {
          make_last_error_fmt(ERROR_NORMAL, errno, "[WS] ws_recv_worker, json_str_to_packet, errno: %d,\n" \
                              "message: %s",
                              errno, last_error()->message);
          goto errorhandler;
        }
      }
    }
    else if(res == ERROR_WARNING)
    {
      if(tmp_size == 0)
      {
        if(tmp_client->on_disconnect != 0)
          tmp_client->on_disconnect((void*)tmp_client);
        tmp_client->custom_worker.state = STATE_STOPPING;
      }
    }
    else if(res >= ERROR_NORMAL)
    {
      // #govnocode
      errorhandler:
      // handle error
      if(tmp_client->on_error != 0)
        tmp_client->on_error((void*)tmp_client, last_error());
      // if too many errors
      if(tmp_errors++ > SOCK_ERRORS_COUNT)
        tmp_client->custom_worker.state = STATE_STOPPING;
    }

    next:
//    sched_yield();
    usleep(5000);
  }

  free(request);
  free(response);

  tmp_client->custom_worker.state = STATE_STOP;

  log_add_fmt(LOG_DEBUG, "[WS] [END] ws_recv_worker, worker id: %d",
              tmp_client->custom_worker.id);

  return NULL;
}
//==============================================================================
void *ws_send_worker(void *arg)
{
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  log_add_fmt(LOG_DEBUG, "[WS] [BEGIN] ws_send_worker, worker id: %d",
              tmp_client->custom_worker.id);

  tmp_client->custom_worker.state = STATE_START;

  int tmp_errors = 0;

  // TODO: maybe deadlock
  // В этом цикле, отправляю собщения от не lock клиента
  // Пока я отправляю, может прийти команда на пополнение данных
  // А тут делается free, как минимум потеря данных, возможен непредсказуемый результат
  // ИМХО нужна очередь
  // NIch 05.04.2016

  while(tmp_client->custom_worker.state == STATE_START)
  {
    if(tmp_client->hand_shake == TRUE)
    {
      if(!tmp_client->custom_worker.is_locked)
        if((tmp_client->out_message != NULL) && (tmp_client->out_message_size != 0))
        {
          if(sock_send(tmp_sock, tmp_client->out_message, tmp_client->out_message_size) >= ERROR_NORMAL)
          {
            tmp_errors++;
          }
          else
          {
            time_t rawtime;
            time (&rawtime);
            tmp_client->send_time = rawtime;
          }

          free(tmp_client->out_message);
          tmp_client->out_message = NULL;
          tmp_client->out_message_size = 0;

          if((tmp_errors > SOCK_ERRORS_COUNT) || (tmp_client->custom_worker.state == STATE_STOP))
            break;
        }
    }

//    sched_yield();
    usleep(5000);
  }

  tmp_client->custom_worker.state = STATE_STOP;

  log_add_fmt(LOG_DEBUG, "[WS] [END] ws_send_worker, worker id: %d",
              tmp_client->custom_worker.id);

  return NULL;
}
//==============================================================================
int on_ws_new_data(void *sender, void *data)
{
  return ERROR_NONE;
}
//==============================================================================
int on_ws_disconnect(void *sender)
{
  custom_client_t *tmp_client = (custom_client_t*)sender;

  log_add_fmt(LOG_INFO, "[WS] on_ws_disconnect, disconnected from server, worker id: %d",
              tmp_client->custom_remote_client.custom_worker.id);

  time_t rawtime;
  time (&rawtime);
  tmp_client->custom_remote_client.connect_state = DISCONNECTED;
  tmp_client->custom_remote_client.disconnect_time = rawtime;

  return ERROR_NONE;
}
//==============================================================================
int on_ws_error(void *sender, ncs_error_t *error)
{
  log_add_fmt(LOG_INFO, "[WS] ws_error,\nmessage: %s",
              error->message);

  return ERROR_NONE;
}
//==============================================================================
int on_ws_recv(void *sender, char *buffer, int size)
{
  return ERROR_NONE;
}
//==============================================================================
int on_ws_send(void *sender)
{
  return ERROR_NONE;
}
//==============================================================================
// http://stackoverflow.com/questions/23392755/application-segmentation-fault-only-when-compiling-on-windows-with-mingw
int ws_remote_clients_register(sock_id_t id, sock_name_t session_id)
{
  char tmp[256];
  sprintf(tmp, "[WS] ws_remote_clients_register, worker id: %d, session id: %s",
          id, session_id);
  log_add(LOG_DEBUG, tmp);

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_remote_client_t *client = &_ws_server.custom_remote_clients_list.items[i];

    if(client->custom_worker.state == STATE_START)
      if(client->custom_worker.id == id)
      {
        log_add_fmt(LOG_INFO, "[WS] ws_remote_clients_register, client found, worker id: %d, session id: %s",
                    id, session_id);

        strcpy((char*)client->custom_worker.session_id, (char*)session_id);

        time_t rawtime;
        time(&rawtime);
        client->register_time = rawtime;

        client->register_state = REGISTER_OK;

        int res = ERROR_NONE;

        // Отправка конфига
//        if(res == ERROR_NONE)
//        {
//          pack_packet_t config_packet;
//          ws_server_send_pack(SOCK_SEND_TO_ALL, &config_packet);
//          // TODO: костыль
//          pthread_yield();
////          usleep(10000);
//        }

        // TODO: тут создали буффер, начали отправку
        if(res == ERROR_NONE)
        {
          pack_packet_t map_packet;
          cmd_map(&map_packet);
          res = ws_server_send_pack(SOCK_SEND_TO_ALL, &map_packet);
          // TODO: костыль
          usleep(10000);
        }

        // TODO: пришли сюда, но отправка еще идет,
        // после отправки буффер чистится и ...
        if(res == ERROR_NONE)
        {
          pack_packet_t clients_packet;
          cmd_remote_client_list(&clients_packet);
          res = ws_server_send_pack(SOCK_SEND_TO_ALL, &clients_packet);
          // TODO: костыль
          usleep(10000);
        }

        return res;
      }
  }

  sprintf(tmp, "[WS] ws_remote_clients_register, client not found, worker id: %d, session id: %s",
          id, session_id);
  log_add(LOG_ERROR, tmp);
  return make_last_error(ERROR_NORMAL, errno, tmp);
}
//==============================================================================
/**
  [
    ["key", "value"],
    ["key", "value"],
    ["key", "value"]
  ]
 */
//==============================================================================
const char *caption_by_key(const char *key)
{
  for(int i = 0; i < PACK_STRUCT_VAL_COUNT; i++)
    if(strcmp(key, pack_struct_keys[i]) == 0)
      return pack_struct_captions[i];

  return NULL;
}
//==============================================================================
int json_to_buffer(json_t *json, pack_buffer_t buffer, int *size)
{
  strcpy((char*)buffer, json_dumps(json, JSON_ENCODE_ANY));

  *size = strlen((char*)buffer);

  return ERROR_NONE;
}
//==============================================================================
// Рекурсивный алгоритм
json_t *pack_to_json(pack_packet_t *packet)
{
  json_t *tmp_json_words = json_array();

  for(int i = 0; i < packet->words_count; i++)
  {
    pack_word_t *tmp_word = &packet->words[i];

    pack_key_t tmp_key;
    strcpy((char*)tmp_key, (char*)tmp_word->key);

    json_t *tmp_json_value;
    if(tmp_word->type == PACK_WORD_PACK)
    {
      pack_packet_t tmp_pack;
      pack_word_as_pack(tmp_word, &tmp_pack);

      tmp_json_value = pack_to_json(&tmp_pack);
    }
    else
    {
      pack_value_t tmp_value;
      pack_word_as_string(tmp_word, tmp_value);

      tmp_json_value = json_string((char*)tmp_value);
    }

    json_t *tmp_json_word = json_object();
    json_object_set_new(tmp_json_word, (char*)tmp_key, tmp_json_value);

    json_array_append_new(tmp_json_words, tmp_json_word);
  }

  return tmp_json_words;
}
//==============================================================================
int packet_to_json_str(pack_packet_t *pack, char *buffer, int *size)
{
  json_t *tmp_json = pack_to_json(pack);

  return json_to_buffer(tmp_json, (unsigned char*)buffer, size);
}
//==============================================================================
int json_str_to_packet(pack_packet_t *pack, char *buffer, int *size)
{
  pack_init(pack);
  pack_set_flag(pack, PACK_FLAG_CMD);

  json_error_t tmp_error;
  json_t *tmp_json = json_loads(buffer, JSON_DECODE_ANY, &tmp_error);

  if(tmp_json != NULL)
  {
    if(json_array_size(tmp_json) >= 0)
    {
      int line = 0;
      size_t tmp_index_word;
      json_t *tmp_word;
      json_array_foreach(tmp_json, tmp_index_word, tmp_word)
      {
        if(json_array_size(tmp_word) == 2)
        {
          // json_t *tmp_key   = json_array_get(tmp_word, 0);
          json_t *tmp_value = json_array_get(tmp_word, 1);

          if(line == 0)
          {
            line++;
            pack_add_cmd(pack, (unsigned char*)json_string_value(tmp_value));
          }
          else
          {
            pack_add_param(pack, (unsigned char*)json_string_value(tmp_value));
          }
        }
      }
    }

    return ERROR_NONE;
  }
  else
  {
    return make_last_error_fmt(ERROR_NORMAL, errno, "json_str_to_packet,\nmesage: %s",
                               tmp_error.text);
  }
}
//==============================================================================
// TODO: https://computing.llnl.gov/tutorials/pthreads/
//==============================================================================
float timedifference_msec(struct timeval t0, struct timeval t1)
{
  return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}
//==============================================================================
int ws_server_send_data(int session_id, pack_packet_t *pack, sock_active_t active)
{
  struct timeval now;
  gettimeofday(&now, 0);

  if(active == ACTIVE_FIRST)
    if(timedifference_msec(_ws_server.send_time_f, now) < ws_refresh_rate)
      return ERROR_WAIT;

  if(active == ACTIVE_SECOND)
    if(timedifference_msec(_ws_server.send_time_s, now) < ws_refresh_rate)
      return ERROR_WAIT;

  pack_add_as_int(pack, (unsigned char*)"ACT", active);
  int res = ws_server_send_pack(session_id, pack);

  if(active == ACTIVE_FIRST)
    gettimeofday(&_ws_server.send_time_f, 0);

  if(active == ACTIVE_SECOND)
    gettimeofday(&_ws_server.send_time_s, 0);

  return res;
}
//==============================================================================
int ws_server_send_pack(int session_id,  pack_packet_t *pack)
{
  if(_ws_server.custom_server.custom_worker.state != STATE_START)
    return make_last_error(ERROR_WARNING, errno, "ws_server_send_pack, ws server not started");

  if(_ws_remote_clients_count(&_ws_server) == 0)
    return make_last_error(ERROR_WARNING, errno, "ws_server_send_pack, no ws clients connected");

  if(_ws_server.custom_server.custom_worker.state == STATE_START)
  {
    for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    {
      custom_remote_client_t *tmp_client = &_ws_server.custom_remote_clients_list.items[i];

      if(!((session_id == SOCK_SEND_TO_ALL) || (tmp_client->custom_worker.id == session_id)))
        continue;

      if(tmp_client->custom_worker.state == STATE_START)
      {
        tmp_client->custom_worker.is_locked = TRUE;

        sock_buffer_t json_buffer;
        int           json_size = 0;
        packet_to_json_str(pack, (char*)json_buffer, &json_size);
//        log_add_fmt(LOG_DEBUG, "[WS] ws_server_send_pack, json:\n%s",
//                    json_buffer);

        sock_buffer_t tmp_buffer;
        int           tmp_size = 0;
        tmp_size = ws_set_frame(TEXT_FRAME, json_buffer, json_size, tmp_buffer, SOCK_BUFFER_SIZE);

        tmp_client->out_message_size = tmp_size;
        tmp_client->out_message = (char*)malloc(tmp_size);
        memcpy(tmp_client->out_message, tmp_buffer, tmp_client->out_message_size);

        tmp_client->custom_worker.is_locked = FALSE;
      }
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int ws_server_send_cmd(int session_id, int argc, ...)
{
  if(_ws_server.custom_server.custom_worker.state != STATE_START)
    return make_last_error(ERROR_WARNING, errno, "ws_server_send_cmd, ws server not started");

  if(_ws_remote_clients_count(&_ws_server) == 0)
    return make_last_error(ERROR_WARNING, errno, "ws_server_send_cmd, no ws clients connected");

  if(_ws_server.custom_server.custom_worker.state == STATE_START)
  {
    for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    {
      custom_remote_client_t *tmp_client = &_ws_server.custom_remote_clients_list.items[i];

      if(!((session_id == SOCK_SEND_TO_ALL) || (tmp_client->custom_worker.id == session_id)))
        continue;

      if(tmp_client->custom_worker.state == STATE_START)
      {
        tmp_client->custom_worker.is_locked = TRUE;

        pack_packet_t tmp_pack;
        pack_init(&tmp_pack);
        pack_set_flag(&tmp_pack, PACK_FLAG_CMD);

        va_list tmp_params;
        va_start(tmp_params, argc);
        char *tmp_cmd = va_arg(tmp_params, char*);
        pack_add_as_string(&tmp_pack, (unsigned char*)PACK_CMD_KEY, (unsigned char*)tmp_cmd);

        for(int i = 1; i < argc; i++)
        {
          char *tmp_param = va_arg(tmp_params, char*);
          pack_add_as_string(&tmp_pack, (unsigned char*)PACK_PARAM_KEY, (unsigned char*)tmp_param);
        }
        va_end(tmp_params);

        pack_buffer_t json_buffer;
        int           json_size = 0;
        packet_to_json_str(&tmp_pack, (char*)json_buffer, &json_size);
//        log_add_fmt(LOG_DEBUG, "[WS] ws_server_send_cmd, json:\n%s",
//                    json_buffer);

        pack_buffer_t tmp_buffer;
        pack_size_t   tmp_size = 0;
        tmp_size = ws_set_frame(TEXT_FRAME, (unsigned char*)json_buffer, json_size, (unsigned char*)tmp_buffer, SOCK_BUFFER_SIZE);

        tmp_client->out_message_size = tmp_size;
        tmp_client->out_message = (char*)malloc(tmp_size+1);
        tmp_client->out_message[tmp_size] = '\0';

        memcpy(tmp_client->out_message, tmp_buffer, tmp_client->out_message_size);

        tmp_client->custom_worker.is_locked = FALSE;
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
