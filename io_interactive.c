
#include	"yasock.h"

/*
 *	Read data from stdin and send it to sd
 *	Read data from sd and send it to stdout
 *
 *	Until EOF is read on stdin
 *
 *
 */
int		yasock_interactive(int sd, sock_env_t *sock_env) {
  int			rc = 0;
  char			*data_buf = NULL;
  ssize_t		size_read = 0;
  ssize_t		size_write = 0;
  int			nfds; // higher socket descriptor + 1
  fd_set		readfds;
  int			stdin_eof = 0;
  
  if (sd < 0 || !sock_env) {
    return -1;
  }
  // Set buffer for read/write operation
  if ((data_buf = malloc(sock_env->rd_buf_size)) == NULL) {
    fprintf(stderr, "[yasock_srv_readwrite] Cannot malloc %u len data\n", sock_env->rd_buf_size);
    return -1;
  }
  // Reads data
  while (1) {
    //fprintf(stderr, "%s>> ", PACKAGE);
    // Init readfds set
    FD_ZERO(&readfds);
    FD_SET(sd, &readfds);
    if (!stdin_eof) {
      FD_SET(STDIN, &readfds);
    }
    nfds = sd + 1;
    if (select(nfds, &readfds, NULL, NULL, &(sock_env->select_timeout))  < 0) {
      perror("Error while doing select");
      break;
    }
    if (FD_ISSET(STDIN, &readfds)) {
      // Read stdin
      size_read = read(STDIN, (void*)data_buf, sock_env->rd_buf_size);
      if (size_read < 0) {
	perror("Error while reading stdin");
	break;
      }
      // EOF on STDIN
      if (size_read == 0) {
	stdin_eof = 1;
	// If requested, performs half close
	if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_SHUTDOWN_FLAG)) {
	  shutdown(sd, SHUT_WR);
	  if (rc < 0) {
	    perror("[yasock_cli_interactive] shutdown failed");
	  }
	  if (rc >= 0 && YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
	    fprintf(stderr, "[yasock_cli_interactive] half-closed connection\n");
	  }
	}
	fprintf(stderr, "\n");
	// End of loop
	break;
      }
      // We have data to send to peer
      if (size_read > 0) {
	// Send to server what we've got
	if ((size_write = send(sd, (void*)data_buf, size_read, 0)) < 0) {
	  perror("[yasock_interactive] Error while writing to peer");
	  rc = -1;
	  break;
	}
	data_buf[0] = '\0';
      }
    } // End of FD_ISSET(STDIN, ...
    // Check for input from peer
    if (FD_ISSET(sd, &readfds)) {
      // Read back from peer
      size_read = recv(sd, (void*)data_buf, sock_env->rd_buf_size, 0);
      // Recv Error
      if (size_read < 0) {
	perror("[yasock_interactive] Error while reading from peer");
	break;
      }
      // EOF
      if (size_read == 0) {
	if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG)) {
	  fprintf(stderr, "Connexion closed by peer\n");
	}
	break;
      }
      // Print on stdout what we've got
      if (write(STDOUT, data_buf, size_read) < 0) {
	perror("[yasock_interactive] Error while writing ouput got from peer");
	break;
      }
    } // End of FD_ISSET(sd, ...
  } // End of while(1)
  // Clean allocated data
  free(data_buf);
  return 0;
}
