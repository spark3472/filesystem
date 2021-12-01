/*
    Simple shell (no backgrounding or job control)
    Handles & and ;, but treats & like a ; since no backgrounding
    No memory errors that I can find!

    built-in commands:
    - ls only supports listing one directory at a time
    - mkdir allows names with any characters (doesn't exclude '(', '$', etc))
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

#define TRUE  1
#define FALSE 0

//global array toks
char** toks;
//start of current command section (breaks up & and ; lines)
char** traverser;

/******PARSER******/

/***** Code outline for parser and tokenizer from HW2Feedback slides *****/
//holds a string and the current position in it
typedef struct tokenizer{
  char* str;
  char *pos;
} TOKENIZER;

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
  if (*(v->pos) == '&'||*(v->pos)==';'){
    b++;
    v->pos++;
  }else {
    while(*(v->pos) != '\0'){
      if (*(v->pos) == '&'||*(v->pos) == ';'|| *(v->pos)== ' '){
        break;
        }else{
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
char* shell_prompt = "shell> ";
int parser(){
  int n = 0;
  int i = 0;
  //valgrind doesn't like this line??
  line = readline(shell_prompt);
  //ctrl-d
  if(line == NULL){
    return 0;
  }
  //newline
  if(strcmp(line,"") == 0){
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

void ls(char *fileName, char flags[2]) {
  printf("doing ls - filename: %s, flags: %s\n", fileName, flags);
}

void chmod() {
  printf("doing chmod\n");
}

void mkdir(char *fileName) {
  printf("doing mkdir - filename: %s\n", fileName);
  //check if directory already exists
}

void rmdir_new(char *fileName) {
  printf("doing rmdir - filename: %s\n", fileName);
}

void cd() {
  printf("doing cd\n");
}

void pwd() {
  printf("doing pwd\n");
}

void cat() {
  printf("doing cat\n");
}

void more() {
  printf("doing more\n");
}

void rm() {
  printf("doing rm\n");
}

void mount() {
  printf("doing mount\n");
}

void unmount() {
  printf("doing unmount\n");
}



int main(){
  char** currentArguments;
  int aftersemi = 0;
  int number;

  while(1){
    number = parser();

    if(number == 0) {
      free(toks);
      continue;
    }

    //if user types "exit", leave
    if(0 == strcmp(toks[0], "exit")) {
      exit(0);
    }

    //int count = 0;
    //int place = 0;
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
    
    int tokensExamined = 0;
    int commandsRun = 0;
    while(tokensExamined < number) {
      //background the job?
      int background = 0;
      //start and end of the current command section
      int start = 0;
      int end = number;

      if(commandsRun > 0) {
        //start one after the last & or ;
        start = ampSemiPlaces[commandsRun-1][0] + 1;
      } 

      if(commandsRun < ampOrSemi) {
        end = ampSemiPlaces[commandsRun][0];
        if(ampSemiPlaces[commandsRun][1] == '&') {
          background = 1;
        }
      }

      char** currentArgs = getArgs(start, end);

      if(0 == strcmp(currentArgs[0], "ls")) {
        char flags[2] = "\0";
        int argPos = 1;
        int flagsSeen = 0;
        char *fileName = NULL;
        int skip = FALSE;
        
        //goes through each argument and classifies it as filename/flag to feed to ls()
        while(argPos < (end - start)) {
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
            if(fileName == NULL) {
              fileName = arg;
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
      } else if(0 == strcmp(currentArgs[0], "chmod")) {
        chmod();
      } else if(0 == strcmp(currentArgs[0], "mkdir")) {
        int argPos = 1;
        //makes a directory for each given name
        while(argPos < (end - start)) {
          char *arg = currentArgs[argPos];
          mkdir(arg);
          argPos++;
        }
        //checks if no directory name was given
        if((end - start) == 1) {
          printf("mkdir: please specify a directory name\n");
        }
      } else if(0 == strcmp(currentArgs[0], "rmdir")) {
        int argPos = 1;
        //makes a directory for each given name
        while(argPos < (end - start)) {
          char *arg = currentArgs[argPos];
          rmdir_new(arg);
          argPos++;
        }
        //checks if no directory name was given
        if((end - start) == 1) {
          printf("rmdir: please specify a directory name\n");
        }
      } else if(0 == strcmp(currentArgs[0], "cd")) {
        cd();
      } else if(0 == strcmp(currentArgs[0], "pwd")) {
        pwd();
      } else if(0 == strcmp(currentArgs[0], "cat")) {
        cat();
      } else if(0 == strcmp(currentArgs[0], "more")) {
        more();
      } else if(0 == strcmp(currentArgs[0], "rm")) {
        rm();
      } else if(0 == strcmp(currentArgs[0], "mount")) {
        mount();
      } else if(0 == strcmp(currentArgs[0], "unmount")) {
        unmount();
      } else {

        pid_t pid;
        if((pid = fork()) == 0) {
          //puts the child process in its own process group
          setpgid(getpid(),0);
          
          if( -1 == execvp(currentArgs[0], currentArgs) ){
            //error message for our use
            /*char errmsg[64];
            snprintf( errmsg, sizeof(errmsg), "exec '%s' failed", currentArgs[0] );
            perror( errmsg );*/
            //error message for user use
            printf("%s: command not found\n", currentArgs[0]);
            for(int i = 0; i < number; i++){
              free(toks[i]);
            }
            free(toks);
            exit(0);
          }
        } else if (pid > 0) {
          waitpid(pid, NULL, 0);
        }

      }

      for(int i = 0; i < (end - start); i++) {
        free(currentArgs[i]);
      }
      free(currentArgs);

      commandsRun++;
      tokensExamined = end + 1;
    }
    for(int i = 0; i < number; i++){
      free(toks[i]);
    }
    free(toks);
  }

  for(int i = 0; i < number; i++){
      free(toks[i]);
  }
  free(toks);
  free(line);

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
