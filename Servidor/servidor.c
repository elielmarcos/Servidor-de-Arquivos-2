#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <windows.h>
#include <locale.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>


#define M 11	// tamanho na tabela hash
#define BYTE 	1024
//#define SERVER_IP "192.168.0.104"
#define PORTA 	5000
#define BACKLOG 10
#define TITULO "\n =======\033[43m SERVIDOR DE ARQUIVOS \033[40m=======\n\n"

void Ajuda(int connfd);
void Criar_DIR_RAIZ(int connfd, char *dir_Caminho);
void Criar_DIR(int connfd, char *dir_Caminho);
void Remover_DIR(int connfd, char *dir_Caminho);
void Entrar_DIR(int connfd, char *dir_Caminho);
void Sair_DIR(int connfd, char *dir_Caminho);
void Mostrar_DIR(int connfd, char *dir_Caminho, int dir_ID);
void Criar_FILE(int connfd, char *dir_Caminho);
void Remover_FILE(int connfd, char *dir_Caminho);
void Escrever_FILE(int connfd, char *dir_Caminho);
void Mostrar_FILE(int connfd, char *dir_Caminho);
void CMD(int connfd);
void Invalido(int connfd);
void retENT(char *recvBuff);
void* Thread_Conexao(void *Con_socket);

int ChamadaPipe(char* comando);

void Inicializa_hash(void);
int Funcao_hash(char *arquivo);
int Inserir_hash(char *arquivo, char operacao);
int Remover_hash(char *arquivo);
int Buscar_hash(char *arquivo);
int R_hash(char *arquivo, int hash, int inc);
int B_hash(char *arquivo, int hash, int inc);
void Mostra_hash(void);

struct Tabela_Hash{
	int status;
	char arquivo[BYTE];
	char operacao;
} Manipulacao_Arquivo[M];

char dir_Raiz[BYTE];
pthread_mutex_t mutex;


int main(int argc, char *argv[]){
	
	setlocale(LC_ALL, "Portuguese");
	system("clear");
	
	Inicializa_hash(); // inicializa tabela hash (ZERA)
	
	/*Listen File Descriptor (listenfd) and Conection File Descriptor (connfd)*/

	int listenfd = 0, connfd = 0, sktbind = 0, sktlisten = 0;
	struct sockaddr_in serv_addr; // por alocação automática
	
	printf(TITULO);

	/* Zera a struct*/
	listenfd = socket(AF_INET, SOCK_STREAM,6); // AF_INET  Arpa Internet Protocols, SOCK_STREAM socket por stream, 0  Internet Protocol
	
	if (listenfd == -1) // verifica se ocorreu erro na criação do socket descritor
	{
		printf("\033[41mErro: Criar socket descritor.\033[40m\n");
	}else
		printf("Criado socket descritor!\n");
	
	
	memset(&serv_addr, 0, sizeof(serv_addr)); // ou poderia usar o bzero


	/*Instancia os campos do Struct*/
	serv_addr.sin_family = AF_INET; // familia
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // endereço
	//serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	serv_addr.sin_port = htons(PORTA); // porta

	/* Associa um endereço ao descritor do socket */
	sktbind = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); // associa esse soquete a um porta na sua máquina local
	
	char servidor_IP[80];
	inet_ntop(AF_INET, &serv_addr.sin_addr, servidor_IP, 80);
	printf("Socket IP:PORTA = %s, %d\n", servidor_IP, ntohs(serv_addr.sin_port));
	
	
	if (sktbind == -1) // verifica se ocorreu erro na associção do socket a um endereço
	{
		printf("\033[41mErro: Bind socket.\033[40m\n");
	}else
		printf("Bind socket!\n");	
	
	sktlisten = listen(listenfd, BACKLOG);	// fila para escutar os clientes da conexão socket
	
	if (sktlisten == -1) // verifica se ocorreu erro na fila
	{
		printf("\033[41mErro: Listen socket.\033[40m\n");
	}else
		printf("Listen socket!\n");	

	
	getcwd(dir_Raiz, BYTE); // getcwd - obtém o nome do caminho do diretório de trabalho Raiz 
	pthread_mutex_init(&mutex, NULL); // inicializa mutex
	
	while(1){
		
		printf("Aguardando conexão.\n\n");
		
		while(connfd = accept(listenfd, (struct sockaddr*)NULL,NULL))  // estabele conexão para cadacliente
		{
			
			pthread_t Thread_Cliente;	// cria thread
			
			pthread_create(&Thread_Cliente, NULL, Thread_Conexao, &connfd); // chama função para cada thread

			sleep(1);
		}

	}
	
	pthread_mutex_destroy(&mutex);
}




void* Thread_Conexao(void *Con_socket) // função de conexão da thread
{
	int connfd = *(int*)Con_socket;
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	int tamBuff=0;
	char dir_Caminho[BYTE];
	int dir_ID=1;
	int *id_socket;
	char comando[BYTE] = "fileExists\n";
	
	
	id_socket = malloc(sizeof(byte)); // aloca memoria para o conteudo do endereço ID do socket.
	*id_socket = connfd; // atribuir o endereço ID do socket, pois connfd carrega em seu conteúdo o endereço de memória do socket e não o ID.
	//*id_socket = 0xffffcbd4; // atribuir o endereço ID do socket, pois connfd carrega em seu conteúdo o endereço de memória do socket e não o ID.
			
	printf("\nCliente %i conectado! \n",id_socket);
	
	ChamadaPipe(comando);
	
	if (atoi(comando)) // se o arquivo não existe
	{
		Criar_DIR_RAIZ(connfd,dir_Caminho);
	}
	
	memset(comando, 0, sizeof(comando)); // preenche área de memoria com 0
	snprintf(comando, sizeof(comando), "raiz\n"); // comando para obter o diretorio raiz
	
	ChamadaPipe(comando);
	
	retENT(comando);
	
	stpcpy(dir_Caminho,comando); // copia o diretorio raiz para diretorio inical da thread

	printf("> Diretório atual> %s/\n\n",dir_Caminho);
	
	memset(sendBuff, 0, sizeof(sendBuff)); // preenche área de memoria com 0
	memset(recvBuff, 0, sizeof(recvBuff)); // preenche área de memoria com 0

	snprintf(sendBuff, sizeof(sendBuff), "Conectado!\n> Diretório atual> %s/\n",dir_Caminho);
	send(connfd,sendBuff,strlen(sendBuff), 0);

		do
		{
			
			tamBuff = recv(connfd,recvBuff,BYTE, 0);
			recvBuff[tamBuff] = 0x00;
			retENT(recvBuff);

			
			if (tamBuff < 0) // erro na recepção de mensagem
			{
				printf("\033[41mErro: Buffer de entrada.\033[40m\n");
				snprintf(recvBuff, sizeof(recvBuff), "sair");
				tamBuff = strlen(recvBuff);

			}else
				
			if (strcmp(recvBuff,"cdirR") == 0)  // criar diretorio Raiz
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Criar_DIR_RAIZ(connfd,dir_Caminho);
			}else
			
			if (strcmp(recvBuff,"cdir") == 0)  // criar diretorio
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Criar_DIR(connfd,dir_Caminho);
			}else
			
			if (strcmp(recvBuff,"rdir") == 0) // excluir diretorio
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Remover_DIR(connfd,dir_Caminho);
			}else
			
			if (strcmp(recvBuff,"edir") == 0) // acessar diretorio
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Entrar_DIR(connfd,dir_Caminho);
			}else
				
			if (strcmp(recvBuff,"sdir") == 0) // voltar um diretorio
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Sair_DIR(connfd,dir_Caminho);
			}else

			if (strcmp(recvBuff,"mdir") == 0) // mostrat diretorio
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Mostrar_DIR(connfd,dir_Caminho,dir_ID);
			}else
			
			if (strcmp(recvBuff,"cfile") == 0) // criar arquivo
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Criar_FILE(connfd,dir_Caminho);
			}else
			
			if (strcmp(recvBuff,"rfile") == 0) // excluir arquivo
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Remover_FILE(connfd,dir_Caminho);
			}else
			
			if (strcmp(recvBuff,"efile") == 0) // escrever no arquivo
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Escrever_FILE(connfd,dir_Caminho);
			}else

			if (strcmp(recvBuff,"mfile") == 0) // mostrar arquivo
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Mostrar_FILE(connfd,dir_Caminho);
			}else
			
			if (strcmp(recvBuff,"cmd") == 0) // digitar um comando
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				CMD(connfd);		
			}else
			
			if (strcmp(recvBuff,"-h") == 0) // ajuda
			{
				printf("> (%i) %s\n",id_socket,recvBuff);
				Ajuda(connfd);
			}else
				
			if (strcmp(recvBuff,"sair") == 0) // finalizar
			{
				printf("> (%i) %s\n",id_socket,recvBuff);	
			}else
				
			if (strcmp(recvBuff,"hash") == 0) // mostrar tabela hash
			{
				printf("> (%i) %s\n",id_socket,recvBuff);	
				Mostra_hash();
				snprintf(sendBuff, sizeof(sendBuff), "\033[42mOK\033[40m \n");
				send(connfd,sendBuff,strlen(sendBuff), 0);
			}else
				{
					printf("> Cliente %i digitou comando inválido.\n",id_socket);
					Invalido(connfd);				
				}
		}while(strcmp(recvBuff,"sair") != 0);

	printf("\nCliente %i desconectado.\n\n", id_socket);
	
	close(connfd); // finalizar socket
		
	free(id_socket); // libera memoria
	
	pthread_exit(0); // finaliza thread

}



int ChamadaPipe(char* comando)
{
	char buffer[BYTE];
	int escrita[2];                      // an array that will hold two file descriptors
	int leitura[2]; 
	pipe(escrita);                       // populates fds with two file descriptors
	pipe(leitura);
	pid_t pid = fork();              // create child process that is a clone of the parent

	if (pid == 0) {                  // if pid == 0, then this is the child process
		close(STDIN_FILENO);
		dup(escrita[0]);

		close(STDOUT_FILENO);
		dup(leitura[1]);
		//dup2(escrita[0], STDIN_FILENO);    // fds[0] (the read end of pipe) donates its data to file descriptor 0
		close(escrita[0]);                 // file descriptor no longer needed in child since stdin is a copy
		close(escrita[1]);                 // file descriptor unused in child
		close(leitura[0]);                 // file descriptor no longer needed in child since stdin is a copy
		close(leitura[1]);                 // file descriptor unused in child

		char *argv[] = {(char *)"./file_manager.exe", NULL};  // create argument vector
		execvp(argv[0], argv);         // run sort command
	} else {                         // in parent process
		close(escrita[0]);                 // file descriptor unused in parent
		close(leitura[1]);

		// write input to the writable file descriptor so it can be read in from child:

		  //dprintf(escrita[1], "%s", words); 
		write(escrita[1],comando,strlen(comando));

		// send EOF so child can continue (child blocks until all input has been processed):
		close(escrita[1]); 
	}


	memset(&buffer,0x00,sizeof(buffer));
	read(leitura[0],buffer,sizeof(buffer));

	//printf("> %s",buffer);
	
	strcpy(comando,buffer);

	int status;
	pid_t wpid = waitpid(pid, &status, 0); // wait for child to finish before exiting
	return wpid == pid && WIFEXITED(status) ? WEXITSTATUS(status) : -1;

}




void Ajuda(int connfd)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	int tamBuff=0;
	
	snprintf(sendBuff, sizeof(sendBuff), "\n\t╔══\033[43m AJUDA \033[40m════════════════X═╗\n\t║ cdirR   -cria diret. RAIZ ║\n\t║ cdir    -  cria diretório ║\n\t║ rdir    -remove diretório ║\n\t║ edir    - entra diretório ║\n\t║ sdir    -  sair diretório ║\n\t║ mdir    -mostra diretório ║\n\t║ cfile   -    cria arquivo ║\n\t║ rfile   -  remove arquivo ║\n\t║ efile   - escreve arquivo ║\n\t║ mfile   -  mostra arquivo ║\n\t║ cmd     -  comando prompt ║\n\t║ sair    -        encerrar ║\n\t╚═══════════════════════════╝\n");
	send(connfd,sendBuff,strlen(sendBuff), 0);


/*
185 ╣	186 ║	187 ╗	188 ╝	200 ╚	201 ╔	202 ╩	203 ╦	204 ╠	205 ═	206 ╬
*/	

}



void Criar_DIR_RAIZ(int connfd, char *dir_Caminho)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	char comando[BYTE] = "cdirR\n";
	int tamBuff=0;
	
	
	snprintf(sendBuff, sizeof(sendBuff), "\033[44mCriar diretório RAIZ, digite o nome:\033[40m\n");
	send(connfd,sendBuff,strlen(sendBuff), 0);
	
	tamBuff = recv(connfd,recvBuff,BYTE, 0);
	recvBuff[tamBuff] = 0x00;
	retENT(recvBuff);
	
	strcat(comando,recvBuff);
	strcat(comando,"\n");
	
	memset(sendBuff,0, sizeof(sendBuff));
	
	ChamadaPipe(comando);
	
	snprintf(sendBuff, sizeof(sendBuff), comando);
	send(connfd,sendBuff,strlen(sendBuff), 0);
		
	printf("> %s",comando);

}



void Criar_DIR(int connfd, char *dir_Caminho)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	char comando[BYTE] = "cdir\n";
	int tamBuff=0;
	
	
	snprintf(sendBuff, sizeof(sendBuff), "\033[44mCriar diretório, digite o nome:\033[40m\n");
	send(connfd,sendBuff,strlen(sendBuff), 0);
	
	tamBuff = recv(connfd,recvBuff,BYTE, 0);
	recvBuff[tamBuff] = 0x00;
	retENT(recvBuff);
	
	strcat(comando,recvBuff);
	strcat(comando,"\n");
	
	ChamadaPipe(comando);
	
	snprintf(sendBuff, sizeof(sendBuff), comando);
	send(connfd,sendBuff,strlen(sendBuff), 0);
		
	printf("> %s",comando);

}



void Remover_DIR(int connfd, char *dir_Caminho)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	int tamBuff=0;

	if (chdir(dir_Caminho) == 0)	// chdir - altera o diretório de trabalho
	{	
		snprintf(sendBuff, sizeof(sendBuff), "\033[44mRemover diretório, digite o nome:\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		
		tamBuff = recv(connfd,recvBuff,BYTE, 0);
		recvBuff[tamBuff] = 0x00;
		retENT(recvBuff);
		
		char comando[BYTE]  = "rmdir ";
		strcat(comando,recvBuff);
			
		if (system(comando) == 0)
		{
			snprintf(sendBuff, sizeof(sendBuff), "\033[42mDiretório removido com sucesso.\033[40m\n");
			send(connfd,sendBuff,strlen(sendBuff), 0);
		}else
			{			
			snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao remover diretório.\033[40m\n");
			send(connfd,sendBuff,strlen(sendBuff), 0);
			}
	}else
		{			
		stpcpy(dir_Caminho,dir_Raiz);
		snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao acessar diretório atual. Redirecionado a Raiz.\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		}			
}  



void Entrar_DIR(int connfd, char *dir_Caminho)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	char pasta[BYTE];
	int tamBuff=0;

	if (chdir(dir_Caminho) == 0)	// chdir - altera o diretório de trabalho
	{	
		snprintf(sendBuff, sizeof(sendBuff), "\033[44mEntrar em diretório, digite o nome:\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		
		tamBuff = recv(connfd,recvBuff,BYTE, 0);
		recvBuff[tamBuff] = 0x00;
		retENT(recvBuff);
		
		memset(pasta, 0, sizeof(pasta)); // preenche área de memoria com 0
		
		strcpy(pasta,dir_Caminho);
		strcat(pasta,"/");
		strcat(pasta,recvBuff);
		
		if (chdir(pasta) == 0)	// chdir - altera o diretório de trabalho
		{
			getcwd(dir_Caminho, BYTE); // getcwd - obtém o nome do caminho do diretório de trabalho atual
			printf("Entrou no diretório> %s \n",dir_Caminho);
			
			snprintf(sendBuff, sizeof(sendBuff), "\033[42mDiretório atual>\033[40m %s\n",dir_Caminho);
			send(connfd,sendBuff,strlen(sendBuff), 0);
		}else
			{			
			snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao entrar em diretório.\033[40m\n");
			send(connfd,sendBuff,strlen(sendBuff), 0);
			}
	}else
		{			
		stpcpy(dir_Caminho,dir_Raiz);
		snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao acessar diretório atual. Redirecionado a Raiz.\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		}
} 



void Sair_DIR(int connfd, char *dir_Caminho)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	char pasta[BYTE];
	int tamBuff=0;

	if (chdir(dir_Caminho) == 0)	// chdir - altera o diretório de trabalho
	{	
		memset(pasta, 0, sizeof(pasta)); // preenche área de memoria com 0
		
		strcpy(pasta,dir_Caminho);
		strcat(pasta,"/..");
		
		if (chdir(pasta) == 0)	// chdir - altera o diretório de trabalho
		{
			getcwd(dir_Caminho, BYTE); // getcwd - obtém o nome do caminho do diretório de trabalho atual
			printf("Retornou no diretório> %s \n",dir_Caminho);
			
			snprintf(sendBuff, sizeof(sendBuff), "\033[42mDiretório atual>\033[40m %s\n",dir_Caminho);
			send(connfd,sendBuff,strlen(sendBuff), 0);
		}else
			{			
			snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao retornar em diretório.\033[40m\n");
			send(connfd,sendBuff,strlen(sendBuff), 0);
			}
	}else
		{			
		stpcpy(dir_Caminho,dir_Raiz);
		snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao acessar diretório atual. Redirecionado a Raiz.\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		}
}



void Mostrar_DIR(int connfd, char *dir_Caminho, int dir_ID)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	char comando[BYTE];
	
	memset(comando, 0, sizeof(comando));
	snprintf(comando, sizeof(comando), "mdir\n%i\n",dir_ID);
	
	ChamadaPipe(comando);
	
	//printf("> %s",comando);

	memset(sendBuff, 0, sizeof(sendBuff)); // preenche área de memoria com 0
		
	strcat(sendBuff,"\033[42mDiretório>\033[40m ");
	strcat(sendBuff,dir_Caminho);
	strcat(sendBuff,"/\n\n");

	strcat(sendBuff,comando);

	send(connfd,sendBuff,strlen(sendBuff),0);
}



void Criar_FILE(int connfd, char *dir_Caminho)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	char comando[BYTE]  = "cfile\n";
	int tamBuff=0;

	snprintf(sendBuff, sizeof(sendBuff), "\033[44mCriar arquivo, digite o nome:\033[40m\n");
	send(connfd,sendBuff,strlen(sendBuff), 0);
	
	tamBuff = recv(connfd,recvBuff,BYTE, 0);
	recvBuff[tamBuff] = 0x00;
	retENT(recvBuff);
	
	strcat(comando,recvBuff);
	
	memset(recvBuff,0, sizeof(recvBuff));
	
	snprintf(sendBuff, sizeof(sendBuff), "\033[44mDigite o conteúdo:\033[40m\n");
	send(connfd,sendBuff,strlen(sendBuff), 0);
	
	tamBuff = recv(connfd,recvBuff,BYTE, 0);
	recvBuff[tamBuff] = 0x00;
	retENT(recvBuff);
	
	strcat(comando,"\n");
	strcat(comando,recvBuff);
	strcat(comando,"\n");
	
	ChamadaPipe(comando);
	
	snprintf(sendBuff, sizeof(sendBuff), comando);
	send(connfd,sendBuff,strlen(sendBuff), 0);
		
	printf("> %s",comando);
	
}



void Remover_FILE(int connfd, char *dir_Caminho)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	char arquivo[BYTE];
	int tamBuff=0;

	
	if (chdir(dir_Caminho) == 0)	// chdir - altera o diretório de trabalho
	{
		snprintf(sendBuff, sizeof(sendBuff), "\033[44mRemover arquivo, digite o nome:\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		
		tamBuff = recv(connfd,recvBuff,BYTE, 0);
		recvBuff[tamBuff] = 0x00;
		retENT(recvBuff);
		
		memset(arquivo, 0, sizeof(arquivo)); // preenche área de memoria com 0
		strcpy(arquivo,dir_Caminho);
		strcat(arquivo,"/");
		strcat(arquivo,recvBuff);
		
		pthread_mutex_lock(&mutex);
			int Em_uso = Buscar_hash(arquivo); // Retorna -1=Não Achou; 0=Mostrar; 1=Escrever; 2=Remover;
			//Mostra_hash();
		pthread_mutex_unlock(&mutex);
		
		if (Em_uso >= 0)
		{
			snprintf(sendBuff, sizeof(sendBuff), "\033[41mArquivo sendo manipulado por outro Cliente.\033[40m\n");
			send(connfd,sendBuff,strlen(sendBuff), 0);
		}else
			{		
			char comando[BYTE]  = "rm ";
			strcat(comando,recvBuff);
			
			pthread_mutex_lock(&mutex);
				if (Inserir_hash(arquivo,'R') == -1)
					printf("\033[41mErro ao inserir na Hash.\033[40m\n");
				//Mostra_hash();
			pthread_mutex_unlock(&mutex);
			
			if (system(comando) == 0)
			{
				snprintf(sendBuff, sizeof(sendBuff), "\033[42mArquivo removido com sucesso.\033[40m\n");
				send(connfd,sendBuff,strlen(sendBuff), 0);
			}else
				{			
				snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao remover arquivo.\033[40m\n");
				send(connfd,sendBuff,strlen(sendBuff), 0);
				}
			
			pthread_mutex_lock(&mutex);
				if (Remover_hash(arquivo) == -1)
					printf("\033[41mErro ao remover na Hash.\033[40m\n");
				//Mostra_hash();
			pthread_mutex_unlock(&mutex);
		}
	}else
		{			
		stpcpy(dir_Caminho,dir_Raiz);
		snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao acessar diretório atual. Redirecionado a Raiz.\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		}
}  



void Escrever_FILE(int connfd, char *dir_Caminho)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	char arquivo[BYTE];
	int tamBuff=0;
	
	
	if (chdir(dir_Caminho) == 0)	// chdir - altera o diretório de trabalho
	{
		snprintf(sendBuff, sizeof(sendBuff), "\033[44mEscrever em arquivo, digite o nome:\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		
		tamBuff = recv(connfd,recvBuff,BYTE, 0);
		recvBuff[tamBuff] = 0x00;
		retENT(recvBuff);
		
		memset(arquivo, 0, sizeof(arquivo)); // preenche área de memoria com 0
		strcpy(arquivo,dir_Caminho);
		strcat(arquivo,"/");
		strcat(arquivo,recvBuff);
		
		pthread_mutex_lock(&mutex);
			int Em_uso = Buscar_hash(arquivo); // Retorna -1=Não Achou; 0=Mostrar; 1=Escrever; 2=Remover;
			//Mostra_hash();
		pthread_mutex_unlock(&mutex);
		
		if (Em_uso >= 0)
		{
			snprintf(sendBuff, sizeof(sendBuff), "\033[41mArquivo sendo manipulado por outro Cliente.\033[40m\n");
			send(connfd,sendBuff,strlen(sendBuff), 0);
		}else
			{
	 
			FILE *nome_arquivo; 
			
			pthread_mutex_lock(&mutex);
				if (Inserir_hash(arquivo,'E') == -1)
					printf("\033[41mErro ao inserir na Hash.\033[40m\n");
				//Mostra_hash();
			pthread_mutex_unlock(&mutex);
			
			if(nome_arquivo = fopen(recvBuff,"a+")) // abertura como escrita no final e não cria novo arquivo se não existir
			{ 
				snprintf(sendBuff, sizeof(sendBuff), "\033[44mDigite o texto:\033[40m\n");
				send(connfd,sendBuff,strlen(sendBuff), 0);	
				
				tamBuff = recv(connfd,recvBuff,BYTE, 0);
				recvBuff[tamBuff] = 0x00;
				retENT(recvBuff);
				
				strcat(recvBuff,"\n");
				
				if(fprintf(nome_arquivo,recvBuff) < 0)
				{
					snprintf(sendBuff, sizeof(sendBuff), "\033[41mNão foi possivel escrever no arquivo.\033[40m\n");
					send(connfd,sendBuff,strlen(sendBuff), 0);
				}else
					{
						snprintf(sendBuff, sizeof(sendBuff), "\033[42mArquivo salvo.\033[40m \n");
						send(connfd,sendBuff,strlen(sendBuff), 0);			
					}
				
			}else
				{			
				snprintf(sendBuff, sizeof(sendBuff), "\033[41mNão foi possivel abrir o arquivo.\033[40m\n");
				send(connfd,sendBuff,strlen(sendBuff), 0);
				}
			fclose(nome_arquivo);
			
			pthread_mutex_lock(&mutex);
				if (Remover_hash(arquivo) == -1)
					printf("\033[41mErro ao remover na Hash.\033[40m\n");
				//Mostra_hash();
			pthread_mutex_unlock(&mutex);
		}
	}else
		{			
		stpcpy(dir_Caminho,dir_Raiz);
		snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao acessar diretório atual. Redirecionado a Raiz.\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		}
}



void Mostrar_FILE(int connfd, char *dir_Caminho)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	char arquivo[BYTE];
	char conteudo[BYTE];
	int tamBuff=0;

	
	if (chdir(dir_Caminho) == 0)	// chdir - altera o diretório de trabalho
	{
		snprintf(sendBuff, sizeof(sendBuff), "\033[44mMostrar arquivo, digite o nome:\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		
		tamBuff = recv(connfd,recvBuff,BYTE, 0);
		recvBuff[tamBuff] = 0x00;
		retENT(recvBuff);
		
		memset(arquivo, 0, sizeof(arquivo)); // preenche área de memoria com 0
		strcpy(arquivo,dir_Caminho);
		strcat(arquivo,"/");
		strcat(arquivo,recvBuff);
		
		pthread_mutex_lock(&mutex);
			int Em_uso = Buscar_hash(arquivo); // Retorna -1=Não Achou; 0=Mostrar; 1=Escrever; 2=Remover;
			//Mostra_hash();
		pthread_mutex_unlock(&mutex);
		
		if (Em_uso >= 1)
		{
			snprintf(sendBuff, sizeof(sendBuff), "\033[41mArquivo sendo manipulado por outro Cliente.\033[40m\n");
			send(connfd,sendBuff,strlen(sendBuff), 0);
		}else
			{
	 
			FILE *nome_arquivo; 
			
			pthread_mutex_lock(&mutex);
				if (Inserir_hash(arquivo,'M') == -1)
					printf("\033[41mErro ao inserir na Hash.\033[40m\n");
				//Mostra_hash();
			pthread_mutex_unlock(&mutex);
			
			if(nome_arquivo = fopen(recvBuff,"r"))
			{ 
				memset(sendBuff, 0, sizeof(sendBuff)); // preenche área de memoria com 0
				memset(conteudo, 0, sizeof(sendBuff)); // preenche área de memoria com 0
				
				fread(conteudo, sizeof(char),BYTE-3,nome_arquivo);
				strcat(sendBuff,"\n\n");
				strcat(sendBuff,conteudo);
				strcat(sendBuff,"\n");
				send(connfd,sendBuff,strlen(sendBuff), 0);
			}else
				{			
				snprintf(sendBuff, sizeof(sendBuff), "\033[41mNão foi possivel abrir o arquivo.\033[40m\n");
				send(connfd,sendBuff,strlen(sendBuff), 0);
				}
			fclose(nome_arquivo);
			
			pthread_mutex_lock(&mutex);
				if (Remover_hash(arquivo) == -1)
					printf("\033[41mErro ao remover na Hash.\033[40m\n");
				//Mostra_hash();
			pthread_mutex_unlock(&mutex);
		}
	}else
		{			
		stpcpy(dir_Caminho,dir_Raiz);
		snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao acessar diretório atual. Redirecionado a Raiz.\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		}
}



void CMD(int connfd)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	int tamBuff=0;
	
	snprintf(sendBuff, sizeof(sendBuff), "\033[44mDigite o comando:\033[40m\n");
	send(connfd,sendBuff,strlen(sendBuff), 0);
	
	tamBuff = recv(connfd,recvBuff,BYTE, 0);
	recvBuff[tamBuff] = 0x00;
	retENT(recvBuff);

	char comando[BYTE]  = "";
	strcat(comando,recvBuff);
	
	if (system(comando) == 0)
	{
		snprintf(sendBuff, sizeof(sendBuff), "\033[42mOK\033[40m \n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
	}else
		{			
		snprintf(sendBuff, sizeof(sendBuff), "\033[41mErro ao chamar comando.\033[40m\n");
		send(connfd,sendBuff,strlen(sendBuff), 0);
		}
		
}



void Invalido(int connfd)
{
	char sendBuff[BYTE];
	char recvBuff[BYTE];
	int tamBuff=0;
	
	snprintf(sendBuff, sizeof(sendBuff), "\033[41mComando inválido. Ajuda -h\033[40m\n");
	send(connfd,sendBuff,strlen(sendBuff), 0);
}



void retENT(char *recvBuff)	// remove ENTER do final do buffer
{
	
	if (recvBuff[strlen(recvBuff)-1] == 10)
		recvBuff[strlen(recvBuff)-1] = 0x00;	
}





//******************** CODIGO TABELA HASH ******************//


int Inserir_hash(char *arquivo, char operacao)
{
	strcpy(arquivo,strlwr(arquivo)); // converte a string para minuscula
	
	int hash = Funcao_hash(arquivo);
	int cont = hash;
	int i;
	int Cheia = 1;
	
	for(i=0;i<M;i++)
	{
		if (Manipulacao_Arquivo[i].status == 0 || Manipulacao_Arquivo[i].status == 3)
			Cheia = 0;
	}
	
	if (Cheia) return(-1);
	
	if (Manipulacao_Arquivo[hash].status == 0)
	{
		Manipulacao_Arquivo[hash].status = 1;
		strcpy(Manipulacao_Arquivo[hash].arquivo,arquivo);
		Manipulacao_Arquivo[hash].operacao = operacao;
		return(0);
	}else 
		
	if (Manipulacao_Arquivo[hash].status == 1 || Manipulacao_Arquivo[hash].status == 2)
	{
		while(Manipulacao_Arquivo[cont].status == 1 || Manipulacao_Arquivo[cont].status == 2)
		{
			Manipulacao_Arquivo[cont++].status = 2;	
			if (cont == M) cont = 0;
			if (cont == hash) return(-1);
		}
		
		if (Manipulacao_Arquivo[cont].status == 3)
			Manipulacao_Arquivo[cont].status = 2;
			else
				Manipulacao_Arquivo[cont].status = 1;
			
		strcpy(Manipulacao_Arquivo[cont].arquivo,arquivo);
		Manipulacao_Arquivo[cont].operacao = operacao;
		return(0);
	}else
	
	if (Manipulacao_Arquivo[hash].status == 3)
	{
		
		Manipulacao_Arquivo[hash].status = 2;
		strcpy(Manipulacao_Arquivo[hash].arquivo,arquivo);
		Manipulacao_Arquivo[hash].operacao = operacao;
		return(0);
	}

	return(-1);
}



int Remover_hash(char *arquivo)
{
	strcpy(arquivo,strlwr(arquivo)); // converte a string para minuscula
	
	int hash = Funcao_hash(arquivo);
	
	return R_hash(arquivo,hash,0);	
}



int R_hash(char *arquivo, int hash, int inc)
{
	int cont = hash;
	
	if (inc == M) return(-1);
	
	if (Manipulacao_Arquivo[hash].status == 0) // não existe nada
	{
		return(-1);
	}else 
		
	if (Manipulacao_Arquivo[hash].status == 1) // existe algo
	{
		if (strcmp(Manipulacao_Arquivo[hash].arquivo,arquivo) == 0) // se esta aqui
		{
			Manipulacao_Arquivo[hash].status = 0; // ZERA POSIÇÃO
			memset(&Manipulacao_Arquivo[hash].arquivo, 0, sizeof(Manipulacao_Arquivo[hash].arquivo));
			Manipulacao_Arquivo[hash].operacao = ' ';
			
			cont--;
			if (cont == -1) cont = M-1;

			while(Manipulacao_Arquivo[cont].status == 3) // enquanto a posição a cima por 3
			{
				Manipulacao_Arquivo[cont--].status = 0; // zera a posição a cima atual
				if (cont == -1) cont = M-1;
			}
			
			if (Manipulacao_Arquivo[cont].status == 2) // se a posição a cima for 2 
			{
				Manipulacao_Arquivo[cont].status = 1; // seta a posição a cima para 1 
			}
			
			return(0);
		}else
			return(-1); // se não, não esta aqui
	}else
		
	if (Manipulacao_Arquivo[hash].status == 2) // existe algo, mas pode estar em outro lugar
	{
		if (strcmp(Manipulacao_Arquivo[hash].arquivo,arquivo) == 0) // se esta aqui
		{
			Manipulacao_Arquivo[hash].status = 3; // INDICA QUE ESTA VAZIO, MAS HOUVE QUE COLISÃO
			memset(&Manipulacao_Arquivo[hash].arquivo, 0, sizeof(Manipulacao_Arquivo[hash].arquivo));
			Manipulacao_Arquivo[hash].operacao = ' ';
			return(0);
		}else
			
		cont++;
		if (cont == M) cont = 0;
		return R_hash(arquivo,cont,inc++); // procura denovo na tabela com hash + 1
	}else
		
	if (Manipulacao_Arquivo[hash].status == 3) // não existe nada, mas pode estar em outro lugar
	{
		cont++;
		if (cont == M) cont = 0;
		return R_hash(arquivo,cont,inc++); // procura denovo na tabela com hash + 1
	}
	
	return(-1);	
}




int Buscar_hash(char *arquivo) // Retorna -1=Não Achou; 0=Mostrar; 1=Escrever; 2=Remover;
{
	strcpy(arquivo,strlwr(arquivo)); // converte a string para minuscula
	
	int hash = Funcao_hash(arquivo);
	
	return B_hash(arquivo,hash,0);
}



int B_hash(char *arquivo, int hash, int inc)
{
	int cont = hash;
	
	if (inc == M) return(-1);
	
	if (Manipulacao_Arquivo[hash].status == 0) // não existe nada
	{
		return(-1);
	}else 
		
	if (Manipulacao_Arquivo[hash].status == 1) // existe algo
	{
		if (strcmp(Manipulacao_Arquivo[hash].arquivo,arquivo) == 0) // se esta aqui
		{
			if (Manipulacao_Arquivo[hash].operacao == 'M') return(0);
			if (Manipulacao_Arquivo[hash].operacao == 'E') return(1);
			if (Manipulacao_Arquivo[hash].operacao == 'R') return(2);
			if (Manipulacao_Arquivo[hash].operacao == ' ') return(-1);
		}else
			return(-1); // se não, não esta aqui
	}else
		
	if (Manipulacao_Arquivo[hash].status == 2) // existe algo, mas pode estar em outro lugar
	{
		if (strcmp(Manipulacao_Arquivo[hash].arquivo,arquivo) == 0) // se esta aqui
		{
			if (Manipulacao_Arquivo[hash].operacao == 'M') return(0);
			if (Manipulacao_Arquivo[hash].operacao == 'E') return(1);
			if (Manipulacao_Arquivo[hash].operacao == 'R') return(2);
			if (Manipulacao_Arquivo[hash].operacao == ' ') return(-1);
		}else
			
		cont++;
		if (cont == M) cont = 0;
		return B_hash(arquivo,cont,inc++); // procura denovo na tabela com hash + 1
	}else
		
	if (Manipulacao_Arquivo[hash].status == 3) // não existe nada, mas pode estar em outro lugar
	{
		cont++;
		if (cont == M) cont = 0;
		return B_hash(arquivo,cont,inc++); // procura denovo na tabela com hash + 1
	}
	
	return(-1);	
}



int Funcao_hash(char *arquivo)
{
	strcpy(arquivo,strlwr(arquivo)); // converte a string para minuscula
	
	int tam = strlen(arquivo);
	int i=0;
	int hash=0;
	
	
	for (i=0;i<tam;i++)
	{
		hash = (int)((int)pow((arquivo[i]+hash),2.7) % M);
		//printf("\n%i - %i",arquivo[i],hash);
	}

	return hash;
	
}

void Mostra_hash(void)
{
	int i=0;
	
	for(i=0;i<M;i++)
		printf("\n\t%03i -| %i | %s | %c |",i,Manipulacao_Arquivo[i].status,Manipulacao_Arquivo[i].arquivo,Manipulacao_Arquivo[i].operacao);
	
	printf("\n\n");
}


void Inicializa_hash(void)
{
	int i;
	
	for(i=0;i<M;i++)
	{
		Manipulacao_Arquivo[i].status = 0;
		memset(&Manipulacao_Arquivo[i].arquivo, 0, sizeof(Manipulacao_Arquivo[i].arquivo));
		Manipulacao_Arquivo[i].operacao = ' ';
	}	
}


/*

		Em linux:

		cd home/
		gcc -o servidor.exe servidor.c
		./servidor.exe
	----------------------------------------------
		Em Windows:

		cd \
		cd cygwin64
		cd home
		ls
		gcc servidor.c -o servidor.exe
		servidor.exe


*/
