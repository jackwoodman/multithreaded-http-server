#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATUS_SUCCESS 200
#define STATUS_CLIENT_ERROR 404
#define TEST_ROOT "/www"
#define REQUEST_DELIM " "
#define REQUEST_GET "GET"

// file constants
#define TYPE_DELIM ','
// size constants
#define METHOD_SIZE 3


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
  char* rootDirectory;
  char* filePath;
  int validRequest; // request is finished and can be used
};

int validPath(char* filePath);
char* getMIMEType(char* filePath);


void initialiseSocket() {
  // code to setup socket

}

void newRequest() {
  // code to handle a new request - will be assigned to threads if I get that far
}


request_t ingestRequest(char* input) {
  // take raw cmndline input and convert to a request type for easy access
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
      if (validPath(newToken)) {
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

    }


    tokenCount++;

  }

  return potentialRequest;
}


void executeRequest(request_t request) {
  // given a valid request, respond and then send file
}

void sendFile(char* filePath) {
  // code to send file if found, through socket
}

int main(int argc, char *argv[]) {







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


int validPath(char* filePath) {
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
