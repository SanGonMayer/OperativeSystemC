#include "global_kernel.h"
#include "kernel.h"
#include <semaphore.h>

char * puerto_escucha;
char * ip_memoria;
char * puerto_memoria;
char * ip_cpu;
char * puerto_cpu_dispatch;
char * puerto_cpu_interrupt;
char * algoritmo_planificacion;
int quantum;

t_config* config;
t_queue* cola_exec;

void procesar_cliente(int* fd){

    handshake_server(*fd, g_logger);
    int iterador = 1;

    while (iterador) {
        int cod_op = recibir_operacion(*fd);
        switch (cod_op) 
        {
        case 1:
            recibir_mensaje(*fd);
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

int main(void){
	

    g_logger = log_create("kernel.log", "server_connection", 1, LOG_LEVEL_INFO);
    
    config = config_create("../kernel/kernel.config");

    if(config == NULL){
        log_error(g_logger, "Mal el path");
        exit(EXIT_FAILURE);
    }

    //colas de planificacion
    g_cola_new = queue_create();
    g_cola_ready = queue_create();
    cola_exec = queue_create();

    //Inicializacion de config
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH" );
	puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT" );
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA" );
    ip_memoria = config_get_string_value(config, "IP_MEMORIA" );
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA" );

    g_grado_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    sem_init(&g_mutex_multiprogramacion, 0, 1);


    algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    
    quantum = config_get_int_value(config, "QUANTUM");

    sem_init(&g_actualizacion_pcb, 1, 0);


    // CONEXIONES DE PRUEBA -- BORRAR LUEGO
    int conexion_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch, "CPU DISPATCH", g_logger);
    handshake_cliente(conexion_cpu_dispatch, g_logger);
    enviar_mensaje("Mensaje KERNEL a CPU DISPATCH", conexion_cpu_dispatch);

    int conexion_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt, "CPU INTERRUPT", g_logger);
    handshake_cliente(conexion_cpu_interrupt, g_logger);
    enviar_mensaje("Mensaje KERNEL a CPU INTERRUPT", conexion_cpu_interrupt);

    g_socket_memoria = crear_conexion(ip_memoria, puerto_memoria, "MEMORIA", g_logger);
    handshake_cliente(g_socket_memoria, g_logger);
    enviar_mensaje("Mensaje KERNEL a MEMORIA", g_socket_memoria);
    // CONEXIONES DE PRUEBA -- BORRAR LUEGO


    //PLANIFICACION
    //Crear Hilo para realizar la ejecucion, que sea bloqueante para esperar respuesta.
    // int algoritmo_planificacion_enum;
    
    // if(strcmp(algoritmo_planificacion, "FIFO") == 0){
    //     algoritmo_planificacion_enum == FIFO;
    // } else if(strcmp(algoritmo_planificacion, "RR") == 0){
    //     algoritmo_planificacion_enum == RR;
    // } else if(strcmp(algoritmo_planificacion, "VRR") == 0){
    //     algoritmo_planificacion_enum == VRR;
    // }
    
    // while(!queue_is_empty(cola_ready)){
    //     t_PCB* pcb = queue_pop(cola_ready);
    //     if(queue_is_empty(cola_exec)){
    //         queue_push(cola_exec, pcb);
    //         switch (algoritmo_planificacion_enum)
    //         {
    //         case FIFO:
    //             //va a tener conexion cliente servidor, es bloqueante, espera recibir el PCB
    //             ejecutar_cpu_FIFO(pcb, conexion_cpu_dispatch, g_logger);
    //             //pcb ya esta actualizado

    //             //Crea Hilo para manejar el desalojo, mientras tanto sigue ejecutando para liberar cola_exec y usar la CPU.
    //             //manejar_desalojo(pcb);
                
    //             //libera cola_exec para volver a entrar al switch.
    //             //queue_pop(cola_exec);
    //             break;
    //         case RR:
    //             break;
    //         case VRR:
    //             break;
    //         default:
    //             break;
    //         }
    //     }
    // }
    //PLANIFICACION

    //HILOS?
    /*if(strcmp(algoritmo_planificacion, "FIFO") == 0){
        pthread_create(&hilo_planificador, NULL, (void*)planificar_FIFO, NULL);

    }else if(strcmp(algoritmo_planificacion, "RR") == 0){
        pthread_create(&hilo_planificador, NULL, (void*)planificar_RR, NULL);

    }else if(strcmp(algoritmo_planificacion, "VRR") == 0){
        pthread_create(&hilo_planificador, NULL, (void*)planificar_VRR, NULL);
    }*/
    
    
    
    //CONSOLA INTERACTIVA

    consola_interactiva(g_logger);

    // pthread_t hilo_consola_interactiva;

    // if (pthread_create(&hilo_consola_interactiva, NULL, (void*)consola_interactiva, (void*)g_logger) != 0){
    //     log_error(g_logger, "error al crear el hilo de la cosola interactiva");
    // }
    // pthread_detach(hilo_consola_interactiva);
    //CONSOLA INTERACTIVA

    // IO
    int socket_servidor = iniciar_servidor(puerto_escucha, g_logger, "CLIENTE KERNEL");
    atender_clientes(socket_servidor, g_logger, (void*) &procesar_cliente);
    // IO


    
    free(g_cola_new);
    free(g_cola_ready);
    free(cola_exec);
    close(conexion_cpu_dispatch);
    close(conexion_cpu_interrupt);
    close(g_socket_memoria);
    config_destroy(config);
    log_destroy(g_logger);
    return 0;
}


