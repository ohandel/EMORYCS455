/*
   udp4-r3.c: reciever version 2
	   - prints the name of sender
*/
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

#define MAXSIZE 1000

typedef struct Packet{
        int done;         // contain 0 if not done, contains 1 if done 
        int seqNo;        // The seqNo used in Stop-And-Wait
        char line[1000];  // The next line in the text file
}Packet;

int swp_recvfrom(int sock, void *buf, int len, int flags, struct sockaddr *from, int *from_len);

Packet _packet;

int seqnum = 0;

int main(int argc, char *argv[])
{
   int s;			   /* s = socket */
   struct sockaddr_in in_addr;	   /* Structure used for bind() */
   struct sockaddr_in sock_addr;   /* Output structure from getsockname */
   struct sockaddr_in src_addr;    /* Used to receive (addr,port) of sender */
   int src_addr_len;		   /* Length of src_addr */
   int len;			   /* Length of result from getsockname */
   struct hostent *host_entry_ptr;
   char line[MAXSIZE];


   if (argc == 1)
    { printf("Usage: %s port\n", argv[0]);
      exit(1);
    }

   /* ---
      Create a socket
      --- */
   s = socket(PF_INET, SOCK_DGRAM, 0);

   /* ---
      Set up socket end-point info for binding
      --- */
   in_addr.sin_family = AF_INET;                   /* Protocol domain */
   in_addr.sin_addr.s_addr = htonl(INADDR_ANY);    /* Use wildcard IP address */
   in_addr.sin_port = htons(atoi(argv[2])); /* UDP port */

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
   printf("Socket is bind to:\n");
   printf("  addr = %u\n", ntohl(sock_addr.sin_addr.s_addr));
   printf("  port = %u\n", ntohs(sock_addr.sin_port));

   FILE *fp = fopen( argv[1], "w" );
   while(1)
    {  
      src_addr_len = sizeof(src_addr);
      len = swp_recvfrom(s, line, MAXSIZE, 0 /* flags */,
                    (struct sockaddr *) &src_addr, &src_addr_len);
      /* ==================================================== Changed ... */
      host_entry_ptr = gethostbyaddr((char *) &(src_addr.sin_addr.s_addr), 
				     sizeof(src_addr.sin_addr.s_addr), AF_INET);
      	if(_packet.done == 1){
		fclose(fp);
	}
	seqnum++;
	strcpy(line, _packet.line);
	fprintf(fp, "%s\n", line);
	fflush(fp);
      printf("Msg from (%s,%u): `%s' (%u bytes)\n", 
		host_entry_ptr->h_name, 
		ntohs(src_addr.sin_port), line, len);
   }
}

int swp_recvfrom(int sock, void *buf, int len, int flags, struct sockaddr *from, int *from_len){
	int length;
	int _ackrecv = 0;
	while(_ackrecv == 0){
		recvfrom(sock, &(_packet), sizeof(_packet), 0, (struct sockaddr*)from, from_len);
		if(seqnum == _packet.seqNo){
			_ackrecv = 1;
			sendto(sock, &(_packet.seqNo), sizeof(int), 0, (struct sockaddr*)from, *(from_len));
		} else if(seqnum != _packet.seqNo){
			sendto(sock, &(_packet.seqNo), sizeof(int), 0, (struct sockaddr*)from, *(from_len));
		}
	}
	length = strlen(_packet.line);
	return length;
}
