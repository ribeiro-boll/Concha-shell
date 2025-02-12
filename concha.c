#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char*path;
void handle_sigint() {
    printf("\nSIGINT (Ctrl+C) detectado...\nUse 'Ctrl+Shift+C' para copiar ou 'Ctrl+Shift+V' para colar.\n\n");
    fflush(stdout);
}

void cd(int contador,char **path,char **args){
    if (contador==1){
        chdir("/");
        free(*path);
        *path = strdup("/");
        if (chdir(*path)!=0) {
            perror("cd");
        }
    }
    else if (chdir(args[1])!=0){
        perror("cd");
    }
    else{
        free(*path);
        *path = strdup(args[1]);
    }
}
void ls(char **arg,int contador){
    pid_t pid = fork();
    if (pid == -1){
        perror("fork");
        exit(1);
    }
    if (pid == 0){
        contador+=2;
        arg = realloc(arg,contador);
        arg[contador-1] = "-F";
        arg[contador] = NULL;
        execve(arg[0],arg,NULL);
        exit(0);
    }
    else {
        int status;
        waitpid(pid,&status, 0);
        if (WIFEXITED(status)!=0 && WEXITSTATUS(status)!=0){
            printf("\nfailed exec ls command!\n");
        }
        return;
    }
}
void execute_app(char **arg){
    pid_t pid = fork();
    if (pid == -1){
        perror("execvp");
        exit(1);
    }
    if (pid == 0){
        execvp(arg[0],arg);
        exit(0);
    }
    else {
        int status;
        waitpid(pid,&status, 0);
        if (WIFEXITED(status)!=0 && WEXITSTATUS(status)!=0){
            printf("\nfailed to exec %s command!\n",arg[0]);
        }
        return;
    }
}

int main(){
    system("clear");
    path = strdup("/");
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    chdir(path);
    printf("Welcome to Concha!\ntype -> 'help' to view the commands!\n\n");
    while (1) {
        char nome[PATH_MAX];
        getcwd(nome, sizeof(nome));
        char *token = NULL,texto[1024],**args=NULL;
        int contador =0;
        printf("$ Concha ~ [ %s ]->%c",nome,32);
        fgets(texto, sizeof(texto), stdin);
        texto[strcspn(texto, "\n")] = '\0';
        if (texto[0]=='\0'){
            printf("\n\n");
        }
        else {
            token = strtok(texto," ");
            while (token!=NULL) {
                contador+=1;
                args = realloc(args, sizeof(char*)*(1+contador));
                args[contador-1] = strdup(token);
                token = strtok(NULL, " ");
            }
            if (contador > 0) {
                args[contador] = NULL;
            }
            if (strcmp(args[0], "exit")==0){
                printf("\n\nthx for using this shell!! :)");
                exit(0);
            }
            else if (strcmp(args[0], "help")==0){
                printf("\ncd -> will move the path to '/' dir if no arg was typed, else will move\nthe current path to the typed arg path.\nex: 'cd /home/your_pc_name/'\n\nclear -> clears the terminal window\n\nexit -> exits this shell.\n\nFor last, if you type the disired program you wish to be executed, and if the program \nhas an executable in the PATH, the program will be executed\nand in the case that the shell does not find the executable\nthe program will ignore your input.\n\nAlso, this shell currently does not support the keyboard arrows for going back or forward\nPlease, use the backspace to fix any typos in your input.\n\nMake sure not to use 'Ctrl+C' or 'Ctrl+V'\ninstead, use 'Ctrl+Shift+C' to copy or 'Ctrl+Shift+V' to paste.\n");
            }
            else if (strcmp(args[0], "ls")==0){
                printf("\n");
                strcpy(args[0],"/bin/ls");
                ls(args,contador);
            }
            else if (strcmp(args[0], "clear") ==0) {
                system("clear");
                free(token);
                free(args);
                continue;
            }
            else if(strcmp(args[0], "cd")==0){
                cd(contador ,&path ,args);
            }
            else {
                printf("\n");
                execute_app(args);
            }
            printf("\n\n");
            free(token);
            for (int i = 0; i < contador; i++) {
                free(args[i]);
            }   
            free(args);
        }

    }
    free(path);
    return 0;
}