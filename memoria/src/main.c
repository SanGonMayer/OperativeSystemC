#include <commons/collections/dictionary.h>
#include "config.h"
#include "global_memoria.h"
#include "memoria.h"
#include "memoria_instrucciones.h"
#include "utils/codigo_operacion.h"
#include <commons/config.h>
#include <utils/server.h>


uint32_t* posicionFinalEscrita = 0;

void retardo_respuesta_memoria(){
    int retardo_respuesta = get_config_memoria()->retardo_respuesta;
    sleep(retardo_respuesta / 1000);
}

void procesar_cliente(int* socket){

    handshake_server(*socket, g_logger);
    int iterador = 1;

    while (iterador) {
        int cod_op = recibir_operacion(*socket);
        log_info(g_logger, "Operacion recibida: %d", cod_op);
        switch (cod_op) 
        {
        case ENVIO_PATH_INSTRUCCIONES: //Recibir contexto de kernel
            retardo_respuesta_memoria();
            procesar_carga_instrucciones(*socket);
            break;

        case ENVIO_PID_PC: //Recibir posicion de codigo de CPU

            log_info(g_logger, "Recibiendo pedido de instruccion PID PC");
            retardo_respuesta_memoria();
            procesar_pedido_instruccion(*socket);
            break;
        case OBTENER_MARCO_MEMORIA:
            log_info(g_logger, "Recibiendo pedido de marco de memoria");
            retardo_respuesta_memoria();
            procesar_pedido_marco(*socket);
            break;
        case RESIZE_MEMORIA:
            log_info(g_logger, "Recibiendo pedido de resize de memoria");
            retardo_respuesta_memoria();
            procesar_resize_memoria(*socket);
            break;
        case ACCEDER_ESPACIO_DE_USUARIO_MEMORIA:
            log_info(g_logger, "Recibiendo pedido de acceso a espacio de usuario");
            retardo_respuesta_memoria();
            procesar_acceso_espacio_usuario(*socket);
            break;
        case FINALIZAR_PROCESO_MEMORIA:
            log_info(g_logger, "Recibiendo pedido de finalizacion de proceso");
            retardo_respuesta_memoria();
            procesar_finalizacion_proceso(*socket);
            break;
        case OBTENER_TAMANIO_PAGINA:
            t_buffer* buffer = buffer_create(sizeof(uint32_t));
            buffer_add_uint32(buffer, get_config_memoria()->tam_pagina);
            enviar_buffer(*socket, buffer, g_logger);
            buffer_destroy(buffer);
            break;
        case -1:
            log_error(g_logger, "el cliente se desconectó.");
            iterador = 0;
            break;
        default:
            log_warning(g_logger,"Operacion desconocida.");
            break;
        }
    }  
}

int main(int argc, char* argv[]) {


    g_logger = log_create("memoria.log", "MEMORIA", 1, LOG_LEVEL_DEBUG);
    
    t_config_memoria* config = get_config_memoria();

    inicializar_memoria(config->tam_memoria, config->tam_pagina);

    int socket_escucha = iniciar_servidor(config->puerto_escucha, g_logger, "CLIENTE");
    
    atender_clientes(socket_escucha, g_logger, (void*)&procesar_cliente);

    log_destroy(g_logger);
    config_memoria_destroy();
    return 0;
}

