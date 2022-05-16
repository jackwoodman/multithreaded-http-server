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
#define PARENT_COMMAND ".."

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
void fileSend(char* filePath, int newfd);

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


char* combinePaths(char* root, char* file) {
  // add file path to end of root location
  char* totalPath = malloc(sizeof(root) + sizeof(file));
  strcat(totalPath, root);
  strcat(totalPath, file);

  return totalPath;

}

request_t ingestRequest(char* input, cmd_args_t config) {
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

      // check if file exists at root
      if (isValidPath(combinePaths(config.rootPath, newToken))) {
        // path is valid, store in request
        potentialRequest.filePath = malloc(sizeof(char)*tokenLength);
        strcpy(potentialRequest.filePath, newToken);

        // get file type, and store
        char* MIMEType = getMIMEType(newToken);
        potentialRequest.fileType = malloc(sizeof(char)*strlen(MIMEType));
        strcpy(potentialRequest.fileType, MIMEType);

        // file was found, success!
        potentialRequest.statusCode = STATUS_SUCCESS;

      } else {
        // file did not exist, return a 404
        potentialRequest.validRequest = 0;
        potentialRequest.statusCode = STATUS_CLIENT_ERROR;
        return potentialRequest;
      }

    } else if (tokenCount == 2) {
      // @ shouldn't need this, but leaving incase we do
    }

    tokenCount++;

  }


  return potentialRequest;
}




void executeRequest(request_t request, int newfd) {
  // given a valid request, respond and then send file

  // respond to request
  if (request.statusCode == STATUS_SUCCESS) {
    // send confirmation
    char httpConfirm[] = "HTTP/1.1 200 OK\n";
    int written = write(newfd, http_confirm, strlen(http_confirm));

    // send file header
    char mimeConfirm[] = "Content-Type: ";
    char* mimeHeader = malloc(sizeof(mimeConfirm) + sizeof(request.fileType));
    strcat(mimeHeader, mimeConfirm);
    strcat(mimeHeader, request.fileType);
    int written = write(newfd, mimeHeader, strlen(mimeHeader));

    // since successful, send file also
    fileSend(request.filePath, newfd);

  } else if (request.statusCode == STATUS_CLIENT_ERROR) {
    // send failure message
    char httpFailure[] = "HTTP/1.1 404";
    int written = write(newfd, httpFailure, strlen(httpFailure));
  }

  // finished all sending
}


void fileSend(char* filePath, int newfd) {
  // code to send file if found, through socket
  int fileSize, writeStatus;
  int localSize = SEND_BUFFER;
  char* fileBuffer = malloc(sizeof(char) * localSize);

  // get file
  FILE* targetFile = fopen(filePath, "r");

  // read file to buffer
  while ((fileSize = fread(fileBuffer, sizeof(char), sizeof(fileBuffer), targetFile)) > 0) {
    // try to write (atm assuming write will be successful, probs unsafe @)
    writeStatus = write(newfd, fileBuffer, fileSize);
  }

  // close file
  fclose(targetFile);

  // @ could return write status here if needed, won't do just yet
}


void serviceRequest(int newfd, char* buffer, cmd_args_t config) {
  // worker to handle new incomming - can be used standalone or multithreaded
  int charsRead;

  while ((charsRead = read(newfd, buffer, sizeof(buffer)-1)) > 0) {
    // writing to buffer
  }

  // received get command, pass to ingest
  request_t newRequest = ingestRequest(buffer, config);

  if (newRequest.validRequest == 0) {
    // request was malformed somehow
    return;
  }

  executeRequest(newRequest, newfd);
  // this request has been serviced.
  // @ here is where thread will close when I get around to it
}

int main(int argc, char *argv[]) {
  char buffer[SEND_BUFFER];
  int connfd;

  // read cmdline input to get addrinfo
  if (argc != ARGUMENT_COUNT) {
    // not enough / too many arguments
    return 0;
  }

  // we have enough arguments, ingest them to config struct
  cmd_args_t config = ingestCommandLine(argv);

  // create socket and start listening on it
  int listenfd = initialiseSocket(config.protocolNumber, config.portNumber);

  // socket is good to go, begin responding to requests
  while (1) {

    // accept new connection
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;

    connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_size);

    // pass addr to thread to deal with
    serviceRequest(connfd, buffer, config);

    // finished servicing new request



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

      // check for escape attempt
      if (strstr(filePath, PARENT_COMMAND) == 0) {
        return 1;
      }
    }
  }

  // unable to open / illegal instruction
  return 0;

}
