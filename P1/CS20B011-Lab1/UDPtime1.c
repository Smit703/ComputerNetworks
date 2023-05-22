/* UDPtime.c - main */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#define	BUFSIZE 64

#define	UNIXEPOCH	2208988800UL	/* UNIX epoch, in UCT secs	*/
#define	MSG		"what time is it?\n"

extern int	errno;
int	errexit(const char *format, ...);

int connectsock(const char *host, const char *service, const char *transport );

/*------------------------------------------------------------------------
 * main - UDP client for TIME service that prints the resulting time
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	char *host = "localhost";	/* host to use if none supplied	*/
	char *service = "time";	/* default service name		*/
	char input[1024];
	int	s, n;			/* socket descriptor, read count*/
	struct sockaddr_in serveraddr;
	socklen_t len;
	int msg;
 
	switch (argc) {
	case 1:
		host = "localhost";
		break;
	case 3:
		service = argv[2];
		/* FALL THROUGH */
	case 2:
		host = argv[1];
		break;
	default:
		fprintf(stderr, "usage: UDPtime [host [port]]\n");
		exit(1);
	}

	s = connectsock(host, service, "udp");

	len = sizeof(serveraddr);
	getpeername(s, (struct sockaddr*)&serveraddr, &len);

	printf("************** \n");
	printf("Server IP address: %s\n", inet_ntoa(serveraddr.sin_addr));
	printf("Server port      : %d\n", ntohs(serveraddr.sin_port));
	printf("************** \n");

	while(1)
	{
		printf("Enter Command : ");
		if(fgets(input, 1024, stdin)==NULL)
		{
			break;
		}
		write(s, input, strlen(input));

		n = read(s, (char *)&msg, sizeof(msg));
		if (n < 0)
		{
			errexit("read failed: %s\n", strerror(errno));
		}
		printf("Answer received from server : %d\n", msg);
	}

	exit(0);
}
