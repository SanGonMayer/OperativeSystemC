#include "client.h"
#include "utils/codigo_operacion.h"
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>


int crear_conexion(char *ip, char* puerto, char* nombreServer, t_log* logger)
{
	int err;
	
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	err = getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = 0;

	socket_cliente = socket(server_info->ai_family, 
							server_info->ai_socktype,
							server_info -> ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	err = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

    if(err == -1){
        error_show("Error al conectar con el servidor");
        printf("\n");
        exit(EXIT_FAILURE);
    } else{
        log_info (logger, "Conexion exitosa a: %s", nombreServer);
    }

	freeaddrinfo(server_info);

	return socket_cliente;
}

void handshake_cliente(int socket_conexion, t_log* logger){
	size_t bytes;

	int32_t handshake = 1;
	int32_t result;

	bytes = send(socket_conexion, &handshake, sizeof(int32_t), 0);
	bytes = recv(socket_conexion, &result, sizeof(int32_t), MSG_WAITALL);

	if (result == 0) {
		log_info(logger, "Handshake OK");
		return;
	} else {
		log_error(logger, "Handshake Error");
		error_show("Error al establecer la comunicacion");
		printf("\n"); 
		exit(EXIT_FAILURE);
	}
}


void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = HANDSHAKE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	void* a_enviar = serializar_paquete(paquete);

	send(socket_cliente, a_enviar, paquete->buffer->size + sizeof(int) + sizeof(uint32_t), 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void* serializar_paquete(t_paquete* paquete)
{
	void * magic = malloc(paquete->buffer->size + sizeof(int) + sizeof(uint32_t));
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento+= sizeof(uint32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = 0;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

int serializar_y_enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	void* a_enviar = serializar_paquete(paquete);

	int result = send(socket_cliente, a_enviar, paquete->buffer->size + sizeof(int) + sizeof(uint32_t), 0);
	
	if (result ==-1){
		perror("Error en la funcion al enviar Paquete \n");
	}

	free(a_enviar);

	return result;
}

void responder_ok(int socket){
	uint32_t ok = 1;

	send(socket, &ok, sizeof(uint32_t), 0);
}


