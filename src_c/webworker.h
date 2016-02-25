#ifndef WEBWORKER_H
#define WEBWORKER_H

#define WEB_LINE_SIZE      256
#define WEB_INITIAL_PATH   "../www"

int web_handle_connection(char *request, char *response, int *size);

#endif //WEBWORKER_H
