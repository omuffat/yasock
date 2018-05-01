
#include	"yasock.h"

/*
 *	Send one buffer then read this buffer back,
 *
 *	And so on until sock_env->wr_count has been sent/read
 *
 */
int		yasock_cli_writeread(int sd, sock_env_t *sock_env) {
  char		*write_buf = NULL;
  char		*rd_buf = NULL;
  ssize_t	wr_size = -1;
  ssize_t	rd_size = -1;
  ssize_t	total_wr = 0;
  ssize_t	total_rd = 0;
  int		i = 0;
  
  if (sd < 0 || !sock_env) {
    printf("[yasock_cli_writeread] invalid parameter(s)\n");
    return -1;
  }
  // Prepare write buffer
  if (sock_env->wr_buf_size) {
    write_buf = malloc(sock_env->wr_buf_size * sizeof(char));
  }
  if (write_buf == NULL) {
    printf("Could not malloc %u bytes for write buffer\n", sock_env->wr_buf_size);
    return -1;
  }
  // Prepare read buffer
  if (sock_env->rd_buf_size) {
    rd_buf = malloc(sock_env->rd_buf_size * sizeof(char));
  }
  if (rd_buf == NULL) {
    printf("Could not malloc %u bytes for read buffer\n", sock_env->wr_buf_size);
    free(write_buf);
    return -1;
  }
  // set data in write buffer
  for (i = 0; i < sock_env->wr_buf_size; i++) {
    write_buf[ i ] = YASOCK_DEFAULT_WR_CHAR;
  }
  // Sleep before first write if set
  if (sock_env->first_write_sleep) {
    usleep(sock_env->first_write_sleep);
  }
  // Send buffer as many time as wr_count
  for (i = 0; i < sock_env->wr_count; i++) {
    // Write one buffer into the wire
    if ((wr_size = send(sd, write_buf, sock_env->wr_buf_size, 0)) < 0) {
      perror("[yasock_cli_writeread] cannot write data to host");
      break;
    }
    // Update total write counter
    total_wr += wr_size;
    if (sock_env->verbose) {
      printf("[yasock_cli_writeread] #%u/%u Wrote %lu (%lu/%u bytes)\n",
	     i+1, sock_env->wr_count, wr_size, total_wr, sock_env->wr_count * sock_env->wr_buf_size);
    }
    // Read one buffer from the wire
    rd_size = recv(sd, rd_buf, sock_env->rd_buf_size, 0);
    if (rd_size < 0) {
      perror("[yasock_cli_writeread] Cannot make a read");
      break;
    }
    total_rd += rd_size;
    if (sock_env->verbose) {
      printf("[yasock_cli_writeread] Read %lu bytes\n", rd_size);
    }
    // If enabled, sleep between each write
    if (sock_env->write_sleep) {
      usleep(sock_env->write_sleep);
    }
  }
  // only if no error before
  if (i >= sock_env->wr_count) {
    // Peer may have not sent all data in our previous read
    while (total_rd < total_wr) {
      rd_size = recv(sd, rd_buf, sock_env->rd_buf_size, 0);
      if (rd_size < 0) {
	perror("[yasock_cli_writeread] Cannot make a read");
	break;
      }
      total_rd += rd_size;
      if (sock_env->verbose) {
	printf("[yasock_cli_writeread] Read %lu bytes\n", rd_size);
      }
    }
  }
  // Clean buffers
  if (write_buf) {
    free(write_buf);
  }
  if (rd_buf) {
    free(rd_buf);
  }
  return 0;
}


/*
 *	Send all data first, then read data back from server
 *
 *	Bad behavior regarding to server implementation (server:for each server read(2), write(2) it)
 *	If (client datalen to send) > (server SO_SNDBUF buffer) ??
 *	==> We have zero window on both side
 *	==> Client won't accept any packet when server will have sent client recvbuf size
 *	==> Server write will be pending and can't do read
 *
 *
 */
int		yasock_cli_writeread_sendall_first(int sd, sock_env_t *sock_env) {
  char		*write_buf = NULL;
  char		*rd_buf = NULL;
  ssize_t	wr_size = -1;
  ssize_t	rd_size = -1;
  ssize_t	total_wr = 0;
  int		i = 0;
  
  if (sd < 0 || !sock_env) {
    printf("[yasock_cli_writeread] invalid parameter(s)\n");
    return -1;
  }
  // Prepare write buffer
  if (sock_env->wr_buf_size) {
    write_buf = malloc(sock_env->wr_buf_size * sizeof(char));
  }
  if (write_buf == NULL) {
    printf("Could not malloc %u bytes for write buffer\n", sock_env->wr_buf_size);
    return -1;
  }
  // Prepare read buffer
  if (sock_env->rd_buf_size) {
    rd_buf = malloc(sock_env->rd_buf_size * sizeof(char));
  }
  if (rd_buf == NULL) {
    printf("Could not malloc %u bytes for read buffer\n", sock_env->wr_buf_size);
    free(write_buf);
    return -1;
  }
  // Prepare send buffer
  for (i = 0; i < sock_env->wr_buf_size; i++) {
    write_buf[ i ] = YASOCK_DEFAULT_WR_CHAR;
  }
  // Sleep before first write if set
  if (sock_env->first_write_sleep) {
    usleep(sock_env->first_write_sleep);
  }
  // Send buffer as many time as wr_count
  for (i = 0; i < sock_env->wr_count; i++) {
    // Write one buffer into the wire
    if ((wr_size = send(sd, write_buf, sock_env->wr_buf_size, 0)) < 0) {
      perror("[yasock_cli_writeread] cannot write data to host");
      break;
    }
    total_wr += wr_size;
    if (sock_env->verbose) {
      printf("[yasock_cli_writeread_sendall_first] #%u/%u Wrote %lu/%u bytes\n",
	     i+1, sock_env->wr_count, wr_size, sock_env->wr_count * sock_env->wr_buf_size);
    }
    // If enabled, sleep between each write
    if (sock_env->write_sleep) {
      usleep(sock_env->write_sleep);
    }
  }
  if (sock_env->verbose) {
    printf("[yasock_cli_writeread_sendall_first] TOTAL: Wrote %lu bytes\n", total_wr);
  }
  // Read one buffer from the wire as long as they are data
  while (total_wr > 0) {
    rd_size = recv(sd, rd_buf, sock_env->rd_buf_size, 0);
    if (rd_size < 0) {
      perror("[yasock_cli_writeread_sendall_first] Cannot make a read");
      break;
    }
    total_wr -= rd_size;
    if (sock_env->verbose) {
      printf("[yasock_cli_writeread_sendall_first] Read %lu bytes\n", rd_size);
    }
  }
  // Clean buffers
  if (write_buf) {
    free(write_buf);
  }
  if (rd_buf) {
    free(rd_buf);
  }
  return 0;
}

int		yasock_cli_writeonly(int sd, sock_env_t *sock_env) {
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
  // Sleep before first write if set
  if (sock_env->first_write_sleep) {
    usleep(sock_env->first_write_sleep);
  }
  // Send buffer as many time as wr_count
  for (i = 0; i < sock_env->wr_count; i++) {
    // Write one buffer into the wire
    if ((wr_size = send(sd, write_buf, sock_env->wr_buf_size, 0)) < 0) {
      perror("[yasock_cli_writeonly] cannot write data to host");
      break;
    }
    total_wr += wr_size;
    if (sock_env->verbose) {
      printf("[yasock_cli_writeonly] #%u/%u Wrote %lu/%u bytes\n",
	     i+1, sock_env->wr_count, wr_size, sock_env->wr_count * sock_env->wr_buf_size);
    }
    // If enabled, sleep between each write
    if (sock_env->write_sleep) {
      usleep(sock_env->write_sleep);
    }
  }
  if (sock_env->verbose) {
    printf("[yasock_cli_writeonly] TOTAL: Wrote %lu bytes\n", total_wr);
  }
  // Clean buffer
  if (write_buf) {
    free(write_buf);
  }
  return 0;
}


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
    if (connect(sd, (const struct sockaddr*)&remote_in, sizeof(struct sockaddr_in)) < 0) {
      perror("Could not connect");
      close(sd);
      return (-1);
    }
    if (sock_env->verbose) {
      printf("Connected to %s:%u\n", sock_env->ip_addr, sock_env->port);
    }
    //rc = yasock_cli_writeread_sendall_first(sd, sock_env);
    //rc = yasock_cli_writeread(sd, sock_env);
    rc = yasock_cli_writeonly(sd, sock_env);
    break;
  default:
    printf("Only AF_INET is handled so far\n");
    break;
  }
  // Close socket
  if (sock_env->fin_sleep) {
    usleep(sock_env->fin_sleep);
  }
  close(sd);
  return rc;
}
  
