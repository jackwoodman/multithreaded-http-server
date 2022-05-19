#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>


#define STATUS_SUCCESS 200
#define STATUS_CLIENT_ERROR 404
#define SYSTEM_ERROR -1
#define TEST_ROOT "/www"
#define REQUEST_DELIM " "
#define REQUEST_GET "GET"
#define CRLF "\r\n"
#define DBL_CRLF "\r\n\r\n"

// marking constants
#define MULTITHREADED
#define IMPLEMENTS_IPV6

// file constants
#define TYPE_DELIM '.'
#define PARENT_COMMAND ".."

// size constants
#define ARGUMENT_COUNT 4
#define METHOD_SIZE 4
#define SEND_BUFFER 4096
#define READ_BUFFER 1024
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
  int fileDescriptor;
  int ignoreConfig; // bad cmd line input, don't use this config

};

int convertsToNumber(char* inString);
int isValidPath(char* filePath);
int isValidDirectory(char* filePath);
char* getMIMEType(char* filePath);
void fileSend(char* filePath, int newfd);

int initialiseSocket(int protocolNumber, char* portNumber) {
  // code to setup socket - adapting from code provided in lectures
  int listenfd, re, s;
  struct addrinfo hints, *res, *p;

  printf("- Initialising Socket\n");

  // create socket
  memset(&hints, 0, sizeof hints);



  // assign based on port number
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
    printf("failed allocation\n");
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
    printf("could not find socket on IPv6\n");
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


  listen(listenfd, ALLOWED_CONNECTIONS);
  printf("- Listening on socket at file description %d\n", listenfd);


  return listenfd;

}

void recompileConfig(cmd_args_t* newConfig, cmd_args_t inputConfig, int fD) {
  // take input config, make individual copy for each thread bundled
  // with the fileDescriptor for that thread

  newConfig->protocolNumber = inputConfig.protocolNumber;
  newConfig->portNumber = malloc(strlen(inputConfig.portNumber)+1);
  newConfig->rootPath = malloc(strlen(inputConfig.rootPath)+1);
  strcpy(newConfig->portNumber, inputConfig.portNumber);
  strcpy(newConfig->rootPath, inputConfig.rootPath);

  // bundle fd
  newConfig->fileDescriptor = fD;

  printf("rt %s\n", newConfig->rootPath);



}


cmd_args_t ingestCommandLine(char *argv[]) {
  // read arguments given from command line and save to cmd_args struct
  cmd_args_t newConfig;

  // (IPv)4 or (IPv)6
  if (atoi(argv[1]) == 4 || atoi(argv[1]) == 6) {
    newConfig.protocolNumber = atoi(argv[1]);
  } else {
    // incorrect protocol number
    newConfig.ignoreConfig = 1;
    printf("ERROR: inavlid protocol number\n");
  }


  // port number (as a string)
  if (atoi(argv[2]) >= 0 && convertsToNumber(argv[2])) {
    // port is number within range
    newConfig.portNumber = malloc(strlen(argv[2]) + 1);
    strcpy(newConfig.portNumber, argv[2]);

  } else {
    // port number not acceptable
    printf("ERROR: inavlid port number\n");
    newConfig.ignoreConfig = 1;

  }


  // read path to root
  if (isValidDirectory(argv[3])) {
    // found directory
    newConfig.rootPath = malloc(strlen(argv[3]) + 1);
    strcpy(newConfig.rootPath, argv[3]);

  } else {
    // directory doesn't exist
    printf("ERROR: inavlid directory path\n");
    newConfig.ignoreConfig = 1;

  }


  printf("- (IPv%d, Port %s, Path %s)\n", newConfig.protocolNumber, newConfig.portNumber, newConfig.rootPath);



  return newConfig;
}


char* combinePaths(char* root, char* file) {
  // add file path to end of root location
  printf("root ih: %s, token ih: %s\n",root, file);
  char* totalPath = malloc(strlen(root) + strlen(file)+1);
  strcpy(totalPath, root);
  printf("partial: %s\n", totalPath);
  strcat(totalPath, file);
  printf("final: %s\n", totalPath);

  return totalPath;

}

request_t ingestRequest(char* input, cmd_args_t config) {
  // take raw tcp input and convert to a request type for easy access
  // should do as much error handling in here as possible
  printf("- Ingesting request\n");
  request_t potentialRequest;
  char method[METHOD_SIZE];
  char* newToken = strtok(input, REQUEST_DELIM);

  int tokenCount = 0;

  while (newToken != NULL) {
    printf("newToken = %s\n",newToken);
    int tokenLength = strlen(newToken);


    if (tokenCount == 0) {
      // trying to read request method
      if ((tokenLength == METHOD_SIZE-1) && (strcmp(newToken, REQUEST_GET) == 0)) {
        // matches size of a get request
        strcpy(method, newToken);

      } else {
        // too big/small
        printf("- Invalid request (too big/small)\n");
        potentialRequest.validRequest = 0;
        return potentialRequest;
      }


    } else if (tokenCount == 1) {
      // tring to read filepath now

      // check if file exists at root
      char* potentialFile =  malloc(strlen(config.rootPath) + strlen(newToken)+1);
      printf("root here: %s, token here: %s\n",config.rootPath, newToken);
      potentialFile = combinePaths(config.rootPath, newToken);

      if (isValidPath(potentialFile)) {
        // path is valid, store in request
        printf("h\n");
        potentialRequest.filePath = malloc(strlen(potentialFile)+1);
        strcpy(potentialRequest.filePath, potentialFile);
        printf("a\n");
        // get file type, and store

        char* MIMEType = getMIMEType(newToken);
        printf("MT is %s\n", MIMEType);
        printf("mallocing %ld\n", strlen(MIMEType));
        potentialRequest.fileType = malloc(strlen(MIMEType)+1);
        strcpy(potentialRequest.fileType, MIMEType);
        printf("c\n");
        // file was found, success!
        potentialRequest.statusCode = STATUS_SUCCESS;

      } else {
        // file did not exist, return a 404
        printf("- Invalid request (404)\n");
        potentialRequest.validRequest = 1;
        potentialRequest.statusCode = STATUS_CLIENT_ERROR;
        return potentialRequest;
      }

    } else if (tokenCount == 2) {
      // @ shouldn't need this, but leaving incase we do (e.g. HTTP type)
    }

    tokenCount++;
    newToken = strtok(NULL, REQUEST_DELIM);

  }

  return potentialRequest;
}



void executeRequest(request_t request, int newfd) {
  // given a valid request, respond and then send file

  printf("- Executing request\n");
  // respond to request
  if (request.statusCode == STATUS_SUCCESS) {
    // send confirmation
    char httpConfirm[] = "HTTP/1.0 200 OK\r\n";
    write(newfd, httpConfirm, strlen(httpConfirm));

    // send file header
    char mimeConfirm[] = "Content-Type: ";
    char* mimeHeader = malloc(strlen(mimeConfirm) + strlen(request.fileType) + strlen(DBL_CRLF) + 2);
    strcpy(mimeHeader, mimeConfirm);
    strcat(mimeHeader, request.fileType);
    strcat(mimeHeader, DBL_CRLF);

    write(newfd, mimeHeader, strlen(mimeHeader));
    // since successful, send file also
    fileSend(request.filePath, newfd);

  } else if (request.statusCode == STATUS_CLIENT_ERROR) {
    // send failure message
    printf("- sending failure\n");
    char httpFailure[] = "HTTP/1.0 404 Not Found\r\n\r\n";
    write(newfd, httpFailure, strlen(httpFailure));
  }

  // close socket

  // finished all sending
}


void fileSend(char* filePath, int newfd) {
  // code to send file if found, through socket
  int fileSize;
  struct stat fileStat;

  // get file
  FILE* targetFile = fopen(filePath, "r");
  int totalSent = 0;
  int fd;

  // calculate size of file and allocate buffer space
  fd = fileno(targetFile);
  fstat(fd, &fileStat);
  printf("pre malloc: \n");
  off_t fSize = fileStat.st_size;
  char* fileBuffer = malloc(fSize + 1);
  printf("- file size = %ld\n",fSize);


  // read file to buffer
  while ((fileSize = fread(fileBuffer, sizeof(char), fSize, targetFile)) > 0) {
    // write read amount
    write(newfd, fileBuffer, fileSize);
    totalSent += fileSize;
  }

  printf("- File sent successfully\n");
  printf("- sent %d\n", totalSent);

  // close file
  fclose(targetFile);

  // @ could return write status here if needed, won't do just yet
}


void* serviceRequest(void* configIn) {
  // worker to handle new incomming - can be used standalone or multithreaded

  cmd_args_t config = *((cmd_args_t*)configIn);

  // store local copies of main args
  int newfd = config.fileDescriptor;
  printf("rooooot %s\n", config.rootPath);
  printf("- new thread is alive, running on %d\n", newfd);
  int charsRead;

  // create threadlocal buffer to receive data
  char buffer[SEND_BUFFER];
  bzero(buffer, SEND_BUFFER);


  printf("- reading to buffer\n");
  int index = 0;
  while ((charsRead = read(newfd, buffer + index, SEND_BUFFER - 1)) >= 0) {
    if (charsRead > 0) {
      if (strstr(buffer, DBL_CRLF)) {
        // found double CRLF in buffer
        break;
      }
    }
    // move buffer index along
    index += charsRead;
  }

  printf("- finished reading\n");

  // received get command, pass to ingest
  request_t newRequest = ingestRequest(buffer, config);

  if (newRequest.validRequest == 0 && newRequest.statusCode != STATUS_CLIENT_ERROR) {
    // request failed, but not due to a 404 error
    printf("- invalid request, dropping\n");
    return NULL;
  }

  printf("- valid request! executing\n");

  executeRequest(newRequest, newfd);
  close(newfd);
  printf("- Socket closed in serviceRequest()\n");
  // this request has been serviced.
  // @ here is where thread will close when I get around to it
  return NULL;
}

int main(int argc, char *argv[]) {
  //

  int connfd;

  printf("- Server Startup: entered main\n");

  // read cmdline input to get addrinfo
  if (argc != ARGUMENT_COUNT) {
    // not enough / too many arguments
    exit(EXIT_FAILURE);
  }


  // we have enough arguments, ingest them to config struct
  printf("- Reading command line arguments (%d found)\n", argc);
  cmd_args_t config = ingestCommandLine(argv);

  if (config.ignoreConfig == 1) {
    // something in the command line input was malformed
    // unable to proceed, so terminate server
    printf(" - bad command line input\n");
    exit(EXIT_FAILURE);
  }

  printf("- Config succesfully ingested\n");
  pthread_t threadIdentifier;
  // create socket and start listening on it
  int listenfd = initialiseSocket(config.protocolNumber, config.portNumber);

  if (listenfd == -1) {
    // could not intialiseSocket, therefore can't run server
    printf("- Socket failure\n");
    exit(EXIT_FAILURE);
  }

  printf("- Socket created successfully\n");

  // socket is good to go, begin responding to requests
  while (1) {
    printf("- Accepting new connection...\n");
    // bundle up arguments to pass through to threads
    // accept new connection
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;
    connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_size);
    cmd_args_t* threadConfig = malloc(sizeof(cmd_args_t));
    recompileConfig(threadConfig, config, connfd);

    printf("root %s\n", threadConfig->rootPath);

    printf("\n- new connection found: servicing request\n");
    // pass addr to thread to deal with
    printf("\n == SPINNING UP NEW THREAD (%d) ==\n", connfd);
    pthread_create(&threadIdentifier, NULL, serviceRequest, (void*)threadConfig);
    //serviceRequest(connfd, config);

    printf(" = DETATCHING THREAD\n");
    pthread_detach(threadIdentifier);
    // finished servicing new request


  }

  exit(0);
}


char* getMIMEType(char* filePath) {
  // return the MIME type given the filepath
  char* fileType;

  char* fullStop = strrchr(filePath, TYPE_DELIM);
  // get pointer to full stop delim in path
  if (fullStop == NULL) {
    // no file extension - but if we're here,it must be a valid file
    fullStop = filePath;
  } else {

    // point now to the filetype itself
    fullStop = fullStop + 1;
  }

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
  printf("type: %s\n",fileType);
  return fileType;
}


int convertsToNumber(char* inString) {
  // checks if this string contains an integer once converted
  for (int i=0; i<strlen(inString); i++) {
      if (!isdigit(inString[i])) {
        // found something that isn't a digit
        return 0;
      }

  }

  // no non-digits found
  return 1;
}

int isValidDirectory(char* filePath) {
  // helper func to check directory exists
  struct stat dirStat;
  if (stat(filePath, &dirStat) != 0) {
    // some error has occured in creating the stat, so not dir
    return 0;
  }


  if (!S_ISDIR(dirStat.st_mode)) {
    // is file, not directory
    printf("- The path you've given is to a file, not a directory\n");
    return 0;
  }
  return 1;
}


int isValidPath(char* filePath) {
  // helper func to check filepath exists and type is correct
  printf("-- trying to open file at: \n");
  printf("-- %s\n",filePath);
  if (strlen(filePath) > 0) {
    // path isn't empty
    if (fopen(filePath, "r") != NULL) {
      // able to open

      // check for escape attempt
      if (strstr(filePath, PARENT_COMMAND) == 0) {
        printf("-- good file\n");
        return 1;
      } else {
        printf("-- escape attempt found\n");
      }
    } else {
      printf("-- unable to open\n");
    }
  } else {
    printf("-- path is empty\n");
  }

  // unable to open / illegal instruction
  return 0;

}
