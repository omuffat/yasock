
#include	"yasock.h"

/*
 *	List of available options for program
 *
 *	if options is available in short format, don't forget to update YASOCK_OPTSTRING
 *
 */
static	struct option	tab_options[] = {
  { "client",		no_argument,		0,	'c' },
  { "write-count",	required_argument,	0,	'n' },
  { "sleep-rw",		required_argument,	0,	'p' },
  { "server",		no_argument,		0,	's' },
  { "read-buffer",	required_argument,	0,	'r' },
  { "write-buffer",	required_argument,	0,	'w' },
  { "verbose",		no_argument,		0,	'v' },
  { "first-sleep-rw",	required_argument,	0,	'P' },
  { "sleep-fin",	required_argument,	0,	'Q' },
  { "sleep-listen",	required_argument,	0,	'O' },
  { "rcv-buf",		required_argument,	0,	'R' },
  { "snd-buf",		required_argument,	0,	'S' },
  { "no-delay",		no_argument,		0,	'N' },
  { "mss",		required_argument,	0,	'X' },
  { YASOCK_VERSION_OPT,	no_argument,		0,	0 },
  { YASOCK_HELP_OPT,	no_argument,		0,	0 },
  { 0,			0,			0,	0 }
};

int		main(int argc, char **argv) {
  int		rc = 0;
  sock_env_t	*sock_env = NULL;

#ifdef	YASOCK_DEBUG
  fprintf(stderr, "Warning: %s is in debug mode\n", PACKAGE);
#endif	// YASOCK_DEBUG
  rc = yasock_init_env(&sock_env);
  if (rc != 0) {
    fprintf(stderr, "Error while initializing environment, aborting\n");
    exit(1);
  }
  if (yasock_parse_options(argc, argv, sock_env) < 0) {
    yasock_print_usage();
    yasock_clean_env(&sock_env);
    exit(2);
  }
  switch (sock_env->mode) {
  case YASOCK_SOCK_CLIENT:
    rc = yasock_launch_client(sock_env);
    break;
  case YASOCK_SOCK_SERVER:
    rc = yasock_launch_server(sock_env);
    break;
    // Print Version
  case YASOCK_SOCK_VERSION:
    yasock_print_version();
    break;
  case YASOCK_SOCK_HELP:
    yasock_print_help();
  default:
    break;
  }
  yasock_clean_env(&sock_env);
  if (rc < 0) {
    exit(42);
  }
  exit(0);
}

int		yasock_init_env(sock_env_t **sock_env) {
  int		rc = 0;

  if (!sock_env) {
    fprintf(stderr, "Cannot initialize a null structure pointer\n");
    return -1;
  }
  if ((*sock_env = malloc(sizeof(struct sock_env_s))) == NULL) {
    fprintf(stderr, "Cannot malloc general environment (%lu bytes)\n", sizeof(struct sock_env_s));
    return -1;
  }
  memset(*sock_env, 0, sizeof(struct sock_env_s));
  (*sock_env)->rd_buf_size = YASOCK_DFT_READ_BUFSIZE;
  (*sock_env)->wr_buf_size = YASOCK_DFT_WRITE_BUFSIZE;
  (*sock_env)->backlog = YASOCK_DEFAULT_BACKLOG;
  (*sock_env)->port = YASOCK_DEFAULT_PORT;
  (*sock_env)->af_family = AF_INET;
  (*sock_env)->wr_count = YASOCK_DEFAULT_WR_COUNT;
  (*sock_env)->ttl = YASOCK_DEFAULT_TTL;
  (*sock_env)->tos = YASOCK_DEFAULT_TOS;
  return rc;
}
int		yasock_clean_env(sock_env_t **sock_env) {
  int		rc = 0;

  if (!sock_env) {
    return -1;
  }
  if (*sock_env) {
    if ((*sock_env)->ip_addr) {
      free((*sock_env)->ip_addr);
    }
    if ((*sock_env)->mcast_addr) {
      free((*sock_env)->mcast_addr);
    }
    free(*sock_env);
    *sock_env = NULL;
  }
  return rc;
}

int		yasock_parse_options(int argc, char **argv, sock_env_t *sock_env) {
  int		rc = 0;
  int		opt = 0;
  int		option_index = -1;

  if (!argc || !argv || !sock_env) {
    return -1;
  }
  while ((opt = getopt_long(argc, argv, YASOCK_OPTSTRING, tab_options, &option_index)) != -1) {
    switch (opt) {
      // Long option that has no associated short option
    case 0:
      // Version option
      if (tab_options[option_index].name &&
	  !strcmp(tab_options[option_index].name, YASOCK_VERSION_OPT)) {
	sock_env->mode = YASOCK_SOCK_VERSION;
	return 0;
      }
      if (tab_options[option_index].name &&
	  !strcmp(tab_options[option_index].name, YASOCK_HELP_OPT)) {
	sock_env->mode = YASOCK_SOCK_HELP;
	return 0;
      }
      // Unknown long option, return error
      return -1;
      // Server mode
    case 's':
      sock_env->mode = YASOCK_SOCK_SERVER;
      break;
      // Client mode
    case 'c':
      sock_env->mode = YASOCK_SOCK_CLIENT;
      break;
      // Write Count
    case 'n':
      if (optarg) {
	sock_env->wr_count = atoi(optarg);
      }
      break;
      // #ms to pause between each read/write
    case 'p':
      if (optarg) {
	sock_env->read_sleep = sock_env->write_sleep = atoi(optarg) * 1000;
      }
      break;
      // Read buffer
    case 'r':
      if (optarg) {
	sock_env->rd_buf_size = atoi(optarg);
      }
      break;
      // Verbose
    case 'v':
      sock_env->verbose++;
      break;
      // Write buffer
    case 'w':
      if (optarg) {
	sock_env->wr_buf_size = atoi(optarg);
      }
      break;
      // NO_DELAY tcp socket option (disable nagle algorithm)
    case 'N':
      sock_env->no_delay = 1;
      break;
      // #ms to pause after listen, but before first accept
    case 'O':
      if (optarg) {
	sock_env->listen_sleep = atoi(optarg) * 1000;
      }
      break;
      // #ms to pause before first read/write
    case 'P':
      if (optarg) {
	sock_env->first_read_sleep = sock_env->first_write_sleep = atoi(optarg) * 1000;
      }
      break;
      // #ms to pause before close
    case 'Q':
      if (optarg) {
	sock_env->fin_sleep = atoi(optarg) * 1000;
      }
      break;
      // SO_RCVBUF size (in bytes)
    case 'R':
      if (optarg) {
	sock_env->so_rcvbuf = atoi(optarg);
      }
      break;
      // SO_SNDBUF size (in bytes)
    case 'S':
      if (optarg) {
	sock_env->so_sndbuf = atoi(optarg);
      }
      break;
      // MSS (in bytes)
#ifdef	HAVE_TCP_MAXSEG_H
    case 'X':
      if (optarg) {
	sock_env->mss = atoi(optarg);
      }
      break;
#endif	// HAVE_TCP_MAXSEG_H
    default:
      rc = -1;
      break;
    }
  }
  // Process non option argument
  switch (sock_env->mode) {
  case YASOCK_SOCK_VERSION:
  case YASOCK_SOCK_HELP:
    // Should not be here, but in case of
#ifdef	YASOCK_DEBUG
    fprintf(stderr, "Mode %u is version or help. But we didn't get out of parse_options\n");
#endif	// YASOCK_DEBUG
    break;
    // First non option for the client mode must be ip address of server
  case YASOCK_SOCK_CLIENT:
    if (optind < argc && argv[optind]) {
      sock_env->ip_addr = strdup(argv[optind++]);
    }
    break;
  case YASOCK_SOCK_SERVER:
    break;
  // one of option -s or -c is mandatory
  default:
    fprintf(stderr, "Don't known what to launch: client or server ?\n");
    rc = -1;
    break;
  }
  // First non option for server and client should be the port
  if (optind < argc && argv[optind]) {
    sock_env->port = atoi(argv[optind]);
  }
  if (sock_env->mode == YASOCK_SOCK_CLIENT && !sock_env->ip_addr) {
    fprintf(stderr, "Don't know where to connect\n");
    rc = -1;
  }
  return rc;
}

void		yasock_print_version(void) {
  printf("%s (%s) %s\n", PACKAGE, PACKAGE_NAME, PACKAGE_VERSION);
}

void		yasock_print_help(void) {
  // Print Description
  printf("yasock is a client/server program which aim to manipulate TCP/IP stack properties. yasock can operate as client or as server.\n");
  printf("Server mode listens on port (defaults to %u) and waits for arbitrary data from client.\n", YASOCK_DEFAULT_PORT);
  printf("Client mode connects to the server program, sends arbitrary data to it and exits when done.\n");
  printf("\nOptions below are used to modify the behavior of the TCP/IP stack.\n");
  printf("\n");
  // Print usage for options
  yasock_print_usage();
}

void		yasock_print_usage(void) {
  printf("usage:  Server: %s [options] -s [port]\n\tClient: %s [options] -c <IP addr> [port]\n\n",
	 PACKAGE, PACKAGE);
  printf(" --help: Prints program description and exits\n");
  printf(" --version: Prints version of program and exits\n");
  printf(" -v: Verbose\n");
  printf(" -r n:  #bytes per read() for \"sink\" server (default %u)\n", YASOCK_DFT_READ_BUFSIZE);
  printf(" -w n:  #bytes per write() for \"source\" server (default %u)\n", YASOCK_DFT_WRITE_BUFSIZE);
  printf(" -n n:  #buffers to write for \"source\" client (default %u)\n", YASOCK_DEFAULT_WR_COUNT);
  printf(" -N:    TCP_NODELAY option (Disable Nagle's algorithm)\n");
  printf(" -P n:  #ms to pause before first read or write (source/sink)\n");
  printf(" -p n:  #ms to pause between each read or write (source/sink)\n");
  printf(" -Q n:  #ms to pause after receiving FIN, but before close\n");
  printf(" -O n:  #ms to pause after listen, but before first accept\n");
  printf(" -R n:  Set the #bytes for the socket receive buffer (SO_RCVBUF option). Some Kernels double this value.\n");
  printf(" -S n:  Set the #bytes for the socket sending buffer (SO_SNDBUF option). Some Kernels double this value.\n");
#ifdef	HAVE_TCP_MAXSEG_H
  printf(" -X n:  Set the Maximum Segment Size (in bytes)\n");
#endif	// HAVE_TCP_MAXSEG_H
  //printf(" \n");
  printf("\nReport bugs to <contact@comoe-networks.com>.\n");
}


/**
usage: sock [ options ] <host> <port>       (for client; default)
       sock [ options ] -s [ <IPaddr> ] <port>     (for server)
       sock [ options ] -i <host> <port>           (for "source" client)
       sock [ options ] -i -s [ <IPaddr> ] <port>  (for "sink" server)
       options: -b n  bind n as client's local port number
         -c    convert newline to CR/LF & vice versa (Replaced by selection of congestion algorithm)
         -f a.b.c.d.p  foreign IP address = a.b.c.d, foreign port# = p
         -g a.b.c.d  loose source route
         -h    issue TCP half close on standard input EOF
         -i    "source" data to socket, "sink" data from socket (w/-s)
         -j a.b.c.d  join multicast group
         -k    write or writev in chunks
         -l a.b.c.d.p  client's local IP address = a.b.c.d, local port# = p
         -n n  #buffers to write for "source" client (default 1024)
         -o    do NOT connect UDP client
         -p n  #ms to pause before each read or write (source/sink)
         -q n  size of listen queue for TCP server (default 5)
         -r n  #bytes per read() for "sink" server (default 1024)
         -s    operate as server instead of client
         -t n  set multicast ttl
         -u    use UDP instead of TCP
         -v    verbose
         -w n  #bytes per write() for "source" client (default 1024)
         -x n  #ms for SO_RCVTIMEO (receive timeout)
         -y n  #ms for SO_SNDTIMEO (send timeout)
         -A    SO_REUSEADDR option
         -B    SO_BROADCAST option
         -C    set terminal to cbreak mode
         -D    SO_DEBUG option
         -E    IP_RECVDSTADDR option
         -F    fork after connection accepted (TCP concurrent server)
         -G a.b.c.d  strict source route
         -H n  IP_TOS option (16=min del, 8=max thru, 4=max rel, 2=min$)
         -I    SIGIO signal
         -J n  IP_TTL option
         -K    SO_KEEPALIVE option
         -L n  SO_LINGER option, n = linger time (in seconds)
         -N    TCP_NODELAY option
         -O n  #ms to pause after listen, but before first accept
         -P n  #ms to pause before first read or write (source/sink)
         -Q n  #ms to pause after receiving FIN, but before close
         -R n  SO_RCVBUF option
         -S n  SO_SNDBUF option
         -T    SO_REUSEPORT option
         -U n  enter urgent mode before write number n (source only)
         -V    use writev() instead of write(); enables -k too
         -W    ignore write errors for sink client
         -X n  TCP_MAXSEG option (set MSS)
         -Y    SO_DONTROUTE option
         -Z    MSG_PEEK
	 

 **/
