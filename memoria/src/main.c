#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <memoria.h>


int main(int argc, char* argv[]) {
    decir_hola("Memoria");

    t_log* logger = log_create("memoria.log", "MEMORIA", 1, LOG_LEVEL_DEBUG);


    t_config* config = config_create("../memoria/memoria.config");

    if(config == NULL){
        log_error(logger, "Error al crear la configuracion");
        exit(EXIT_FAILURE);
    }
    
    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    int tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    int tam_pagina = config_get_int_value(config, "TAM_PAGINA");
    char* path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
    int retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");
    

    int server_fd = iniciar_servidor(puerto_escucha, logger, "CLIENTE");
    
    int cliente_fd = esperar_cliente(server_fd, logger);

    //handshake_server(cliente_fd, logger);

    t_list* lista;
    int iterador = 1;

    while (iterador) {
        int cod_op = recibir_operacion(cliente_fd);

        switch (cod_op) 
        {
        case 1:
            recibir_mensaje(cliente_fd);
            break;
        case -1:
            log_error(logger, "el cliente se desconectó.");
            iterador = 0;
            break;
        default:
            log_warning(logger,"Operacion desconocida.");
            break;
        }
    }

    iterador = 1;

    cliente_fd = esperar_cliente(server_fd, logger);

    while (iterador) {
        int cod_op = recibir_operacion(cliente_fd);

        switch (cod_op) 
        {
        case 1:
            recibir_mensaje(cliente_fd);
            break;
        case -1:
            log_error(logger, "el cliente se desconectó.");
            iterador = 0;
            break;
        default:
            log_warning(logger,"Operacion desconocida.");
            break;
        }
    }   

    close(server_fd);
    close(cliente_fd);
    log_destroy(logger);
    config_destroy(config);

    return 0;
}

