
#include "global_io.h"
#include <commons/config.h>
#include <stdlib.h>



ConfiguracionIO* leer_configuracion(t_config* config){

    if(config == NULL){
        log_error(g_logger, "No se pudo cargar el archivo de configuración");
        exit(EXIT_FAILURE);
    }

    g_config_io = malloc(sizeof(ConfiguracionIO));

    g_config_io->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    g_config_io->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    g_config_io->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    g_config_io->block_size = config_get_int_value(config, "BLOCK_SIZE");
    g_config_io->block_count = config_get_int_value(config, "BLOCK_COUNT");
    g_config_io->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    g_config_io->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    g_config_io->tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    g_config_io->path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    
    return g_config_io;
}

void configuracionIO_destroy(ConfiguracionIO* config){

    free(config->ip_kernel);
    free(config->ip_memoria);
    free(config->puerto_kernel);
    free(config->tipo_interfaz);
    free(config->path_base_dialfs);
    free(config);
}