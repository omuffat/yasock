
#include	"yasock.h"

/*
 *	Open tcp_info destination file and write header
 *	The existing content of this file will be truncated
 *
 *	Careful, we use tcp_info from /usr/include/netinet/tcp.h,
 *	not from /usr/include/linux/tcp.h as stated in tcp(7)
 */
int			yasock_prepare_tcpi(sock_env_t *sock_env) {
  int			rc = 0;
  char			buf[ YASOCK_DFT_WRITE_BUFSIZE ];
  int			buflen = 0;

  if (!sock_env || !sock_env->tcpi_file) {
#ifdef	YASOCK_DEBUG
    fprintf(stderr, "[yasock_prepare_tcpi] Invalid parameters\n");
#endif	// YASOCK_DEBUG
    return -1;
  }
  if ((sock_env->tcpi_fd = fopen(sock_env->tcpi_file, "w")) == NULL) {
    snprintf(buf, YASOCK_DFT_WRITE_BUFSIZE, "Cannot open file %s", sock_env->tcpi_file);
    perror(buf);
  }
  buflen = snprintf(buf, YASOCK_DFT_WRITE_BUFSIZE, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
		    "time",
		    "state", "ca_state", "retransmits", "probes", "backoff", "options",
		    "snd_wscale", "rcv_wscale", "rto", "ato", "snd_mss", "rcv_mss",
		    "unacked", "sacked", "lost", "retrans", "fackets",
		    "last_data_sent", "last_ack_sent", "last_data_recv", "last_ack_recv",
		    "pmtu", "rcv_ssthresh", "rtt", "rttvar", "snd_ssthresh", "snd_cwnd", "advmss", "reorder",
		    "rcv_rtt", "rcv_space", "total_retrans");
  fwrite((void*)buf, sizeof(char), buflen, sock_env->tcpi_fd);
  return rc;
}

/*
 *	Get TCP_INFO data about socket sd, and writes it into file sock_env->tcpi_fd
 *
 */
int			yasock_write_tcpi(int sd, sock_env_t *sock_env) {
  int			rc = 0;
  struct tcp_info	tcpi;
  char			buf[ YASOCK_DFT_WRITE_BUFSIZE ];
  int			optlen = 0;
  int			buflen = 0;

  if (!sock_env || !sock_env->tcpi_fd) {
#ifdef	YASOCK_DEBUG
    fprintf(stderr, "[yasock_write_tcpi] Invalid parameters\n");
#endif	// YASOCK_DEBUG
    return -1;
  }
  // Get tcp_info structure
  optlen = sizeof(struct tcp_info);
  memset(&tcpi, 0, sizeof(struct tcp_info));
  if (getsockopt(sd, IPPROTO_TCP, TCP_INFO, &tcpi, &optlen) < 0) {
    perror("Cannot get tcp_info data");
    return -1;
  }
#ifdef	YASOCK_DEBUG
  if (optlen != sizeof(struct tcp_info)) {
    fprintf(stderr, "[yasock_write_tcpi] Returned optlen of tcp_info is different from the one given\n");
  }
#endif	// YASOCK_DEBUG
  // output data in csv formatted file
  // 1
  buflen = snprintf(buf, YASOCK_DFT_WRITE_BUFSIZE, "%lu,%u,%u,%u,%u,%u,%u",
		    time(NULL),
		    tcpi.tcpi_state, tcpi.tcpi_ca_state, tcpi.tcpi_retransmits,
		    tcpi.tcpi_probes, tcpi.tcpi_backoff, tcpi.tcpi_options);
  fwrite((void*)buf, sizeof(char), buflen, sock_env->tcpi_fd);
  // 2
  buflen = snprintf(buf, YASOCK_DFT_WRITE_BUFSIZE, ",%u,%u,%u,%u,%u,%u",
		    tcpi.tcpi_snd_wscale, tcpi.tcpi_rcv_wscale, tcpi.tcpi_rto,
		    tcpi.tcpi_ato, tcpi.tcpi_snd_mss, tcpi.tcpi_rcv_mss);
  fwrite((void*)buf, sizeof(char), buflen, sock_env->tcpi_fd);
  // 3
  buflen = snprintf(buf, YASOCK_DFT_WRITE_BUFSIZE, ",%u,%u,%u,%u,%u",
		    tcpi.tcpi_unacked, tcpi.tcpi_sacked, tcpi.tcpi_lost,
		    tcpi.tcpi_retrans, tcpi.tcpi_fackets);
  fwrite((void*)buf, sizeof(char), buflen, sock_env->tcpi_fd);
  // times
  buflen = snprintf(buf, YASOCK_DFT_WRITE_BUFSIZE, ",%u,%u,%u,%u",
		    tcpi.tcpi_last_data_sent, tcpi.tcpi_last_ack_sent,
		    tcpi.tcpi_last_data_recv, tcpi.tcpi_last_ack_recv);
  fwrite((void*)buf, sizeof(char), buflen, sock_env->tcpi_fd);
  // metrics 1
  buflen = snprintf(buf, YASOCK_DFT_WRITE_BUFSIZE, ",%u,%u,%u,%u,%u,%u,%u,%u",
		    tcpi.tcpi_pmtu, tcpi.tcpi_rcv_ssthresh, tcpi.tcpi_rtt,
		    tcpi.tcpi_rttvar, tcpi.tcpi_snd_ssthresh, tcpi.tcpi_snd_cwnd,
		    tcpi.tcpi_advmss, tcpi.tcpi_reordering);
  fwrite((void*)buf, sizeof(char), buflen, sock_env->tcpi_fd);
  // metrics 2
  buflen = snprintf(buf, YASOCK_DFT_WRITE_BUFSIZE, ",%u,%u,%u",
		    tcpi.tcpi_rcv_rtt, tcpi.tcpi_rcv_space, tcpi.tcpi_total_retrans);
  fwrite((void*)buf, sizeof(char), buflen, sock_env->tcpi_fd);
  // Write final newline of this line
  fwrite((void*)"\n", sizeof(char), 1, sock_env->tcpi_fd);
  return rc;
}
