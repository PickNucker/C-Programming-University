#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h> // pid_t = int
#include <unistd.h>    // fork, exec
#include <sys/wait.h>  // wait

static void die(const char* message){
    perror(message);
    exit(EXIT_FAILURE);
}

static void runCommands(char *argv, char* token){

    printf("→ %s : %s\n", argv, token);

    char buffer[1000];
    strcpy(buffer, token);

    char*command[500];
    command[0] = argv;

    char* newToken = strtok(buffer, " ");
    int i;

    for(i = 1; newToken != NULL; i++){
        command[i] = newToken;
        newToken = strtok(NULL, " ");
    }

    command[i] = NULL;

    pid_t pid = fork();

    if(pid < 0){
        die("pid");
    }
    
    if(pid == 0){
        printf("Kind prozess: %d\n", getpid());
        execvp(command[0], command);
        //exit(EXIT_SUCCESS);
        die("execvp");
    }
    else{
        wait(NULL);
        printf("Im Elternprozess vom Kind: %d\n", pid);
    }
}

int main(int argc, char** argv){
    
    if(argc < 2){
        fprintf(stderr, "./listRun <program> <arguments...>");
        exit(EXIT_FAILURE);
    }

    for(int i = 2; i < argc; i++){
        
        runCommands(argv[1], argv[i]);
    }

    fflush(stdout);
    exit(EXIT_SUCCESS);
}
