#include "server.h"

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
		*fd_conexion_ptr = accept(socket_servidor, NULL, NULL);
		if(*fd_conexion_ptr == -1){
			error_show("Error en la espera del cliente");
			exit(EXIT_FAILURE);
		} else{
			log_info(logger, "Se conecto un cliente!");
		}
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
void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void handshake_server(int socket_cliente, t_log* logger){
    size_t bytes;

    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

    bytes = recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (handshake == 1) {
		log_info(logger, "Handshake recibido");
        bytes = send(socket_cliente, &resultOk, sizeof(int32_t), 0);
    } else {
		log_error(logger, "Handshake Error");
        bytes = send(socket_cliente, &resultError, sizeof(int32_t), 0);
    }
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);

	t_log *logger = log_create("cpu.log", "messagge", 1, LOG_LEVEL_INFO);
	log_info(logger, "Me llego el mensaje %s", buffer);
	log_destroy(logger);

	free(buffer);
}

void recibir_mensaje_logger(int socket_cliente, t_log* logger){
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);

	log_info(logger, "Me llego el mensaje %s", buffer);

	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
    int size;
    int desplazamiento = 0;
    void * buffer;
    t_list* valores = list_create();
    int tamanio;

    buffer = recibir_buffer(&size, socket_cliente);
    while(desplazamiento < size)
    {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento+=sizeof(int);
        char* valor = malloc(tamanio);
        memcpy(valor, buffer+desplazamiento, tamanio);
        desplazamiento+=tamanio;
        list_add(valores, valor);
    }
    free(buffer);
    return valores;
}