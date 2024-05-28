#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <commons/log.h>
#include <string.h>
#include "buffer.h"


typedef struct
{
	int codigo_operacion;
	t_buffer* buffer;
} t_paquete;

int crear_conexion(char *ip, char* puerto, char* nombreServer, t_log* logger);

void handshake_cliente(int socket_conexion, t_log* logger);

void enviar_mensaje(char* mensaje, int socket_cliente);

void paquete(int conexion);

void crear_buffer(t_paquete* paquete);

t_paquete* crear_paquete(void);

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);

int serializar_y_enviar_paquete(t_paquete* paquete, int socket_cliente);

void eliminar_paquete(t_paquete* paquete);

void* serializar_paquete(t_paquete* paquete);

void responder_ok(int socket);
#endif