#ifndef ERROR_H
#define ERROR_H

typedef struct
{
  int  number;
  char message[2048];
}error_t;

int make_error(int number, char *message);
error_t *last_error();

#endif //ERROR_H
