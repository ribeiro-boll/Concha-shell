![os](https://camo.githubusercontent.com/e6d28433c0c1041770537fc7f5af3110f9d9cb0b8e8aded756769aebdba81135/68747470733a2f2f696d672e736869656c64732e696f2f62616467652f2d4c696e75782d677265793f6c6f676f3d6c696e7578)
![license](https://img.shields.io/badge/License-Unlicense-green)  ![badge](https://img.shields.io/badge/Lang-C-blue)
# 🐚 *Concha shell* 
![image](https://media4.giphy.com/media/v1.Y2lkPTc5MGI3NjExOWN6aDd4ZGYzYWkyZjN2MGh4cTA0eWp2NjQ4NDJ5ZWkyenphOWY3cyZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/adAyFMeT3HF5uzuNKr/giphy.gif)



- *English version: [English version](#English)*

- *Versão pt-BR: [Versão pt-BR](#pt-BR)*


## English 

A simple shell implemented in C for educational purposes. It provides basic command execution, directory navigation, and minimal signal handling.

**Now with Command history and arrow keys are now usable for editing the command input!**

### Features
- **Change Directory (`cd`)**: Changes the current directory. If no argument is provided, defaults to the directory ($HOME).
- **List Directory Contents (`ls`)**: Displays the contents of the current directory.
- **Clear Screen (`cls`)**: Clears the terminal window.
- **Clear History (`hcls`)**: clears the file that stores the command and deletes the current history.
- **Built-in Commands**: Includes `help` (displays available commands) and `exit` (exits the shell).
- **Execution of External Commands**: If a command is not built-in, the shell attempts to execute it using the system PATH.
- **Signal Handling**: Captures SIGINT (Ctrl+C) and provides a custom message.

### Requirements
- Linux operating system
- GCC or a compatible C compiler
- POSIX-compliant environment

### Compilation
Clone the repository and compile the source code using GCC:

```bash
gcc -o concha concha.c -lreadline
```

## pt-BR

Um shell simples implementado em C para fins educacionais. Ele fornece execução básica de comandos, navegação de diretórios e tratamento mínimo de sinais.

**Agora com o histórico de comandos e as teclas de seta estão disponíveis para editar o comando digitado!**

### Funcionalidades
- **Mudar Diretório (`cd`)**: Altera o diretório atual. Se nenhum argumento for fornecido, utiliza como padrão o diretório ($HOME).
- **Listar Conteúdo do Diretório (`ls`)**: Exibe o conteúdo do diretório atual.
- **Limpar a Tela (`cls`)**: Limpa a janela do terminal.
- **Limpar o Historico (`hcls`)**: limpa o arquivo que guarda os comandos e apaga o historico atual
- **Comandos Internos**: Inclui `help` (exibe os comandos disponíveis) e `exit` (encerra o shell).
- **Execução de Comandos Externos**: Se um comando não for interno, o shell tenta executá-lo utilizando o PATH do sistema.
- **Tratamento de Sinais**: Captura SIGINT (Ctrl+C) e exibe uma mensagem customizada.

### Requisitos
- Sistema operacional Linux
- GCC ou um compilador C compatível
- Ambiente compatível com POSIX

### Compilação
Clone o repositório e compile o código-fonte usando o GCC:

```bash
gcc -o concha concha.c -lreadline
```
