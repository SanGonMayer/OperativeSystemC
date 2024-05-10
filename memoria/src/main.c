#include <stdlib.h>
#include <stdio.h>
#include "memoria.h"
#include <commons/config.h>
#include <utils/server.h>

t_log* logger;
t_config* config;

char* puerto_escucha;
int tam_memoria;
int tam_pagina;
char* path_instrucciones;
int retardo_respuesta;

void procesar_cliente(int* socket){

    handshake_server(*socket, logger);
    int iterador = 1;

    while (iterador) {
        int cod_op = recibir_operacion(*socket);

        switch (cod_op) 
        {
        case 1:
            recibir_mensaje(*socket);
            break;
        case 2: //Recibir contexto de kernel
            uint32_t posicionDeCodigo = 0;
            t_paqueteMemoria* paqueteMemoria = inicializar_paquete_memoria();
            recibir_contexto_de_kernel(*socket, paqueteMemoria);
            //logica para conseguir la posicionDeCodigo
            enviar_posicion_de_codigo(*socket, posicionDeCodigo);
            free(paqueteMemoria);
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
}

int main(int argc, char* argv[]) {

    logger = log_create("memoria.log", "MEMORIA", 1, LOG_LEVEL_DEBUG);

    config = config_create("../memoria/memoria.config");

    if(config == NULL){
        log_error(logger, "Error al crear la configuracion");
        exit(EXIT_FAILURE);
    }
    
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    tam_pagina = config_get_int_value(config, "TAM_PAGINA");
    path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
    retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");

    int socket_escucha = iniciar_servidor(puerto_escucha, logger, "CLIENTE");
    
    atender_clientes(socket_escucha, logger, &procesar_cliente);

    log_destroy(logger);
    config_destroy(config);

    return 0;
}

