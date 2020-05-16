
#include	"yasock.h"

/*
 *	TEST ONLY. NOT USED SO FAR
 *
 *	handle multiple connections
 *
 */
int			yasock_io(int listen_sd, sock_env_t *sock_env)
{
  int			rc = 0;
  int			nfds = 0;
  fd_set		rfds;
  fd_set		wfds;
  fd_set		efds;
  int			new_sd = -1;
  char			*data_buf = NULL;
  ssize_t		size_read = 0;
  ssize_t		size_write = 0;
  struct timeval	select_timeout = {0, 0};
  struct timeval	select_itimeout = YASOCK_SELECT_ITIMEOUT;
  struct timeval	select_stimeout = YASOCK_SELECT_STIMEOUT;
  int			i = 0;
  int			ready = 0;
  
  if (listen_sd < 0 || !sock_env) {
    return -1;
  }
  // malloc a list of sd. the max number of accepted connections (backlog) is the size
  if ((sock_env->list_sd = malloc(sizeof(int) * sock_env->backlog)) == NULL) {
    fprintf(stderr, "[yasock_io] Cannot malloc %u connections\n", sock_env->backlog);
    return -1;
  }
  // Init sd list with value -1
  memset(sock_env->list_sd, -1, sock_env->backlog);
  // Set buffer for read/write operation
  if ((data_buf = malloc(sock_env->rd_buf_size)) == NULL) {
    fprintf(stderr, "[yasock_io] Cannot malloc buffer of %u len\n", sock_env->rd_buf_size);
    return -1;
  }
  // Set Select timeout
  if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_INTERACTIVE_FLAG)) {
    memcpy(&select_timeout, &select_itimeout, sizeof(struct timeval));
  } else {
    memcpy(&select_timeout, &select_stimeout, sizeof(struct timeval));
  }
  // Open connections, read/write operations
  while (1) {
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    // Fill fds with listening socket
    FD_SET(listen_sd, &rfds);
    FD_SET(listen_sd, &wfds);
    FD_SET(listen_sd, &efds);
    nfds = listen_sd + 1;
    // Fill fds with all active connections
    for (i = 0; i < sock_env->backlog; i++) {
      if (sock_env->list_sd[i] >= 0) {
	FD_SET(sock_env->list_sd[i], &rfds);
	FD_SET(sock_env->list_sd[i], &wfds);
	FD_SET(sock_env->list_sd[i], &efds);
	// nfds must the highest socket descriptor in fds + 1
	if (sock_env->list_sd[i] + 1 > nfds) {
	  nfds = sock_env->list_sd[i] + 1;
	}
      }
    }
    // Call select(2)
    ready = select(nfds, &rfds, &wfds, &efds, &select_timeout);
    // If error on select, stop the loop
    if (ready < 0) {
      perror("[yasock_io] Cannot select:");
      break;
    }
    if (ready == 0) {
      continue;
    }
    // We have no error and sd set
    // Check if we have new connection
    if (FD_ISSET(listen_sd, &rfds)) {
      rc = yasock_accept(listen_sd, sock_env);
    }
    // Process read/write for client connections
    for (i = 0; i < sock_env->backlog; i++) {
      if (sock_env->list_sd[i] >= 0 && FD_ISSET(sock_env->list_sd[i], &rfds)) {
      }
    }
  } // End while(1)
  
  return rc;
}


int			yasock_accept(int listen_sd, sock_env_t *sock_env)
{
  int			rc = 0;
  int			new_con_sd = -1;
  struct sockaddr_in	new_con;
  socklen_t		new_con_len = 0;
  char			ip_buf[ YASOCK_DFT_IP_BUFSIZE ];
  int			i = 0;
  
  if (listen_sd < 0 || !sock_env) {
    return -1;
  }
  // Accept a new connection
  new_con_len = sizeof(struct sockaddr_in);
  if ((new_con_sd = accept(listen_sd, (struct sockaddr*)&new_con, &new_con_len)) < -1) {
    perror("Cannot accept a new connection");
  }
  // Verbose message for accepting a new connexion
  inet_ntop(sock_env->af_family, (const void*)&(new_con.sin_addr), ip_buf, YASOCK_DFT_IP_BUFSIZE);
  printf("Accepting connection from %s:%u\n", ip_buf, ntohs(new_con.sin_port));
  // Add new sd in the sd list. First empty place
  for (i = 0; i < sock_env->backlog; i++) {
    if (*sock_env->list_sd < 0) {
      *sock_env->list_sd = new_con_sd;
    }
  }
  // performs sleep after accept
  if (sock_env->init_sleep > 0) {
    usleep(sock_env->init_sleep);
  }
  return rc;
}
