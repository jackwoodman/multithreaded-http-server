#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATUS_SUCCESS 200
#define STATUS_CLIENT_ERROR 404
#define TEST_ROOT "/www"




typedef struct request_t request_t;
struct request_t {
  int statusCode;
  int handlerID;
  char[] rootDirectory;
  char[] filePath;
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
