#ifndef IO_H_
#define IO_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <utils/client.h>
#include <unistd.h>

typedef struct {
    int tiempo_unidad_trabajo;
    char* puerto_kernel;
    int puerto_memoria;
    int block_size;
    int block_count;
    char* tipo_interfaz;
    char* ip_kernel;
    char* ip_memoria;
    char* path_base_dialfs;
} ConfiguracionIO;

ConfiguracionIO* leer_configuracion(t_log* logger, t_config* config);

#endif