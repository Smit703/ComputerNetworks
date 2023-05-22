// NAME: Smit Bagul
// Roll Number: CS20B011
// Course: CS3205 Jan. 2023 semester
// Lab number: 4
// Date of submission: 5/04/23
// I confirm that the source file is entirely written by me without
// resorting to any dishonest means.
// Website(s) that I used for basic socket programming code are: codes from assignment 1

// set up as UDP server

#include <bits/stdc++.h>
#include <cstdlib>
#include <sys/time.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <fstream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

using namespace std;

#ifndef	INADDR_NONE
#define	INADDR_NONE	0xffffffff
#endif	/* INADDR_NONE */

#define BUFSIZE 1024
unsigned short	portbase = 0;	

int errexit(const char *format, ...)
{
	va_list	args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

//Used to run a server
int passivesock(const char *service, const char *transport, int qlen)
/*
 * Arguments:
 *      service   - service associated with the desired port
 *      transport - transport protocol to use ("tcp" or "udp")
 *      qlen      - maximum server request queue length
 */
{
	struct servent	*pse;	/* pointer to service information entry	*/
	struct protoent *ppe;	/* pointer to protocol information entry*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, type;	/* socket descriptor and socket type	*/

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

    /* Map service name to port number */
	if ( (pse = getservbyname(service, transport)) != NULL )
		sin.sin_port = htons(ntohs((unsigned short)pse->s_port)
			+ portbase);
	else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0)
		errexit("can't get \"%s\" service entry\n", service);

    /* Map protocol name to protocol number */
	if ( (ppe = getprotobyname(transport)) == 0)
		errexit("can't get \"%s\" protocol entry\n", transport);

    /* Use protocol to choose a socket type */
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

    /* Allocate a socket */
	s = socket(PF_INET, type, ppe->p_proto);
	if (s < 0)
		errexit("can't create socket: %s\n", strerror(errno));

    /* Bind the socket to the given port number */
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		errexit("can't bind to %s port: %s\n", service,
			strerror(errno));

	/* If TCP socket, call listen() for incoming connection requests.  */
	if (type == SOCK_STREAM && listen(s, qlen) < 0)
		errexit("can't listen on %s port: %s\n", service,
			strerror(errno));
	
	return s;
}

int main(int argc, char * argv[])
{
	bool flag = false;
	char* portnum;
	int n;
	float e;

	int i = 0;
	while(i<argc)
	{
		if(strcmp(argv[i],"-d")==0)
		{
			flag = true;
		}
		if(strcmp(argv[i],"-p")==0)
		{
			portnum = argv[i+1];
			i++;
		}
		if(strcmp(argv[i],"-n")==0)
		{
			n = atoi(argv[i+1]);
			i++;
		}
		if(strcmp(argv[i],"-e")==0)
		{
			e = atof(argv[i+1]);
			i++;
		}
		i++;
	}

	srand(time(0));

	int sock = passivesock(portnum,"udp",0);

	int rec = 0;
	int NFE = 0;
	char buf[1024];
	int ack = 0;
	struct sockaddr_in fsin;
	unsigned int alen = sizeof(fsin);
	
	struct timeval t1, t2;
	double seconds, microSeconds;

	gettimeofday(&t1, NULL);
	while(rec < n)
	{
		memset(buf,0,strlen(buf));
		int count = recvfrom(sock,buf,sizeof(buf),0,(struct sockaddr *)&fsin, &alen);
		//buf[count] = '\0';

		gettimeofday(&t2, NULL);
		seconds = t2.tv_sec - t1.tv_sec;
		microSeconds = t2.tv_usec - t1.tv_usec;
		
		int randm = rand()%(int)(1/e);

		if(randm == 1)
		{
			if(flag)
			{
				printf("Time Received: %.3f Packet dropped : true\n",1000*seconds + microSeconds/1000);
			}
			continue;                     //packet dropped
		}

		if(flag)
		{
			printf("Time Received: %.3f Packet dropped : false\n",1000*seconds + microSeconds/1000);
		}

		int seq;
		seq = (int) buf[0];
		if(seq<0)
		{
			seq+=256;
		}
		if(NFE==seq)
		{
			rec++;
			ack = NFE;
			NFE++;
			if(NFE==255)
			{
				NFE=0;
			}
			//update ack
			(void) sendto(sock, (char *)&ack, sizeof(ack), 0,(struct sockaddr *)&fsin, sizeof(fsin));
		}
	}

	return 0;
}
