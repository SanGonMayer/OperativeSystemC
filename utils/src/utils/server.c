#include "server.h"
#include "utils/client.h"
#include <asm-generic/socket.h>
#include <commons/error.h>
#include <commons/log.h>
#include <sys/socket.h>

int iniciar_servidor(char* puerto, t_log* logger, char* clienteEsperado){
    int socket_servidor, err;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	err = getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor

	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);


	if(setsockopt(
		socket_servidor, 
		SOL_SOCKET, 
		SO_REUSEADDR,
		&(int){1}, 
		sizeof(int)) < 0
	)
	{
		error_show("Error al setear opciones del socket");
		exit(EXIT_FAILURE);
	}

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

int esperar_nueva_conexion_cliente(int socket_servidor, t_log* logger)
{
	// Aceptamos un nuevo cliente
	int socket_cliente;
	socket_cliente = accept(socket_servidor, NULL, NULL);

    if(socket_cliente == -1){
        error_show("Error en la espera del cliente");
        exit(EXIT_FAILURE);
    } else{
        log_info(logger, "Se conecto un cliente!");
    }

	return socket_cliente;
}

void atender_clientes(int socket_servidor, t_log* logger,ProcesarRequestFunc procesar_request){
	while (true) {
		pthread_t thread;
		int *fd_conexion_ptr = malloc(sizeof(int));
		*fd_conexion_ptr = esperar_nueva_conexion_cliente(socket_servidor, logger);

		pthread_create(&thread,
						NULL,
						(void*) procesar_request,
						fd_conexion_ptr);
		pthread_detach(thread);
	}
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void handshake_server(int fd, t_log* logger){
    size_t bytes;

    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

    bytes = recv(fd, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (handshake == 1) {
		log_info(logger, "Handshake recibido");
        bytes = send(fd, &resultOk, sizeof(int32_t), 0);
    } else {
		log_error(logger, "Handshake Error");
        bytes = send(fd, &resultError, sizeof(int32_t), 0);
    }
}

void confirmar_recepcion(int socket){
    uint32_t ok = 1;
    send(socket, &ok, sizeof(uint32_t), 0);
}

bool recibir_ok(int socket){
	uint32_t ok;
	int codigo_error = recv(socket, &ok, sizeof(uint32_t), MSG_WAITALL);

	if(codigo_error == 0){
		return false;
	}

	return ok > 0;
}