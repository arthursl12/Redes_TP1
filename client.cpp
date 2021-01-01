#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char *argv[]){
    printf("usage: %s <server IP> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

#define BUFSZ 1024

void* send_msg_handler(void* data) {
    int s = *((int *) data);
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Message input
    printf("message1> ");
    fflush(stdout);
    fgets(buf, BUFSZ-1, stdin);

    while(1) {
        // Send the message
        size_t count = send(s, buf, strlen(buf)+1, 0);
        if (count != strlen(buf)+1){ logexit("send");}

        // Copy buffer to comparison
        char cpybuf[BUFSZ];
        strcpy(cpybuf,buf);
        cpybuf[strlen(cpybuf)-1] = '\0';
        if (strcmp(cpybuf,"##quit") == 0){
            // Command to exit;
            break;
        }

        // Message input
        // printf("message3> ");
        // fflush(stdout);
        fgets(buf, BUFSZ-1, stdin);
    }
    pthread_exit(EXIT_SUCCESS);
}

void* recv_msg_handler(void* data) {
    int s = *((int *) data);
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    unsigned total = 0;
    // printf("Listening to: %i\n", s);

    while (1) {
        size_t count = recv(s, buf + total, BUFSZ - total, 0);

        if (count > 0) {
            // Print message
            removeNewLine(buf);
            puts(buf);
            printf("received %ld bytes\n", count);
            // Print input text again
            printf("message2> ");
            fflush(stdout);
        } else{
            // count == 0
            printf("Server closed connection.\n");
            close(s);
            exit(EXIT_FAILURE);
            break;
        }
        total += count;
    }
    printf("received total %d bytes\n", total);
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
    if (argc < 3){
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (addrparse(argv[1], argv[2], &storage) != 0){
        usage(argc, argv);
    }

    // Connect to server
    int s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) { logexit("socket");}
    struct sockaddr* addr = (struct sockaddr*)(&storage);
    if (connect(s, addr, sizeof(storage)) != 0){ logexit("connect");}
    char addrstr[BUFSZ];       
    addrtostr(addr, addrstr, BUFSZ);        // Get server IP to print
    printf("Sucessfully connected to %s\n", addrstr);

    pthread_t recv_msg_thread;
    int *arg = (int*) malloc(sizeof(*arg));
    if (arg == NULL) { logexit("malloc");}
    *arg = s;
    if(pthread_create(&recv_msg_thread, NULL, recv_msg_handler, arg) != 0){
        logexit("pthread");
    }

    pthread_t send_msg_thread;
    arg = (int*) malloc(sizeof(*arg));
    if (arg == NULL) { logexit("malloc");}
    *arg = s;
    pthread_create(&send_msg_thread, NULL, send_msg_handler, arg);
    (void)pthread_join(send_msg_thread, NULL); // <--- will wait for thread

    close(s);
    

    
    exit(EXIT_SUCCESS);
}   