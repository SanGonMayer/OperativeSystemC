#ifndef IO_MEMORIA_H_
#define IO_MEMORIA_H_ 
#include "global_io.h"
#include "utils/buffer.h"
#include "utils/codigo_operacion.h"
#include "utils/peticiones_memoria.h"
#include "utils/client.h"
#include "utils/server.h"
#include <commons/string.h>
#include <readline/readline.h>
#include <commons/collections/list.h>

char* leer_de_memoria(int socket_memoria,int tamanio, t_list* peticionesMemoria);
void guardar_en_memoria(int socket_memoria, char* texto, t_list* peticionesMemoria);

#endif