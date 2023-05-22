/* TCPechod.c - main, TCPechod */

#define	_USE_BSD
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#define	QLEN 32	/* maximum connection queue length	*/
#define	BUFSIZE 4096

extern int errno;

void reaper(int);
int	TCPechod(int fd);
int	errexit(const char *format, ...);
int	passivesock(const char *service, const char *transport, int qlen);

/*------------------------------------------------------------------------
 * main - Concurrent TCP server for ECHO service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	char	*service = "echo";	/* service name or port number	*/
	struct	sockaddr_in fsin;	/* the address of a client	*/
	unsigned int	alen;		/* length of client's address	*/
	int	msock;			/* master server socket		*/
	int	ssock;			/* slave server socket		*/

	switch (argc) {
	case	1:
		break;
	case	2:
		service = argv[1];
		break;
	default:
		errexit("usage: TCPechod [port]\n");
	}

	msock = passivesock(service, "tcp", QLEN);

	(void) signal(SIGCHLD, reaper);

	while (1) {
	  alen = sizeof(fsin);
	  ssock = accept(msock, (struct sockaddr *)&fsin, &alen);

	  if (ssock < 0) {
			if (errno == EINTR)
				continue;
			errexit("accept: %s\n", strerror(errno));
		}

	  switch (fork()) {

	  case 0:		/* child */
	    (void) close(msock);
	    exit(TCPechod(ssock));

	  default:	/* parent */
	    (void) close(ssock);
	    break;

		case -1:
			errexit("fork: %s\n", strerror(errno));
		}
	}
}

/*------------------------------------------------------------------------
 * TCPechod - echo data until end of file
 *------------------------------------------------------------------------
 */
int TCPechod(int s)
{
	char buf[BUFSIZ];
	char msg[BUFSIZ];
	char name[BUFSIZ];
	int i;
	int siz;
	int	cc;
	struct sockaddr_in clientaddr;
	socklen_t len;
	int fptr;

	len = sizeof(clientaddr);
	getpeername(s, (struct sockaddr*)&clientaddr, &len);

	printf("************** \n");
	printf("Client IP address: %s\n", inet_ntoa(clientaddr.sin_addr));
	printf("Client port      : %d\n", ntohs(clientaddr.sin_port));
	printf("************** \n");
	
	
	memset(buf,0,sizeof(buf));
	while (cc = read(s, buf, sizeof(buf))) {
	  	memset(name,0,sizeof(name));
		memset(msg,0,sizeof(msg));

		if (cc < 0)
		{
			errexit("echo read: %s\n", strerror(errno));
		}

		i = 0;
		siz = 0;
		while(buf[i]!=' ')
		{
			name[i]=buf[i];
			i++;
		}
		name[i]='\0';
		i++;
		while(i<cc-1)
		{
			siz*=10;
			siz+=buf[i]-'0';
			i++;
		}

		fptr = open(name,O_RDONLY);	
		if(fptr==-1)
		{
			if(write(s,"SORRY!",7) < 0)
			{
				errexit("echo write: %s\n", strerror(errno));
			}
		}
		else
		{
			lseek(fptr, -siz, SEEK_END);
	   		siz=read(fptr, msg, siz);
	   		msg[siz]='\0';
			if (write(s, msg, siz+1) < 0)
			{	
				errexit("echo write: %s\n", strerror(errno));
			}
		}
		close(fptr);
		memset(buf,0,sizeof(buf));
	  	memset(name,0,sizeof(name));
		memset(msg,0,sizeof(msg));
	}
	return 0;
}

/*------------------------------------------------------------------------
 * reaper - clean up zombie children
 *------------------------------------------------------------------------
 */
void reaper(int sig)
{
	int	status;

	while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0)
		/* empty */;
}
