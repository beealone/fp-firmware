//***************************************************************************************
//	作	成：		周赫			2004204276		pro1
//
//	作成时间：		2005/03/16
//
//  traceroute.h - 定义数据结构及实现具体的函数
//
//	修改时间：
//
//  编译方法：gcc -o mytraceroute traceroute.c ERREXIT.c
//								
//***************************************************************************************
#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#include <arpa/inet.h>
#define LOG_TAG "TRACEROUTE"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <memory.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "arca.h"

/* rfc1716 */
#ifndef ICMP_UNREACH_FILTER_PROHIB
#define ICMP_UNREACH_FILTER_PROHIB	13	/* admin prohibited filter */
#endif
#ifndef ICMP_UNREACH_HOST_PRECEDENCE
#define ICMP_UNREACH_HOST_PRECEDENCE	14	/* host precedence violation */
#endif
#ifndef ICMP_UNREACH_PRECEDENCE_CUTOFF
#define ICMP_UNREACH_PRECEDENCE_CUTOFF	15	/* precedence cutoff */
#endif

/* Maximum number of gateways (include room for one noop) */
#define NGATEWAYS ((int)((MAX_IPOPTLEN - IPOPT_MINOFF - 1) / sizeof(u_int32_t)))

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	64
#endif


#define ERREXIT(format,arg...) do { printf(format, ## arg); return 0;} while (0)



/* Host name and address list */
struct hostinfo {
	char *name;
	int n;
	u_int32_t *addrs;
};

/* UDP包的数据部分，即payload */
struct outdata {
	u_char seq;		/* sequence number of this packet */
	u_char ttl;		/* ttl packet left with */
	struct timeval tv;	/* time packet left */
};



u_char	packet[512];		/* 接收返回的icmp包的缓冲区 */

struct ip *outip;		/* 最新发送的udp包的ip头 */
struct udphdr *outudp;		/* 最新发送的udp包的udp头 */
struct outdata *outdata;	/* 最新发送的udp包的数据部分 */



/* 松散源路由中间路由节点地址表，包括目的节点地址 */
u_int32_t gwlist[NGATEWAYS + 1];

int s;				/* 接收icmp包的socket描述符 */
int sndsock;			/* 发送udp包的socket描述符 */

struct sockaddr whereto;	/* Who to try to reach */
struct sockaddr wherefrom;	/* Who we are */
int packlen;			/* total length of packet */
int minpacket;			/* min ip packet size */
int maxpacket = 32 * 1024;	/* max ip packet size */

char *prog;
char *source;
char *hostname;

int nprobes = 3;		/* 发送udp探测包的次数 */
int max_ttl = 30;
int first_ttl = 1;
u_short ident;
u_short port = 32768 + 666;	/* start udp dest port # for probe packets */

int options;			/* socket options */
int waittime = 3;		/* time to wait for response (in seconds) */
int nflag;			/* print addresses numerically */

int optlen;			/* length of ip options */

extern int optind;
extern int opterr;
extern char *optarg;


double	deltaT(struct timeval *, struct timeval *);
void	freehostinfo(struct hostinfo *);
void	getaddr(u_int32_t *, char *);
struct	hostinfo *gethostinfo(char *);
u_short	in_cksum(u_short *, int);
char	*inetname(struct in_addr);
int		packet_ok(u_char *, int, struct sockaddr_in *, int);
char	*pr_type(u_char);
void	print(u_char *, int, struct sockaddr_in *);
void	send_probe(int, int, struct timeval *);
int		str2val(const char *, const char *, int, int);
void	tvsub(struct timeval *, struct timeval *);
int		wait_for_reply(int, struct sockaddr_in *, const struct timeval *);
void	setsin(register struct sockaddr_in *sin, register u_int32_t addr);


/*------------------------------------------------------------------------

 * wait_for_reply - 在指定时间waittime内等待传回的icmp数据包，如未获得，则返回0

 *------------------------------------------------------------------------

*/
int wait_for_reply(register int sock, register struct sockaddr_in *fromp,
		register const struct timeval *tp)
{
	fd_set fds;
	struct timeval now, wait;
	struct timezone tz;
	register int cc = 0;
	unsigned int fromlen = sizeof(*fromp);

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	wait.tv_sec = tp->tv_sec + waittime;
	wait.tv_usec = tp->tv_usec;
	(void)gettimeofday(&now, &tz);

	tvsub(&wait, &now);


	if (select(sock + 1, &fds, NULL, NULL, &wait) > 0)		
		cc = recvfrom(sock, (char *)packet, sizeof(packet), 0, (struct sockaddr *)fromp, &fromlen);

	return(cc);
}


/*------------------------------------------------------------------------

 * send_probe - 发送udp数据包，生存期为ttl，序号为seq

 *------------------------------------------------------------------------

*/
void send_probe(register int seq, int ttl, register struct timeval *tp)
{
	register int cc;
	//register struct udpiphdr *ui, *oui;//udp overlaid ip structure
	//struct ip tip;

	outip->ip_off = 0;
	outip->ip_p = IPPROTO_UDP;
	outip->ip_len = packlen;
	outip->ip_ttl = ttl;
	outip->ip_sum = 0;
	in_cksum((u_short *)outip, sizeof(*outip) + optlen);
	if (outip->ip_sum == 0)
		outip->ip_sum = 0xffff;

	outudp->source = htons(ident);
	outudp->dest = htons(port+seq);
	outudp->check = 0;
	//outudp->check = in_cksum((u_short *), packlen);
	//if (outudp->check == 0)
	//	outudp->check = 0xffff;

	outdata->seq = seq;
	outdata->ttl = ttl;
	outdata->tv = *tp;

	//	printf("\n outip->ip_hl=%d, outip->ip_v=%d, outip->ip_tos=%d, outip->ip_len=%d,",outip->ip_hl,outip->ip_v,outip->ip_tos,outip->ip_len);

	//	printf("\n outip->ip_id=%d, outip->ip_off=%d, outip->ip_ttl=%d, outip->ip_p=%d,",outip->ip_id, outip->ip_off,outip->ip_ttl, outip->ip_p);

	//	printf("\n outip->ip_sum=%d\n", outip->ip_sum);
	// outip->ip_src=%s, outip->ip_dst=%s,",outip->ip_sum, inet_ntoa(outip->ip_src), inet_ntoa(outip->ip_dst));

	cc = sendto(sndsock, (char *)outip,
			packlen, 0, &whereto, sizeof(whereto));
	if (cc < 0 || cc != packlen)  {
		if (cc < 0){
			//ERREXIT( " sendto: %s\n", prog);
			printf("sendto: %s\n", prog);
		}

		printf("%s: wrote %s %d chars, ret=%d\n", prog, hostname, packlen, cc);
		(void)fflush(stdout);
	}
}

/*------------------------------------------------------------------------

 * deltaT - 计算两个时间差值，得一double型的微秒值（ms）

 *------------------------------------------------------------------------

*/
double deltaT(struct timeval *t1p, struct timeval *t2p)
{
	register double dt;

	dt = (double)(t2p->tv_sec - t1p->tv_sec) * 1000.0 +
		(double)(t2p->tv_usec - t1p->tv_usec) / 1000.0;
	return (dt);
}


/*------------------------------------------------------------------------

 * packet_ok - 获得接收的icmp包，根据icmp的type等返回不同的值

 *------------------------------------------------------------------------

*/
int packet_ok(register u_char *buf, int cc, register struct sockaddr_in *from,
		register int seq)
{
	register struct icmp *icp;
	register u_char type, code;
	register int hlen;

#ifndef ARCHAIC
	register struct ip *ip;

	ip = (struct ip *) buf;

	hlen = ip->ip_hl << 2;		/*计算ip包头长*/

	if (cc < hlen + ICMP_MINLEN) 
	{
		printf("packet too short (%d bytes) from %s\n", cc,	inet_ntoa(from->sin_addr));
		return (0);
	}

	//获得icmp头结构
	cc -= hlen;
	icp = (struct icmp *)(buf + hlen);

#else
	icp = (struct icmp *)buf;
#endif

	type = icp->icmp_type;

	code = icp->icmp_code;

	if ((type == ICMP_TIMXCEED && code == ICMP_TIMXCEED_INTRANS) ||
			type == ICMP_UNREACH || type == ICMP_ECHOREPLY) 
	{
		register struct ip *hip;
		register struct udphdr *up;
		//register struct icmp *hicmp;

		hip = &icp->icmp_ip;
		hlen = hip->ip_hl << 2;

		up = (struct udphdr *)((u_char *)hip + hlen);

		/* XXX 8 is a magic number */
		if (hlen + 12 <= cc && hip->ip_p == IPPROTO_UDP &&
				up->source == htons(ident) &&	up->dest == htons(port + seq))

			return (type == ICMP_TIMXCEED ? -1 : code + 1);

	}

	return(0);
}


/*------------------------------------------------------------------------

 * print - 根据nflag的值，打印域名或ip地址，主程序中调用

 *------------------------------------------------------------------------

*/
void print(register u_char *buf, register int cc, register struct sockaddr_in *from)
{
	register struct ip *ip;
	register int hlen;

	ip = (struct ip *) buf;
	hlen = ip->ip_hl << 2;
	cc -= hlen;

	if (nflag)
		printf(" %s", inet_ntoa(from->sin_addr));
	else
		printf(" %s (%s)", inetname(from->sin_addr),
				inet_ntoa(from->sin_addr));

}


/*------------------------------------------------------------------------

 * in_cksum - 校验和函数

 *------------------------------------------------------------------------

*/
u_short in_cksum(register u_short *addr, register int len)
{
	register int nleft = len;
	register u_short *w = addr;
	register u_short answer;
	register int sum = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1)
		sum += *(u_char *)w;

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}


/*------------------------------------------------------------------------

 * tvsub - 计算两个时间差值，存入第一个变量（假设out>= in）

 *------------------------------------------------------------------------

*/
void tvsub(register struct timeval *out, register struct timeval *in)
{

	if ((out->tv_usec -= in->tv_usec) < 0)   {
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}


/*------------------------------------------------------------------------

 * inetname - 获得网络地址，假如nflag为真，则提供数字形式，
 *			  否则则为字符形式，函数print()中调用

 *------------------------------------------------------------------------

*/
char * inetname(struct in_addr in)
{
	register char *cp;
	register struct hostent *hp;
	static int first = 1;
	static char domain[MAXHOSTNAMELEN + 1], line[MAXHOSTNAMELEN + 1];

	if (first && !nflag) {
		first = 0;
		if (gethostname(domain, sizeof(domain) - 1) < 0)
			domain[0] = '\0';
		else {
			cp = strchr(domain, '.');
			if (cp == NULL) {
				hp = gethostbyname(domain);
				if (hp != NULL)
					cp = strchr(hp->h_name, '.');
			}
			if (cp == NULL)
				domain[0] = '\0';
			else {
				++cp;
				(void)strncpy(domain, cp, sizeof(domain) - 1);
				domain[sizeof(domain) - 1] = '\0';
			}
		}
	}
	if (!nflag && in.s_addr != INADDR_ANY) {
		hp = gethostbyaddr((char *)&in, sizeof(in), AF_INET);
		if (hp != NULL) {
			if ((cp = strchr(hp->h_name, '.')) != NULL &&
					strcmp(cp + 1, domain) == 0)
				*cp = '\0';
			(void)strncpy(line, hp->h_name, sizeof(line) - 1);
			line[sizeof(line) - 1] = '\0';
			return (line);
		}
	}
	return (inet_ntoa(in));
}

/*------------------------------------------------------------------------

 * hostinfo - 根据目的节点的hostname填充struct hostinfo结构信息

 *------------------------------------------------------------------------

*/
struct hostinfo * gethostinfo(register char *hostname)
{
	register int n;
	register struct hostent *hp;
	register struct hostinfo *hi;
	register char **p;
	register u_int32_t addr, *ap;

	if (strlen(hostname) > 64) 
		ERREXIT("%s: hostname \"%.32s...\" is too long\n", prog, hostname);		

	hi = calloc(1, sizeof(*hi));
	if (hi == NULL) 
		ERREXIT("%s: calloc %s\n", prog, strerror(errno));

	/*hostname为点分十进制ip地址形式*/
	addr = inet_addr(hostname);
	if ((int32_t)addr != -1) 
	{
		hi->name = strdup(hostname);
		hi->n = 1;
		hi->addrs = calloc(1, sizeof(hi->addrs[0]));

		if (hi->addrs == NULL) 
			ERREXIT("%s: calloc %s\n", prog, strerror(errno));

		hi->addrs[0] = addr;
		return (hi);
	}

	/*hostname为域名形式*/
	hp = gethostbyname(hostname);
	if (hp == NULL) 
		ERREXIT("%s: unknown host %s\n", prog, hostname);

	if (hp->h_addrtype != AF_INET || hp->h_length != 4) 
		ERREXIT("%s: bad host %s\n", prog, hostname);

	hi->name = strdup(hp->h_name);

	for (n = 0, p = hp->h_addr_list; *p != NULL; ++n, ++p)
		continue;

	hi->n = n;
	hi->addrs = calloc(n, sizeof(hi->addrs[0]));
	if (hi->addrs == NULL) 
		ERREXIT("%s: calloc %s\n", prog, strerror(errno));

	for (ap = hi->addrs, p = hp->h_addr_list; *p != NULL; ++ap, ++p)
		memcpy(ap, *p, sizeof(*ap));

	return (hi);
}


/*------------------------------------------------------------------------

 * freehostinfo - 释放hostinfo结构

 *------------------------------------------------------------------------

*/
void freehostinfo(register struct hostinfo *hi)
{
	if (hi->name != NULL) 
	{
		FREE(hi->name);
		hi->name = NULL;
	}

	FREE(hi->addrs);
	FREE(hi);
}

/*------------------------------------------------------------------------

 * getaddr - 把hostname所指的ip地址存入ap字符串中，主程序中调用，
 *			 用于从命令选项中获得gateway

 *------------------------------------------------------------------------

*/
void getaddr(register u_int32_t *ap, register char *hostname)
{
	register struct hostinfo *hi;

	hi = gethostinfo(hostname);
	*ap = hi->addrs[0];
	freehostinfo(hi);
}


/*------------------------------------------------------------------------

 * setsin - 初始化sockaddr_in结构信息

 *------------------------------------------------------------------------

*/
void setsin(register struct sockaddr_in *sin, register u_int32_t addr)
{

	memset(sin, 0, sizeof(*sin));

#ifdef HAVE_SOCKADDR_SA_LEN
	sin->sin_len = sizeof(*sin);
#endif

	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = addr;
}



