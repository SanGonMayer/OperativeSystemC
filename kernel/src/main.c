#include "kernel.h"

int puerto_escucha;
char * ip_memoria;
char * puerto_memoria;
char * ip_cpu;
char * puerto_cpu_dispatch;
char * puerto_cpu_interrupt;
char *algoritmo_planificacion;

t_config* config;
t_log* logger;

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
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA" );
    ip_memoria = config_get_string_value(config, "IP_MEMORIA" );
    int conexion_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch, "CPU DISPATCH", logger);
    int conexion_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt, "CPU INTERRUPT", logger);

    handshake_cliente(conexion_cpu_dispatch, logger);

    enviar_mensaje("Mensaje KERNEL a CPU DISPATCH", conexion_cpu_dispatch);
    close(conexion_cpu_dispatch);

    enviar_mensaje("Mensaje KERNEL a CPU INTERRUPT", conexion_cpu_interrupt);
    close(conexion_cpu_interrupt);

    int conexion_memoria_fd = crear_conexion(ip_memoria, puerto_memoria, "MEMORIA", logger);

    enviar_mensaje("Mensaje KERNEL a MEMORIA", conexion_memoria_fd);
    close(conexion_memoria_fd);

    config_destroy(config);
    log_destroy(logger);
	
    
    return 0;
}


