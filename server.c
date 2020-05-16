
#include	"yasock.h"

int			yasock_launch_server(sock_env_t *sock_env) {
  int			rc = 0;
  int			listen_sd = -1;
  int			new_con_sd = -1;
  struct sockaddr_in	new_con;
  socklen_t		new_con_len = 0;
  char			ip_buf[ YASOCK_DFT_IP_BUFSIZE ];

  if (!sock_env) {
#ifdef	YASOCK_DEBUG
    fprintf(stderr, "[yasock_launch_server] Invalid parameters\n");
#endif	// YASOCK_DEBUG
    return -1;
  }
  // Create Socket
  // TODO:  handle UDP Stream mode
  if ((listen_sd = socket(sock_env->af_family, SOCK_STREAM, 0)) < 0) {
    perror("Could not create a socket");
    return -1;
  }
  // Set up socket for listening
  if ((rc = yasock_do_listen(listen_sd, sock_env)) < 0) {
    return rc;
  }
  while (1) {
    // Wait on a new connection. For now, handle only one connection
    // Need to use select(2) to handle several connections. And make socket NON BLOCKING
    new_con_len = sizeof(struct sockaddr_in);
    if ((new_con_sd = accept(listen_sd, (struct sockaddr*)&new_con, &new_con_len)) < -1) {
      perror("Cannot accept a new connection");
      break;
    }
    // Verbose message for accepting a new connexion
    inet_ntop(sock_env->af_family, (const void*)&(new_con.sin_addr), ip_buf, YASOCK_DFT_IP_BUFSIZE);
    printf("Accepting connection from %s:%u\n", ip_buf, ntohs(new_con.sin_port));
    // performs sleep after accept
    if (sock_env->init_sleep > 0) {
      usleep(sock_env->init_sleep);
    }
    // We have a client connected
    // launch interactive or bulk transfer
    if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_INTERACTIVE_FLAG)) {
      rc = yasock_interactive(new_con_sd, sock_env);
    } else {
      rc = yasock_srv_readonly(new_con_sd, sock_env);
    }
    // Close client socket
    if (sock_env->fin_sleep) {
      usleep(sock_env->fin_sleep);
    }
    if (close(new_con_sd) < 0) {
      perror("Error while closing client socket");
      break;
    }
  } // End while (1)
  // Close listening socket
  if (close(listen_sd) < 0) {
    perror("Error while closing server socket");
  }
  return rc;
}

/*
 *	Read only data
 *	No write back
 *
 *
 */
int			yasock_srv_readonly(int cli_sd, sock_env_t *sock_env)
{
  int			rc = 0;
  char			*data_buf = NULL;
  ssize_t		size_read = 0;
  
  if (cli_sd < 0 || !sock_env) {
#ifdef	YASOCK_DEBUG
    fprintf(stderr, "[yasock_srv_readonly] Invalid parameters\n");
#endif	// YASOCK_DEBUG
    return -1;
  }
  // Set socket options
  // Inheritance of setsockopt is not guaranteed
  rc = yasock_set_socket_options(cli_sd, sock_env);
  // Set buffer for reading data
  if ((data_buf = malloc(sock_env->rd_buf_size)) == NULL) {
    printf("[yasock_srv_readwrite] Cannot malloc %u len data\n", sock_env->rd_buf_size);
    close(cli_sd);
    return -1;
  }
  // Reads data
  while ((size_read = recv(cli_sd, (void*)data_buf, sock_env->rd_buf_size, 0)) > 0) {
    if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
      printf("[yasock_srv_readwrite] Read %lu bytes\n", size_read);
    }
    // If enabled, sleep between each read
    if (sock_env->rw_sleep) {
      usleep(sock_env->rw_sleep);
    }
  }
  free(data_buf);
  return rc;
}

/*
 *	Set up listening socket sd:
 *	--> call bind(2) on INADDR_ANY and port given in sock_env
 *	--> set socket options
 *	--> call listen(2)
 *	--> call usleep(2) after listen call if requested
 *
 */
int			yasock_do_listen(int sd, sock_env_t *sock_env) {
  int			rc = 0;
  struct sockaddr_in	in;
  int			in_len = 0;
  char			ip_buf[ YASOCK_DFT_IP_BUFSIZE + 1 ];

  if (sd < 0 || !sock_env) {
#ifdef	YASOCK_DEBUG
    fprintf(stderr, "[yasock_do_listen] Invalid parameters\n");
#endif	// YASOCK_DEBUG
    return -1;
  }
  switch (sock_env->af_family) {
  case AF_INET:
    // Prepare sockaddr_in structure for bind
    in.sin_family = sock_env->af_family;
    in.sin_port = htons(sock_env->port);
    in.sin_addr.s_addr = htonl(INADDR_ANY);
    in_len = sizeof(struct sockaddr_in);
    if (inet_ntop(sock_env->af_family, (const void*)&(in.sin_addr),
		ip_buf, YASOCK_DFT_IP_BUFSIZE) == NULL) {
      perror("warning inet_ntop");
      ip_buf[0] = '\0';
    }
    break;
  case AF_INET6:
  default:
    printf("Only AF_INET is handled so far\n");
    break;
  }
  // Make bind for our listening socket
  if (bind(sd, (const struct sockaddr*)&in, in_len) < 0) {
    perror("Could not bind socket");
    return -1;
  }
  // Set socket options
  rc = yasock_set_socket_options(sd, sock_env);
  // Prepare our socket for listening
  if (listen(sd, sock_env->backlog) < 0) {
    perror("Could not listen on socket");
    return -1;
  }
  // Display a informational message. But we need first a human printable ip address
  // inet_ntoa is deprecated (and does not support IPv6)
  if (ip_buf[0]) {
    printf("Listening new connections on %s:%u\n", ip_buf, sock_env->port);
  }
  // Sleep before accepting new connection if set
  if (sock_env->listen_sleep > 0) {
    usleep(sock_env->listen_sleep);
  }
  return rc;
}
