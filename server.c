#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#define STATUS_SUCCESS 200
#define STATUS_CLIENT_ERROR 404
#define TEST_ROOT "/www"
#define REQUEST_DELIM " "
#define REQUEST_GET "GET"


// file constants
#define TYPE_DELIM ','

// size constants
#define ARGUMENT_COUNT 3
#define METHOD_SIZE 3
#define SEND_BUFFER 2048
#define ALLOWED_CONNECTIONS 10


// file types (MIME)
#define MIME_HTML "text/html"
#define MIME_JPEG "image/jpeg"
#define MIME_CSS "text/css"
#define MIME_JAVASCRIPT "text/javascript"
#define MIME_OTHER "application/octet-stream"




typedef struct request_t request_t;
struct request_t {
  int statusCode;
  int handlerID;
  char* fileType;
  char* rootPath;
  char* filePath;
  int validRequest; // request is finished and can be used
};


typedef struct cmd_args_t cmd_args_t;
struct cmd_args_t {
  int protocolNumber;
  char* portNumber;
  char* rootPath;
};

int isValidPath(char* filePath);
char* getMIMEType(char* filePath);


int initialiseSocket(int protocolNumber, char* portNumber) {
  // code to setup socket - adapting from code provided in lectures
  int listenfd, connfd, re, s;
  struct addrinfo hints, *res;


  // create socket
  memset(&hints, 0, sizeof(hints));

  // assign based on prot number
  if (protocolNumber == 4) {
    hints.ai_family = AF_INET;
  } else if (protocolNumber == 6) {
    hints.ai_family = AF_INET6;
  }

  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  s = getaddrinfo(NULL, portNumber, &hints, &res);

  // create socket and allow reuse
  re = 1;
  listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(re));

  // socket binding
  bind(listenfd, res->ai_addr, res->ai_addrlen);
  listen(listenfd, ALLOWED_CONNECTIONS);


  return listenfd;

}

 {
  // code to handle a new request - will be assigned to threads if I get that far
}


cmd_args_t ingestCommandLine(char *argv[]) {
  // read arguments given from command line and save to cmd_args struct
  cmd_args_t newConfig;

  // (IPv)4 or (IPv)6
  newConfig.protocolNumber = atoi(argv[0]);

  // port number (as a string)
  newConfig.portNumber = malloc(sizeof(argv[1]));
  strcpy(newConfig.portNumber, argv[1]);

  // read path to root
  newConfig.rootPath = malloc(sizeof(argv[2]));
  strcpy(newConfig.rootPath, argv[2]);

  return newConfig;
}


request_t ingestRequest(char* input) {
  // take raw tcp input and convert to a request type for easy access
  // should do as much error handling in here as possible
  request_t potentialRequest;
  char method[METHOD_SIZE];
  char* newToken;

  int tokenCount = 0;

  while ((newToken = strtok(input, REQUEST_DELIM)) != NULL) {
    int tokenLength = strlen(newToken);

    if (tokenCount == 0) {
      // trying to read request method
      if ((tokenLength == METHOD_SIZE) && (strcmp(newToken, REQUEST_GET) == 0)) {
        // matches size of a get request
        strcpy(method, newToken);

      } else {
        // too big/small
        potentialRequest.validRequest = 0;
        return potentialRequest;
      }



    } else if (tokenCount == 1) {
      // tring to read filepath now
      if (isValidPath(newToken)) {
        // path is valid, store in request
        potentialRequest.filePath = malloc(sizeof(char)*tokenLength);
        strcpy(potentialRequest.filePath, newToken);

        // get file type, and store
        char* MIMEType = getMIMEType(newToken);
        potentialRequest.fileType = malloc(sizeof(char)*strlen(MIMEType));
        strcpy(potentialRequest.fileType, MIMEType);

      } else {
        // too big/small
        potentialRequest.validRequest = 0;
        return potentialRequest;
      }

    } else if (tokenCount == 2) {
      // @ shouldn't need this, but leaving incase we do
    }

    tokenCount++;

  }

  return potentialRequest;
}


void executeRequest(request_t request) {
  // given a valid request, respond and then send file

  // respond to request

  // join expected file path to root path
  char* totalPath = malloc(sizeof(request.rootPath) + sizeof(request.filePath));
  strcat(totalPath, request.rootPath);
  strcat(totalPath, request.filePath);

  if (isValidPath(totalPath))

}

void sendFile(char* filePath) {
  // code to send file if found, through socket
}


void newRequest(int newfd, char buffer, cmd_args_t config) {
  // worker to handle new incomming - can be used standalone or multithreaded

  while ((charsRead = read(newfd, buffer, sizeof(buffer)-1)) > 0) {
    // writing to buffer
  }

  // received get command, pass to ingest
  request_t newRequest = ingestRequest(buffer);

  if (newRequest == NULL) {
    // request was malformed somehow
    return 0;
  } else {
    // request was good, add root path
    newRequest.rootPath = malloc(sizeof(config.rootPath));
    strcmp(newRequest.rootPath, config.rootPath);
  }


  executeRequest(newRequest);

}

int main(int argc, char *argv[]) {
  char buffer[SEND_BUFFER];

  // read cmdline input to get addrinfo
  if (argc != ARGUMENT_COUNT) {
    // not enough / too many arguments
    return 0;
  }

  // we have enough arguments, ingest them to config struct
  cmd_args_t config = ingestCommandLine(argv);

  // create socket and start listening on it
  int listen = initialiseSocket(config.protocolNumber, config.portNumber);

  // socket is good to go, begin responding to requests
  while (1) {

    // accept new connection
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;

    connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_size);

    // pass addr to thread to deal with
    newRequest(connfd);



  }



  return 0;
}


char* getMIMEType(char* filePath) {
  // return the MIME type given the filepath
  char* fileType;

  char* fullStop = strrchr(filePath, TYPE_DELIM);
  // get pointer to full stop delim in path
  if (fullStop == NULL || strlen(fullStop+1) <= 0) {
    // malformed input: no delim or filetype does not exist
    return NULL;
  }

  // point now to the filetype itself
  fullStop = fullStop + 1;

  // check filetype exists

  // ugly if statement to return MIME type
  if (strcmp("html", fullStop) == 0) {
    fileType = MIME_HTML;
  } else if (strcmp("jpg", fullStop) == 0) {
    fileType = MIME_JPEG;
  } else if (strcmp("css", fullStop) == 0) {
    fileType = MIME_CSS;
  } else if (strcmp("js", fullStop) == 0) {
    fileType = MIME_JAVASCRIPT;
  } else {
    fileType = MIME_OTHER;
  }

  return fileType;
}


int isValidPath(char* filePath) {
  // helper func to check filepath exists and type is correct

  if (strlen(filePath) > 0) {
    // path isn't empty
    if (fopen(filePath, "r") != NULL) {
      // able to open
      return 1;
    }
  }

  // unable to open
  return 0;

}
