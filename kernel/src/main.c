#include "kernel.h"

char * puerto_escucha;
char * ip_memoria;
char * puerto_memoria;
char * ip_cpu;
char * puerto_cpu_dispatch;
char * puerto_cpu_interrupt;
char * algoritmo_planificacion;
int grado_multiprogramacion;
int quantum;

t_config* config;
t_log* logger;
t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_exec;

void procesar_cliente(int* fd){

    handshake_server(*fd, logger);
    int iterador = 1;

    while (iterador) {
        int cod_op = recibir_operacion(*fd);

        switch (cod_op) 
        {
        case 1:
            recibir_mensaje(*fd);
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

int main(void){
	
    logger = log_create("kernel.log", "server_connection", 1, LOG_LEVEL_INFO);
    config = config_create("../kernel/kernel.config");

    if(config == NULL){
        log_error(logger, "Mal el path");
        exit(EXIT_FAILURE);
    }

    //colas de planificacion
    cola_new = queue_create();
    cola_ready = queue_create();
    cola_exec = queue_create();

    //Inicializacion de config
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH" );
	puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT" );
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA" );
    ip_memoria = config_get_string_value(config, "IP_MEMORIA" );
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA" );

    grado_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");

    algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    
    int algoritmo_planificacion_enum;
    
    if(strcmp(algoritmo_planificacion, "FIFO") == 0){
        algoritmo_planificacion_enum == FIFO;
    } else if(strcmp(algoritmo_planificacion, "RR") == 0){
        algoritmo_planificacion_enum == RR;
    } else if(strcmp(algoritmo_planificacion, "VRR") == 0){
        algoritmo_planificacion_enum == VRR;
    }
    
    quantum = config_get_int_value(config, "QUANTUM");

    int conexion_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch, "CPU DISPATCH", logger);
    handshake_cliente(conexion_cpu_dispatch, logger);
    enviar_mensaje("Mensaje KERNEL a CPU DISPATCH", conexion_cpu_dispatch);

    int conexion_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt, "CPU INTERRUPT", logger);
    handshake_cliente(conexion_cpu_interrupt, logger);
    enviar_mensaje("Mensaje KERNEL a CPU INTERRUPT", conexion_cpu_interrupt);

    int conexion_memoria_fd = crear_conexion(ip_memoria, puerto_memoria, "MEMORIA", logger);
    handshake_cliente(conexion_memoria_fd, logger);
    enviar_mensaje("Mensaje KERNEL a MEMORIA", conexion_memoria_fd);

    int socket_servidor = iniciar_servidor(puerto_escucha, logger, "CLIENTE KERNEL");

    //Consola interactiva
    pthread_t hilo_consola_interactiva;
    consola_interactiva(logger);
    
    if (pthread_create(&hilo_consola_interactiva, NULL, consola_interactiva, (void*)logger) != 0)
        log_error(logger, "error al crear el hilo de la cosola interactiva");
    
    if (pthread_join(hilo_consola_interactiva, NULL))
        log_error(logger, "error con el join del hilo de la consola interactiva");

    //Pedido por consola
    iniciar_proceso("path", cola_new);
    //Si hay lugar lo mete en cola ready
    if(grado_multiprogramacion>0){
        enviar_proceso_a_ready(cola_new, cola_ready);
        grado_multiprogramacion --;
    }
    log_info(logger, "grado multiprogramacion = %d", grado_multiprogramacion);
    t_PCB* pcb = queue_pop(cola_ready);
    ejecutar_cpu_FIFO(pcb, conexion_cpu_dispatch, logger);

    //Crear Hilo para realizar la ejecucion, que sea bloqueante para esperar respuesta.
    while(!queue_is_empty(cola_ready)){
        t_PCB* pcb = queue_pop(cola_ready);
        if(queue_is_empty(cola_exec)){
            queue_push(cola_exec, pcb);
            switch (algoritmo_planificacion_enum)
            {
            case FIFO:
                //va a tener conexion cliente servidor, es bloqueante, espera recibir el PCB
                ejecutar_cpu_FIFO(pcb, conexion_cpu_dispatch, logger);
                //pcb ya esta actualizado

                //Crea Hilo para manejar el desalojo, mientras tanto sigue ejecutando para liberar cola_exec y usar la CPU.
                //manejar_desalojo(pcb);
                
                //libera cola_exec para volver a entrar al switch.
                //queue_pop(cola_exec);
                break;
            case RR:
                break;
            case VRR:
                break;
            default:
                break;
            }
        }
    }

    free(pcb);

    /*
    for(int i = 0; i <= queue_size(cola_new); i++){
        t_PCB* pcb = queue_pop(cola_new);
        log_info(logger, "%d", pcb->PID);
    }
    */
    atender_clientes(socket_servidor, logger, &procesar_cliente);

    close(conexion_cpu_dispatch);
    close(conexion_cpu_interrupt);
    close(conexion_memoria_fd);
    config_destroy(config);
    log_destroy(logger);
    return 0;
}


