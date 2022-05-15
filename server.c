#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATUS_SUCCESS 200
#define STATUS_CLIENT_ERROR 404
#define TEST_ROOT "/www"
#define REQUEST_DELIM " "
#define REQUEST_GET "GET"

// size constants
#define METHOD_SIZE 3




typedef struct request_t request_t;
struct request_t {
  int statusCode;
  int handlerID;
  char[] rootDirectory;
  char[] filePath;
  int validRequest; // request is finished and can be used
};


void initialiseSocket() {
  // code to setup socket

}

void newRequest() {
  // code to handle a new request - will be assigned to threads if I get that far
  int a = 2;
}


request_t ingestRequest(char* input) {
  // take raw cmndline input and convert to a request type for easy access
  // should do as much error handling in here as possible
  request_t potentialRequest;
  char[METHOD_SIZE] method;
  char* newToken;

  int tokenCount = 0;

  while ((newToken = strtok(input, REQUEST_DELIM) != NULL) {
    int tokenLength = strlen(newToken);

    if (tokenCount == 0) {
      // trying to read request method
      if ((tokenLength == METHOD_SIZE) && (strcmp(newToken, REQUEST_GET) == 0)) {
        // matches size of a get request
        method = newToken;

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


      } else {
        // too big/small
        potentialRequest.validRequest = 0;
        return potentialRequest;
      }
    } else if (tokenCount == 2) {

    }





    tokenCount++;

  }


};


void executeRequest(reuqest_t request) {
  // given a valid request, respond and then send file
}

void sendFile(char* filePath) {
  // code to send file if found, through socket
}

int main(int argc, char *argv[]) {







  return 0;
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
