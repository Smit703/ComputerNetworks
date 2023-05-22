/* UDPtimed.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <math.h>

extern int	errno;

int	passiveUDP(const char *service);
int	errexit(const char *format, ...);

int passivesock(const char *service, const char *transport, int qlen);

/*------------------------------------------------------------------------
 * main - Iterative UDP server for TIME service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	struct sockaddr_in fsin;	/* the from address of a client	*/
	char *service = "time";	/* service name or port number	*/
	char buf[1024];			/* "input" buffer; any size > 0	*/
	int sock;			/* server socket		*/
	unsigned int alen;		/* from-address length		*/
	char op[4];
	int op1,op2;
	int ans;
	int i;
	double num;

	switch (argc) {
	case	1:
		break;
	case	2:
		service = argv[1];
		break;
	default:
		errexit("usage: UDPtimed [port]\n");
	}

	sock = passivesock(service, "udp", 0);
	/* Last parameter is Queue length and not applicable for UDP sockets*/

	while (1) {
	  	int count;
	  	alen = sizeof(fsin);
		memset(buf,0,strlen(buf));

		count = recvfrom(sock, buf, sizeof(buf), 0,
				(struct sockaddr *)&fsin, &alen);

		printf("************** \n");
		printf("Client IP address: %s\n", inet_ntoa(fsin.sin_addr));
		printf("Client port      : %d\n", ntohs(fsin.sin_port));
		
		if (count < 0)
			errexit("recvfrom: %s\n", strerror(errno));

		printf("Received message from client : %s",buf);

		memset(op,0,strlen(op));
		i = 0;
		op1 = 0;
		op2 = 0;
		ans = 0;
		while(buf[i]!=' ')
		{
			op[i]=buf[i];
			i++;
		}
		i++;
		while(buf[i]!=' ')
		{
			op1*=10;
			op1+=buf[i]-'0';
			i++;
		}
		i++;
		while(i<count-1)
		{
			op2*=10;
			op2+=buf[i]-'0';
			i++;
		}

		if(strcmp(op,"add")==0)
	  	{
			ans = op1 + op2;
	  	}
	  	else if(strcmp(op,"mul")==0)
	  	{
			ans = op1 * op2;
	  	}
	  	else if(strcmp(op,"mod")==0)
	  	{
			ans = op1%op2;
	  	}
	  	else if(strcmp(op,"hyp")==0)
	  	{
			ans = op1*op1 + op2*op2;
			ans = sqrt(ans);
	  	}

		(void) sendto(sock, &ans, sizeof(ans), 0,
				(struct sockaddr *)&fsin, sizeof(fsin));

		printf("************** \n");
		memset(buf,0,strlen(buf));
		memset(op,0,strlen(op));
	}
}
