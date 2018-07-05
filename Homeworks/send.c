/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - OWEN HANDEL */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <memory.h>
#include <ctype.h>
#include <time.h>
#include <poll.h>
#include <signal.h>

typedef struct Packet{
        int done;         // contain 0 if not done, contains 1 if done 
        int seqNo;        // The seqNo used in Stop-And-Wait
        char line[1000];  // The next line in the text file
}Packet;

Packet _packet;

int seqnum = 0;

int swp_sendto(int sock, void *data, int len, int flags, struct sockaddr *to, int to_len, int time_out);

int main(int argc, char *argv[])
{
   int s;				/* s = socket */
   struct sockaddr_in in_addr;		/* Structure used for bind() */
   struct sockaddr_in sock_addr;	/* Output structure from getsockname */
   struct sockaddr_in dest_addr;	/* Destination socket */
   char line[1000];
   char **p;
   int len;
   FILE *stream;

   struct hostent *host_entry_ptr;     /* Structure to receive output */

   if (argc < 4)
    { printf("Usage: %s <symbolic-dest-addr> <dest-port>\n", argv[0]);
      printf("   Program sends messages to <symbolic-dest-addr> <dest-port>\n");
      exit(1);
    }

   /* -------
      Fill in destination socket - this will identify IP-host + (UDP) port
      ------- */
   dest_addr.sin_family = AF_INET;		 /* Internet domain */

   host_entry_ptr = gethostbyname(argv[2]);   /* E.g.: cssun.mathcs.emory.edu */

   if (host_entry_ptr == NULL)
    { printf("Host `%s' not found...\n", argv[2]);     /* Just for safety.... */
      exit(1);
    }

   /**********************************************************************
    * NOTE: DO NOT use htonl on the h_addr_list[0] returned by 
    * gethostbyname() !!!
    **********************************************************************/
   memcpy((char *) &(dest_addr.sin_addr.s_addr), 
		   host_entry_ptr->h_addr_list[0], 
		   host_entry_ptr->h_length);

   /**********************************************************************/

   dest_addr.sin_port = htons(atoi(argv[3]));    /* Destination (UDP) port # */

   /* ---
      Create a socket
      --- */
   s = socket(PF_INET, SOCK_DGRAM, 0);

   /* ---
      Set up socket end-point info for binding
      --- */
   memset((char *)&in_addr, 0, sizeof(in_addr));
   in_addr.sin_family = AF_INET;                   /* Protocol domain */
   in_addr.sin_addr.s_addr = htonl(INADDR_ANY);    /* Use wildcard IP address */
   in_addr.sin_port = htons(0);	           	   /* Use any UDP port */

   /* ---
      Here goes the binding...
      --- */
   if (bind(s, (struct sockaddr *)&in_addr, sizeof(in_addr)) == -1)
    { printf("Error: bind FAILED\n");
    }
   else
    { printf("OK: bind SUCCESS\n");
    }

   /* ----
      Check what port I got
      ---- */
   len = sizeof(sock_addr);
   getsockname(s, (struct sockaddr *) &sock_addr, &len);
   printf("Socket s is bind to:\n");
   printf("  addr = %u\n", ntohl(sock_addr.sin_addr.s_addr));
   printf("  port = %d\n", ntohs(sock_addr.sin_port));
   int milli = atoi(argv[4]);
   stream = fopen(argv[1], "r");
   printf("Sending file: `%s' to host: `%s'\n",argv[1],argv[2]);
   while((fscanf(stream, "%s\n", line) != EOF)){
      /* ----
	 sendto() will automatically use UDP layer
	 ---- */
      swp_sendto(s, line, strlen(line)+1, 0 /* flags */, 
	     (struct sockaddr *)&dest_addr, sizeof(dest_addr), milli);
      seqnum++;
	}

	swp_sendto(s, NULL, 0, 0 /* flags */, 
        (struct sockaddr *)&dest_addr, sizeof(dest_addr), milli);
	printf("File: `%s' completely sent\n", argv[1]);
	fclose(stream);
	close(s);
	exit(1);
}

int swp_sendto(int sock, void *data, int len, int flags, struct sockaddr *to, int to_len, int time_out){
	struct pollfd fds[1];
	struct sockaddr_in send;
	int timeout = time_out;
	int ackrecv;
	int recv = 0;
	if(data == NULL){
		_packet.seqNo = seqnum;
		_packet.done = 1;
		memset(&(_packet.line[0]), 0, sizeof(_packet.line));
	} else if(data != NULL){
		_packet.done = 0;
		_packet.seqNo = seqnum;
		strcpy(_packet.line, data);
	}
	sendto(sock, &(_packet), sizeof(_packet), 0, to, to_len);
	fds[0].fd = sock;
	fds[0].events = POLLIN;
	while(recv == 0){
		if(poll(fds, 1, timeout) != 0){
			recvfrom(sock, &ackrecv, sizeof(int), 0, (struct sockaddr*)&send, (int *)sizeof(send));
			if(seqnum != ackrecv){
				continue;
			} else if(seqnum == ackrecv){
				recv = 1;
			}
		} else{
			sendto(sock, &(_packet), sizeof(_packet), 0, to, to_len);
		}
	}
}
