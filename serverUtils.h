// serverUtils.h - helper functions that provide services to the server

#ifndef REQUEST_TYPE
#define REQUEST_TYPE
typedef struct request_t request_t;
struct request_t {
  int statusCode;
  int handlerID;
  char* fileType;
  char* filePath;
  int validRequest; // request is finished and can be used

};
#endif

#ifndef CMD_ARGS_TYPE
#define CMD_ARGS_TYPE
typedef struct cmd_args_t cmd_args_t;
struct cmd_args_t {
  int protocolNumber;
  char* portNumber;
  char* rootPath;
  int fileDescriptor;
  int ignoreConfig; // bad cmd line input, don't use this config

};
#endif

// file/response types (MIME)
#define MIME_HTML "text/html"
#define MIME_JPEG "image/jpeg"
#define MIME_CSS "text/css"
#define MIME_JAVASCRIPT "text/javascript"
#define MIME_OTHER "application/octet-stream"

// file constants
#define TYPE_DELIM '.'
#define ESCAPE_COMMAND "/../"


// checks if this string represents a number
int convertsToNumber(char* inString);

// check if path meets spec requirements for a 'valid path'
int isValidPath(char* filePath);

// check if filepath points to a (valid) directory
int isValidDirectory(char* filePath);

// returns the MIME type of the passed file
char* getMIMEType(char* filePath);

// send passed file 'filePath' to passed socket file descriptor 'newfd'
void fileSend(char* filePath, int newfd);
char* combinePaths(char* root, char* file);
void recompileConfig(cmd_args_t* newConfig, cmd_args_t inputConfig, int fD);
