#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>
#include "serverUtils.h"


// checks if this string represents a number
int convertsToNumber(char* inString) {
  // checks if this array is a string representing an integer
  for (int i=0; i<strlen(inString); i++) {
      if (!isdigit(inString[i])) {
        // found something that isn't a digit
        return 0;
      }
  }

  // no non-digits found
  return 1;
}

// check if filepath points to a (valid) directory
int isValidDirectory(char* filePath) {
  struct stat dirStat;
  if (stat(filePath, &dirStat) != 0) {
    // some error has occured in creating the stat, so not dir
    return 0;
  }


  if (!S_ISDIR(dirStat.st_mode)) {
    // is file, not directory
    return 0;
  }

  // path exists and isn't a file, must be dir
  return 1;
}

// check if path meets spec requirements for a 'valid path'
int isValidPath(char* filePath) {
  if (strlen(filePath) > 0) {
    // path isn't empty

    if (fopen(filePath, "r") != NULL) {
      // able to open

      // check for escape attempt - root path should have been resolved at this
      // point so safe to just check entire path for the /../ component
      if (strstr(filePath, ESCAPE_COMMAND) == 0) {
        return 1;
      }
    }
  }

  // unable to open / illegal instruction / path empty
  return 0;

}

// returns the MIME type of the passed file
char* getMIMEType(char* filePath) {

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

  // set fileType to matching MIME constant
  if (strcmp("html", fullStop) == 0) {
    fileType = MIME_HTML;
  } else if (strcmp("jpg", fullStop) == 0) {
    fileType = MIME_JPEG;
  } else if (strcmp("css", fullStop) == 0) {
    fileType = MIME_CSS;
  } else if (strcmp("js", fullStop) == 0) {
    fileType = MIME_JAVASCRIPT;
  } else {
    // none matched, uses octet stream
    fileType = MIME_OTHER;
  }

  return fileType;
}


// send passed file 'filePath' to passed socket file descriptor 'newfd'
void fileSend(char* filePath, int newfd) {
  // code to send file through socket
  int fileSize;
  struct stat fileStat;

  // get file
  FILE* targetFile = fopen(filePath, "r");
  int totalSent = 0;
  int fd;

  // calculate size of file and allocate buffer space
  fd = fileno(targetFile);
  fstat(fd, &fileStat);

  off_t fSize = fileStat.st_size;
  char* fileBuffer = malloc(fSize + 1);

  // read file to buffer
  while ((fileSize = fread(fileBuffer, sizeof(char), fSize, targetFile)) > 0) {
    // write read amount
    write(newfd, fileBuffer, fileSize);
    totalSent += fileSize;
  }

  // close file
  fclose(targetFile);
}


// returns pointer to the 'file' path concatenated to the 'root' path
 char* combinePaths(char* root, char* file) {
  // concat file path to end of root filepath
  char* totalPath = malloc(strlen(root) + strlen(file)+1);
  strcpy(totalPath, root);
  strcat(totalPath, file);

  return totalPath;

}


// copies 'inputConfig' values into 'newConfig', inserting the 'fD' value also
void recompileConfig(cmd_args_t* newConfig, cmd_args_t inputConfig, int fD) {
  // take input config, copies values into the malloced newConfig, and inserts
  // the thread-specific file descriptor value

  newConfig->protocolNumber = inputConfig.protocolNumber;

  // malloc space for strings for this thread
  newConfig->portNumber = malloc(strlen(inputConfig.portNumber)+1);
  newConfig->rootPath = malloc(strlen(inputConfig.rootPath)+1);

  strcpy(newConfig->portNumber, inputConfig.portNumber);
  strcpy(newConfig->rootPath, inputConfig.rootPath);

  // bundle fd
  newConfig->fileDescriptor = fD;

}
