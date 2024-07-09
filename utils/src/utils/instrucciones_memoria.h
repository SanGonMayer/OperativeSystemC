#ifndef INSTRUCCIONES_MEMORIA_H_
#define INSTRUCCIONES_MEMORIA_H_

#include "utils/buffer.h"
#include "utils/codigo_operacion.h"
#include "utils/peticiones_memoria.h"
#include "utils/client.h"
#include "utils/server.h"
#include <commons/string.h>
#include <readline/readline.h>
#include <commons/collections/list.h>
#include <commons/log.h>

char* leer_de_memoria(int socket_memoria, int tamanio, t_list* peticionesMemoria, t_log* logger);

void guardar_en_memoria(int socket_memoria, char* texto, t_list* peticionesMemoria, t_log* g_logger);

void actualizar_peticiones_con_valor(t_list* peticionesMemoria, char* valor);

#endif