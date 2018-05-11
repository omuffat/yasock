
#ifndef	__YASOCK_H__
#define	__YASOCK_H__

#include	<sys/time.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/select.h>
#include	<unistd.h>
#include	<netinet/in.h>
#include	<netinet/ip.h>
#include	<netinet/tcp.h>
#include	<arpa/inet.h>

#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<getopt.h>
#include	<errno.h>

#include	"config.h"

#ifndef	MIN
#define	MIN(x,y)	(((x) < (y)) ? (x) : (y))
#endif	// MIN
#ifndef	MAX
#define	MAX(x,y)	(((x) > (y)) ? (x) : (y))
#endif	// MIN

// Options related
#define	YASOCK_OPTSTRING		"c:hn:p:r:svw:x:y:L:NO:P:Q:R:S:X:"
// version string for comparison in yasock_parse_options
#define	YASOCK_VERSION_OPT		"version"
#define	YASOCK_HELP_OPT			"help"
#define	YASOCK_SHUTDOWN_OPT		"shutdown"
#define	YASOCK_LINGER_OPT		"linger"
#define	YASOCK_RCVTIMEO_OPT		"rtimeout" // x
#define	YASOCK_SNDTIMEO_OPT		"stimeout" // y

// yasock Mode of processing
#define	YASOCK_SOCK_UNKNOWN		0x00
#define	YASOCK_SOCK_CLIENT		0x01
#define	YASOCK_SOCK_SERVER		0x02
#define	YASOCK_SOCK_VERSION		0x03
#define	YASOCK_SOCK_HELP		0x04

// seconds, microsecond
#define	YASOCK_DEFAULT_TIMEOUT		{ 0, 500000 }
// Default number of write
#define	YASOCK_DEFAULT_WR_COUNT		16
// Default char used in write buffer
#define	YASOCK_DEFAULT_WR_CHAR		'A'
// Port information
#define	YASOCK_DEFAULT_PORT		62067
#define	YASOCK_ANY_PORT			0
// IP Default socket options
#define	YASOCK_DEFAULT_TTL		16
#define	YASOCK_DEFAULT_TOS		IPTOS_THROUGHPUT
// Queue value for listen backlog
#define	YASOCK_DEFAULT_BACKLOG		5

// BUFFER SIZE
#define	YASOCK_DFT_IP_BUFSIZE		64
#define	YASOCK_DFT_READ_BUFSIZE		2048
#define	YASOCK_DFT_WRITE_BUFSIZE	2048
#define	YASOCK_MIN_SO_RCVBUF		256
#define	YASOCK_MIN_SO_SNDBUF		2048

// OPTION FLAGS
#define	YASOCK_VERBOSE_FLAG		0x0001
#define	YASOCK_NODELAY_FLAG		0x0002
#define	YASOCK_SHUTDOWN_FLAG		0x0004
// OPTION FLAGS MACROs
#define	YASOCK_SET_FLAG(set, flag)	((set) |= (flag))
#define	YASOCK_ISSET_FLAG(set, flag)	(((set) & (flag)) == (flag))
#define YASOCK_CLEAR_FLAG(set, flag)	((set) &= ~(flag))

typedef	struct		sock_env_s {
  char			*ip_addr;
  unsigned char		af_family;
  unsigned char		mode;
  unsigned short	opt_flags;
  // Sleep part (in microseconds)
  unsigned int		listen_sleep;	// Sleep before accept(2) (for server only)
  unsigned int		first_read_sleep;	// Sleep before first read(2)
  unsigned int		first_write_sleep;	// Sleep before first write(2)
  unsigned int		read_sleep;	// Sleep between read(2)
  unsigned int		write_sleep;	// Sleep between write(2)
  unsigned int		fin_sleep;	// Sleep before close(2)
  // IP Socket option
  unsigned int		ttl;
  unsigned int		tos;
  char			*mcast_addr;
  // Linger option
  int			linger;
  // read/write timeout
  unsigned int		recv_timeout; // in milliseconds
  unsigned int		snd_timeout; // in milliseconds
  // TCP/UDP related
  unsigned short	mss;
  unsigned short	port;
  // Buffer related
  unsigned int		so_rcvbuf;
  unsigned int		so_sndbuf;
  unsigned int		wr_count;	// Number of time that client sends wr_buf_size
  unsigned int		rd_buf_size;
  unsigned int		wr_buf_size;
  unsigned int		backlog;	// Queue size for server listen(2) call
}			sock_env_t;

/*
 *	main.c
 */
int		yasock_init_env(sock_env_t**);
int		yasock_clean_env(sock_env_t**);
int		yasock_parse_options(int, char**, sock_env_t*);
void		yasock_print_help(void);
void		yasock_print_usage(void);
void		yasock_print_version(void);

/*
 *	server.c
 */
int		yasock_launch_server(sock_env_t*);
int		yasock_srv_readwrite(int, const struct sockaddr*, sock_env_t*);
int		yasock_srv_readonly(int, const struct sockaddr*, sock_env_t*);

/*
 *	client.c
 */
int		yasock_launch_client(sock_env_t*);
int		yasock_cli_writeread(int, sock_env_t*);
int		yasock_cli_writeread_sendall_first(int, sock_env_t*);
int		yasock_cli_writeonly(int, sock_env_t*);

/*
 *	socket_opt.c
 */
// all options socket
int		yasock_set_socket_options(int, sock_env_t*);
// socket level socket options
int		yasock_set_socket_sockopt(int, sock_env_t*);
// ip level socket options
int		yasock_set_socket_ipopt(int, sock_env_t*);
// tcp level socket options
int		yasock_set_socket_tcpopt(int, sock_env_t*);

#endif	/* __YASOCK_H__ */
