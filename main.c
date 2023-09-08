#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int create_process_and_run(char **command){
    int shell_status = fork();
    if (shell_status < 0){
        perror("Failed Fork\n");
        exit(0);
    } else if (shell_status == 0){
        execvp(command[0], command);
        perror("Failed Execution\n");
        exit(0);
    } else {
        int ret;
        int pid = wait(&ret);
        if (!WIFEXITED(ret)){
            printf("Abnormal Termination of %d\n", pid);
        }
    }
    return shell_status;
}

int launch(char **command){
    int shell_status;
    shell_status = create_process_and_run(command);
    return shell_status;
}

char **read_user_input(char *input, char **command){
    char *delim = " ";
    char *word;
    int i = 0;
    word = strtok(input, delim);
    while (word != NULL){
        command[i] = word;
        ++i;
        word = strtok(NULL, delim);
    }
    command[i] = NULL;
    return command;
}

void shell_loop()
{
    int shell_status;
    char *input;
    char **command;
    do {
        input = malloc(256);
        command = malloc(256);
        printf("group-28@shell:~$ ");
        gets(input);
        command = read_user_input(input, command);
        shell_status = launch(command);
        free(command);
        free(input);
    } while (shell_status);
}

int main()
{
    shell_loop();
    return 0;
}