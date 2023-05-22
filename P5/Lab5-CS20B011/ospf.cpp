
#include <bits/stdc++.h>
#include <cstdio>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

using namespace std;

#ifndef	INADDR_NONE
#define	INADDR_NONE	0xffffffff
#endif	/* INADDR_NONE */

unsigned short	portbase = 0;	
string HOST = "localhost";
mutex bMutex;

int errexit(const char *format, ...)
{
	va_list	args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

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

struct truple
{
    int neighbour;
    int min;
    int max;
    int weight;
};

vector<truple> adj;                //stores adjacent nodes to id
vector<vector<int>> G;

void Writes(int wait,int id)                     //thread to write hello packets to neighbours and gets hello reply (this is implemented as using connectsock)
{
    const char * host = HOST.c_str();

    string sid = to_string(id);
    string pkt = "HELLO|"+sid;
    char packet[16];
    memset(packet,0,strlen(packet));
    for(int i=0;i<pkt.size();i++)
    {
        packet[i] =  pkt[i];
    }
    
    while(1)
    {
        this_thread::sleep_for(chrono::seconds(wait));
        for(int i=0;i<adj.size();i++)
        {
            unique_lock<mutex> lock(bMutex);
            int nid = adj[i].neighbour;
            lock.unlock();
            int sock = 10000+nid;
            string str = to_string(sock);
            const char *sock_ptr = str.c_str();

            int sc = connectsock(host,sock_ptr,"udp");

            write(sc,packet,strlen(packet));
        }

    }
    
}

void Reads(int s, int id, int N)                      //thread to get data from neighbours, after receiving a hello packet it will send a reply
{
    struct sockaddr_in fsin;
    unsigned int alen;
    const char * host = HOST.c_str();

    int seq[N];
    for(int i=0;i<N;i++)
    {
        seq[i] = -1;
    }


    while(1)
    {
        char buf[128];
        memset(buf,0,strlen(buf));

        int count = recvfrom(s,buf,sizeof(buf),0,(struct sockaddr *)&fsin, &alen);
        if(count<0)
        {
            printf("Error in receiving data from client in NR\n");
        }
        buf[count]='\0';

        string tmp = "";
        int i = 0;
        while(buf[i]!='|')
        {
            tmp+=buf[i];
            i++;
        }

        if(tmp=="HELLO")
        {
            //printf("Rcd a hello\n");
            string rcdid = "";
            for(int j=i+1;j<count;j++)
            {
                rcdid += buf[j];
            }
            int rid = stoi(rcdid);
            int cost = 0;

            unique_lock<mutex> lock(bMutex);
            for(int k=0;k<adj.size();k++)
            {   
                if(adj[k].neighbour==rid)
                {
                    int tmp = adj[k].max - adj[k].min;
                    int rnd = rand()%(tmp+1);
                    cost = adj[k].min + rnd;
                    break;
                }
            }
            lock.unlock();

            string sid = to_string(id);
            string scost = to_string(cost);
            string pkt = "HELLOREPLY|"+sid+"|"+rcdid+"|"+scost;

            char packet[100];
            memset(packet,0,strlen(packet));
            for(int j=0;j<pkt.size();j++)
            {
                packet[j] = pkt[j];
            }

            // for(int k=0;k<pkt.size();k++)
            // {
            //     printf("%c",packet[k]);
            // }
            // cout<<endl;

            int sock = 10000+rid;
            string str = to_string(sock);
            const char *sock_ptr = str.c_str();

            int sc = connectsock(host,sock_ptr,"udp");

            write(sc,packet,strlen(packet));

            //(void)sendto(s, (char *)&packet, sizeof(packet), 0,(struct sockaddr *)&fsin, sizeof(fsin)); 
        }
        else if(tmp=="HELLOREPLY")
        {
            //printf("Rcd a hello reply\n");
            i++;
            string jidstr = "",iidstr = "",cstr = "";
            while(buf[i]!='|')
            {
                jidstr+=buf[i];
                i++;
            }
            i++;
            while(buf[i]!='|')
            {
                iidstr+=buf[i];
                i++;
            }
            i++;
            while(i<count)
            {
                cstr+=buf[i];
                i++;
            }

            int iid = stoi(iidstr);
            int jid = stoi(jidstr);
            int cost = stoi(cstr);

            if(iid==id)
            {
                unique_lock<mutex> lock(bMutex);
                for(int k=0;k<adj.size();k++)
                {
                    if(adj[k].neighbour==jid)
                    {
                        adj[k].weight = cost;
                        G[iid][jid] = cost;
                        G[jid][iid] = cost;
                        break;
                    }
                }
                lock.unlock();
            }

        }
        else if(tmp=="LSA")
        {
            //printf("Rcd a lsa\n");
            i++;
            string srcstr = "",seqstr = "";
            while(buf[i]!='|')
            {
                srcstr+=buf[i];
                i++;
            }
            i++;
            while(buf[i]!='|')
            {
                seqstr+=buf[i];
                i++;
            }
            i++;

            int src = stoi(srcstr);
            int seqn = stoi(seqstr);

            if(seq[src]<seqn)
            {
                seq[src] = seqn;

                string noen = "";
                while(buf[i]!='|')
                {
                    noen+=buf[i];
                    i++;
                }
                i++;

                int numen = stoi(noen);

                for(int k=0;k<numen;k++)
                {
                    string neighstr = "", cststr = "";
                    while(buf[i]!='|')
                    {
                        neighstr+=buf[i];
                        i++;
                    }
                    i++;
                    while(buf[i]!='|')
                    {
                        cststr+=buf[i];
                        i++;
                    }
                    i++;

                    int Neigh = stoi(neighstr);
                    int cst = stoi(cststr);

                    unique_lock<mutex> lock(bMutex);
                    G[src][Neigh] = cst;
                    G[Neigh][src] = cst;
                    lock.unlock();
                }

                unique_lock<mutex> lock(bMutex);
                for(int j=0;j<adj.size();j++)
                {
                    if(adj[j].neighbour!=src)
                    {
                        int nid = adj[j].neighbour;
                        int sock = 10000+nid;
                        string str = to_string(sock);
                        const char *sock_ptr = str.c_str();

                        int sc = connectsock(host,sock_ptr,"udp");

                        write(sc,buf,strlen(buf));
                    }
                }
                lock.unlock();


            }   
        }
    }
}

void LSAs(int wait, int id)
{
    const char * host = HOST.c_str();
    int seq = 0;

    while(1)
    {
        this_thread::sleep_for(chrono::seconds(wait));
        
        string sid = to_string(id);
        int num = adj.size();
        string pkt = "LSA|"+sid+"|"+to_string(seq)+"|"+to_string(num)+"|";
        unique_lock<mutex> lock(bMutex);
        for(int i=0;i<num;i++)
        {
            pkt+=to_string(adj[i].neighbour)+"|"+to_string(adj[i].weight)+"|";
        }
        seq++;
        char packet[128];
        memset(packet,0,strlen(packet));
        for(int i=0;i<pkt.size();i++)
        {
            packet[i] =  pkt[i];
        }
        
        for(int i=0;i<num;i++)
        {
            int nid = adj[i].neighbour;
            int sock = 10000+nid;
            string str = to_string(sock);
            const char *sock_ptr = str.c_str();

            int sc = connectsock(host,sock_ptr,"udp");

            write(sc,packet,strlen(packet));
        }
        lock.unlock();
    }
}

void dijkstra(int N, int s, vector<int>& dist, vector<int> &prev)
{
    dist.assign(N,INT_MAX);
    prev.assign(N,-1);

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.push(make_pair(0, s));
    dist[s] = 0;

    while (!pq.empty()) 
    {
        int u = pq.top().second;
        pq.pop();

        for (int v = 0; v < N; v++) 
        {
            if (G[u][v] != -1) {
                int w = G[u][v];
                if (dist[u] + w < dist[v]) 
                {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    pq.push(make_pair(dist[v], v));
                }
            }
        }
    }
}

void Creates(int wait, int N, string filename,int id)                   //thread to create a network topology
{
    int time = 0;

    FILE* output;  
    int j = 0;
    string outfilei = "";
    while(filename[j]!='.')
    {
        outfilei+=filename[j];
        j++;
    }
    outfilei+='-'+to_string(id);
    while(j<filename.size())
    {
        outfilei+=filename[j];
        j++;
    }   

    output = fopen(outfilei.c_str(),"w");
    if(output == NULL)
    {
        printf("output-%d.txt failed to open.",id);
    }

    fclose(output);
    
    while(1)
    {
        output = fopen(outfilei.c_str(),"a");
        this_thread::sleep_for(chrono::seconds(wait));
        time+=wait;

        unique_lock<mutex> lock(bMutex);
        vector<int> dist, prev;

        // for(int i=0;i<N;i++)
        // {
        //     for(int j=0;j<N;j++)
        //     {
        //         printf("%d ",G[i][j]);
        //     }
        //     cout<<endl;
        // }

        dijkstra(N,id,dist,prev);

        // for(int k=0;k<dist.size();k++)
        // {
        //     printf("%d ",dist[k]);
        // }
        // printf("\n");

        fprintf(output,"|----------------------------------------|\n");
        fprintf(output,"| Routing Table for Node No. %d at Time %d|\n",id,time);
        fprintf(output,"|----------------------------------------|\n");
        fprintf(output,"|Destination| Cost |        Path         |\n");
        fprintf(output,"|----------------------------------------|\n");

        for(int j=0;j<N;j++)
        {
            if(j==id)
            {
                continue;
            }

            if(dist[j]==INT_MAX)
            {
                fprintf(output,"|    %d      |  --   |       NO PATH       |\n",j);
            }
            else
            {
                fprintf(output,"|    %d      ",j);
                fprintf(output,"|  %d   |         ",dist[j]);
                vector<int> path;
                int v = j;
                while (v != -1) 
                {
                    path.push_back(v);
                    v = prev[v];
                }
                for (int k = path.size()-1; k > 0; k--) 
                {
                    fprintf(output,"%d-",path[k]);
                }
                fprintf(output,"%d         |\n",j);
            }
        }
        fprintf(output,"|----------------------------------------|\n\n\n");
        
        lock.unlock();

        printf("Routing tables at time %d created\n",time);

        fclose(output);
    }

}



int main(int argc,char * argv[])
{
    int id;
    int H = 1, A = 5, S = 20;
    char * in, *out;
    int N,L;

    srand(time(NULL));

    int i = 0;
    while(i<argc)
    {
        if(strcmp(argv[i],"-i")==0)
        {
            id = atoi(argv[i+1]);
            i++;
        }
        if(strcmp(argv[i],"-f")==0)
        {
            in = argv[i+1];
            i++;
        }
        if(strcmp(argv[i],"-o")==0)
        {
            out = argv[i+1];
            i++;
        }
        if(strcmp(argv[i],"-h")==0)
        {
            H = atoi(argv[i+1]);
            i++;
        }
        if(strcmp(argv[i],"-a")==0)
        {
            A = atoi(argv[i+1]);
            i++;
        }
        if(strcmp(argv[i],"-s")==0)
        {
            S = atoi(argv[i+1]);
            i++;
        }
        i++;
    }

    FILE *input;
    input = fopen(in,"r");
    if ( input == NULL )  
    {
        printf( "input.txt file failed to open." );
        return 0; 
    }

    fscanf(input,"%d",&N);
    fscanf(input,"%d",&L);
    bool curr;

    for(int i=0;i<L;i++)
    {
        int n1, n2, min, max;
        curr = false;
        int neigh;
        fscanf(input,"%d %d %d %d",&n1,&n2,&min,&max);
        if(id==n1)
        {
            curr = true;
            neigh = n2;
        }
        else if(id == n2)
        {
            curr = true;
            neigh = n1;
        }

        if(curr)
        {
            truple element = {neigh,min,max,-1};
            adj.push_back(element);
        }
    }

    G.resize(N);
    for(int i=0;i<N;i++)
    {
        G[i].assign(N,-1);
    }
    string filename;
    filename.assign(out);

    int portnum = 10000+id;
    string str = to_string(portnum);
    const char *ptr = str.c_str();
    int s = passivesock(ptr,"udp",0);

    thread writer(Writes,H,id);
    thread reader(Reads,s,id,N);
    thread LSA(LSAs,A,id);
    thread topology(Creates,S,N,filename,id);


    writer.join();
    reader.join();
    LSA.join();
    topology.join();


    fclose(input);
}