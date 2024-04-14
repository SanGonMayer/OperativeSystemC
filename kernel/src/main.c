#include <stdlib.h>
#include <stdio.h>
#include "kernel.h"
#include<unistd.h>

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

    config = config_create("../kernel/kernel.config");

    if(config == NULL){
        log_error(logger, "Mal el path");
        exit(EXIT_FAILURE);
    }

    ip_cpu = config_get_string_value(config, "IP_CPU");

    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH" );
	puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT" );

    int conexion_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch, "CPU");
    int conexion_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt, "CPU");

    enviar_mensaje("todo bien con el dispactch", conexion_cpu_dispatch);
    //enviar_mensaje("todo bien con el interrupt", conexion_cpu_interrupt);

    config_destroy(config);
	close(conexion_cpu_dispatch);
    //close(conexion_cpu_interrupt);



    return 0;
}


