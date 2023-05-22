// NAME: Smit Bagul
// Roll Number: CS20B011
// Course: CS3205 Jan. 2023 semester
// Lab number: 2
// Date of submission: 3 March 2023
// I confirm that the source file is entirely written by me without
// resorting to any dishonest means.
// I used for basic socket programming codes from Assignment 1
#include <bits/stdc++.h>

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

unsigned short portbase = 0;
int K;                    //portnum

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

void NameResolver(map<string,string> servers)
{
    string t = to_string(K+53);
	const char *service = t.c_str();
    char buf[100];
    char ans[100];
    int sock;
    struct sockaddr_in fsin;
    unsigned int alen;
    int sR,sT,sA;
    int n,nT,n1,n2,nA,n3;
    bool flag = false;
    memset(buf,0,strlen(buf));
    string t1,t2,t3;
    const char * serviceTLD; 
    const char * serviceR;
    const char * serviceADS;

    string rhost = servers["RDS"];
    char *hostR = new char[rhost.length()+1];
    hostR[rhost.length()]='\0';
    for(int i=0;i<rhost.length();i++)
    {
        hostR[i]=rhost[i];
    }

    char *hostT;
    char *hostA;

    sock = passivesock(service, "udp", 0);                  //connects to client

    while(1)
    {
        FILE *fptr = fopen("NRoutput.txt","a");
        if(fptr==NULL)
        {
            printf("Errror : Could not open NRoutput.txt\n");
        }
        int count;
        alen = sizeof(fsin);
        memset(buf,0,strlen(buf));

        count = recvfrom(sock, buf, sizeof(buf), 0,(struct sockaddr *)&fsin, &alen);
        if(count<0)
        {
            printf("Error in receiving data from client in NR\n");
        }
        buf[count]='\0';

        fprintf(fptr,"NR input from client : %s\n",buf);

        
        char responseR[100];
        int portR;
        char responseT[100];
        int portT;
        memset(responseR,0,strlen(responseR));
        
        t1 = to_string(K+54);
	    serviceR = t1.c_str();
        sR = connectsock(hostR, serviceR, "udp");                 //connects to Root DNS server
        write(sR,buf,strlen(buf));
        fprintf(fptr,"NR query to RDS : %s\n",buf);

        n = read(sR,(char *)responseR,sizeof(responseR));          //reads ip address of TLD from Root DNS
        responseR[n] = '\0';
        if(n<0)
        {
            printf("Read Failed from Root DNS\n");
        }

        n1 = read(sR,(char *)&portR,sizeof(portR));                //reads port number of TLD from Root DNS
        if(n<0) 
        {
            printf("Read Failed from Root Dns\n");
        }

        fprintf(fptr,"RDS response : IP address = %s, Port = %d\n",responseR,portR);
        if(strcmp(responseR,"DNE")==0)
        {
            flag = true;
            goto L1;
        }

        t2 = to_string(portR);
        serviceTLD = t2.c_str();
        hostT = (char *)responseR;
        sT = connectsock(hostT,serviceTLD,"udp");                    //connects to TLD server
        memset(responseR,0,strlen(responseR));

        write(sT,buf,strlen(buf));
        if(portR==K+55)
        {
            fprintf(fptr,"NR query to TLD_com : %s\n",buf);
        }
        else
        {
            fprintf(fptr,"NR query to TLD_edu : %s\n",buf);
        }

        nT = read(sT,(char *)responseT,sizeof(responseT));           //reads ip address of ADS from TLD
        responseT[nT] = '\0';
        if(n<0)
        {
            printf("Read failed from TLD\n");
        }

        n2 = read(sT,(char *)&portT,sizeof(portT));                  //reads port number of ADS from TLD
        if(n<0)
        {
            printf("Read failed from TLD\n");
        }

        if(portR==K+55)
        {
            fprintf(fptr,"TLD_com server response : IP address = %s, Port = %d\n",responseT,portT);
        }
        else
        {
            fprintf(fptr,"TLD_edu server response : IP address = %s, Port = %d\n",responseT,portT);
        }
        

        if(strcmp(responseT,"DNE")==0)
        {
            flag = true;
            goto L1;
        }

        t3 = to_string(portT);
        serviceADS = t3.c_str();
        hostA = (char *)responseT;
        sA = connectsock(hostA,serviceADS,"udp");                     //connects to the ADS server

        memset(ans,0,strlen(ans));

        write(sA,buf,strlen(buf));
        nA = read(sA,(char *)ans,sizeof(ans));                         //reads the ip of given domain from ADS
        ans[nA] = '\0';
        if(n<0)
        {
            printf("Read failed from ADS\n");
        }
        
        if(portT==K+57)
        {
            fprintf(fptr,"NR query to ADS1 : %s\n",buf);
            fprintf(fptr,"ADS1 server response : IP address = %s\n",ans);
        }
        else if(portT==K+58)
        {
            fprintf(fptr,"NR query to ADS2 : %s\n",buf);
            fprintf(fptr,"ADS2 server response : IP address = %s\n",ans);
        }
        else if(portT==K+59)
        {
            fprintf(fptr,"NR query to ADS3 : %s\n",buf);
            fprintf(fptr,"ADS3 server response : IP address = %s\n",ans);
        }
        else if(portT==K+60)
        {
            fprintf(fptr,"NR query to ADS4 : %s\n",buf);
            fprintf(fptr,"ADS4 server response : IP address = %s\n",ans);
        }
        else if(portT==K+61)
        {
            fprintf(fptr,"NR query to ADS5 : %s\n",buf);
            fprintf(fptr,"ADS5 server response : IP address = %s\n",ans);
        }
        else if(portT==K+62)
        {
            fprintf(fptr,"NR query to ADS6 : %s\n",buf);
            fprintf(fptr,"ADS6 server response : IP address = %s\n",ans);
        }

        L1:
        if(flag)
        {
            flag = false;
            ans[0] = 'D', ans[1] = 'N', ans[2] = 'E', ans[3] = '\0';
        }
        
        fprintf(fptr,"\n\n");
        (void) sendto(sock, (char *)&ans, sizeof(ans), 0,(struct sockaddr *)&fsin, sizeof(fsin));         //sends the IP of domain back to client
        fclose(fptr);
    }
}

void RootDns(map<string,string> servers)
{
    string t = to_string(K+54);
	const char *service = t.c_str();
    char buf[100];
    char ans[100];
    int sock;
    struct sockaddr_in fsin;
    unsigned int alen;

    sock = passivesock(service,"udp",0);                       //connects to NR

    while(1)
    {
        FILE *fptr = fopen("RDSoutput.txt","a");
        if(fptr==NULL)
        {
            printf("Errror : Could not open RDSoutput.txt\n");
        }

        int count;
        alen = sizeof(fsin);
        memset(buf,0,strlen(buf));

        count = recvfrom(sock, buf, sizeof(buf), 0,(struct sockaddr *)&fsin, &alen);
        if(count<0)
        {
            printf("Error in receiving data from NR in Root DNS\n");
        }
        buf[count]='\0';

        fprintf(fptr,"Root DNS received query from NR : %s\n",buf);
        memset(ans,0,strlen(ans));
        int port;

        if(buf[count-1]=='m' && buf[count-2]=='o' && buf[count-3]=='c')          //.com
        {
            port = K+55;
            string tmp = servers["TDS_com"]; 
            for(int i=0;i<tmp.length();i++)
            {
                ans[i] = tmp[i];
            }
            ans[tmp.length()]='\0';
        }
        else if(buf[count-1]=='u' && buf[count-2]=='d' && buf[count-3]=='e')         //.edu
        {
            port = K+56;
            string tmp = servers["TDS_edu"]; 
            for(int i=0;i<tmp.length();i++)
            {
                ans[i] = tmp[i];
            }
            ans[tmp.length()]='\0';
        }
        else
        {
            ans[0] = 'D', ans[1] = 'N', ans[2] = 'E', ans[3] = '\0';
            port = 0;
            
        }

        fprintf(fptr,"RDS responds to NR : IP address = %s, port = %d\n\n\n",ans,port);       

        fclose(fptr); 

        sendto(sock, (char *)&ans, sizeof(ans), 0,(struct sockaddr *)&fsin, sizeof(fsin));          //sends the ip of TLD to NR
        sendto(sock, &port, sizeof(port), 0,(struct sockaddr *)&fsin, sizeof(fsin));                //sends the port number of TLD to Nr

    }
}

void TLD_server(map<string,string> mp,map<string,string> servers,int port)
{
    string t = to_string(port);
	const char *service = t.c_str();
    char buf[100];
    char ans[100];
    int sock;
    struct sockaddr_in fsin;
    unsigned int alen;

    sock = passivesock(service,"udp",0);                                                    //connects to NR

    while(1)
    {
        FILE *fptr = fopen("TLDoutput.txt","a");
        if(fptr==NULL)
        {
            printf("Errror : Could not open TLDoutput.txt\n");
        }
        int count;
        alen = sizeof(fsin);
        memset(buf,0,strlen(buf));

        count = recvfrom(sock, buf, sizeof(buf), 0,(struct sockaddr *)&fsin, &alen);
        if(count<0)
        {
            printf("Error in receiving data from NR in TLD\n");
        }
        buf[count]='\0';
        memset(ans,0,strlen(ans));

        int j = 0;
        bool dot = false;
        char tmp[100];
        for(int i=0;i<count+1;i++)
        {
            if(dot)
            {
                tmp[j] = buf[i];
                j++;
            }
            if(buf[i]=='.')
            {
                dot = true;
            }
        }
        tmp[j] = '\0';

        bool flag = false;
        string ans1;
        char ans2;

        map<string,string>::iterator it = mp.begin();
        while(it!=mp.end())                                                                   //searches for the ADS in which the domain exist
        {
            string s1 = it->first;
            string s2 = it->second;
            char s3[100];
            bool dot1 = false;
            int k = 0;
            for(int i=0;i<s2.length();i++)
            {
                if(dot1)
                {
                    s3[k] = s2[i];
                    k++;
                }
                if(s2[i]=='.')
                {
                    dot1 = true;
                }
            }
            s3[k]='\0';

            if(strcmp(s3,tmp)==0)
            {
                flag = true;
                ans1 = servers[tmp];
                ans2 = s1[3];
            }
            it++;
        }

        int port = 0;
        if(flag)
        {
            port = K+56+ans2-'0';
            for(int i=0;i<ans1.length();i++)
            {
                ans[i] = ans1[i];
            }
            ans[ans1.length()]='\0';

        }
        else
        {
            ans[0] = 'D', ans[1] = 'N', ans[2] = 'E', ans[3] = '\0';
        }

        if(port==K+55)
        {
            fprintf(fptr,"TLD_com received query from NR : %s\n",buf);
            fprintf(fptr,"TLD_com responds to NR : IP address = %s, port = %d\n",ans,port);
        }
        else
        {
            fprintf(fptr,"TLD_edu received query from NR : %s\n",buf);
            fprintf(fptr,"TLD_edu responds to NR : IP address = %s, port = %d\n",ans,port);
        }
        fprintf(fptr,"\n\n");
        fclose(fptr);

        sendto(sock, (char *)&ans, sizeof(ans), 0,(struct sockaddr *)&fsin, sizeof(fsin));                //returns the ip of ADS to NR
        sendto(sock, &port, sizeof(port), 0,(struct sockaddr *)&fsin, sizeof(fsin));                      //returns the port number of ADS to NR
    }
}

void ADS_server(map<string,string> mp,int port)
{
    string t = to_string(port);
	const char *service = t.c_str();
    char buf[100];
    char ans[100];
    int sock;
    struct sockaddr_in fsin;
    unsigned int alen;

    sock = passivesock(service,"udp",0);                                   //connects to NR

    while(1)
    {
        // fstream fptr;
        // fptr.open("ADSoutput.txt",ios::app);
        // if(!fptr)
        // {
        //     printf("Errror : Could not open ADSoutput.txt\n");
        // }
        FILE *fptr = fopen("ADSoutput.txt","a");
        if(fptr==NULL)
        {
            printf("Errror : Could not open ADSoutput.txt\n");
        }

        int count;
        alen = sizeof(fsin);
        memset(buf,0,strlen(buf));

        count = recvfrom(sock, buf, sizeof(buf), 0,(struct sockaddr *)&fsin, &alen);
        if(count<0)
        {
            printf("Error in receiving data from NR in ADS\n");
        }
        buf[count]='\0';
        memset(ans,0,strlen(ans));

        if(mp.find(buf)!=mp.end())                                                                 //searches the ip of Domain in ADS map
        {
            string tmp = mp[buf];
            for(int i=0;i<tmp.length();i++)
            {
                ans[i] = tmp[i];
            }
            ans[tmp.length()]='\0';
        }
        else
        {
            ans[0] = 'D', ans[1] = 'N', ans[2] = 'E', ans[3] = '\0';   
        }

        if(port==K+57)
        {
            fprintf(fptr,"ADS1 receives query from NR : %s\n",buf);
            fprintf(fptr,"ADS1 responds to NR : IP address = %s\n",ans);
        }
        else if(port==K+58)
        {
            fprintf(fptr,"ADS2 receives query from NR : %s\n",buf);
            fprintf(fptr,"ADS2 responds to NR : IP address = %s\n",ans);
        }
        else if(port==K+59)
        {
            fprintf(fptr,"ADS3 receives query from NR : %s\n",buf);
            fprintf(fptr,"ADS3 responds to NR : IP address = %s\n",ans);
        }
        else if(port==K+60)
        {
            fprintf(fptr,"ADS4 receives query from NR : %s\n",buf);
            fprintf(fptr,"ADS4 responds to NR : IP address = %s\n",ans);
        }
        else if(port==K+61)
        {
            fprintf(fptr,"ADS5 receives query from NR : %s\n",buf);
            fprintf(fptr,"ADS5 responds to NR : IP address = %s\n",ans);
        }
        else if(port==K+62)
        {
            fprintf(fptr,"ADS6 receives query from NR : %s\n",buf);
            fprintf(fptr,"ADS6 responds to NR : IP address = %s\n",ans);
        }
        sendto(sock, (char *)&ans, sizeof(ans), 0,(struct sockaddr *)&fsin, sizeof(fsin));               //sends the ip of Domain to NR
        fprintf(fptr,"\n\n");
        fclose(fptr);
    }
}

int main(int argc,char * argv[])
{
    K = atoi(argv[1]);
    char* file = argv[2];
    pid_t pid[10];
    FILE* inputFilePtr = fopen(file,"r");
    if(inputFilePtr==NULL)
    {
        printf("Failed to open input file\n");
    }
    
    map<string,string> servers;
    map<string,string> ADS1,ADS2,ADS3,ADS4,ADS5,ADS6;
    map<string,string> TLD_com,TLD_edu;

    //Reading from input file and storing in maps
    char tmp[1024],tmp1[1024],tmp2[1024];
    fscanf(inputFilePtr, "%s", tmp);
    for(int i=0;i<10;i++)
    {
        fscanf(inputFilePtr, "%s", tmp1);
        fscanf(inputFilePtr, "%s", tmp2);
        servers.insert({tmp1,tmp2});
    }
    fscanf(inputFilePtr, "%s", tmp1);
    for(int i=0;i<5;i++)
    {
        fscanf(inputFilePtr, "%s", tmp1);
        fscanf(inputFilePtr, "%s", tmp2);
        ADS1.insert({tmp1,tmp2});
    }
    fscanf(inputFilePtr, "%s", tmp1);
    for(int i=0;i<5;i++)
    {
        fscanf(inputFilePtr, "%s", tmp1);
        fscanf(inputFilePtr, "%s", tmp2);
        ADS2.insert({tmp1,tmp2});
    }
    fscanf(inputFilePtr, "%s", tmp1);
    for(int i=0;i<5;i++)
    {
        fscanf(inputFilePtr, "%s", tmp1);
        fscanf(inputFilePtr, "%s", tmp2);
        ADS3.insert({tmp1,tmp2});
    }
    fscanf(inputFilePtr, "%s", tmp1);
    for(int i=0;i<5;i++)
    {
        fscanf(inputFilePtr, "%s", tmp1);
        fscanf(inputFilePtr, "%s", tmp2);
        ADS4.insert({tmp1,tmp2});
    }
    fscanf(inputFilePtr, "%s", tmp1);
    for(int i=0;i<5;i++)
    {
        fscanf(inputFilePtr, "%s", tmp1);
        fscanf(inputFilePtr, "%s", tmp2);
        ADS5.insert({tmp1,tmp2});
    }
    fscanf(inputFilePtr, "%s", tmp1);
    for(int i=0;i<5;i++)
    {
        fscanf(inputFilePtr, "%s", tmp1);
        fscanf(inputFilePtr, "%s", tmp2);
        ADS6.insert({tmp1,tmp2});
    }
    fscanf(inputFilePtr, "%s", tmp);

    TLD_com["ADS1"] = ADS1.begin()->first;
    TLD_com["ADS2"] = ADS2.begin()->first;
    TLD_com["ADS3"] = ADS3.begin()->first;
    TLD_edu["ADS4"] = ADS4.begin()->first;
    TLD_edu["ADS5"] = ADS5.begin()->first;
    TLD_edu["ADS6"] = ADS6.begin()->first;

    //forking to run all the servers in child proccesses
    pid[0] = fork();
    if(pid[0]<0)
    {
        printf("Error in forking");
    }
    if(pid[0]==0)
    {
        //local DNS server
        NameResolver(servers);
    }

    pid[1] = fork();
    if(pid[1]<0)
    {
        printf("Error in forking");
    }
    if(pid[1]==0)
    {
        //root DNS server
        RootDns(servers);
    }

    pid[2] = fork();
    if(pid[2]<0)
    {
        printf("Error in forking");
    }
    if(pid[2]==0)
    {
        //TLD com server
        TLD_server(TLD_com,servers,K+55);
    }  

    pid[3] = fork();
    if(pid[3]<0)
    {
        printf("Error in forking");
    }
    if(pid[3]==0)
    {
        //TLD edu server
        TLD_server(TLD_edu,servers,K+56);
    }

    pid[4] = fork();
    if(pid[4]<0)
    {
        printf("Error in forking");
    }
    if(pid[4]==0)
    {
        //ADS1 server
        ADS_server(ADS1,K+57);
    }
    pid[5] = fork();
    if(pid[5]<0)
    {
        printf("Error in forking");
    }
    if(pid[5]==0)
    {
        //ADS2 server
        ADS_server(ADS2,K+58);
    }
    pid[6] = fork();
    if(pid[6]<0)
    {
        printf("Error in forking");
    }
    if(pid[6]==0)
    {
        //ADS3 server
        ADS_server(ADS3,K+59);
    }
    pid[7] = fork();
    if(pid[7]<0)
    {
        printf("Error in forking");
    }
    if(pid[7]==0)
    {
        //ADS4 server
        ADS_server(ADS4,K+60);
    }
    pid[8] = fork();
    if(pid[8]<0)
    {
        printf("Error in forking");
    }
    if(pid[8]==0)
    {
        //ADS5 server
        ADS_server(ADS5,K+61);
    }
    pid[9] = fork();
    if(pid[9]<0)
    {
        printf("Error in forking");
    }
    if(pid[9]==0)
    {
        //ADS6 server
        ADS_server(ADS6,K+62);
    }

    char inp[100];
    char response[100];
    string lhost = servers["NR"];
    char *host = new char[lhost.length()+1];
    host[lhost.length()]='\0';
    for(int i=0;i<lhost.length();i++)
    {
        host[i]=lhost[i];
    }
    string t = to_string(K+53);
	const char *service = t.c_str();
    int s,n;
    memset(inp,0,sizeof(inp));

    s = connectsock(host, service, "udp");	     //connecting client to NR
    while(1)
    {
        memset(inp,0,sizeof(inp));
        printf("Enter server name : ");
        scanf("%s",inp);
        inp[strlen(inp)]='\0';
        
        if(strcmp(inp,"bye")==0)                  //kills all child proccesses
        {
            //kill are servers
            for(int i=0;i<10;i++)
            {
                kill(pid[i],SIGKILL);
            }
            printf("All Server Processes are killed. Exiting.\n");
            break;
        }

        int dot = 0;
        for(int i=0;i<strlen(inp);i++)
        {
            if(inp[i]=='.')
            {
                dot++;
            }
        }

        if(dot!=2)                              //invalid input
        {
            printf("Invalid input format\n\n");
            continue;
        }

        write(s,inp,strlen(inp));

        memset(response,0,sizeof(response));
        n = read(s,(char *)response,sizeof(response));
        response[n] = '\0';
        if(n<0)
        {
            printf("Read Failed");
        }

        if(strcmp(response,"DNE")==0)                //Domain does not exist
        {
            printf("No DNS record found\n\n");
        }
        else
        {
            printf("DNS Mapping of %s : %s\n\n",inp,response);
        }
    }
}