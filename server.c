#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>
#include <limits.h>
#include "serverUtils.h"


// marking constants
#define MULTITHREADED
#define IMPLEMENTS_IPV6

#define REQUEST_DELIM " "
#define REQUEST_GET "GET"
#define DBL_CRLF "\r\n\r\n"

// index constants
#define METHOD_INDEX 0
#define HTTP_PATH_INDEX 1
#define PROTOCOL_INDEX 1
#define PORT_INDEX 2
#define PATH_INDEX 3

// size constants
#define ARGUMENT_COUNT 4
#define METHOD_SIZE 4
#define READ_BUFFER 2048
#define ALLOWED_CONNECTIONS 10

// response constants
#define STATUS_SUCCESS 200
#define STATUS_CLIENT_ERROR 404
#define OK_RESPONSE "HTTP/1.0 200 OK\r\n"
#define NF_RESPONSE "HTTP/1.0 404 Not Found\r\n\r\n"


int initialiseSocket(int protocolNumber, char* portNumber) {
  // code to setup socket - adapting from code provided in lectures
  int listenfd, re, s;
  struct addrinfo hints, *res, *p;

  // create socket
  memset(&hints, 0, sizeof hints);

  // assign based on requested protocol number
  if (protocolNumber == 4) {
    hints.ai_family = AF_INET;

  } else if (protocolNumber == 6) {
    hints.ai_family = AF_INET6;
  }

  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  s = getaddrinfo(NULL, portNumber, &hints, &res);

  if (s != 0) {
    // get addr info failed to allocate properly
    printf("ERROR: failed socket allocation\n");
    exit(EXIT_FAILURE);
  }


  if (protocolNumber == 6) {
    // loop over getddrinfo() response to find IPv6 specific socket
    // code adapted from lecture slides

    for (p = res; p != NULL; p = p->ai_next) {
      if (p->ai_family == AF_INET6 &&
            (s = socket(p->ai_family,
                           p->ai_socktype,
                           p->ai_protocol)) < 0) {
                             // failed creation
                           }
                         }
    }


  if (s < 0) {
    // could not find IPv6 socket
    printf("ERROR: could not find socket on IPv6\n");
    exit(EXIT_FAILURE);
  }

  // create socket and allow reuse
  re = 1;
  listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(re));

  // override code required in spec
  int enable = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // socket binding
  int bindStatus = bind(listenfd, res->ai_addr, res->ai_addrlen);

  if (bindStatus == -1) {
    // tried to bind to an already-bound port
    return bindStatus;
  }

  // start listening on decided on file descriptor
  listen(listenfd, ALLOWED_CONNECTIONS);

  return listenfd;

}



cmd_args_t ingestCommandLine(char *argv[]) {
  // read arguments given from command line and save to cmd_args struct
  cmd_args_t newConfig;

  // (IPv)4 or (IPv)6
  if (atoi(argv[PROTOCOL_INDEX]) == 4 || atoi(argv[PROTOCOL_INDEX]) == 6) {
    newConfig.protocolNumber = atoi(argv[PROTOCOL_INDEX]);
  } else {
    // incorrect protocol number
    newConfig.ignoreConfig = 1;
    printf("ERROR: invalid protocol number (must be 4 or 6)\n");
  }


  // port number (as a string)
  if (atoi(argv[PORT_INDEX]) >= 0 && convertsToNumber(argv[PORT_INDEX])) {
    // port is number within range
    newConfig.portNumber = malloc(strlen(argv[PORT_INDEX]) + 1);
    strcpy(newConfig.portNumber, argv[PORT_INDEX]);

  } else {
    // port number not acceptable
    printf("ERROR: invalid port number\n");
    newConfig.ignoreConfig = 1;

  }

  // read path to root
  if (isValidDirectory(argv[PATH_INDEX])) {
    // found directory - resolve to 'real path' and store

    char truePath[PATH_MAX];
    char *pathSuccess = realpath(argv[PATH_INDEX], truePath);

    if (pathSuccess) {
      newConfig.rootPath = malloc(strlen(truePath) + 1);
      strcpy(newConfig.rootPath, truePath);

    } else {
      // could not resolve path
      printf("ERROR: invalid directory path (contains '..')'\n");
      newConfig.ignoreConfig = 1;
    }


  } else {
    // directory doesn't exist
    printf("ERROR: inavlid directory path (doesn't exist)\n");
    newConfig.ignoreConfig = 1;

  }

  return newConfig;
}



request_t ingestRequest(char* input, cmd_args_t config) {
  // take raw tcp input and convert to a request type

  char method[METHOD_SIZE];
  char* newToken = strtok(input, REQUEST_DELIM);
  request_t potentialRequest;

  int tokenCount = 0;
  potentialRequest.validRequest = 1;  // assume requests are safe to start with

  while (newToken != NULL) {
    // iterate over
    int tokenLength = strlen(newToken);


    if (tokenCount == METHOD_INDEX) {
      // trying to read request method
      if ((tokenLength == METHOD_SIZE-1) && (strcmp(newToken, REQUEST_GET) == 0)) {
        // matches size of a get request, store it
        strcpy(method, newToken);

      } else {
        // too big/small - mark request as invalid
        potentialRequest.validRequest = 0;
        return potentialRequest;
      }


    } else if (tokenCount == HTTP_PATH_INDEX) {
      // tring to read filepath now

      // check if file exists at root
      char* potentialFile =  malloc(strlen(config.rootPath) + strlen(newToken)+1);
      potentialFile = combinePaths(config.rootPath, newToken);

      if (isValidPath(potentialFile)) {
        // path is valid, store in request
        potentialRequest.filePath = malloc(strlen(potentialFile)+1);
        strcpy(potentialRequest.filePath, potentialFile);

        // get file type, and store
        char* MIMEType = getMIMEType(newToken);
        potentialRequest.fileType = malloc(strlen(MIMEType)+1);
        strcpy(potentialRequest.fileType, MIMEType);

        // file was found, success! mark request as 200
        potentialRequest.statusCode = STATUS_SUCCESS;

      } else {
        // file did not exist, mark request as a 404 but still valid
        potentialRequest.statusCode = STATUS_CLIENT_ERROR;
        return potentialRequest;
      }

    }
    tokenCount++;
    newToken = strtok(NULL, REQUEST_DELIM);
  }

  return potentialRequest;
}

void executeRequest(request_t request, int newfd) {
  // given a valid request, respond and then send file

  // respond to request
  if (request.statusCode == STATUS_SUCCESS) {
    // send confirmation
    printf("- result: 200 OK\n");
    char httpConfirm[] = OK_RESPONSE;
    write(newfd, httpConfirm, strlen(httpConfirm));

    // build header with confirmation and content type
    char mimeConfirm[] = "Content-Type: ";
    char* mimeHeader = malloc(strlen(mimeConfirm) + strlen(request.fileType) + strlen(DBL_CRLF) + 2);
    strcpy(mimeHeader, mimeConfirm);
    strcat(mimeHeader, request.fileType);
    strcat(mimeHeader, DBL_CRLF);

    // send file header
    write(newfd, mimeHeader, strlen(mimeHeader));

    // since successful, send file also
    fileSend(request.filePath, newfd);

  } else if (request.statusCode == STATUS_CLIENT_ERROR) {
    // send failure message
    printf("- result: 404, not found\n");
    char httpFailure[] = NF_RESPONSE;
    write(newfd, httpFailure, strlen(httpFailure));
  }
}




void* serviceRequest(void* configIn) {
  // worker to handle new incomming - can be used standalone or multithreaded

  cmd_args_t config = *((cmd_args_t*)configIn);
  // store local copies of main args
  int newfd = config.fileDescriptor;
  int charsRead;

  // create threadlocal buffer to receive data
  char buffer[READ_BUFFER];
  bzero(buffer, READ_BUFFER);

  int index = 0;

  while ((charsRead = read(newfd, buffer + index, READ_BUFFER - 1)) >= 0) {
    // check chars exist to be read
    if (charsRead > 0) {
      if (strstr(buffer, DBL_CRLF)) {
        // found double CRLF in buffer
        break;
      }
    }
    // move buffer index along
    index += charsRead;
  }


  // received get command, pass to ingest
  request_t newRequest = ingestRequest(buffer, config);

  if (newRequest.validRequest == 0 && newRequest.statusCode != STATUS_CLIENT_ERROR) {
    // request failed, but not due to a 404 error
    close(newfd);
    return NULL;
  }

  executeRequest(newRequest, newfd);
  close(newfd);

  // this request has been serviced.
  return NULL;
}






int main(int argc, char *argv[]) {
  int connfd;

  printf("\nServer startup, configuring...\n\n");

  // read cmdline input to get addrinfo
  if (argc != ARGUMENT_COUNT) {
    // not enough / too many arguments
    exit(EXIT_FAILURE);
  }


  // we have enough arguments, ingest them to config struct
  cmd_args_t config = ingestCommandLine(argv);

  if (config.ignoreConfig == 1) {
    // something in the command line input was malformed
    // unable to proceed, so terminate server
    exit(EXIT_FAILURE);
  }

  pthread_t threadIdentifier;
  // create socket and start listening on it
  int listenfd = initialiseSocket(config.protocolNumber, config.portNumber);

  if (listenfd == -1) {
    // could not intialiseSocket, therefore can't run server
    printf("ERROR: Socket initialisation failure\n");
    exit(EXIT_FAILURE);
  }

  // setup was a success, display info to the user
  printf("Server Configured Succesfully: \n");
  printf(" - IPv%d\n - Port %s\n - Root Directory: %s\n", config.protocolNumber, config.portNumber, config.rootPath);

  // socket is good to go, begin responding to requests
  while (1) {

    // accept new connection on listening socket (code adapted from lectures)
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;
    connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_size);

    // recompile config structure -> make malloc'd copy for this specific thread
    // to prevent memory issues, and bundle in the 'connfd' value to be passed
    // through p_thread
    cmd_args_t* threadConfig = malloc(sizeof(cmd_args_t));
    recompileConfig(threadConfig, config, connfd);

    // everything is setup, spin up new thread with passed config file
    printf("\nNew request: creating thread for fd: %d\n", connfd);
    pthread_create(&threadIdentifier, NULL, serviceRequest, (void*)threadConfig);

    // detatch thread so we can continue listening -> don't need to wait for join
    pthread_detach(threadIdentifier);
  }

  exit(0);
}
