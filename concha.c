
#include <linux/limits.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

char *aspas = "'";
char aspas_dupla = '"';

typedef struct temp {
    char **argumentos_execucao;
    char *tipo_sequencia;
    int quantidade_argumentos;
    struct temp *next;
    struct temp *prev;
} Comando;

Comando *final = NULL;
Comando *inicio = NULL;
int var_condicional_execução = 0; // muda para 0, caso a execução de algo falhe


void handle_sigint(int signum) {
    printf("\nSIGINT (Ctrl+C) detected...\nUse 'Ctrl+Shift+C' for copying ou 'Ctrl+Shift+V' for pasting.\nto exit, use 'exit'\n\nto continue, press enter");
    fflush(stdout);
    return;
    // printa uma mensagem de aviso, pois ctrl + c foi apertado
}

void read_his(char* caminho,char *user){
    // confere se o historico de comando existe

    // 1- se existe, ele irá ler o arquivo e extrairá os comandos
    // 2- se não existe, irá criar o arquivo para começar a guardar comandos
    struct stat st = {0};
    char temp[5124];
    snprintf(temp,sizeof(temp),"%s/%s",user,".config/concha/"); 
    if (stat(temp, &st) ==-1){
        mkdir(temp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        write_history(caminho);
        return;
    }
    else {
        if(read_history(caminho) == 0){
            return;
        }
        else {
            write_history(caminho);
            return;
        }
    }
}

void free_list() {
    Comando *comando = inicio;
    while (comando != NULL) {
        Comando *temp = comando;
        comando = comando->next;
        // Liberar argumentos_execucao
        if (temp->argumentos_execucao != NULL) {
            for (int i = 0; temp->argumentos_execucao[i] != NULL; i++) {
                free(temp->argumentos_execucao[i]);
            }
            free(temp->argumentos_execucao);
        }

        // Liberar tipo_sequencia
        if (temp->tipo_sequencia != NULL) {
            free(temp->tipo_sequencia);
        }

        free(temp);
    }
    inicio = NULL;
    final = NULL;
}

void add_command(char **exec_commands, char *exec_sequence, int quantidade_argumentos) {
    Comando *new_command = (Comando*) malloc(sizeof(Comando));
    new_command->argumentos_execucao = NULL;
    int i = 1;
    for (int j = 0; j < quantidade_argumentos; j++) {
        new_command->argumentos_execucao = (char **) realloc(new_command->argumentos_execucao, sizeof(char*) * i);
        new_command->argumentos_execucao[i - 1] = strdup(exec_commands[i - 1]);
        i++;
    }
    new_command->argumentos_execucao = (char **) realloc(new_command->argumentos_execucao, sizeof(char*) * i);
    new_command->argumentos_execucao[i - 1] = NULL;
    if (exec_sequence == NULL) {
        new_command->tipo_sequencia = NULL;
    } else {
        new_command->tipo_sequencia = strdup(exec_sequence);
    }
    new_command->quantidade_argumentos = quantidade_argumentos;
    new_command->next = NULL;
    new_command->prev = final;
    if (final == NULL) {
        inicio = new_command;
    } else {
        final->next = new_command;
    }
    final = new_command;
}

int comando_valido(char** argumentos) {
    if (argumentos == NULL || argumentos[0]==NULL){
        return 0;
    }
    return 1;
}

int detectar_tipo_sequencia(Comando *temp){
    char *tipo = temp->tipo_sequencia;
    if (temp->tipo_sequencia == NULL){
        return 0;
    }
    if (strcmp(tipo, "&&")==0){
        return 1;
    }
    else if (strcmp(tipo, "||")==0){
        return 2;
    }
    else if (strcmp(tipo, ";")==0){
        return 3;
    }
    else if (strcmp(tipo, "|")==0){
        return 4;
    }
    else {
        return 0;
    }
}

void exec_programa_padrao(char **arg){
    pid_t pid = fork();
    if (pid == -1){
        perror("\nerror! failed execution!\n");
        exit(1);
    }
    else if (pid == 0) {
        execvp(arg[0], arg);
        var_condicional_execução = 1;
        exit(0);
    }
    else {
        int status;
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) || WIFEXITED(status) != 0){
            var_condicional_execução = 0;
        }
    }
}

void exec_pipe(Comando *cmd_left, Comando *cmd_right) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        var_condicional_execução = 0;
        return;
    }
    
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        var_condicional_execução = 0;
        return;
    }
    if (pid1 == 0) {
        // Filho 1: executa o comando à esquerda
        close(pipefd[0]);                 // fecha a extremidade de leitura
        dup2(pipefd[1], STDOUT_FILENO);     // redireciona STDOUT para a escrita do pipe
        close(pipefd[1]);
        execvp(cmd_left->argumentos_execucao[0], cmd_left->argumentos_execucao);
        perror("execvp (left command)");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        var_condicional_execução = 0;
        return;
    }
    if (pid2 == 0) {
        // Filho 2: executa o comando à direita
        close(pipefd[1]);                 // fecha a extremidade de escrita
        dup2(pipefd[0], STDIN_FILENO);      // redireciona STDIN para a leitura do pipe
        close(pipefd[0]);
        execvp(cmd_right->argumentos_execucao[0], cmd_right->argumentos_execucao);
        perror("execvp (right command)");
        exit(EXIT_FAILURE);
    }
    
    // No pai: fecha ambas as extremidades e espera os filhos
    close(pipefd[0]);
    close(pipefd[1]);
    int status;
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
}

void exec_ls(char **arg,int contador){
    // mesma funcionalidade do execute_app, porem com algumas mudanças para generalizar o uso do "ls"
    // e fazer o mesmo exibir o tipo dos arquivos no diretorio atual no qual o shell está sendo executado
    pid_t pid = fork();
    if (pid == -1){
        perror("fork");
        exit(1);
    }
    if (pid == 0){
        contador+=2;
        arg = (char **)realloc(arg,contador);
        arg[contador-1] = strdup("-F");
        arg[contador] = NULL;
        execvp(arg[0],arg);
        var_condicional_execução = 1;
        exit(0);
    }
    else {
        int status;
        waitpid(pid,&status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status)!=0){
            printf("\nfailed exec ls command!\n");
            var_condicional_execução = 0;
        }
        return;
    }
}

void exec_cd(char **arg,char **diretorio_de_exec_atual,int contador_argumentos){
    char* home = getenv("HOME");
    if (contador_argumentos == 1){
        int status = chdir(home);
        if (status != 0) {
            perror("\ncd failure error!\n");
            var_condicional_execução = 0;
        }
        else {
            var_condicional_execução = 1;
            *diretorio_de_exec_atual = home;
        }
    }
    else if (contador_argumentos == 2){
        int status = chdir(arg[1]);
        if (status != 0) {
            var_condicional_execução = 0;
            perror("\ncd failure error!\n");
        }
        else {
            var_condicional_execução = 1;
            *diretorio_de_exec_atual = arg[1];
        }
    }
}
void exec_geral(Comando *programa,char **path,char *home_history){
    printf("\n");
    if (strcmp(programa->argumentos_execucao[0],"cd")==0){
        exec_cd(programa->argumentos_execucao, path,programa->quantidade_argumentos);
        fflush(stdout);
    }
    else if (strcmp(programa->argumentos_execucao[0],"ls")==0) {
        exec_ls(programa->argumentos_execucao,programa->quantidade_argumentos);
        fflush(stdout);
    }
    else if (strcmp(programa->argumentos_execucao[0], "cls") ==0) {
        system("clear");
        printf("Welcome to Concha!\ntype -> 'help' to view the commands!\n\n");
        fflush(stdout);
        // captura o comando "exit" e limpa a saida do terminal
    }
    else if (strcmp(programa->argumentos_execucao[0], "help")==0){
        fflush(stdout);
        printf("\ncd -> will move the execution initial_dir of this shell to '/' dir if no arg was typed, else will move\nthe current exec initial_dir to the typed argument initial_dir.\nex: 'cd /home/your_pc_name/'\n\nArrow up and Down -> move between the history of used commands\n\ncls -> clears the terminal window\n\nhcls -> clears the command history\n\nexit -> exits this shell.\n\nIf you type the disired program you wish to be executed, and the program \nhas an executable in the initial_dir, the program will be executed\nand in the case that the shell does not find the executable\nthe program will ignore your input.\n\nAlso, this shell currently does not support the keyboard arrows for going back or forward\nPlease, use the backspace to fix any typos in your input.\n\nMake sure not to use 'Ctrl+C' or 'Ctrl+V'\ninstead, use 'Ctrl+Shift+C' to copy or 'Ctrl+Shift+V' to paste.");
        // captura o comando "help" e printa a lista de comandos
    }
    else if (strcmp(programa->argumentos_execucao[0], "hcls")==0) {
        // captura o comando "hcls" que apagará o historico de comandos caso o usuario digite y ou Y 
        fflush(stdout);
        int escolha;
        printf("\nAre you sure you want to do this? (default: N) y/N?%c",32);
        escolha = getchar();
        if (escolha == 'y'|| escolha == 'Y'){
            if(remove(home_history)==0){
                clear_history();
                write_history(home_history);
                printf("\nCommands history was wiped with success!");
                return;
            }
            else {
                printf("Error! could not delete %s",home_history);
                return;
            }
        }
        else{
            return;
        }
    }
    else {
        exec_programa_padrao(programa->argumentos_execucao);
    }
    
}


int main() {
    int contador_aspas=0;
    char *path_path = getenv("PATH");
    char *user = getenv("USER");

    const char *env_path = path_path;
    if (env_path == NULL) {
       env_path = "/usr/local/bin:/usr/bin:/bin";
    }
    setenv("PATH", env_path, 1);

    char home_history[5124];
    char *path_home = getenv("HOME");
    printf("\033]0;%s\007","Concha-shell");
    fflush(stdout);
    snprintf(home_history, sizeof(home_history), "%s/%s",path_home,".config/concha/shell_history.txt");
    system("clear");
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    char *diretorio_de_exec = getenv("HOME");
    char *cwd = getcwd(NULL, 0);
    if (cwd != NULL) {
        chdir(cwd);
        free(cwd);
    }
    else {
        chdir(diretorio_de_exec);
    }

    read_his(home_history, path_home);
    printf("Welcome to Concha!\ntype -> 'help' to view the commands!\n\n\n\n");
    char buffer_espaco[5128];
    char buffer_espaco_continuo[5128];
    while (1) {
        time_t timer;
        time(&timer);
        struct tm* tm_info;
        char tempo[80];
        tm_info = localtime(&timer);
        strftime(tempo,80,"%T",tm_info);
        char diretorio[PATH_MAX];
        getcwd(diretorio, sizeof(diretorio)); 
        var_condicional_execução = 1;
        char *texto;
        char frase[5128];
        snprintf(frase,sizeof(frase),"┌─(Concha & %s - %s)\n╰─[%s]─➤ ",user,tempo,diretorio);
        texto = readline(frase);
        if (texto[0]=='\0'){
            printf("\n\n");
            continue;
        }
        add_history(texto); // adiciona na lista de historico
        write_history(home_history);
        char **argumentos = NULL;
        char *token = strtok(texto, " ");
        int contador = 0;
        while (token != NULL) {
            if (strcmp(token, "&&") == 0 || strcmp(token, "||") == 0 || strcmp(token, ";") == 0 || strcmp(token, "|") == 0) {
                // Executa caso o comando anterior tenha sucesso
                if (comando_valido(argumentos)){
                    contador++;
                    argumentos = (char **) realloc(argumentos, sizeof(char*) * contador);
                    argumentos[contador - 1] = NULL;
                    add_command(argumentos, token, contador-1);
                    argumentos = NULL;
                    contador = 0;
                    token = strtok(NULL, " ");
                }
                else {
                    if (inicio != NULL || final != NULL){
                        free_list();
                    }
                    printf("\ninvalid command!\n\n\n");
                    continue;
                }
            }
            else {
                if (token[0] == '$'){
                    char *temp = token+1;
                    contador++;
                    argumentos = (char**) realloc(argumentos, sizeof(char*) * contador);
                    argumentos[contador - 1] = strdup(getenv(temp));
                    token = strtok(NULL, " ");
                }
                else if (token[0] == aspas[0] && token[strlen(token)-1] == aspas[0]) {
                    char *temp;
                    temp = strdup(token);
                    temp++;
                    temp[strlen(temp)-1] = '\0';
                    contador++;
                    argumentos = (char**) realloc(argumentos, sizeof(char*) * contador);
                    argumentos[contador - 1] = strdup(temp);
                    temp = NULL;
                    token = strtok(NULL, " ");
                }
                else if (token[0] == aspas_dupla && token[strlen(token)-1] == aspas_dupla) {
                    char *temp;
                    temp = strdup(token);
                    temp++;
                    temp[strlen(temp)-1] = '\0';
                    contador++;
                    argumentos = (char**) realloc(argumentos, sizeof(char*) * contador);
                    argumentos[contador - 1] = strdup(temp);
                    temp = NULL;
                    token = strtok(NULL, " ");
                }
                else if (token[strlen(token)-1] == aspas[0] && contador_aspas) {
                    contador_aspas--;
                    if (contador_aspas == 0){
                        char frase_espacos[5128];
                        char *temp;
                        temp = strdup(token);
                        temp[strlen(temp)-1] = '\0';
                        snprintf(frase_espacos, sizeof(frase_espacos), "%s %s",buffer_espaco+1,temp);
                        strcpy(buffer_espaco,"");
                        temp = NULL;
                        contador++;
                        argumentos = (char**) realloc(argumentos, sizeof(char*) * contador);
                        argumentos[contador - 1] = strdup(frase_espacos);
                        token = strtok(NULL, " ");
                    }
                    else {
                        snprintf(buffer_espaco_continuo, sizeof(buffer_espaco_continuo), "%s %s", buffer_espaco,token);
                        strcpy(buffer_espaco,buffer_espaco_continuo);
                        strcpy(buffer_espaco_continuo,"");
                        token = strtok(NULL, " ");
                    }
                }
                else if (token[0] == aspas[0]) {
                    strcpy(buffer_espaco, token);
                    contador_aspas++;
                    token = strtok(NULL, " ");
                }
                else if (token[strlen(token)-1] == aspas_dupla && contador_aspas) {
                    contador_aspas = 0;
                    if (contador_aspas == 0){
                        char frase_espacos[5128];
                        char *temp;
                        temp = strdup(token);
                        temp[strlen(temp)-1] = '\0';
                        snprintf(frase_espacos, sizeof(frase_espacos), "%s %s",buffer_espaco+1,temp);
                        strcpy(buffer_espaco,"");
                        temp = NULL;
                        contador++;
                        argumentos = (char**) realloc(argumentos, sizeof(char*) * contador);
                        argumentos[contador - 1] = strdup(frase_espacos);
                        token = strtok(NULL, " ");
                    }
                    else {
                        snprintf(buffer_espaco_continuo, sizeof(buffer_espaco_continuo), "%s %s", buffer_espaco,token);
                        strcpy(buffer_espaco,buffer_espaco_continuo);
                        strcpy(buffer_espaco_continuo,"");
                        token = strtok(NULL, " ");
                    }
                }
                else if (token[0] == aspas_dupla) {
                    strcpy(buffer_espaco, token);
                    contador_aspas++;
                    token = strtok(NULL, " ");
                }
                else if (contador_aspas) {
                    snprintf(buffer_espaco_continuo, sizeof(buffer_espaco_continuo), "%s %s", buffer_espaco,token);
                    strcpy(buffer_espaco,buffer_espaco_continuo);
                    strcpy(buffer_espaco_continuo,"");
                    token = strtok(NULL, " ");
                }
                else if (token[0] == '~' && token[1] == '/') {
                    char frase_extra[5128];
                    snprintf(frase_extra, sizeof(frase_extra), "%s/%s", path_home,token+2);
                    contador++;
                    argumentos = (char**) realloc(argumentos, sizeof(char*) * contador);
                    argumentos[contador - 1] = strdup(frase_extra);
                    token = strtok(NULL, " ");
                }
                else {
                    if (strcmp(token, "exit") == 0 && contador == 0){
                        printf("\n\nthx for using this shell!! :)");
                        exit(0);
                    }
                    contador++;
                    argumentos = (char**) realloc(argumentos, sizeof(char*) * contador);
                    argumentos[contador - 1] = strdup(token);
                    token = strtok(NULL, " ");
                }
            }
        }
        if (contador_aspas>0){
            strcpy(buffer_espaco, "");
            strcpy(buffer_espaco_continuo, "");
            printf("\nerror: quotation marks ('') not closed\n\n");
            free_list();
            contador_aspas=0;
            continue;
        }
        contador++;
        argumentos = (char **) realloc(argumentos, sizeof(char*) * contador);
        argumentos[contador - 1] = NULL;
        add_command(argumentos, NULL, contador -1);
        argumentos = NULL;
        contador = 0;
        int logica_shell = 3;
        // Termina a parte de adicionar comandos/filtrar
        Comando *comando = inicio;
        printf("\033]0;%s\007",comando->argumentos_execucao[0]);
        free(texto);
        while (comando) {
            // && -> 1  executa caso o antecessor fechar com sucesso
            // || -> 2  executa caso o antecessor der erro
            //  ; -> 3  executa independente
            //  | -> 4  executa com a saida do anterior
            //  
            int atual = detectar_tipo_sequencia(comando); 
            if (atual == 4) {
                if (comando->next == NULL) {
                    fprintf(stderr, "Erro: pipe sem comando à direita.\n");
                    var_condicional_execução = 0;
                    break;
                }
                exec_pipe(comando, comando->next);
                comando = comando->next->next;
                if (comando == NULL) break;
                logica_shell = detectar_tipo_sequencia(comando);
                continue;
            }
            switch (logica_shell) {
                case 1:
                    if(var_condicional_execução ==1){
                        exec_geral(comando, &diretorio_de_exec,home_history);
                    }break;
                case 2:
                    if(!var_condicional_execução){
                        exec_geral(comando, &diretorio_de_exec,home_history);
                    }break;
                case 3:
                    exec_geral(comando, &diretorio_de_exec,home_history);break;
            }
            logica_shell = detectar_tipo_sequencia(comando);
            comando = comando->next;
        }
        free_list();
        printf("\n\n");
    }
    return 0;
}
