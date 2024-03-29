
#include	"yasock.h"

int			yasock_set_socket_options(int sd, sock_env_t *sock_env) {
  int			rc = 0;

  if (sd < 0 || !sock_env) {
    return -1;
  }
  if (yasock_set_socket_sockopt(sd, sock_env) < 0) {
    return -1;
  }
  if (yasock_set_socket_ipopt(sd, sock_env) < 0) {
    return -1;
  }
  if (yasock_set_socket_tcpopt(sd, sock_env) < 0) {
    return -1;
  }
  return rc;
}
/*
 *	Socket level options: SOL_SOCKET
 *	man socket(7)
 *
 */
int			yasock_set_socket_sockopt(int sd, sock_env_t *sock_env) {
  int			rc = 0;
  int			value = 0;
#ifdef	HAVE_SO_LINGER_H
  struct linger		linger_opt = { 0, 0 };
#endif	// HAVE_SO_LINGER_H
  struct timeval	tv = { 0, 0 };

  if (sd < 0 || !sock_env) {
    return -1;
  }
  // Set receive buffer size
  if (sock_env->so_rcvbuf) {
    // receive buffer value must not be less than 256
    value = MAX(sock_env->so_rcvbuf, YASOCK_MIN_SO_RCVBUF);
    rc = setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &value, sizeof(int));
    if (rc < 0) {
      perror("[yasock_set_socket_opt] failed to set SO_RCVBUF");
    }
  }
  // Set send buffer size
  if (sock_env->so_sndbuf) {
    // send buffer value must not be less than 2048
    value = MAX(sock_env->so_sndbuf, YASOCK_MIN_SO_SNDBUF);
    rc = setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &value, sizeof(int));
    if (rc < 0) {
      perror("[yasock_set_socket_opt] failed to set SO_SNDBUF");
    }
  }
#ifdef	HAVE_SO_LINGER_H
  // Linger option
  if (sock_env->linger > 0) {
    linger_opt.l_onoff = 1;
    linger_opt.l_linger = sock_env->linger;
    rc = setsockopt(sd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(struct linger));
    if (rc < 0) {
      perror("[yasock_set_socket_opt] failed to set SO_LINGER");
    }
  }
#endif	// HAVE_SO_LINGER_H
  // RECV TIMEOUT OPTION
  if (sock_env->recv_timeout) {
    tv.tv_sec = (unsigned int)(sock_env->recv_timeout / (unsigned int)1000);
    tv.tv_usec = (unsigned int)((sock_env->recv_timeout % (unsigned int)1000) * 1000);
    rc = setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
    if (rc < 0) {
      perror("[yasock_set_socket_opt] failed to set SO_RCVTIMEO");
    }
  }
  // SND TIMEOUT OPTION
  if (sock_env->snd_timeout) {
    tv.tv_sec = (unsigned int)(sock_env->snd_timeout / (unsigned int)1000);
    tv.tv_usec = (unsigned int)((sock_env->snd_timeout % (unsigned int)1000) * 1000);
    rc = setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
    if (rc < 0) {
      perror("[yasock_set_socket_opt] failed to set SO_SNDTIMEO");
    }
  }
  return rc;
}

/*
 *	IP Socket level options: IPPROTO_IP
 *	man ip(7)
 *
 */
int			yasock_set_socket_ipopt(int sd, sock_env_t *sock_env) {
  int			rc = 0;
  unsigned int		value = 0;
#ifdef	HAVE_IP_MREQ_H
  struct ip_mreq	ip_mreq;
#endif /* HAVE_IP_MREQ_H */

  if (sd < 0 || !sock_env) {
    return -1;
  }
  // Set TTL
  value = sock_env->ttl;
  rc = setsockopt(sd, IPPROTO_IP, IP_TTL, &value, sizeof(unsigned int));
  if (rc < 0) {
    perror("[yasock_set_socket_ipopt] failed to set TTL");
  }
  // Set ToS
  value = sock_env->tos;
  rc = setsockopt(sd, IPPROTO_IP, IP_TOS, &value, sizeof(unsigned int));
  if (rc < 0) {
    perror("[yasock_set_socket_ipopt] failed to set TOS");
  }
#ifdef	HAVE_IP_MREQ_H
  // Join multicast group if set
  if (sock_env->mcast_addr) {
    inet_pton(sock_env->af_family, sock_env->mcast_addr, &(ip_mreq.imr_multiaddr));
    ip_mreq.imr_interface.s_addr = INADDR_ANY;
    rc = setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &ip_mreq, sizeof(struct ip_mreq));
    if (rc < 0) {
      perror("[yasock_set_socket_ipopt] failed to set Multicast IPv4 Address");
    }
  }
#endif /* HAVE_IP_MREQ_H */
  //setsockopt(sd, IPPROTO_IP, , &value, sizeof(unsigned int));
  return rc;
}

/*
 *	TCP Socket options: IPPROTO_TCP
 *	man tcp(7)
 *
 */
int			yasock_set_socket_tcpopt(int sd, sock_env_t *sock_env) {
  int			rc = 0;
  unsigned int		value = 0;

  if (sd < 0 || !sock_env) {
    return -1;
  }
  if (YASOCK_ISSET_FLAG(sock_env->opt_flags, YASOCK_NODELAY_FLAG)) {
    value = 1;
    rc = setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(unsigned int));
    if (rc < 0) {
      perror("[yasock_set_socket_tcpopt] Could not disable Nagle algorithm");
    }
  }
#ifdef	HAVE_TCP_CONGESTION_H
  if (sock_env->congestion_algo) {
    rc = setsockopt(sd, IPPROTO_TCP, TCP_CONGESTION, sock_env->congestion_algo,
		    strlen(sock_env->congestion_algo));
    if (rc < 0) {
      perror("[yasock_set_socket_tcpopt] Could not set the specified congestion algorithm");
    }
  }
#endif	// HAVE_TCP_CONGESTION_H
#ifdef	HAVE_TCP_MAXSEG_H
  /*
   *	on *BSD systems, net.inet.tcp.mssdflt is usually set to 536.
   *	So, you can't set a MSS > 536 if you don't set this value higher (like 1460
   *	on Ethernet link).
   *	See sysctl variable: net.inet.tcp.mssdflt
   *	See also tcp_default_ctloutput routine in sys/netinet/tcp_usrreq.c
   *	See also tcp_mss_update, tcp_mss, tcp_mssopt in sys/netinet/tcp_input.c
   *
   */
  if (sock_env->mss) {
    value = (unsigned int)sock_env->mss;
    rc = setsockopt(sd, IPPROTO_TCP, TCP_MAXSEG, &value, sizeof(unsigned int));
    if (rc < 0) {
      perror("[yasock_set_socket_tcpopt] Could not set Maximum Segment Size");
    }
  }
#endif	// HAVE_TCP_MAXSEG_H
  // User Timeout
  if (sock_env->user_timeout) {
    value = (unsigned int)sock_env->user_timeout;
    rc = setsockopt(sd, IPPROTO_TCP, TCP_USER_TIMEOUT, &value, sizeof(unsigned int));
    if (rc < 0) {
      perror("[yasock_set_socket_tcpopt] Could not set User Timeout Value");
    }
  }
  
  return rc;
}
