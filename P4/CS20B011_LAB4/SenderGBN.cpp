// NAME: Smit Bagul
// Roll Number: CS20B011
// Course: CS3205 Jan. 2023 semester
// Lab number: 4
// Date of submission: 5/04/23
// I confirm that the source file is entirely written by me without
// resorting to any dishonest means.
// Website(s) that I used for basic socket programming code are: codes from assignment 1

// set up as UDP client

#include <bits/stdc++.h>
#include <cstdlib>
#include <sys/time.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <fstream>
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

queue<vector<char>> trans_buffer;
queue<vector<char>> curr_window;
mutex bMutex;
double RTT_avg = 0;
int attempts = 0;
int wmin,wmax;
int rec = 0;
struct timeval t1[255], t2[255], tg[255], t0;
bool timeouts[255];
int atmpt[255];
int sum = 0;
bool retrans = false;

int errexit(const char *format, ...)
{
	va_list	args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

//Used to connect from client to server
int connectsock(const char *host, const char *service, const char *transport )
/*
 * Arguments:
 *      host      - name of host to which connection is desired
 *      service   - service associated with the desired port
 *      transport - name of transport protocol to use ("tcp" or "udp")
 */
{
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct servent	*pse;	/* pointer to service information entry	*/
	struct protoent *ppe;	/* pointer to protocol information entry*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, type;	/* socket descriptor and socket type	*/


	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

    /* Map service name to port number */
	if ( pse = getservbyname(service, transport) )
		sin.sin_port = pse->s_port;
	else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0)
		errexit("can't get \"%s\" service entry\n", service);

    /* Map host name to IP address, allowing for dotted decimal */
	if ( phe = gethostbyname(host) )
		memcpy(&sin.sin_addr, phe->h_addr_list[0], phe->h_length);
	else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		errexit("can't get \"%s\" host entry\n", host);

    /* Map transport protocol name to protocol number */
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

    /* Connect the socket */
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		errexit("can't connect to %s.%s: %s\n", host, service,
			strerror(errno));
	return s;
}

void packetGenerator(int length, int rate, int max, int maxp)
{
	int seq = 0;
	int n = 0;
	while(n<maxp)
	{
		vector<char> packet(length+1);		//+1 cause 1 byte for sequence number
		for(int i=0;i<length+1;i++)
		{
			packet[i] = 'a';
		}

		unique_lock<mutex> lock(bMutex);
		if(trans_buffer.size()<max)
		{	           
			packet[0] = seq;
			gettimeofday(&tg[n],NULL);
			trans_buffer.push(packet);
			seq++;
			n++;
			if(seq==255)
			{
				seq=0;
			}
		}
		lock.unlock();

		this_thread::sleep_for(chrono::microseconds(1000000/rate));
	}
}

void timeout(double ms, int id)
{
	this_thread::sleep_for(chrono::milliseconds((int)ms));

	//unsigned id = this_thread::get_id();
	unique_lock<mutex> lock(bMutex);
	if(timeouts[id]==false)                //timeout has occured
	{
		retrans = true;		
	}
	lock.unlock();
}

void Writes(int w,int s,int n)
{
	int curr = 0;
	int LFT = 0;
	sleep(1);
	
	while(rec<n)
	{
		//unique_lock<mutex> lock2(Mutex2);
		unique_lock<mutex> lock(bMutex);
		bool tmp = retrans;
		lock.unlock();
		if(tmp)
		{
			unique_lock<mutex> lock2(bMutex);
			LFT = wmin;
			int tmp = curr_window.size();
			lock2.unlock();

			for(int i=0;i<tmp;i++)
			{
				unique_lock<mutex> lock3(bMutex);
				vector <char> packet = curr_window.front();
				curr_window.push(packet);
				curr_window.pop();

				double to = 2*RTT_avg;
				if(curr<10)
				{
					to = 100;
				}
				
				timeouts[LFT] = false;

				atmpt[LFT]++;
				attempts++;

				lock3.unlock();

				char bufff[packet.size()];
				for(int j = 0;j<packet.size();j++)
				{
					bufff[j] = packet[j]; 
				}

				LFT++;
				if(LFT==255)
				{
					LFT=0;
				}
				
				write(s,bufff,strlen(bufff));
				thread toes(timeout,to,LFT);
				toes.detach();
				//tothreads[LFT] = thread(timeout,to,LFT);
			}
			unique_lock<mutex> lock4(bMutex);
			retrans = false;
			lock4.unlock();
		}
		//lock2.unlock();

		if(trans_buffer.size()==0)
		{
			continue;
		}

		//unique_lock<mutex> lock(bMutex);
		if(curr_window.size()<w)
		{
			unique_lock<mutex> lock5(bMutex);
			vector <char> packet = trans_buffer.front();
			curr_window.push(packet);
			trans_buffer.pop();

			char bufff[packet.size()];
			for(int j = 0;j<packet.size();j++)
			{
				bufff[j] = packet[j]; 
			}
			gettimeofday(&t1[LFT],0);
			double to = 2*RTT_avg;
			if(curr<10)
			{
				to = 10;
			}
			timeouts[LFT] = false;

			atmpt[LFT] = 1;
			attempts++;

			lock5.unlock();

			LFT++;
			if(LFT==255)
			{
				LFT=0;
			}
			curr++;

			write(s,bufff,strlen(bufff));
			thread tos(timeout,to,LFT);
			tos.detach();
			//tothreads[LFT] = thread(timeout,to,LFT);	
		}
	}
}

int main(int argc, char* argv[])
{
	bool flag = false;
	char* ip;              // receiver Ip/name
	char* portnum;			// receiver port num
	int pktL;				// packet length
	int pktR;				// packet rate
	int n;					// max_packets
	int w;					// window_size
	int bufSize;			// max buf size

	int i = 0;
    while(i<argc)
	{
		if(strcmp(argv[i],"-d")==0)
		{
			flag = true;
		}
		if(strcmp(argv[i],"-s")==0)
		{
			ip = argv[i+1];
			i++;
		}
		if(strcmp(argv[i],"-p")==0)
		{
			portnum = argv[i+1];
			i++;
		}
		if(strcmp(argv[i],"-l")==0)
		{
			pktL = atoi(argv[i+1]);
			i++;
		}
		if(strcmp(argv[i],"-r")==0)
		{
			pktR = atoi(argv[i+1]);
			i++;
		}
		if(strcmp(argv[i],"-n")==0)
		{
			n = atoi(argv[i+1]);
			i++;
		}
		if(strcmp(argv[i],"-w")==0)
		{
			w = atoi(argv[i+1]);
			i++;
		}
		if(strcmp(argv[i],"-f")==0)
		{
			bufSize = atoi(argv[i+1]);
			i++;
		}

		i++;
	}
	
	float e;

	int s = connectsock(ip,portnum,"udp");
	string packets;              //seq number 2 bytes and data
	vector<string> window;
	int seq = 0;
	int ack = 0;


	double seconds, microSeconds;
	double RTT[n];
	double ratio;

	wmin = 0;
	wmax = w-1;     //min and max pointers on window changes as per ack

	for(int i=0;i<255;i++)
	{
		timeouts[i] = false;
	}

	gettimeofday(&t0, NULL);

	thread packetGen(packetGenerator,pktL,pktR,bufSize,n);	
	thread writer(Writes,w,s,n);

	while(rec < n)
	{
		int ackr;
		int n = read(s, (char *)&ackr, sizeof(ackr));

	
		if(ackr==ack)
		{

			gettimeofday(&t2[ack],0);

			unique_lock<mutex> lock(bMutex);
			seconds = t2[ack].tv_sec - t1[ack].tv_sec;
			microSeconds = t2[ack].tv_usec - t1[ack].tv_usec;
			RTT[rec] = 1000*seconds + microSeconds/1000;
			sum+=RTT[rec];
			RTT_avg = (float)sum/(float)(rec+1);
			
			timeouts[ack] = true;
			curr_window.pop();

			wmin = ack+1;
			wmax = (ack+w)%255;
			lock.unlock();

			if(flag)
			{
				double s,micros;
				s = tg[rec].tv_sec - t0.tv_sec;
				micros = tg[rec].tv_usec - t0.tv_usec;

				unique_lock<mutex> lock(bMutex);
				printf("Sequence Number = %d, Time generated = %.3f, RTT = %f, Number of attempts = %d\n",rec,1000*s+micros/1000,RTT[rec],atmpt[ack]);
				lock.unlock();
			}

			ack++;
			if(ack==255)
			{
				ack=0;
			}

			rec++;
		}
	}

	packetGen.join();
	writer.join();

	//read(s,(char *)&e,sizeof(e));
	e = (float)(attempts-n)/(float)(n);
	ratio = (float)attempts/(float)((float)(n));
	for(int i=0;i<n;i++)
	{
		RTT_avg+=RTT[i];
	}
	RTT_avg/=(float)n;

	printf("Packet Rate = %d,\nDrop Probability %.8f,\nLength = %d,\nRetransmission Ratio = %f,\nAverage Round Trip Time = %.3f\n",pktR,e,pktL,ratio,RTT_avg);

	return 0;
}