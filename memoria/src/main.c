#include <commons/collections/dictionary.h>
#include <stdlib.h>
#include <stdio.h>
#include "memoria.h"
#include "utils/codigo_operacion.h"
#include "utils/instrucciones.h"
#include <commons/config.h>
#include <utils/server.h>

t_log* logger;
t_config* config;

char* puerto_escucha;
int tam_memoria;
int tam_pagina;
char* path_instrucciones;
int retardo_respuesta;
t_dictionary* memoria_instrucciones;

uint32_t* posicionFinalEscrita = 0;

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
            
        case ENVIO_PATH_INSTRUCCIONES: //Recibir contexto de kernel
            
            t_paqueteMemoria* paqueteMemoria = inicializar_paquete_memoria();
            
            recibir_contexto_de_kernel(*socket, paqueteMemoria, logger);
            
            cargar_instrucciones(memoria_instrucciones, paqueteMemoria->PID, paqueteMemoria->path);
            
            free(paqueteMemoria);
            break;

        case ENVIO_PID_PC: //Recibir posicion de codigo de CPU
            
            t_paquete_instruccion* pedido_instruccion = recibir_instruccion(*socket, logger);

            char* instruccion = leer_instruccion(memoria_instrucciones, pedido_instruccion->pid, &pedido_instruccion->pc);

            enviar_instruccion(*socket, instruccion, logger);
            free(instruccion);
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

    memoria_instrucciones = dictionary_create();

    cargar_instrucciones(memoria_instrucciones, 1, "../memoria/instrucciones.dummy");
    int* pc = malloc(sizeof(int));
    *pc = 1;
    char* instruccion1 = leer_instruccion(memoria_instrucciones, 1, pc);

    log_info(logger, "Linea %d - Instruccion: %s", *pc - 1, instruccion1);

    *pc = 4;

    char* instruccion4 = leer_instruccion(memoria_instrucciones, 1, pc);

    log_info(logger, "Linea %d - Instruccion: %s", *pc - 1, instruccion4);


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
    
    atender_clientes(socket_escucha, logger, (void*)&procesar_cliente);

    log_destroy(logger);
    config_destroy(config);

    return 0;
}

