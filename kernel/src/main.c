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

    ip_cpu = config_get_string_value(config, "IP_CPU");

    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH" );
	puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT" );

    int conexion_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch, "CPU", logger);
    int conexion_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt, "CPU", logger);

    enviar_mensaje("todo bien con el dispactch", conexion_cpu_dispatch);
    //enviar_mensaje("todo bien con el interrupt", conexion_cpu_interrupt);

    config_destroy(config);
    log_destroy(logger);
	close(conexion_cpu_dispatch);
    //close(conexion_cpu_interrupt);

    return 0;
}


