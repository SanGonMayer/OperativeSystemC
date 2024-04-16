#include "cpu.h"

int main(int argc, char* argv[]) {
    
    logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
    
    config = config_create("../cpu/cpu.config");
    
    if(config == NULL){
        log_error(logger, "Mal el path");
        exit(EXIT_FAILURE);
    }

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    cantidad_entradas_tlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    algoritmo_tlb = config_get_string_value(config, "ALGORITMO_TLB");

    //Server Dispatch
    int server_dispatch_fd = iniciar_servidor(puerto_escucha_dispatch, logger, "CLIENTE DISPATCH");
    //Server Interrupt
    int server_interrupt_fd = iniciar_servidor(puerto_escucha_interrupt, logger, "CLIENTE INTERRUPT");
    //while para estar siempre esperando
    int cliente_dispatch_fd = esperar_cliente(server_dispatch_fd, logger);

    int cliente_interrupt_fd = esperar_cliente(server_interrupt_fd, logger);

    handshake_server(cliente_dispatch_fd, logger);
    
   

    // Modo servidor dispatch con Kernel

    int iterador = 1;

    while(iterador){
        int cod_op = recibir_operacion(cliente_dispatch_fd);
        
        switch (cod_op)
        {
        case 1:
            recibir_mensaje(cliente_dispatch_fd);
            break;
        case -1:
            error_show("cliente desconectado de CPU dispatch");
            close(server_dispatch_fd);
            close(cliente_dispatch_fd);
            iterador = 0;
            break;
        default:
            log_info(logger, "No entiendo el mensaje");
            iterador = 0;
            break;
        }
        
    }

    //Modo servidor interrupt kernel

    iterador =1;
    while (iterador){
        int cod_op = recibir_operacion(cliente_interrupt_fd);
        
        switch (cod_op)
        {
        case 1:
            recibir_mensaje(cliente_interrupt_fd);
            break;
        case -1:
            error_show("cliente desconectado de CPU interrupt");
            close(server_interrupt_fd);
            close(cliente_interrupt_fd);
            iterador = 0;
            break;
        default:
            log_info(logger, "No entiendo el mensaje");
            iterador = 0;
            break;
        }
    }

    //Modo cliente con Memoria
    int conexion_memoria_fd = crear_conexion(ip_memoria, puerto_memoria, "MEMORIA", logger);
    handshake_cliente(conexion_memoria_fd, logger);

    enviar_mensaje("Conexion CPU a MEMORIA, Mandando desde CPu", conexion_memoria_fd);
    close(conexion_memoria_fd);

    config_destroy(config);
    log_destroy(logger);

    return 0;
}
