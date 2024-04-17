#include "io.h"


ConfiguracionIO* leer_configuracion(t_log* logger, t_config* config){

    if(config == NULL){
        log_error(logger, "No se pudo cargar el archivo de configuración");
        exit(EXIT_FAILURE);
    }

    ConfiguracionIO* config_io = malloc(sizeof(ConfiguracionIO));

    config_io->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    config_io->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    config_io->puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    config_io->block_size = config_get_int_value(config, "BLOCK_SIZE");
    config_io->block_count = config_get_int_value(config, "BLOCK_COUNT");
    config_io->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    config_io->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    config_io->tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    config_io->path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    
    return config_io;
}

void configuracionIO_destroy(ConfiguracionIO* config){
    
    free(config->ip_kernel);
    free(config->ip_memoria);
    free(config->puerto_kernel);
    free(config->tipo_interfaz);
    free(config->path_base_dialfs);
    free(config);
}