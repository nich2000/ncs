#ifndef WSWORKER_H
#define WSWORKER_H

#include "sha1.h"
#include "base64.h"

#include "protocol.h"

typedef enum
{
  ERROR_FRAME=0xFF00,
  INCOMPLETE_FRAME=0xFE00,

  OPENING_FRAME=0x3300,
  CLOSING_FRAME=0x3400,

  INCOMPLETE_TEXT_FRAME=0x01,
  INCOMPLETE_BINARY_FRAME=0x02,

  TEXT_FRAME=0x81,
  BINARY_FRAME=0x82,

  PING_FRAME=0x19,
  PONG_FRAME=0x1A
} WebSocketFrameType;

int ws_hand_shake   (char *request, char *response);
int ws_handle_buffer(pack_buffer buffer);

int ws_make_frame(WebSocketFrameType frame_type, unsigned char* msg, int msg_length, unsigned char* buffer, int buffer_size);
WebSocketFrameType ws_get_frame(unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length);

#endif //WSWORKER_H
