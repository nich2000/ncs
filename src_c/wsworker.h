#ifndef WSWORKER_H
#define WSWORKER_H

#include "sha1.h"
#include "base64.h"

#include "protocol.h"

int ws_hand_shake   (char *request, char *response);
int ws_handle_buffer(pack_buffer buffer);

#endif //WSWORKER_H
