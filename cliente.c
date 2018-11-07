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
	char buffer_in[1024], buffer_out[1024], input[1024], to[20], from[20], subject[20];// Es el tamaño maximo de 1024 del buffer de los datos recibidos, enviados y los que entran.
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
			if (strcmp(ipdest, "") == 0) // Compara con una cadena de caracteres vacia y al ser enter una cadena de caracteres vacia se cumple y copia la de default en la ipdest
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);


			server_in4.sin_family = AF_INET;
			server_in4.sin_port = htons(TCP_SERVICE_PORT);
			//server_in4.sin_addr.s_addr = inet_addr(ipdest);//Para meter una ip de manera numerica
			inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
			server_in = (struct sockaddr*)&server_in4;
			address_size = sizeof(server_in4);

			estado = S_HELO;//Asigna estado a S_HELO

			//PRIMITIVA CONNECT
			// establece la conexion de transporte
			if (connect(sockfd, server_in, address_size) == 0) {
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);

				recibidos = recv(sockfd, buffer_in, 512, 0); //recibimos la informacion del servidor para saber que esta listo
				buffer_in[recibidos] = 0x00;
				printf(buffer_in);
				//Inicio de la máquina de estados
				do {
					switch (estado) {
					case S_HELO:
						printf("CLIENTE> Para comenzar, ingrese el comando HELO: ");
						gets_s(input, sizeof(input));
						
							sprintf_s(buffer_out, sizeof(buffer_out), "HELO %s%s", ipdest, CRLF);//Mensaje inicial HELO
						

						break;
					case S_MAIL_FROM:

							printf("CLIENTE> MAIL FROM: (Pulse intro para salir) ");
							gets_s(input, sizeof(input));
							if (strlen(input) == 0) {
								// Si la longitud de input es 0, 
								//Escribe en el servidor QUIT y pasamos al estado QUIT
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", "QUIT", CRLF);
								estado = S_QUIT;
							}
							else {
									sprintf_s(buffer_out, sizeof(buffer_out), "MAIL FROM:<%s>%s", input, CRLF);//MENSAJE DE REMITENTE
									strcpy_s(from,sizeof(from), input);//guardamos para el mensaje completo
								}


						break;
					case S_RCPT_TO:
							
							printf("CLIENTE> RCPT TO: (Pulse intro para salir) ");
							gets_s(input, sizeof(input));

							if (strlen(input) == 0) {
								// Si la longitud de input es 0, 
								//Escribe en el servidor QUIT y pasamos al estado QUIT
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", "QUIT", CRLF);
								estado = S_QUIT;
							}
							else {

								sprintf_s(buffer_out, sizeof(buffer_out), "RCPT TO:<%s>%s", input, CRLF);//enviamos el destinatario
								strcpy_s(to,sizeof(to), input);
							}
						
							break;

					case S_DATA:
						sprintf_s(buffer_out, sizeof(buffer_out), "DATA%s", CRLF);//comando para pasar al mensaje
						estado++;
						break;
					
					case S_MENSAJE:
						printf("CLIENTE> subject: ");
						gets(input);
						strcpy_s(subject,sizeof(subject), input);
						sprintf_s(buffer_out, sizeof(buffer_out), "Asunto: %s%sDe:%s%A: %s%s", subject, CRLF, to, CRLF, from, CRLF);//enviamos las cabeceras

						do {//vamos montando un mensaje en el buffer out hasta que sea distinto de un .
							printf("Redacte su mensaje: (Si desea terminar teclee '.'):", to, CRLF);
							gets(input);
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s", buffer_out, CRLF, input);
							fflush(stdin);
							fflush(stdout);
						} while (strcmp(input, ".") != 0);
						sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", buffer_out, CRLF);

						break;




					case S_QUIT:
						/*estado para Cerrar conexión*/
						break;


					}


					//**************************************************************************************************************************

					//Envio
					

					enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
			
					/* vamos ha realizar una comprobacion de los datos enviados muy parecida a lo que se hace en recibidos*/
					if (enviados <= 0) {// si los datos enviados tienen un valor de menor o igual que 0
						DWORD error = GetLastError();
						if (enviados < 0) { // si los datos enviados tienen un valor de -1 hay error
							printf("SERVIDOR> Error %d en el envio de datos\r\n", error);
							estado = S_QUIT;
							continue;
						}
						else { // si no, es decir, si su valor es 0 se cierra la conexion
							printf("SERVIDOR> Conexión con el cliente cerrada\r\n");
							estado = S_QUIT;
							continue;
						}
					}
					else { // el valor es mayor que 0 y por tanto los datos se envian correctamente
						//printf("SERVIDOR> Datos enviados correctamente\r\n");
					}
					//}

					//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	




					//Recibo


					recibidos = recv(sockfd, buffer_in, 512, 0);

					if (recibidos <= 0) {
						DWORD error = GetLastError();
						if (recibidos < 0)
						{
							printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
							estado = S_QUIT;
							continue;
						}
						else
						{
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado = S_QUIT;
							continue;


						}
					}
					else
					{
						buffer_in[recibidos] = 0x00;
						printf(buffer_in);

						switch (estado) {
						case S_HELO:
							if (estado != S_QUIT) {
								estado++;
							}
							else estado = S_QUIT;
							break;
						case S_MAIL_FROM:
							if (estado != S_QUIT) {
								estado++;
							}
							else estado = S_QUIT;
							break;
						case S_RCPT_TO:
							if (estado != S_QUIT) {
								estado++;
								}
							else estado = S_QUIT;
							break;

							/*Las siguientes lineas de codigo lo utilizamos para enviar mas mensajes antes de finalizar la sesion*/
						case S_MENSAJE:
							if (buffer_in[0] == '2') {
								printf("¿Quiere mandar otro mensaje antes de finalizar sesion?(si/no)\r\n");
								if (_getch() == 's') {
									estado = S_MAIL_FROM;
								}
								else { estado = S_QUIT; }
								break;


							}
						}

					}
				} while (estado != S_QUIT);





			}
			else
			{
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
			}
			// fin de la conexion de transporte
			closesocket(sockfd);//cerramos el socket

		}
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option = _getche();

	} while (option != 'n' && option != 'N');



	return(0);

}