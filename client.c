
#include	"yasock.h"


/*
 *	Launch client part
 *	Called when acting as a client
 *	Call procedures for reading/writing data
 *
 */
int			yasock_launch_client(sock_env_t *sock_env) {
  int			rc = 0;
  int			sd = -1;

  if (!sock_env || !sock_env->ip_addr) {
    return -1;
  }
  // Create Socket
  if ((sd = socket(sock_env->af_family, SOCK_STREAM, 0)) < 0) {
    perror("Could not create a socket");
    return -1;
  }
  // connect socket
  if ((rc = yasock_do_connect(sd, sock_env)) < 0) {
    return rc;
  }
  // Now we are connected to server
  // Sleep before first write if set
  if (sock_env->init_sleep) {
    usleep(sock_env->init_sleep);
  }
  // set up tcp_info if requested. Ignore errors
  if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_TCPINFO_FLAG)) {
    yasock_prepare_tcpi(sock_env);
  }
  // launch interactive or bulk transfer
  if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_INTERACTIVE_FLAG)) {
    yasock_interactive(sd, sock_env);
  } else {
    rc = yasock_writeonly(sd, sock_env);
  }
  if (sock_env->fin_sleep) {
    usleep(sock_env->fin_sleep);
  }
  // Close socket. Linger may be set
  if (close(sd) < 0) {
    perror("Close error");
  }
  return rc;
}

/*
 *	Send only data.
 *
 *
 */
int			yasock_writeonly(int sd, sock_env_t *sock_env) {
  int			rc = 0;
  char			*write_buf = NULL;
  ssize_t		wr_size = -1;
  ssize_t		total_wr = 0;
  int			i = 0;
  int			nfds = sd + 1;
  fd_set		fdset;
  struct timeval	select_timeout;
  
  if (sd < 0 || !sock_env) {
    printf("[yasock_cli_writeonly] invalid parameter(s)\n");
    return -1;
  }
  // Prepare write buffer
  if (sock_env->wr_buf_size) {
    write_buf = malloc(sock_env->wr_buf_size * sizeof(char));
  }
  if (write_buf == NULL) {
    printf("[yasock_cli_writeonly] Could not malloc %u bytes for write buffer\n", sock_env->wr_buf_size);
    return -1;
  }
  // Prepare send buffer
  for (i = 0; i < sock_env->wr_buf_size; i++) {
    write_buf[ i ] = YASOCK_DEFAULT_WR_CHAR;
  }
  i = 0;
  while (1) {
    // Stop if we have sent wr_count buffers
    if (i >= sock_env->wr_count) {
      break;
    }
    select_timeout = sock_env->select_timeout;
    FD_ZERO(&fdset);
    FD_SET(sd, &fdset);
    // check if we can write
    if ((rc = select(nfds, NULL, &fdset, NULL, &select_timeout)) < 0) {
      perror("Error while select for writing");
      return rc;
    }
    if (FD_ISSET(sd, &fdset)) {
      // Write one buffer into the wire
      if ((wr_size = send(sd, write_buf, sock_env->wr_buf_size, 0)) < 0) {
	perror("[yasock_cli_writeonly] cannot write data to host");
	break;
      }
      if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
	printf("[yasock_cli_writeonly] #%u/%u Wrote %lu bytes\n",
	       i+1, sock_env->wr_count, wr_size);
      }
      // increase total bytes counter
      total_wr += wr_size;
      // increment couting of buffer sent
      i++;
      // If enabled, get tcp_info data. Ignore errors
      if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_TCPINFO_FLAG)) {
	yasock_write_tcpi(sd, sock_env);
      }
      // If enabled, sleep after each write
      if (sock_env->rw_sleep) {
	usleep(sock_env->rw_sleep);
      }
    } // end of if FD_ISSET
  } // end of while (1)
  if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
    printf("[yasock_cli_writeonly] TOTAL: Wrote %lu bytes\n", total_wr);
  }
  // Send FIN, we have no data to send any more.
  if ((rc = shutdown(sd, SHUT_WR)) < 0) {
    perror("Error while calling shutdown");
    return rc;
  }
  // Wait for FIN from peer
  while (1) {
    select_timeout = sock_env->select_timeout;
    FD_ZERO(&fdset);
    FD_SET(sd, &fdset);
    // check if we have something to read
    if ((rc = select(nfds, &fdset, NULL, NULL, &select_timeout)) < 0) {
      perror("Error while select for reading");
      return rc;
    }
    // If enabled, get tcp_info data. Ignore errors
    if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_TCPINFO_FLAG)) {
      yasock_write_tcpi(sd, sock_env);
    }
    if (FD_ISSET(sd, &fdset)) {
      if ((wr_size = recv(sd, write_buf, sock_env->wr_buf_size, 0)) < 0) {
	perror("[yasock_cli_writeonly] cannot read data from host");
	break;
      }
      if (wr_size == 0) {
	break;
      }
    }
  } // end of while (1)
  // Clean buffer
  if (write_buf) {
    free(write_buf);
  }
  return 0;
}

/*
 *	Connect the socket descriptor to the host as given in sock_env :
 *
 *	Make local bind
 *	call setsockopt
 *	Make connect
 *
 */
int			yasock_do_connect(int sd, sock_env_t *sock_env) {
  int			rc = 0;
  struct sockaddr_in	local_in;
  struct sockaddr_in	remote_in;
  int			sockaddr_len = 0;

  if (sd < 0 || !sock_env) {
#ifdef	YASOCK_DEBUG
    fprintf(stderr, "[yasock_do_connect] Invalid parameters\n");
#endif	// YASOCK_DEBUG
    return -1;
  }    
  // Treat IPv4 and IPv6 separately (as sockaddr is different)
  switch (sock_env->af_family) {
  case AF_INET:
    // Prepare local sockaddr_in structure for bind
    local_in.sin_family = sock_env->af_family;
    local_in.sin_port = htons(YASOCK_ANY_PORT);
    local_in.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddr_len = sizeof(struct sockaddr_in);
    // Prepare remote sockaddr_in structure for connect
    remote_in.sin_family = sock_env->af_family;
    remote_in.sin_port = htons(sock_env->port);
    sockaddr_len = sizeof(struct sockaddr_in);
    inet_pton(sock_env->af_family, sock_env->ip_addr, (void*)&(remote_in.sin_addr));
    break;
  case AF_INET6:
  default:
    printf("Only AF_INET is handled so far\n");
    break;
  }
  // Make bind for our local socket
  if ((rc = bind(sd, (const struct sockaddr*)&local_in, sockaddr_len)) < 0) {
    perror("Could not bind socket");
    close(sd);
    return rc;
  }
  // Set socket options
  rc = yasock_set_socket_options(sd, sock_env);
  // Do connect
  if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
    printf("Trying to connect to %s:%u\n", sock_env->ip_addr, sock_env->port);
  }
  if ((rc = connect(sd, (const struct sockaddr*)&remote_in, sockaddr_len)) < 0) {
    perror("Could not connect");
    close(sd);
    return rc;
  }
  // verbose message to tell we have succeed to connect
  if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
    printf("Connected to %s:%u\n", sock_env->ip_addr, sock_env->port);
  }
  return rc;
}
