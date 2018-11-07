#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile
#endif

// COMANDOS DE APLICACION
#define HE "HELO"
#define DATA "DATA"


// RESPUESTAS A COMANDOS DE APLICACION
#define OK  "OK"
#define ER  "ER"

//FIN DE RESPUESTA
#define CRLF "\r\n"

//ESTADOS
#define S_HELO 0
#define S_MAIL_FROM 1
#define S_RCPT_TO 2
#define S_DATA 3
#define S_MENSAJE 4
#define S_QUIT 5
#define S_EXIT 6

//PUERTO DEL SERVICIO
#define TCP_SERVICE_PORT	25

