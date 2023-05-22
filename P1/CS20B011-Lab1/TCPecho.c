/* TCPecho.c - main, TCPecho */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

extern int	errno;

void TCPecho(const char *host, const char *service);
int	errexit(const char *format, ...);

int	connectsock(const char *host, const char *service,const char *transport);

#define	LINELEN	1024

/*------------------------------------------------------------------------
 * main - TCP client for ECHO service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	char	*host = "localhost";	/* host to use if none supplied	*/
	char	*service = "echo";	/* default service name		*/

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
		fprintf(stderr, "usage: TCPecho [host [port]]\n");
		exit(1);
	}

	TCPecho(host, service);

	exit(0);
}

/*------------------------------------------------------------------------
 * TCPecho - send input to ECHO service on specified host and print reply
 *------------------------------------------------------------------------
 */
void TCPecho(const char *host, const char *service)
{
	char buf[LINELEN+1];		/* buffer for one line of text	*/
	char rec[LINELEN+1];
	char name[LINELEN+1];
	int	s, n;			/* socket descriptor, read count*/
	int	outchars, inchars;	/* characters sent and received	*/
	struct sockaddr_in serveraddr;
	socklen_t len;
	int siz;
	int i;
	FILE *fptr;

	s = connectsock(host, service, "tcp");
	if (s < 0)
	  {
		fprintf(stderr, "usage: TCPecho connectsock failed. \n");
		exit(1);
	  }
	
	len = sizeof(serveraddr);
	getpeername(s, (struct sockaddr*)&serveraddr, &len);

	printf("************** \n");
	printf("Server IP address: %s\n", inet_ntoa(serveraddr.sin_addr));
	printf("Server port      : %d\n", ntohs(serveraddr.sin_port));
	printf("************** \n");

	memset(buf,0,sizeof(buf));
	memset(rec,0,sizeof(rec));
	memset(name,0,sizeof(name));

	printf("Enter filename and number of bytes to be read : ");
	fgets(buf, sizeof(buf), stdin);
	buf[LINELEN] = '\0';	/* insure line null-terminated	*/
	outchars = strlen(buf);

	i = 0;
	siz=0;
	while(buf[i]!='.')
	{
		name[i]=buf[i];
		i++;
	}
	name[i]='1';
	while(buf[i]!=' ')
	{
		name[i+1]=buf[i];
		i++;
	}
	name[i+1]='\0';
	i++;
	while(i<outchars-1)
	{
		siz*=10;
		siz+=buf[i]-'0';
		i++;
	}

	(void) write(s, buf, outchars);
	/* read it back */
	for (inchars = 0; inchars < siz; inchars+=siz) {
		n = read(s, &rec[inchars], siz-inchars);
		if (n < 0)
		errexit("socket read failed: %s\n",
			strerror(errno));
	}

	if(strcmp(rec,"SORRY!")!=0)
	{
		fptr = fopen(name,"w+");
		fwrite(rec,strlen(rec),1,fptr);
		fclose(fptr);
	}
	else
	{
		printf("Server says that the file does not exist.");
	}

	memset(buf,0,sizeof(buf));
	memset(rec,0,sizeof(rec));
	memset(name,0,sizeof(name));

	exit(0);
}
