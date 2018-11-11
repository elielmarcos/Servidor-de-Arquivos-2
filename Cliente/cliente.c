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



//#define SERVER_IP "192.168.0.104"
#define SERVER_IP "127.0.0.1"
#define BYTE 16384
#define PORTA 5000
#define TITULO "\n ====\033[43m EXPLORADOR DE ARQUIVOS CLIENTE \033[40m====\n\n"

void Aguarde(void);


int main(int argc, char *argv[])
{

	setlocale(LC_ALL, "Portuguese");
    int tamBuff, skt;
	char sendBuff[BYTE];
	char recvBuff[BYTE];
    struct sockaddr_in serv;

    /**INICIALIZA ESTRUTURA SOCKETS*/
    skt = socket(AF_INET, SOCK_STREAM, 6);
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv.sin_port = htons (PORTA);
    memset (&(serv.sin_zero), 0x00, sizeof (serv.sin_zero));

    /**INICIA COMUNICAÇÃO COM SERVIDOR*/
    while(connect (skt, (struct sockaddr *)&serv, sizeof (struct sockaddr)) != 0){
        Aguarde();      ///AGUARDA SERVIDOR SE COMUNICAR
    }
	system("clear");
	printf(TITULO);
    printf("> A Conexão com o Servidor %s foi estabelecida na porta %d\n\n",SERVER_IP,PORTA);
    printf("> Envie sair pra encerrar ou -h para ajuda \n\n");


	memset(sendBuff, 0, sizeof(sendBuff)); // preenche área de memoria com 0
	memset(recvBuff, 0, sizeof(recvBuff)); // preenche área de memoria com 0
	
    /**RECEBE MENSAGEM DO SERVIDOR*/
    tamBuff = recv (skt, recvBuff, BYTE, 0);
    recvBuff[tamBuff] = 0x00;
    printf ("> %s\n",recvBuff);


    /**LOOP DE COMUNICAÇÃO ENTRE CLIENTE E SERVIDOR*/
    do{
        ///envia
        printf("> ");
        gets(sendBuff);
		if (strlen(sendBuff)==0) strcpy(sendBuff," ");
        send(skt, sendBuff, strlen(sendBuff), 0);

		if (strcmp(sendBuff,"sair")!= 0)
		{
			///recebe
			tamBuff = recv (skt, recvBuff, BYTE, 0);
			recvBuff[tamBuff] = 0x00;
			printf ("> %s\n",recvBuff);
		}

    }while(strcmp(sendBuff,"sair")!= 0);    ///COMUNICAÇÃO SE ENCERRA QUANDO USUARIO DIGITAR sair


    /**FINALIZA CONEXÃO*/
    close(skt);
    printf (">> \033[43mA conexão com o servidor foi finalizada!!!\033[40m\n\n");
	sleep(2);
    exit(0);
}



void Aguarde(){
    int i=0;
    char ponto[12] = "";
    for(i=0; i<4;i++){
        system("clear");
        printf(TITULO);
        printf("\n\nProcurando servidor.");
        printf("\nAguarde %s\n\n", ponto);
        strcat(ponto,".");
        sleep(1);
    }
    strcpy(ponto, "");
}