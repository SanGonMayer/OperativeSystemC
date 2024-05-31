#ifndef SERVER_H_
#define SERVER_H_

#include "utils/client.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <commons/log.h>
#include <commons/error.h>
#include <commons/collections/list.h>
#include <pthread.h>

typedef enum
{
    MENSAJE,
    PAQUETE
}op_code;

typedef void* (*ProcesarRequestFunc)(int* fd);

/**
* @fn    iniciar_servidor
* @brief inicia un nuevo servidor en modo escucha y devuelve un fd con el socket. Recibe como parametro el puerto de escucha.
*/
int iniciar_servidor(char* puerto, t_log* logger, char* aQuienEspera);
/**
* @fn    esperar_nueva_conexion_cliente
* @brief Devuelve el Socket del cliente. Recibe como paramentro el socket del servidor.
*/
int esperar_nueva_conexion_cliente(int socket_servidor, t_log* logger);

void atender_clientes(int socket_servidor, t_log* logger,ProcesarRequestFunc procesar_request);

int recibir_operacion(int socket_cliente);

void handshake_server(int fd, t_log* logger);

void confirmar_recepcion(int socket);

bool recibir_ok(int socket);
#endif