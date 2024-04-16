#include "kernel.h"

char * puerto_escucha;
char * ip_memoria;
char * puerto_memoria;
char * ip_cpu;
char * puerto_cpu_dispatch;
char * puerto_cpu_interrupt;
char *algoritmo_planificacion;

t_config* config;
t_log* logger;

void procesarPeticiones(int* fd){
    log_info(logger, "Testing hilos");
    handshake_server(*fd, logger);

    int cod_op = recibir_operacion(*fd);
    recibir_mensaje_logger(*fd, logger);
    close(*fd);
    free(fd);
}

int main(void){
	logger = log_create("kernel.log", "server_connection", 1, LOG_LEVEL_INFO);
    config = config_create("../kernel/kernel.config");

    if(config == NULL){
        log_error(logger, "Mal el path");
        exit(EXIT_FAILURE);
    }

    //Inicializacion de config
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH" );
	puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT" );
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    // int conexion_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch, "CPU", logger);
    // int conexion_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt, "CPU", logger);

    // handshake_cliente(conexion_cpu_dispatch, logger);

    // enviar_mensaje("primer mensaje enviado de cliente kernel a cpu dispatch", conexion_cpu_dispatch);
    // close(conexion_cpu_dispatch);

    // enviar_mensaje("primer mensaje enviado de cliente kernel a cpu interrupt", conexion_cpu_interrupt);
    // close(conexion_cpu_interrupt);

	
    
    int socket_servidor = iniciar_servidor(puerto_escucha, logger, "CLIENTE KERNEL");

    atender_clientes(socket_servidor, logger, &procesarPeticiones);

    config_destroy(config);
    log_destroy(logger);
    return 0;
}


