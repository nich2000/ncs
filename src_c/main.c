#include "log.h"
#include "protocol.h"
#include "socket.h"
#include "test.h"
//==============================================================================
// "-DCMAKE_BUILD_TYPE=Debug"
//==============================================================================
int main(int argc, char *argv[])
{
  add_to_log("Hello!", LOG_INFO);

  char tmp[128];

  if(argc > 1)
  {
    // 0     1  2 3     4 5
    // name -c -p 5600 -h 127.0.0.1
    int port = 5600;
    if(argc > 3)
      if(strcmp(argv[2], "-p") == 0)
        port = atoi(argv[3]);

    char *host = "127.0.0.1";
    if(argc > 5)
      if(strcmp(argv[4], "-h") == 0)
        host = argv[5];

    if(strcmp(argv[1], "-s") == 0)
    {
      set_log_name("server_log.txt");
      sprintf(tmp, "Server mode, port: %d", port);
      add_to_log(tmp, LOG_INFO);

      sock_init();
      sock_server_start(port);
      sock_server_work();
      sock_server_stop();
      sock_deinit();
    }
    else if(strcmp(argv[1], "-c") == 0)
    {
      set_log_name("client_log.txt");
      sprintf(tmp, "Client mode, port: %d, host: %s", port, host);
      add_to_log(tmp, LOG_INFO);

      sock_init();
      sock_client_start(port, host);
      sock_client_work();
      sock_client_stop();
      sock_deinit();
    }
  }
  else
  {
    set_log_name("test_log.txt");
    add_to_log("Test mode", LOG_INFO);

    test();
  };

  return 0;
}
//==============================================================================
