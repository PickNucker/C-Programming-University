#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> // pid_t = int
#include <unistd.h>    // fork, exec, getcwd
#include <sys/wait.h>  // wait

#include "plist.h"

#define MAX_BUFFER 1337

static void die(char* message){
    perror(message);
    exit(EXIT_FAILURE);
}

static void printcwd(void){

    char buffer[4096];

    if(getcwd(buffer, sizeof(buffer)) != NULL){
        if (printf("%s: ", buffer) < 0){
            die("printf: gegtcwd, Zeile 5");
        }
    }else{
        die("getcwd");
    }
 
    if(fflush(stdout) != 0){
            die("fflush");
    }
}

    static bool print_jobs(pid_t pid, const char*cmdLine){
        printf("pid: %d, cmdLine: %s\n", (int)pid, cmdLine);
        return false;

    }


    static void collect_zombies(void) {
    pid_t pid;
    int status;
    
    /**
     * waitpid:
     * -1, um auf beliebigen Kindprozess zu warten
     * WNOHANG, um sofort zurückzukehren, falls es keine fertigen Kindprozesse gibt
     */
    while ((pid = waitpid(-1, &status, WNOHANG)) != 0) {
        if (pid < 0) {
          if (errno == ECHILD) {
              // Es gibt keine Kindprozesse -> dies ist kein Fehler
              break;
          }
          die("waitpid");
        }

        char buf[MAX_BUFFER + 1];

        /**
         * für Infos zu removeElement siehe plist.h
         * Die Liste enthält nur die laufenden Hintergrundprozesse.
         * Kindprozess mit der ProzessID pid ist fertig, kann aus Liste entfernt werden.
         * Kommandozeile des Prozesses aus der Liste in buf speichern.
         */
        if (removeElement(pid, buf, sizeof(buf)) < 0) {
          continue;
        }
        
        if(WIFEXITED(status)){
                    printf("Exitstatus [%s] = %d\n", buf, WEXITSTATUS(status));
                }else{
                    printf("no Exitstatus [%s]\n", buf);
                }
      }
  }
int main (void){

    while(1){
        printcwd();
        collect_zombies();
        char argv_buffer[MAX_BUFFER + 1];

        // Guckt ob die Eingabe beendet ist
        if(fgets(argv_buffer, MAX_BUFFER + 1, stdin) == NULL){
            if(feof(stdin)){
                break;
            }
            die("fgets");
        }

        // Guckt ob Eingabe zu lang ist
        if(strlen(argv_buffer) > MAX_BUFFER && argv_buffer[strlen(argv_buffer) - 1 != '\n']){
            fprintf(stderr, "Zeileninput zu lang");

            int c;
            do{
                c = fgetc(stdin);
            }while(c != EOF && c != '\n');

            if(ferror(stdin)){
                die("ferror, stdin");
            }
            continue;
        }

        // Zu wenig 
        if(strlen(argv_buffer) < 2) continue;

        // \n ersetzen
        argv_buffer[strlen(argv_buffer) - 1] = '\0';

        // guckt ob hintergrundprozess wird oder nicht
        bool background = false;
        if(argv_buffer[strlen(argv_buffer) - 1] == '&'){
            argv_buffer[strlen(argv_buffer) - 1] = '\0';
            background = true;
        }

        // Bevor es zerstückelt wird, wird die eingabe kopiert, um sie später nochmal auszugeben
        char cpy[sizeof(argv_buffer)];
        strcpy(cpy, argv_buffer);

        // Strtok, wird unterteilt
        int i = 0;
        char* argv[MAX_BUFFER / 2 + 2];
        argv[i++] = strtok(argv_buffer, " \t");

        while((argv[i++] = strtok(NULL, " \t")) != NULL){
            continue;
        }

        // leeres Argument
        if(argv[0] == NULL){
            continue;
        }

        // Wenn Kommando cd ist, dann gucken wir ob ein Pfad angegeben wurde sonst wechsel und auf Fehler überprüfen
        if(strcmp(argv[0], "cd") == 0){
            if(argv[0] != NULL && argv[1] == NULL){
                fprintf(stderr, "usage cd <argument...>");
                continue;
            }
            else if(chdir(argv[1]) != 0){
                die("chdir");
            }
        }

        // Gucken ob Jobs
        if (strcmp(argv[0], "jobs") == 0) {
            if (argv[1] == NULL) {
            // Kein Argument vorhanden, also kann "jobs" normal ausgeführt werden
            printf("Jobs werden geprintet\n");
            walkList(print_jobs);
            } else {
            // Argument vorhanden, zeige eine Fehlermeldung an
            fprintf(stderr, "usage: jobs\n");
            }
            continue;
        }

        // Kindprozesse

        pid_t pid = fork();
        if(pid == -1) die("pid");

        if(pid == 0){
            execvp(argv[0], argv);
            die("execvp");
        }else{
            if(background){
                if(insertElement(pid, cpy) == -2){
                    fprintf(stderr, "insertElement");
                }
                    exit(EXIT_FAILURE);
            }else{
                
                int status;
                if(wait(&status) == -1) die("wait");

                if(WIFEXITED(status)){
                    printf("Exitstatus [%s] = %d\n", cpy, WEXITSTATUS(status));
                }else{
                    printf("no Exitstatus [%s]\n", cpy);
                }
            }
        }

    }
    exit(EXIT_SUCCESS);
}
