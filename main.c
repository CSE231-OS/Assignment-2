#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#define max_commands 128

int command_count = 0;
int command_index = 0;
char *history[max_commands];

void add_command_to_history(char *input){
    if (command_count != 0) {
        if (strcmp(history[(command_index - 1)%max_commands], input) == 0) return;
    }
    if (command_index >= max_commands){
        command_index %= max_commands;
        free(history[command_index]);
    }
    history[command_index++] = strdup(input);
    ++command_count;
}

void print_history(){
    if (command_count == command_index){
        for (int ind = 0; ind < command_count; ind++){
            printf("%d %s\n", ind + 1, history[ind]);
        }
    } else {
        for (int ind = command_index + 1; ind <= command_index + max_commands; ind++){
            printf("%d %s\n", ind - command_index, history[(ind%max_commands)]);
        }
    }
}

void signal_handler(int sig_num){
    printf("1\n");
    exit(0);
}

int create_process_and_run(char **command, int fds[2]){
    int shell_status = fork();
    if (shell_status < 0){
        perror("Failed fork");
        exit(0);
    } else if (shell_status == 0){
        if (fds != NULL) {
            close(fds[0]);
            dup2(fds[1], STDOUT_FILENO);
        }
        execvp(command[0], command);
        perror("Failed execution");
        exit(0);
    } else {
        int ret;
        int pid = wait(&ret);
        if (!WIFEXITED(ret)){
            printf("Abnormal termination of %d\n", pid);
        }
    }
    return shell_status;
}

int launch(char **command, int n, int *offsets){
    // Shell's input stream should not be affected by piping
    if (fork() == 0) {
        if (n == 1) {
            // No need to redirect stdout
            return create_process_and_run(command + offsets[0], NULL);
        } else {
            int fds[2];
            pipe(fds);

            create_process_and_run(command + offsets[0], fds);

            close(fds[1]);
            dup2(fds[0], STDIN_FILENO);
            int status;
            wait(&status);
            if (status != 0) {
                fprintf(stderr, "Stopping pipe\n");
                exit(0);
            }
            launch(command, n-1, offsets+1);
        }
        exit(0);
    } else {
        wait(NULL);
    }
}

int read_user_input(char *input, char **command, int *n, int *offsets){
    char *token, *subtoken;
    *n = 1;
    int k = 0;  // Corresponds to each word
    int pipeOpen = 0;
    offsets[0] = 0;
    add_command_to_history(input);
    for (int i = 0; (token = strsep(&input, " ")) && pipeOpen != -1; i++) {
        for (int j = 0; (subtoken = strsep(&token, "|")); j++) {
            if (j >= 1) {
                if (pipeOpen == 1) {
                    fprintf(stderr, "Invalid command: empty pipe\n");
                    pipeOpen = -1;
                    break;
                }
                command[k++] = NULL;
                offsets[*n] = k;
                *n += 1;
                pipeOpen = 1;
            }
            if (strlen(subtoken) > 0) {
                pipeOpen = 0;
                command[k++] = subtoken;
            }
        }
    }
    if (pipeOpen == 1) {
        fprintf(stderr, "Invalid command: open pipe\n");
        pipeOpen = -1;
    }
    command[k] = NULL;
    if (pipeOpen == -1) {
        return 0;
    } else {
        return 1;
    }
}

void shell_loop()
{
    int shell_status;
    char *input = malloc(sizeof(char)*256);
    char **command = malloc(sizeof(char *)*128);
    char *cwd;
    int *offsets = malloc(sizeof(int *)*128);
    int n;
    clock_t start, end, program_start;
    program_start = clock();
    signal(SIGINT, signal_handler);
    do {
        cwd = getcwd(NULL, 0);
        printf("\033[1m\033[33mgroup-28@shell\033[0m:\033[1m\033[35m%s\033[0m$ ", cwd);
        fgets(input, sizeof(char)*256, stdin);
        input[strlen(input)-1] = '\0';
        if (input[0] == '\0') continue;
        int valid = read_user_input(input, command, &n, offsets);
        if (strcmp(input, "history") == 0) {print_history(); continue;}
        if (!valid) {
            continue;
        }
        shell_status = launch(command, n, offsets);
    } while (shell_status);
    free(input);
    free(command);
    free(offsets);
}

int main()
{
    shell_loop();
    return 0;
}