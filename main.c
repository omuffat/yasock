
#include	"yasock.h"

/*
 *	List of available options for program
 *
 *	if options is available in short format, don't forget to update YASOCK_OPTSTRING
 *
 */
static	struct option	tab_options[] = {
  { "client",			no_argument,		0,	'c' },
  { YASOCK_TCPINFO_OPT,		required_argument,	0,	'd' },
  { YASOCK_SHUTDOWN_OPT,	no_argument,		0,	'h' },
  { YASOCK_INTERACTIVE_OPT,	no_argument,		0,	'i' },
  { "write-count",		required_argument,	0,	'n' },
  { "sleep-rw",			required_argument,	0,	'p' },
  { "read-buffer",		required_argument,	0,	'r' },
  { "server",			no_argument,		0,	's' },
  { "verbose",			no_argument,		0,	'v' },
  { "write-buffer",		required_argument,	0,	'w' },
  { YASOCK_RCVTIMEO_OPT,	required_argument,	0,	'x' },
  { YASOCK_SNDTIMEO_OPT,	required_argument,	0,	'y' },
  { YASOCK_LINGER_OPT,		required_argument,	0,	'L' },
  { "no-delay",			no_argument,		0,	'N' },
  { "sleep-listen",		required_argument,	0,	'O' },
  { "first-sleep-rw",		required_argument,	0,	'P' },
  { "sleep-fin",		required_argument,	0,	'Q' },
  { "rcv-buf",			required_argument,	0,	'R' },
  { "snd-buf",			required_argument,	0,	'S' },
  { "mss",			required_argument,	0,	'X' },
  { YASOCK_VERSION_OPT,		no_argument,		0,	0 },
  { YASOCK_HELP_OPT,		no_argument,		0,	0 },
  { YASOCK_SRCPORT_OPT,		required_argument,	0,	0 },
  { 0,				0,			0,	0 }
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
    break;
    // one of option -s or -c is mandatory
  default:
    fprintf(stderr, "Don't known what to launch: client or server ?\n");
    yasock_print_usage();
    rc = -1;
    break;
  }
  yasock_clean_env(&sock_env);
  if (rc < 0) {
    exit(42);
  }
  exit(0);
}

int			yasock_init_env(sock_env_t **sock_env) {
  int			rc = 0;
  struct timeval	dft_select_timeout = YASOCK_SELECT_TIMEOUT;
  
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
  (*sock_env)->select_timeout = dft_select_timeout;
  // Defaults to interactive
  YASOCK_SET_FLAG((*sock_env)->opt_flags, YASOCK_INTERACTIVE_FLAG);
  return rc;
}
int			yasock_clean_env(sock_env_t **sock_env) {
  int			rc = 0;

  if (!sock_env) {
    return -1;
  }
  if (*sock_env) {
    if ((*sock_env)->tcpi_file) {
      free((*sock_env)->tcpi_file);
    }
    if ((*sock_env)->ip_addr) {
      free((*sock_env)->ip_addr);
    }
    if ((*sock_env)->mcast_addr) {
      free((*sock_env)->mcast_addr);
    }
    if ((*sock_env)->list_sd) {
      free((*sock_env)->list_sd);
    }
    free(*sock_env);
    *sock_env = NULL;
  }
  return rc;
}

int			yasock_parse_options(int argc, char **argv, sock_env_t *sock_env) {
  int			rc = 0;
  int			opt = 0;
  int			option_index = -1;

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
      // Source port
      if (tab_options[option_index].name &&
	  !strcmp(tab_options[option_index].name, YASOCK_SRCPORT_OPT)) {
	sock_env->sport = atoi(optarg);
	continue;
      }
      // Unknown long option, return error
      return -1;
      // Client mode
    case 'c':
      sock_env->mode = YASOCK_SOCK_CLIENT;
      if (optarg) {
	sock_env->ip_addr = strdup(optarg);
      }
      break;
      // TCP_INFO debug
    case 'd':
      if (optarg) {
	YASOCK_SET_FLAG(sock_env->opt_flags, YASOCK_TCPINFO_FLAG);
	sock_env->tcpi_file = strdup(optarg);
      }
      break;
      // Half close
    case 'h':
      YASOCK_SET_FLAG(sock_env->opt_flags, YASOCK_SHUTDOWN_FLAG);
      break;
      // source client/sink server ==> bulk transfer. Disable default interactive transfer
    case 'i':
      YASOCK_CLEAR_FLAG(sock_env->opt_flags, YASOCK_INTERACTIVE_FLAG);
      break;
      // Write Count
    case 'n':
      if (optarg) {
	sock_env->wr_count = atoi(optarg);
      }
      break;
      // #ms to pause after each read/write
    case 'p':
      if (optarg) {
	sock_env->rw_sleep = atoi(optarg) * 1000;
      }
      break;
      // Read buffer
    case 'r':
      if (optarg) {
	sock_env->rd_buf_size = atoi(optarg);
      }
      break;
      // Server mode
    case 's':
      sock_env->mode = YASOCK_SOCK_SERVER;
      break;
      // Verbose
    case 'v':
      YASOCK_SET_FLAG(sock_env->opt_flags, YASOCK_VERBOSE_FLAG);
      break;
      // Write buffer
    case 'w':
      if (optarg) {
	sock_env->wr_buf_size = atoi(optarg);
      }
      break;
      // RECVTIMEO
    case 'x':
      if (optarg) {
	sock_env->recv_timeout = atoi(optarg);
      }
      break;
      // SNDTIMEO
    case 'y':
      if (optarg) {
	sock_env->snd_timeout = atoi(optarg);
      }
      break;
      // SO_LINGER socket option (see socket(7))
    case 'L':
      if (optarg) {
	sock_env->linger = atoi(optarg);
      }
      break;
      // NO_DELAY tcp socket option (disable nagle algorithm)
    case 'N':
      YASOCK_SET_FLAG(sock_env->opt_flags, YASOCK_NODELAY_FLAG);
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
	sock_env->init_sleep = atoi(optarg) * 1000;
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
  // First non option for server and client should be the port
  if (optind < argc && argv[optind]) {
    sock_env->port = atoi(argv[optind]);
  }
  return rc;
}

void		yasock_print_version(void) {
  printf("%s (%s) %s\n", PACKAGE, PACKAGE_NAME, PACKAGE_VERSION);
}

void		yasock_print_help(void) {
  // Print Description
  printf("yasock is a client/server program that can manipulate TCP/IP socket properties. yasock can operate as client or as server and uses the port %u by default.\n", YASOCK_DEFAULT_PORT);
  printf("Default operation is to wait for input from stdin and write it to peer, the connection is ended by typing Ctrl-D on one side.\n\n");
  printf("On source/sink transfer (option -i), server mode reads arbitrary data from client and discards it.\n");
  printf("Client mode connects to the server program, sends arbitrary data to it and exits when done. It can seen as a bulk transfer.\n");
  printf("\nOptions below are used to modify the behavior of the TCP/IP socket.\n");
  printf("\n");
  // Print usage for options
  yasock_print_usage();
}

void		yasock_print_usage(void) {
  printf("usage:  Server: %s [options] -s [port]\n\tClient: %s [options] -c <IP addr> [port]\n\n",
	 PACKAGE, PACKAGE);
  printf(" --help: Prints program description and exits\n");
  printf(" --version: Prints version of program and exits\n");
  printf(" -d f:  get tcp_info and write results in csv file 'f' (should not be used with pauses (read/write/accept/connect))\n");
  printf(" -h:    half closes TCP connexion on EOF input from stdin\n");
  printf(" -i:    \"source\" data to socket, \"sink\" data from socket (w/-s)\n");
  printf(" -n n:  #buffers to write for \"source\" client (default %u)\n", YASOCK_DEFAULT_WR_COUNT);
  printf(" -p n:  #ms to pause between each read or write (source/sink)\n");
  printf(" -r n:  #bytes per read() for \"sink\" server (default %u)\n", YASOCK_DFT_READ_BUFSIZE);
  printf(" -v:    verbose\n");
  printf(" -w n:  #bytes per write() for \"source\" server (default %u)\n", YASOCK_DFT_WRITE_BUFSIZE);
  printf(" -x n   #ms for SO_RCVTIMEO (receive timeout)\n");
  printf(" -y n   #ms for SO_SNDTIMEO (send timeout)\n");
#ifdef	HAVE_SO_LINGER_H
  printf(" -L n   SO_LINGER option, n = linger time (in seconds)\n");
#endif	// HAVE_SO_LINGER_H
  printf(" -N:    TCP_NODELAY option (Disable Nagle's algorithm)\n");
  printf(" -O n:  #ms to pause after listen, but before first accept\n");
  printf(" -P n:  #ms to pause before first read or write (source/sink)\n");
  printf(" -Q n:  #ms to pause after receiving FIN, but before close\n");
  printf(" -R n:  Set the #bytes for the socket receive buffer (SO_RCVBUF option). Some Kernels double this value.\n");
  printf(" -S n:  Set the #bytes for the socket sending buffer (SO_SNDBUF option). Some Kernels double this value.\n");
#ifdef	HAVE_TCP_MAXSEG_H
  printf(" -X n:  Set the Maximum Segment Size (in bytes)\n");
#endif	// HAVE_TCP_MAXSEG_H
  printf(" --%s:  Set the source port for the client\n", YASOCK_SRCPORT_OPT);
  //printf(" \n");
  printf("\nReport bugs to <contact@comoe-networks.com>.\n");
}


/**
   usage: sock [ options ] <host> <port>       (for client; default)
   sock [ options ] -s [ <IPaddr> ] <port>     (for server)
   sock [ options ] -i <host> <port>           (for "source" client)
   sock [ options ] -i -s [ <IPaddr> ] <port>  (for "sink" server)
   options: -b n  bind n as client's local port number
   -c    convert newline to CR/LF & vice versa (Replaced by client mode)
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
   -C    set terminal to cbreak mode (Replaced by selection of congestion control algorithm)
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
