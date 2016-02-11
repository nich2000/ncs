#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "socket.h"
#include "test.h"

#include "protocol.h"
#include "gps_parse.h"
//==============================================================================
// "-DCMAKE_BUILD_TYPE=Debug"
//==============================================================================
int port = 5600;
char *host = "127.0.0.1";
//==============================================================================
int handle_command(char *command)
{
  if(strcmp(command, "exit") == 0)
  {
    return 0;
  }
  else if(strcmp(command, "server") == 0)
  {
    sock_server(port);
    return 1;
  }
  else if(strcmp(command, "client") == 0)
  {
    sock_client(port, host);
    return 1;
  }
  else if(strcmp(command, "test") == 0)
  {
    return 1;
  }
  else
    return -1;
}
//==============================================================================
int main(int argc, char *argv[])
{
  char tmp[128];
  sprintf(tmp, "word size: %d, packet size: %d, out buffer size: %d, in buffer size: %d, pack_protocol: %d",
          sizeof(pack_word), sizeof(pack_packet), sizeof(pack_out_packets_list), sizeof(pack_in_packets_list), sizeof(pack_protocol));
  log_add(tmp, LOG_INFO);

  sprintf(tmp, "worker size: %d, clients size: %d",
          sizeof(sock_worker), sizeof(sock_worker_list));
  log_add(tmp, LOG_INFO);

  if(argc > 1)
  {
    // 0     1  2 3     4 5
    // name -c -p 5600 -h 127.0.0.1
    if(argc > 3)
      if(strcmp(argv[2], "-p") == 0)
        port = atoi(argv[3]);

    if(argc > 5)
      if(strcmp(argv[4], "-h") == 0)
        host = argv[5];

    if(strcmp(argv[1], "-s") == 0)
    {
      log_set_name("server_log.txt");
      sprintf(tmp, "Server mode, port: %d", port);
      log_add(tmp, LOG_INFO);

//      sock_server(port);
    }
    else if(strcmp(argv[1], "-c") == 0)
    {
      log_set_name("client_log.txt");
      sprintf(tmp, "Client mode, port: %d, host: %s", port, host);
      log_add(tmp, LOG_INFO);

//      sock_client(port, host);
    }
  }
  else
  {
    log_set_name("log.txt");
    log_add("Test mode", LOG_INFO);

//    test_pack();

    // См. коммент в test_gps
    // NIch 06.02.2016
    gps_init();
    gps_parse_str(GPS_TEST_STR);
    GPRMC_t *tmp_data = gps_data();
    sprintf(tmp, "%s", tmp_data->time_gps);
    log_add(tmp, LOG_DEBUG);
  };

  printf(">");
  char command[128];
  while(1)
  {
    gets(command);
    printf("echo: %s\n", command);

    switch(handle_command(command))
    {
      case 0:
        return 0;
        break;
      case -1:
        printf("unknown command: %s\n", command);
        break;
    }

    printf(">");
  }

  return 0;
}
//==============================================================================
