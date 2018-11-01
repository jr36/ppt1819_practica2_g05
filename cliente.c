/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 2.0
Fecha: 09/2018
Descripción:
	Cliente sencillo TCP para IPv4 e IPv6

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <ws2tcpip.h>//Necesaria para las funciones IPv6
#include <conio.h>
#include "protocol.h"






#pragma comment(lib, "Ws2_32.lib")

int main(int *argc, char *argv[])
{
	SOCKET sockfd; //Crea el socket
	struct sockaddr *server_in = NULL; //Crea la estructura del socket y la añade al servidor
	struct sockaddr_in server_in4;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024];// Es el tamaño maximo de 1024 del buffer de los datos recibidos, enviados y los que entran.
	int recibidos = 0, enviados = 0;
	int estado = S_HELO;//Primer valor en la variable estado ya que será una maquina de estados
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[16];
	char default_ip4[16] = "127.0.0.1";
	
	
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
   
	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		return(0);

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	printf("**************\r\nCLIENTE TCP SENCILLO\r\n*************\r\n");


	do {

		

		sockfd = socket(ipversion, SOCK_STREAM, 0);
		if (sockfd == INVALID_SOCKET) {//Comprobacion para ver si se a creado bien el SOCKET
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else {//Si crea bien el socket pide la ip de destino por pantalla(donde queremos acceder).
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets_s(ipdest, sizeof(ipdest));//obtiene la ip de destino en la variable ipdest y almacena en la variable

			//Dirección por defecto según la familia
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET)
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);

		

			if (ipversion == AF_INET) {
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(TCP_SERVICE_PORT);
				//server_in4.sin_addr.s_addr=inet_addr(ipdest);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}

			
			estado = S_HELO;//Asigna estado a S_HELO

			//PRIMITIVA CONNECT
			// establece la conexion de transporte
			if (connect(sockfd, server_in, address_size) == 0) {
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);

				recibidos = recv(sockfd, buffer_in, 512, 0); //recibimos la informacion del servidor para saber que esta listo
				buffer_in[recibidos] = 0x00;
				printf(buffer_in);

				//Inicio de la máquina de estados
				do{
					switch(estado){
					
					case S_HELO:

						sprintf_s(buffer_out, sizeof(buffer_out), "HELO %s%s", ipdest, CRLF);//Mensaje inicial HELO
						
						break;
					case S_USER:
						// establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el usuario (enter para salir): ");
						gets_s(input,sizeof(input));
						if(strlen(input)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else

						sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",SC,input,CRLF);
						break;
					case S_PASS:
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets_s(input, sizeof(input));
						if(strlen(input)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",PW,input,CRLF);
						break;
					case S_DATA:
						printf("CLIENTE> Introduzca datos (enter o QUIT para salir): ");
						gets_s(input, sizeof(input));
						if(strlen(input)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",ECHO,input,CRLF);
						break;
				
					}

					if(estado!=S_HELO){
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);
						if(enviados==SOCKET_ERROR){
							 estado=S_QUIT;
							 continue;
						}
					}
						
					recibidos=recv(sockfd,buffer_in,512,0);
					if(recibidos<=0){
						DWORD error=GetLastError();
						if(recibidos<0){
							printf("CLIENTE> Error %d en la recepción de datos\r\n",error);
							estado=S_QUIT;
						}
						else{
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado=S_QUIT;
						}
					}else{
						buffer_in[recibidos]=0x00;
						printf(buffer_in);
						if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0) 
							estado++;  
					}
					case S_QUIT:
						/*estado para Cerrar conexión*/
						break;

				}while(estado!=S_QUIT);		
			}
			else{
				int error_code=GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			}		
			// fin de la conexion de transporte
			closesocket(sockfd);
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();

	}while(option!='n' && option!='N');

	return(0);
}
