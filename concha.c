#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>


char*initial_dir;
void handle_sigint() {
    printf("\nSIGINT (Ctrl+C) detectado...\nUse 'Ctrl+Shift+C' para copiar ou 'Ctrl+Shift+V' para colar.\nPara sair, use 'exit'\n\n");
    fflush(stdout);
    // printa uma mensagem de aviso, pois ctrl + c foi apertado
}

void cd(int contador,char **initial_dir,char **args){
    // a função muda o diretorio de execução do shell para o caminho especificado pelo usuario
    // caso o caminho nao exista, o shell exibirá um erro dizendo que o  arquivo ou o diretorio não existe
    if (contador==1){
        char *home = getenv("HOME");
        chdir(home);
        // muda para o diretorio padrão caso o chamado de cd, nao possua nenhum argumento
        free(*initial_dir);
        //  libera a memoria do initial_dir passado 
        *initial_dir = strdup(home);
        // muda para o diretorio padrão para ser usado pelo resto do shell
        if (chdir(*initial_dir)!=0) {
            perror("cd");
        }
    }
    else if (chdir(args[1])!=0){
        perror("cd");
        // nessa parte, a execução do chdir para mudar o diretorio, ja é feita na propria logica do "else if"
        // caso o "else if" retorne qualquer coisa diferente de 0, é um sinal que a mudança de diretorio foi um sucesso
        // caso o contrario, printa um mensagem de erro
    }
    else{
        // caso o chdir(), mude o diretorio, o codigo cai nesse else
        // que irá liberar a memoria de initial_dir
        // e clonará o caminho especificado pelo usuario em initial_dir
        free(*initial_dir);
        *initial_dir = strdup(args[1]);
    }
}

void execute_app(char **arg){
    //
    // função para execução geral de comandos, caso a execução retorne em erro de criação de preocesso filho
    // um erro será gerado
    //
    // caso o processo filho seja gerado, o processo pai espera o processo filho 
    // terminar ou ser terminado, para voltar a ser executado e aguardar novos inputs
    // 
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

void ls(char **arg,int contador){
    // mesma funcionalidade do execute_app, porem com algumas mudanças para generalizar o uso do "ls"
    // e fazer o mesmo exibir o tipo dos arquivos no diretorio atual no qual o shell está sendo executado
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

void read_his(char* caminho){
    // confere se o historico de comando existe

    // 1- se existe, ele irá ler o arquivo e extrairá os comandos
    // 2- se não existe, irá criar o arquivo para começar a guardar comandos
    if(read_history(caminho) == 0){
        return;
    }
    else {
        write_history(caminho);
        return;
    }
}
int main(){
    char home[5124];
    
    char *user = getenv("USER");
    snprintf(home, sizeof(home), "%s/%s/%s","/home",user,"Documents/shell_history.txt");
    read_his(home);
    system("clear");
    const char *env_path = getenv("PATH");
    if (env_path == NULL) {
        env_path = "/usr/local/bin:/usr/bin:/bin";
    }
    setenv("PATH", env_path, 1);
    initial_dir = strdup(getenv("HOME"));
    // strdup serve para "copiar" uma string para um ponteiro char (bem util!)
    //*
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    //* apenas impede de fechar o programa quando (ctrl + c) for apertado
    chdir(initial_dir);
    printf("Welcome to Concha!\ntype -> 'help' to view the commands!\n\n");
    while (1) {
        char nome[PATH_MAX];
        getcwd(nome, sizeof(nome)); 
        // pega o nome do diretorio atual e copia para o buffer essa função getcwd() é extremamente interessante!
        char *token = NULL,*texto = NULL,**args=NULL;
        int contador =0;
        // contador de args
        char frase[5128];
        snprintf(frase,sizeof(frase),"$ Concha & %s [-> %s <-] =❱ ",user,nome);
        texto = readline(frase);
        // adorei a ideia do readline, alem de ser simples, é extremamente pratico de implementar
        // ele basicamente serve para que o historico funcione, podendo extrair da lista do historico
        // e colocando no buffer de input (stdin)

        //  remove qualquer "\n" que vaze do input do usuario
        if (texto[0]=='\0'){
            // caso o input do usuaro seja vazio (apenas um "\n") ele ignora o input
            // e printa os espaços
        }
        else {
            add_history(texto); // adiciona na lista de historico
            write_history(home); // grava no arquivo do historico, para memoria persistente dos comandos
            token = strtok(texto," ");
            while (token!=NULL) {
                contador+=1;
                args = realloc(args, sizeof(char*)*(1+contador));
                args[contador-1] = strdup(token);
                token = strtok(NULL, " ");
            }
            // loop para transformar o input do usuario em tokens
            // ex: cd /home/ -> args[0] = cd, args[1] = /home/ 
            if (contador > 0) {
                args[contador] = NULL;
                // apenas garante que o final da lista de strings arg, terminará em NULL
                // pois para o shell executar arquivos, o execvp quando recebe os argumentos,
                // o ultimo item da lista de args, tem que ser um NULL 
            }
            if (strcmp(args[0],"echo") == 0) {
                char *var = strchr(args[1], '$');
                if (var) {
                    char *env_var = getenv(var + 1);
                    if (env_var) {
                        printf("%s\n", env_var);
                    }
                    else {
                        printf("\n");
                    }
                } 
                else {
                    printf("%s\n", args[1]);
                }
                continue;
            }
            if (strcmp(args[0], "exit")==0){
                printf("\n\nthx for using this shell!! :)");
                exit(0);
                // captura o comando "exit" e fecha o shell
            }
            else if (strcmp(args[0], "help")==0){
                printf("\ncd -> will move the execution initial_dir of this shell to '/' dir if no arg was typed, else will move\nthe current exec initial_dir to the typed argument initial_dir.\nex: 'cd /home/your_pc_name/'\n\nArrow up and Down -> move between the history of used commands\n\ncls -> clears the terminal window\n\nhcls -> clears the command history\n\nexit -> exits this shell.\n\nIf you type the disired program you wish to be executed, and the program \nhas an executable in the initial_dir, the program will be executed\nand in the case that the shell does not find the executable\nthe program will ignore your input.\n\nAlso, this shell currently does not support the keyboard arrows for going back or forward\nPlease, use the backspace to fix any typos in your input.\n\nMake sure not to use 'Ctrl+C' or 'Ctrl+V'\ninstead, use 'Ctrl+Shift+C' to copy or 'Ctrl+Shift+V' to paste.");
                // captura o comando "help" e printa a lista de comandos
            }
            else if (strcmp(args[0], "hcls")==0) {
                // captura o comando "hcls" que apagará o historico de comandos caso o usuario digite y ou Y 
                int escolha;
                printf("\nAre you sure you want to do this? (default: N) y/N?%c",32);
                escolha = getchar();
                if (escolha == 'y'|| escolha == 'Y'){
                    if(remove(home)==0){
                        clear_history();
                        write_history(home);
                        printf("\nCommands history was wiped with success!");
                    }
                    else {
                        printf("Error! could not delete %s",initial_dir);
                    }
                }
            }
            else if (strcmp(args[0], "ls")==0){
                printf("\n");
                strcpy(args[0],"/bin/ls");
                ls(args,contador);
                // captura o comando "ls" e chama a função que checa se ls foi executado sem argumentos ou nao
                // é adiciona o argumento "-F" para exibir o formato do arquivo
            }
            else if (strcmp(args[0], "cls") ==0) {
                system("clear");
                free(token);
                free(args);
                continue;
                // captura o comando "exit" e limpa a saida do terminal
            }
            else if(strcmp(args[0], "cd")==0){
                cd(contador ,&initial_dir ,args);
                // captura o comando "cd", e executa a função que troca o diretorio em que o shell está sendo executado, via o chdir()
                // ex: 
                //      initial_dir atual-> "/"
                //      cd /home/bolota/
                //      initial_dir atual-> "/home/bolota"    
            }
            else {
                printf("\n");
                execute_app(args);
                // tenta executar a entrada do usuario, por meio do execvp(), caso a entrada do usuario, bata com um executavel
                // na initial_dir do sistema, o shell se clona e passa executar o comando do usuario em um processo filho, ate o comando
                // para de executar (ex: " exit() ") ou ele for terminado por um sinal (ex: " SIGTERM ")
            }
            printf("\n");
            for (int i = 0; i < contador; i++) {
                free(args[i]);
                // libera a memoria de todas as strings na lista de argumentos
            }   
            free(args);
            free(texto);
            // libera a memoria da lista em si
        }

    }
    free(initial_dir);
    // por fim, libera a memoria da string que contem o diretorio atual no qual o shell está
    return 0;
}
