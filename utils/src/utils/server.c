#include "server.h"

int iniciar_servidor(char* PUERTO, t_log* logger, char* clienteEsperado){
    int socket_servidor, err;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	err = getaddrinfo(NULL, PUERTO, &hints, &servinfo);

	// Creamos el socket de escucha del servidor

	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	// Asociamos el socket a un puerto
	err = bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	err = listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
    
    if(err == -1){
        error_show("Error al iniciar el servidor");
        exit(EXIT_FAILURE);
    } else{
        log_info(logger, "Listo para escuchar a mi cliente: %s", clienteEsperado);
    }

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger)
{
	// Aceptamos un nuevo cliente
	int socket_cliente, err;
	socket_cliente = accept(socket_servidor, NULL, NULL);

    if(err == -1){
        error_show("Error en la espera del cliente");
        exit(EXIT_FAILURE);
    } else{
        log_info(logger, "Se conecto un cliente!");
    }

	return socket_cliente;
}