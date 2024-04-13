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
    return 0;
}
