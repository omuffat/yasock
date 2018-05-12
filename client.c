
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
  struct sockaddr_in	local_in;
  struct sockaddr_in	remote_in;

  if (!sock_env || !sock_env->ip_addr) {
    return -1;
  }
  // Create Socket
  if ((sd = socket(sock_env->af_family, SOCK_STREAM, 0)) < 0) {
    perror("Could not create a socket");
    return -1;
  }
  // Treat IPv4 and IPv6 separately (as sockaddr is different)
  switch (sock_env->af_family) {
  case AF_INET:
    // Prepare local sockaddr_in structure for bind
    local_in.sin_family = sock_env->af_family;
    local_in.sin_port = htons(YASOCK_ANY_PORT);
    local_in.sin_addr.s_addr = htonl(INADDR_ANY);
    // Make bind for our local socket
    if (bind(sd, (const struct sockaddr*)&local_in, sizeof(struct sockaddr_in)) < 0) {
      perror("Could not bind socket");
      close(sd);
      return -1;
    }
    // Prepare remote sockaddr_in structure for connect
    remote_in.sin_family = sock_env->af_family;
    remote_in.sin_port = htons(sock_env->port);
    inet_pton(sock_env->af_family, sock_env->ip_addr, (void*)&(remote_in.sin_addr));
    // Set socket options
    rc = yasock_set_socket_options(sd, sock_env);
    // Do connect
    if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
      printf("Trying to connect to %s:%u\n", sock_env->ip_addr, sock_env->port);
    }
    if (connect(sd, (const struct sockaddr*)&remote_in, sizeof(struct sockaddr_in)) < 0) {
      perror("Could not connect");
      close(sd);
      return (-1);
    }
    // verbose message to tell we have succeed to connect
    if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
      printf("Connected to %s:%u\n", sock_env->ip_addr, sock_env->port);
    }
    break;
  default:
    printf("Only AF_INET is handled so far\n");
    break;
  }
  // Now we are connected to server
  // Sleep before first write if set
  if (sock_env->init_sleep) {
    usleep(sock_env->init_sleep);
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
int		yasock_writeonly(int sd, sock_env_t *sock_env) {
  int		rc = 0;
  char		*write_buf = NULL;
  ssize_t	wr_size = -1;
  ssize_t	total_wr = 0;
  int		i = 0;
  
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
  // Send buffer as many time as wr_count
  for (i = 0; i < sock_env->wr_count; i++) {
    // Write one buffer into the wire
    if ((wr_size = send(sd, write_buf, sock_env->wr_buf_size, 0)) < 0) {
      perror("[yasock_cli_writeonly] cannot write data to host");
      break;
    }
    // Add last sent bytes to total write bytes count
    total_wr += wr_size;
    if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
      printf("[yasock_cli_writeonly] #%u/%u Wrote %lu/%u bytes\n",
	     i+1, sock_env->wr_count, wr_size, sock_env->wr_count * sock_env->wr_buf_size);
    }
    // If enabled, sleep after each write
    if (sock_env->rw_sleep) {
      usleep(sock_env->rw_sleep);
    }
  } // End of for
  if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
    printf("[yasock_cli_writeonly] TOTAL: Wrote %lu bytes\n", total_wr);
  }
  // Clean buffer
  if (write_buf) {
    free(write_buf);
  }
  return 0;
}

