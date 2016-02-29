#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <rdma/rdma_verbs.h>
#include "ibmsg.h"
#include <string.h>

#define APPLICATION_NAME "ibmsg-rdma-send"
#define MSGSIZE (4096)

int
main(int argc, char** argv)
{
  char* ip;
  unsigned short port = 12345;

  if(argc < 2 || argc > 3)
  {
    fprintf(stderr, APPLICATION_NAME": error: illegal number of arguments\n");
    fprintf(stderr, APPLICATION_NAME": usage: %s IP [PORT]\n", APPLICATION_NAME);
    exit(EXIT_FAILURE);
  }
  ip = argv[1];
  if(argc == 3)
    port = atoi(argv[2]);

  /* Setup */
  ibmsg_event_loop event_loop;
  if(ibmsg_init_event_loop(&event_loop))
  {
    fprintf(stderr, APPLICATION_NAME": error: could not initialize event loop\n");
    exit(EXIT_FAILURE);
  }


  /* Connect */
  ibmsg_socket connection;
  if(ibmsg_connect(&event_loop, &connection, ip, port))
  {
    fprintf(stderr, APPLICATION_NAME": error: could not connect to remote host\n");
    exit(EXIT_FAILURE);
  }
  while(connection.status != IBMSG_CONNECTED)
  {
    ibmsg_dispatch_event_loop(&event_loop);
    if (connection.status == IBMSG_ERROR)
    {
      fprintf(stderr, APPLICATION_NAME": error: could not connect to remote host\n");
      exit(EXIT_FAILURE);
    }
  }

  for (int iii = 0; iii < 10; iii++) {
  /* Data transfers */
    fprintf(stdout, APPLICATION_NAME": transfer %i\n", iii);
    fprintf(stdout, APPLICATION_NAME": sleep\n");
    sleep(1);

    ibmsg_buffer msg;
    fprintf(stdout, APPLICATION_NAME": alloc\n");
    if(ibmsg_alloc_msg(&msg, &connection, MSGSIZE))
    {
      fprintf(stderr, APPLICATION_NAME": error: could not allocate message memory\n");
      exit(EXIT_FAILURE);
    }

    fprintf(stdout, APPLICATION_NAME": post\n");
    if(ibmsg_post_send(&connection, &msg))
    {
      fprintf(stderr, APPLICATION_NAME": error: could not send message\n");
      exit(EXIT_FAILURE);
    }

    while(msg.status != IBMSG_SENT) {
      fprintf(stdout, APPLICATION_NAME": wait in status %i\n", msg.status);

      if(ibmsg_dispatch_event_loop(&event_loop))
      {
        fprintf(stderr, APPLICATION_NAME": error: something went wrong while working in the event loop\n");
        exit(EXIT_FAILURE);
      }
      /* printf(APPLICATION_NAME": msg status %i, ncomp %i\n", msg.status, n_comp) */;
      printf(APPLICATION_NAME": msg status %i\n", msg.status);
    }

    fprintf(stdout, APPLICATION_NAME": free\n");
    if(ibmsg_free_msg(&msg))
    {
      fprintf(stderr, APPLICATION_NAME": error: could not free memory\n");
    }
  }

  /* Disconnect */
  if(ibmsg_disconnect(&connection))
  {
    fprintf(stderr, APPLICATION_NAME": error: could not disconnect\n");
  }
  while(connection.status != IBMSG_UNCONNECTED)
  {
    if(ibmsg_dispatch_event_loop(&event_loop))
    {
      fprintf(stderr, APPLICATION_NAME": error: something went wrong while working in the event loop\n");
    }
    if (connection.status == IBMSG_ERROR)
    {
      fprintf(stderr, APPLICATION_NAME": error: could not disconnect\n");
    }
  }


  /* Teardown */
  if(ibmsg_destroy_event_loop(&event_loop))
  {
    fprintf(stderr, APPLICATION_NAME": error: could not destroy event loop\n");
  }

    return EXIT_SUCCESS;
}
