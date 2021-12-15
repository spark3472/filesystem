//TO-RUN: make, ./format DISK, ./sampleDisk

/*
Links to try to fix VSC thing:
https://askubuntu.com/questions/1022923/cannot-open-visual-studio-code

*/

/*
    Simple shell (no backgrounding or job control)
    Handles & and ;, but treats & like a ; since no backgrounding
    No memory errors that I can find!

    built-in commands:
    - ls only supports listing one directory at a time
    - mkdir allows names with any characters (doesn't exclude '(', '$', etc))
    - cat doesn't support the echo feature linux does when no file is given
    - more uses enter instead of space and only goes through pages at a time, have to press enter after typing q

    mounting a disk????
    - no clue if I did it right
    - implement manual disk mounting if one not there automatically
    - am I allowed to use fopen to mount??

    redirection
    - how to do < redirection???? when used???
*/

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <termios.h>
#include <signal.h>
#include <sys/wait.h>
#include "vfs.h"

#define FILELENGTH 256
#define PATHLENGTH 15

int mounted;
FILE *disk;

//global array toks
char **toks;

char *workingDirectory;
char *parentDirectory;

int amChild;

/******PARSER******/

/***** Code outline for parser and tokenizer from HW2Feedback slides *****/
//holds a string and the current position in it
typedef struct tokenizer{
  char* str;
  char *pos;
} TOKENIZER;

/* Identifies if a symbol is one of the set delimiters
 * @param symbol The character to check
 * Returns true if the character is a delimiter
 */
int isDelimiter(char symbol) {
  int numDelim = 5;
  //implement '>>' later
  char delimiters[] = {'&', ';', '>', '<', '>'};
  int included = FALSE;

  for(int i = 0; i < numDelim; i++) {
    if(symbol == delimiters[i]) {
      included = TRUE;
    }
  }

  return included;
}

/* Gets the next delimiter or the string between delimiters
 * @param tokenizer
 * @return Pointer to the string between the delimiters or the delimiter
 */
int a = 0;
char* get_next_token(TOKENIZER *v){
	//if current char is a delimiter, just return it
  //else go until next char is a delimiter
  //return the substring without white spaces
  //returned strings are malloced copies
  //return NULL when string ends
  
  char* string;
  int b = 0;
  if(*(v->pos) == '\0'){
    return NULL;
  }
  
  if (*(v->pos) == ' '){
    a++;
    v->pos++;
  }
  //add other parsing here
  if (isDelimiter(*(v->pos)) == TRUE){
    if(*(v->pos) == '>') {
      if(*(v->pos + 1) == '>') {
        b++;
        v->pos++;
      }
    }
    b++;
    v->pos++;
  }else {
    while(*(v->pos) != '\0'){
      if (isDelimiter(*(v->pos)) == TRUE || *(v->pos) == ' '){
        break;
      } else {
        v->pos++;
        b++;
      }
    }
  }
  string = (char*)malloc((b+1)*sizeof(char));
  //valgrind doesn't like this
  //memory error happens 
  memcpy(string, &v->str[a], b);
  string[b] = '\0';
  a += b;
  return string;
}

/* Gets the starting point of the line
 * @param line
 * @return pointer that points to the start of the line
 */
TOKENIZER init_tokenizer(char* line){
  TOKENIZER s;
  s.str = line;
  s.pos = s.str;
  return s;
}

/* Stores pointers to tokens in global array toks
 * @return Number of tokens
 */
TOKENIZER t;
TOKENIZER u;
char* line;
char* shell_prompt;
int parser(){
  int n = 0;
  //valgrind doesn't like this line??
  line = readline(shell_prompt);
  //ctrl-d
  if(line == NULL){
    free(line);
    return 0;
  }
  //newline
  if(strcmp(line,"") == 0){
    free(line);
    return 0;
  }
  t = init_tokenizer(line);
  //how many tokens
  char* string;
  a = 0;
  while((string = get_next_token(&t)) != NULL){
    n++;
    free(string);
  }
  //allocate pointers to tokens +1 for the ending NULL
  toks = (char**) malloc(sizeof(char*) * (n+1));
  toks[n] = NULL;
  //start from beginning again
  u = init_tokenizer(line);
  a = 0;
 
  for(int i = 0; i < n; i++){
    char* string = get_next_token(&u);
    toks[i] = (char*)malloc((strlen(string)+1)*sizeof(char));
    strcpy(toks[i], string);
    //maybe don't need
    //toks[i][strlen(string)] = '\0';
    free(string);
  }
  free(line);
  return n;
}

/*
Isolates and returns part of the typed command line (from position start to end)
*/
char** getArgs(int start, int end){
  int args = end - start;
  char** currentArguments = (char**) malloc(sizeof(char*)*((end-start)+1));

  currentArguments[args] = NULL;
  int count = 0;
  for (int j = start; j < end; j++){
    currentArguments[count] = malloc(sizeof(char*) * (strlen(toks[j]) + 1));
    strcpy(currentArguments[count], toks[j]);
    //printf("%s\n", currentArguments[count]);
    count++;
  }

  return currentArguments;
}

char **directoryPath;

int pathLength(char *path_split) {
  int length = 1;
  char *token;
  char *path = malloc(FILELENGTH);
  strcpy(path, path_split);
  token = strtok(path, "/");
  while(token != NULL) {
    token = strtok(NULL, "/");
    length++;
  }
  free(path);
  return length;
}

char** split(char *path_split, int directory) {
  char **splitPath = malloc(sizeof(char*) * PATHLENGTH);
  char *token;
  char *path = malloc(FILELENGTH);
  strcpy(path, path_split);
  int count = 0;
  //printf("Tokens:\n");
  //add root to the list
  /*if(path_split[0] == '/') {
    splitPath[count] = malloc(FILELENGTH);
    strcpy(splitPath[count], "/");
    count++;
  }*/
  token = strtok(path, "/");
  while(token != NULL) {
    splitPath[count] = malloc(FILELENGTH);
    strcpy(splitPath[count], token);
    //printf("%s\n", token);
    token = strtok(NULL, "/");
    count++;
  }
  free(path);
  return splitPath;
}

/* Gets the absolute path of a file
 * 
 */
char* getAbsPath(char *path, int directory) {
  //MODIFY free this
  split(path, directory);
  char *absPath = malloc(FILELENGTH);
  strcpy(absPath, workingDirectory);
  return strcat(absPath, path);
}

int dirContains(char *dirPath, char *item) {
  int contains = FALSE;
  
  int dir = f_opendir(dirPath);
  if(dir == -1) {
    fprintf(stderr, "error opening directory\n");
    return -1;
  }

  if(f_rewinddir(dir) == -1) {
    fprintf(stderr, "error with directory\n");
    return -1;
  }

  DirEntry *child;

  while((child = f_readdir(dir)) != NULL) {
    if(strcmp(item, child->fileName) == 0) {
      contains = TRUE;
      break;
    }
  }

  //f_closedir(dir);

  return contains;
}

void ls(char *pathList, char flags[2]) {
  //printf("doing ls - filename: %s, flags: %s\n", pathList, flags);
  //getAbsPath(pathList, TRUE);
  char *path = malloc(FILELENGTH);
  if(strcmp(pathList, ".") == 0) {
    strcpy(path, workingDirectory);
  } else if(strcmp(pathList, "..") == 0) {
    strcpy(path, parentDirectory);
  } else {
    //fprintf(stderr, "Not supported yet, use . or ..\n");
    //strcpy(path, getAbsPath(pathList, TRUE));
    strcpy(path, workingDirectory);
    strcat(path, pathList);
    strcat(path, "/");
  }
  printf("path is %s\n", path);

  //must not be opening it
  int dir = f_opendir(path);
  if(dir == -1) {
    fprintf(stderr, "ls: error opening directory\n");
    return;
  }

  if(f_rewinddir(dir) == -1) {
    fprintf(stderr, "ls: error with directory\n");
    return;
  }

  DirEntry *child;

  while((child = f_readdir(dir)) != NULL) {
    printf("%s\t", child->fileName);
  }
  printf("\n");

  free(path);
  //f_closedir(dir);
  //support '.' and '..'
}

void chmod(char *fileName, char *permisisons, int directory) {
  printf("doing chmod - filename: %s, permissions: %s, directory?: %d\n", fileName, permisisons, directory);
}

void mkdir(char *fileName) {
  //printf("doing mkdir - filename: %s\n", fileName);
  //check if directory already exists
  //deal with permissions later
  //deal with .. later
  char *path = malloc(FILELENGTH);
  strcpy(path, workingDirectory);
  char **splitPath = split(fileName, TRUE);

  int path_length = pathLength(fileName);
  int entries = 0;
  //printf("Split paths:\n");
  //maybe edit
  while(splitPath[entries] != NULL) {
    //printf("%s\n", splitPath[entries]);
    entries++;
  }
  //move back to last entry
  entries--;

  printf("successive paths\n");
  char *parent = malloc(FILELENGTH);
  for(int i = 0; i <= entries; i++) {
    printf("%s\n", path);
    strcpy(parent, path);
    strcat(path, splitPath[i]);
    int openDir;
    if(dirContains(parent, splitPath[i]) == TRUE) {
      if(i == entries) {
        fprintf(stderr, "mkdir: cannot create directory %s because it already exists\n", fileName);
        return;
      } else {
        if((openDir = f_opendir(path)) == -1) {
          fprintf(stderr, "mkdir: error opening %s\n", splitPath[i]);
          return;
        }
      }

    } else {
      if(i == entries) {
        if(f_mkdir(parent, splitPath[i], 777) == -1){
          fprintf(stderr, "mkdir: error making directory %s\n", splitPath[i]);
          return;
        }
        if((openDir = f_opendir(path)) == -1) {
          fprintf(stderr, "mkdir: error opening %s\n", splitPath[i]);
          return;
        }
      } else {
        fprintf(stderr, "mkdir: cannot create directory %s: No such file or directory\n", fileName);
        return;
      }
    }
    /*if(( openDir = f_opendir(path) ) != -1) {
      if(i == entries) {
        fprintf(stderr, "mkdir: cannot create directory %s because it already exists\n", fileName);
        return;
      }
      //f_closedir(openDir);
    } else {
      if(f_mkdir(parent, splitPath[i], 777) == -1){
        fprintf(stderr, "mkdir: error making directory %s\n", fileName);
        return;
      }
    }*/
    
    strcat(path, "/");
  }
  for(int i = 0; i < path_length; i++) {
    free(splitPath[i]);
  }
  free(splitPath);
  free(parent);
  free(path);
}

void rmdir_new(char *fileName) {
  printf("doing rmdir - filename: %s\n", fileName);
}

void cd(char *filePath) {
  char *path = malloc(FILELENGTH);
  if(filePath[0] == '/') {
    strcpy(path, filePath);
  } else {
    strcpy(path, workingDirectory);
    if(strcmp(filePath, ".") != 0) {
      strcat(path, filePath);
    }
  }

  printf("doing cd - filepath: %s\n", path);

  int path_length = pathLength(path);
  char **splitPath = split(path, TRUE);
  printf("Path length is %d\n", path_length);

  printf("successive paths\n");
  char *parent = malloc(FILELENGTH);
  strcpy(path, "/");
  strcpy(parent, "/");
  for(int i = 0; i < path_length-1; i++) {
    printf("%s\n", path);
    strcpy(parent, path);
    strcat(path, splitPath[i]);
    int openDir;
    if(dirContains(parent, splitPath[i]) == TRUE) {
      if(i == path_length) {
        //desired outcome
      } else {
        if((openDir = f_opendir(path)) == -1) {
          fprintf(stderr, "cd: error opening %s\n", splitPath[i]);
          return;
        }
      }

    } else {
      fprintf(stderr, "cd: the following directory doesn't exist: %s\n", path);
      return;
    }
    
    strcat(path, "/");
  }

  strcpy(workingDirectory, path);
  strcpy(parentDirectory, parent);
  printf(". is %s, .. is %s\n", workingDirectory, parentDirectory);
  strcpy(shell_prompt, workingDirectory);
  strcat(shell_prompt, "> ");

  for(int i = 0; i < path_length; i++) {
    free(splitPath[i]);
  }
  free(splitPath);
  free(parent);
  free(path);
  
}

void pwd() {
  //printf("doing pwd\n");
  printf("%s\n", workingDirectory);
}

//works with small file - test with bigger file when have one
void cat(char **files, int num) {
  int file;
  for(int i = 0; i < num; i++) {
    file = f_open(workingDirectory, files[i], OREAD);
    int n;
    int size = FILELENGTH;
    char buffer[size+1];
    if(file != -1) {
      while((n = f_read(&buffer, size, 1, file)) != 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
      }
    } else {
      printf("cat: %s - no such file or directory\n", files[i]);
    }
    f_close(file);
    //printf("\n");
  }
}

void cat2(char **files, int num) {
  FILE *file;
  for(int i = 0; i < num; i++) {
    printf("cat on %s\n", files[i]);
    file = fopen(files[i], "r");
    int n;
    int size = FILELENGTH;
    char buffer[size+1];
    //printf("Made it to here, file is %d\n", file);
    if(file != NULL) {
      printf("Starting reading\n");
      while((n = fread(&buffer, 1, size, file)) != 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
      }
    } else {
      printf("cat: %s - no such file or directory\n", files[i]);
    }
    printf("\n");
  }
}

/*
Prints a set amount of each file at a time (doesn't support line-by-line paging)
Press q + enter to exit
*/
void more2(char **files, int num) {
  printf("Press enter to page through, and q + enter to exit\n");
  FILE *file;
  char c;
  //for each file...
  for(int i = 0; i < num; i++) {
    file = fopen(files[i], "r");
    //if multiple files, print the name before each
    if(num > 1) {
      printf("==========\n%s\n==========\n", files[i]);
    }
    if(file == NULL) {
      printf("more: %s - no such file or directory\n", files[i]);
    } else {
      int n;
      int size = 100;
      char buffer[size+1];
      //read a set number of bytes at a time
      while((n = fread(&buffer, 1, size, file)) != 0) {
        //printf("Head of buffer\n");
        buffer[n] = '\0';
        printf("%s", buffer);
        //continue until the next new line
        if(buffer[size-1] != '\n') {
          char smallBuffer[] = {'\0', '\0'};
          while((n = fread(&smallBuffer, 1, 1, file)) != 0 && smallBuffer[0] != '\n') {
            printf("%s", smallBuffer);
          }
        }

        c = getchar();
        //if q typed, exit
        if(c == 'q') {
          return;
        }

      }
    }
    printf("\n");
    if(i != num-1) {
      c = getchar();
      //if q typed, exit
      if(c == 'q') {
        return;
      }
    }
  }
}

//getting conditional jump etc in valgrind, investigate
void more(char **files, int num) {
  printf("Press enter to page through, and q + enter to exit\n");
  int file;
  char c;
  //for each file...
  for(int i = 0; i < num; i++) {
    file = f_open(workingDirectory, files[i], OREAD);
    //if multiple files, print the name before each
    if(num > 1) {
      printf("==========\n%s\n==========\n", files[i]);
    }
    if(file == -1) {
      printf("more: %s - no such file or directory\n", files[i]);
    } else {
      int n;
      int size = 100;
      char buffer[size+1];
      //read a set number of bytes at a time
      while((n = f_read(&buffer, 1, size, file)) != 0) {
        //printf("Head of buffer\n");
        buffer[n] = '\0';
        printf("%s", buffer);
        //continue until the next new line
        if(buffer[size-1] != '\n') {
          char smallBuffer[] = {'\0', '\0'};
          while((n = f_read(&smallBuffer, 1, 1, file)) != 0 && smallBuffer[0] != '\n') {
            printf("%s", smallBuffer);
          }
        }

        c = getchar();        
        //if q typed, exit
        if(c == 'q') {
          return;
        }
      }
    }
    printf("\n");
    if(i != num-1) {
      c = getchar();
      //if q typed, exit
      if(c == 'q') {
        return;
      }
    }
    f_close(file);
  }
}

void rm(char *fileName) {
  printf("doing rm - filename: %s\n", fileName);
}

void mount(char *fileSys, char *location) {
  printf("doing mount - needs to be motified\n");
  //grafting disk onto a point in existing file tree
    // treat disk like located at particular folder
    // if stuff already there, gets hidden until disk unmounted
}

void unmount(char *fileSys, char *location) {
  printf("doing unmount - needs to be modified\n");
}

void sig_handler(int signo) {
  printf("Signal caught\n");
  if(amChild == TRUE) {
    printf("Am child\n");
    exit(0);
  }
}

int main(int argc, char *argv[]){

  //blocking listed signals in the shell so you can't quit it
  setpgid(0,0);
  sigset_t sigset, sigset_old;
  sigemptyset(&sigset);
  sigemptyset(&sigset_old);
  sigaddset(&sigset, SIGQUIT);
  sigaddset(&sigset, SIGTTIN);
  sigaddset(&sigset, SIGTTOU);
  sigaddset(&sigset, SIGINT);
  sigaddset(&sigset, SIGTSTP);
  sigprocmask(SIG_BLOCK, &sigset, &sigset_old);

  amChild = FALSE;
  int number;

  //is this how you mount a disk???
  if(access("./DISK", F_OK ) == 0) {
    // file exists
    // mount

    // set up and get ready to read stuff 
    // set up directory structure so root is root of disk
    // do log in
    int outcome = f_mount("./DISK", "/");
    if(outcome == -1) {
      fprintf(stderr, "Error mounting disk\n");
    }
    
  } else {
    printf("No disk was found, please use \"format\" to create a disk.\n");
    printf("To format a disk, type \"./format DISK\"\n");
    
    int diskMounted = FALSE; 
    while(diskMounted == FALSE) {
      number = parser();
      if(number == 0) {
        if(toks == NULL) {
          free(toks);
        }
        continue;
      }
      //if user types "exit", leave
      if(0 == strcmp(toks[0], "./format")) {
        pid_t pid;
        if((pid = fork()) == 0) {
          //puts the child process in its own process group
          setpgid(getpid(), 0);
          if( -1 == execvp(toks[0], toks) ){
            fprintf(stderr, "%s: command not found\n", toks[0]);
            for(int i = 0; i < number; i++){
              free(toks[i]);
            }
            free(toks);
          }
          exit(0);
          
        } else if (pid > 0) {
          waitpid(pid, NULL, 0);
          tcsetpgrp(STDIN_FILENO, getpid());
        }
    
      } else {
        fprintf(stderr, "Please use \"format DISK\" to create a disk\n");
      }

      /*for(int i = 0; i < number; i++){
        free(toks[i]);
      }
      free(toks);
      free(line); */

      int outcome = f_mount("./DISK", "/");
      if(outcome == -1) {
        fprintf(stderr, "Error mounting disk, try again\n");
      } else {
        diskMounted = TRUE;
      }

    }

  }

  int maxPathSize = FILELENGTH;
  workingDirectory = malloc(maxPathSize * sizeof(char));
  parentDirectory = malloc(maxPathSize * sizeof(char));

  strcpy(workingDirectory, "/");
  strcpy(parentDirectory, "/");

  //put at user directory later

  //log-on procedure outline
  /*
  int length = 21;
  char *username = malloc(length * sizeof(char));
  char *password = malloc(length * sizeof(char));
  printf("Please log on. Username (type anything): ");
  scanf("%20s", username);
  while ((getchar()) != '\n');
  printf("Password (type anything): ");
  scanf("%20s", password);
  printf("Welcome %s!\n", username);
  */

  //int aftersemi = 0;
  
  shell_prompt = malloc(maxPathSize);
  strcpy(shell_prompt, workingDirectory);
  strcat(shell_prompt, "> ");

  while(1){
    number = parser();

    if(number == 0) {
      //can cause issues :( if things typed while command is executing throws double free
      if(toks == NULL) {
        free(toks);
      }
      continue;
    }

    //if user types "exit", leave
    if(0 == strcmp(toks[0], "exit")) {
      exit(0);
    }

    //holds the number of ampersands and semicolons in the typed command
    int ampOrSemi = 0;
    for (int i = 0; i < number; i++){
      if ((strcmp(toks[i], "&") == 0) || (strcmp(toks[i], ";") == 0)){
        ampOrSemi++;
      }      
    }

    //making an array of the location of the &'s and ;'s
    int countPlaces = 0;
    char ampSemiPlaces[ampOrSemi][2];
    for (int i = 0; i < number; i++){
      if ((strcmp(toks[i], "&") == 0) || (strcmp(toks[i], ";") == 0)){
        ampSemiPlaces[countPlaces][0] = i;
        ampSemiPlaces[countPlaces][1] = toks[i][0];
        countPlaces++;
      }      
    }

    printf("====\n");
    for(int i = 0; i < number; i++) {
      printf("%s\n", toks[i]);
    } 
    printf("====\n");

    int tokensExamined = 0;
    int commandsRun = 0;
    char *redirection = malloc(4 * sizeof(char));
    while(tokensExamined < number) {
      //start and end of the current command section
      int start = 0;
      int end = number;

      /* Update the start and end position of the current command if it contains
       * an ampersand or semicolon */
      if(commandsRun > 0) {
        //start one after the last & or ;
        start = ampSemiPlaces[commandsRun-1][0] + 1;
      } 
      if(commandsRun < ampOrSemi) {
        end = ampSemiPlaces[commandsRun][0];
      }

      //get the current section of the typed command
      char** currentArgs = getArgs(start, end);
      int length = end - start;

      /* DOING REDIRECTION HERE */
      //char *redirection = malloc(4 * sizeof(char));
      strcpy(redirection, "no");
      char *fileRedirect = malloc(FILELENGTH * sizeof(char));

      //Check if the subsection contains <, >, or >> and update currentArgs and redirect info accordingly
      int redir = FALSE;
      for (int i = 0; i < length; i++){
        if ((strcmp(currentArgs[i], ">") == 0) || (strcmp(currentArgs[i], "<") == 0) || (strcmp(currentArgs[i], ">>") == 0)){
          if(redir == TRUE) {
            //MODIFY m_error = ....
            printf("Multiple redirection not supported.\n");
            continue;
          }
          if (i == length - 1) {
            printf("Please enter a file after the redirection");
            continue;
          }
          redir = TRUE;
          if(strcmp(currentArgs[i], "<") == 0) {
            strcpy(redirection, "in");
          } else if(strcmp(currentArgs[i], ">") == 0) {
            strcpy(redirection, "out");
          } else {
            //append to end of out file
            strcpy(redirection, "app");
          }
          strcpy(fileRedirect, currentArgs[length - 1]);

          //free old args and make the new ones (excluding redirection info)
          for(int i = 0; i < length; i++) {
            free(currentArgs[i]);
          }
          free(currentArgs);
          currentArgs = getArgs(start, end - 2);
          length -= 2;
        }      
      }

      if(0 == strcmp(currentArgs[0], "mkdir")) {
        int argPos = 1;
        //makes a directory for each given name
        while(argPos < length) {
          char *arg = currentArgs[argPos];
          mkdir(arg);
          argPos++;
        }
        //checks if no directory name was given
        if(length == 1) {
          printf("mkdir: please specify a directory name\n");
        }
      }


      if(0 == strcmp(currentArgs[0], "cd")) {
        char *filePath = ".";
        int skip = FALSE;
        
        if(length == 2) {
          filePath = currentArgs[1];
        } else if(length > 2) {
          printf("cd: too many arguments\n");
          skip = TRUE;
        }

        if(skip == FALSE) {
          cd(filePath);
        }
      }


      //printf("Parent pgid: %d; ", getpgrp());
      pid_t pid;
      if((pid = fork()) == 0) {
        amChild = TRUE;
        //puts the child process in its own process group
        setpgid(getpid(), 0);
        tcsetpgrp(STDIN_FILENO, getpid());

        //reset signal masks to default
        int outcome = sigprocmask(SIG_SETMASK, &sigset_old, NULL);
        if(outcome == -1) {
          printf("Error setting signal mask in child process\n");
        }

        //setting up stream redirection for stdout and stdin as necessary
        int outTemp, inTemp; //not closing currently I think, add if problems arise
        if(redir == TRUE && strcmp(redirection, "in") != 0) {
          //for output redirection
          //redirect stdout to temp.txt
          outTemp = open("temp.txt", O_RDWR|O_CREAT, 0600);
          if (-1 == dup2(outTemp, fileno(stdout))) {
            perror("Stdout redirection error");
            exit(0);
          }
        } else if(redir == TRUE && strcmp(redirection, "in") == 0) {
          //for input redirection
          //make a temporary file on UNIX with the input redirection
          int infd = f_open(workingDirectory, fileRedirect, ORDWR);
          struct stat_t *buf = malloc(sizeof(struct stat_t));
          int s = f_stat(buf, infd);
          if(s == -1) {
            fprintf(stderr, "Stdin redirection error\n");
          }
          
          //read in file
          void *ptr = malloc(buf->size);
          f_read(ptr, buf->size, 1, infd);
          
          //copy into temp.txt
          inTemp = open("./temp.txt", O_RDWR|O_CREAT, 0600);
          int w = write(inTemp, ptr, buf->size);
          if(w == -1) {
            fprintf(stderr, "Stdin redirection error\n");
            exit(0);
          }
          int l = lseek(inTemp, 0, SEEK_SET);
          if(l == -1) {
            fprintf(stderr, "Stdin redirection error\n");
            exit(0);
          }
          free(ptr);
          
          if(inTemp == -1) {
            perror("Stdin redirection error");
            exit(0);
          }
          if (-1 == dup2(inTemp, fileno(stdin))) {
            perror("Stdin redirection error");
            exit(0);
          }
        }
        
        if(0 == strcmp(currentArgs[0], "ls")) {
          char flags[2] = "\0";
          int argPos = 1;
          int flagsSeen = 0;
          char *fileName = malloc(FILELENGTH);
          strcpy(fileName, ".");
          int skip = FALSE;
          
          //goes through each argument and classifies it as a filename or flag to feed to ls()
          while(argPos < length) {
            char *arg = currentArgs[argPos];
            if(arg[0] == '-') {
              if((arg[1] != 'l' || arg[1] != 'F') && arg[2] != '\0') {
                printf("ls: invalid option -- '%s'\n'-l' and '-F' are the only supported flags.\n", arg);
                skip = TRUE;
                break;
              }
              flags[flagsSeen] = currentArgs[argPos][1];
              flagsSeen++;
            } else {
              if(strcmp(fileName, ".") == 0) {
                strcpy(fileName, arg);
              } else {
                printf("ls only supports listing one directory - please enter %s on a seperate line\n", arg);
                break;
              }
            }
            argPos++;
          }

          if(skip == FALSE) {
            ls(fileName, flags);
          }
          free(fileName);
        } else if(0 == strcmp(currentArgs[0], "chmod")) {
          int skip = FALSE;
          int directory = FALSE;
          char *fileName = NULL;
          char *permissions = NULL;

          if(length < 3 || length > 4) {
            printf("chmod: wrong number of arguments\n");
            skip = TRUE;
          } else {
            permissions = currentArgs[1];
            if(length == 3) {
              fileName = currentArgs[2];
            } else {
              fileName = currentArgs[3];
              if(0 != strcmp(currentArgs[2], "-r")) {
                printf("chmod: unsupported flag\n");
                skip = TRUE;
              }
              directory = TRUE;
            }
          }

          if(skip == FALSE) {
            chmod(fileName, permissions, directory);
          }
        } else if(0 == strcmp(currentArgs[0], "mkdir")) {
          /*int argPos = 1;
          //makes a directory for each given name
          while(argPos < length) {
            char *arg = currentArgs[argPos];
            mkdir(arg);
            argPos++;
          }
          //checks if no directory name was given
          if(length == 1) {
            printf("mkdir: please specify a directory name\n");
          }*/
        } else if(0 == strcmp(currentArgs[0], "rmdir")) {
          int argPos = 1;
          //removes the directory with each given name
          while(argPos < length) {
            char *arg = currentArgs[argPos];
            rmdir_new(arg);
            argPos++;
          }
          //checks if no directory name was given
          if(length == 1) {
            printf("rmdir: please specify a directory name\n");
          }
        } else if(0 == strcmp(currentArgs[0], "cd")) {
          /*char *filePath = NULL;
          int skip = FALSE;
          
          if(length == 2) {
            filePath = currentArgs[1];
          } else if(length > 2) {
            printf("cd: too many arguments\n");
            skip = TRUE;
          }

          if(skip == FALSE) {
            cd(filePath);
          }*/
        } else if(0 == strcmp(currentArgs[0], "pwd")) {        
          if(length > 1) {
            printf("pwd: no arguments supported\n");
          }
          pwd();
        } else if(0 == strcmp(currentArgs[0], "cat")) {
          if(strcmp(redirection, "in") != 0) {
            if(length == 1) {
              printf("cat: please enter file(s) to see\n");
            } else {
              cat(currentArgs+1, length - 1);
            }
          } else {
            cat(&fileRedirect, 1);
          }
        } else if(0 == strcmp(currentArgs[0], "more")) {
          if(strcmp(redirection, "in") != 0) {
            if(length == 1) {
              printf("more: please enter file(s) to see\n");
            } else {
              more(currentArgs+1, length - 1);
            }
          } else {
            more(&fileRedirect, 1);
          }
        } else if(0 == strcmp(currentArgs[0], "rm")) {
          int argPos = 1;
          //removes each file given
          while(argPos < length) {
            char *arg = currentArgs[argPos];
            rm(arg);
            argPos++;
          }
          //checks if no file name was given
          if(length == 1) {
            printf("rm: please specify a file name\n");
          }
        } else if(0 == strcmp(currentArgs[0], "mount")) {
          if(length == 3) {
            mount(currentArgs[1], currentArgs[2]);
          } else {
            printf("mount: incorrect number of arguments\n");
          }
        } else if(0 == strcmp(currentArgs[0], "unmount")) {
          if(length == 3) {
            unmount(currentArgs[1], currentArgs[2]);
          } else {
            printf("mount: incorrect number of arguments\n");
          }
        } else {
          if( -1 == execvp(currentArgs[0], currentArgs) ){
            //error message for our use
            /*char errmsg[64];
            snprintf( errmsg, sizeof(errmsg), "exec '%s' failed", currentArgs[0] );
            perror( errmsg );*/
            //error message for user use
            fprintf(stderr, "%s: command not found\n", currentArgs[0]);
            for(int i = 0; i < number; i++){
              free(toks[i]);
            }
            free(toks);
            free(redirection);
          }
        }
        exit(0);
        
      } else if (pid > 0) {
        waitpid(pid, NULL, 0);
        tcsetpgrp(STDIN_FILENO, getpid());
      }
      if(redir == TRUE && strcmp(redirection, "in") != 0) {
        if(access("./temp.txt", F_OK ) == 0) {
          //fprintf(stderr, "Redirecting file things\n");

          //read in UNIX file
          FILE *inputfile = fopen("./temp.txt", "rwb");
          if(!inputfile) {
            fprintf(stderr, "Error redirecting input/output\n");
            continue;
          }

          fseek(inputfile, 0L, SEEK_END);
          int size = ftell(inputfile);
          rewind(inputfile);
          //reading file into memory to copy to our file system
          void *file = malloc(size);
          
          size_t bytes = fread(file, 1, size, inputfile);
          if (bytes != size) {
            fprintf(stderr, "Error redirecting input/output\n");
            continue;
          }
          fclose(inputfile);
          remove("./temp.txt");

          //copy to our file system
          //MODIFY change to our library system calls

          //set to create if doesn't exist; for >>, just set to append
          /*FILE *outfile;
          if (strcmp(redirection, "out") == 0) {
            outfile = fopen(fileRedirect, "w");
          } else if (strcmp(redirection, "app") == 0) {
            outfile = fopen(fileRedirect, "a");
          }
          
          if(!outfile) {
            fprintf(stderr, "Error redirecting input/output\n");
            continue;
          }
          fwrite(file, size, 1, outfile);
          fclose(outfile);*/
          int outfile;
          outfile = f_open(workingDirectory, fileRedirect, OCREAT);
          f_close(outfile);
          //add file + directory options - function to parse out path name?
          if (strcmp(redirection, "out") == 0) {
            outfile = f_open(workingDirectory, fileRedirect, OWRITE);
          } else if (strcmp(redirection, "app") == 0) {
            outfile = f_open(workingDirectory, fileRedirect, OAPPEND);
            f_write("\n", 1, 1, outfile);
          }
          
          if(outfile == -1) {
            fprintf(stderr, "Error redirecting input/output\n");
            continue;
          }
          f_write(file, size, 1, outfile);
          f_close(outfile);
          free(file);
        } else {
          fprintf(stderr, "no file :( \n");
        }
      } else {
        remove("./temp.txt");
      }

      

      for(int i = 0; i < length; i++) {
        free(currentArgs[i]);
      }
      free(currentArgs);
      free(fileRedirect);

      commandsRun++;
      tokensExamined = end + 1;
    }
    for(int i = 0; i < number; i++){
      free(toks[i]);
    }
    free(toks);
    free(redirection);
  }

  for(int i = 0; i < number; i++){
      free(toks[i]);
  }
  free(toks);
  free(line);

  f_unmount("/", 0);

  free(workingDirectory);
  free(parentDirectory);

}

//shell to test just the parser
/*int main(int argc, char *argv[]) {
  int number;
  while(1){
    number = parser();
    for(int i = 0; i < number; i++){
      printf("%s\n", toks[i]);
    }
    for(int i = 0; i < number; i++){
      free(toks[i]);
    }
    free(toks);
  }
}*/
