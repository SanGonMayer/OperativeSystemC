#include "global_kernel.h"
#include "kernel.h"
#include "utils/buffer.h"
#include "utils/instrucciones_io.h"
#include "utils/server.h"
#include <commons/collections/dictionary.h>
#include <pthread.h>
#include <semaphore.h>
#include <threads.h>

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

void procesar_cola_interfaz(t_interfaz_conectada* interfaz){
    while(1){
        sem_wait(&interfaz->semaforo);
        t_parametro_cola_interfaz* instruccion = queue_pop(interfaz->cola);
        t_buffer* buffer = serializar_instruccion_io(instruccion->instruccion);
        enviar_buffer(interfaz->fd, buffer, g_logger);

        recibir_ok(interfaz->fd);
        // enviar proceso a ready
    }
}

void procesar_cliente(int* fd){

    handshake_server(*fd, g_logger);
    int iterador = 1;

    while (iterador) {
        int cod_op = recibir_operacion(*fd);
        switch (cod_op) 
        {
        case ENVIO_INTERFAZ_CONECTADA:
        
            t_buffer* buffer = recibir_buffer(*fd);
            t_interfaz* interfaz = deserializar_interfaz(buffer);
            t_interfaz_conectada* interfaz_conectada = malloc(sizeof(t_interfaz_conectada));

            *interfaz_conectada = (t_interfaz_conectada) {
                .fd = *fd,
                .cola = queue_create(),
                .interfaz = interfaz
            };
            
            sem_init(&interfaz_conectada->semaforo, 0, 0);

            dictionary_put(g_interfaces, interfaz->nombre, interfaz_conectada);

            pthread_t hilo;
            pthread_create(&hilo, NULL, (void*) &procesar_cola_interfaz, interfaz_conectada);
            pthread_detach(hilo);
            break;
        case -1:
            log_error(g_logger, "la interfaz se desconectó.");
            iterador = 0;
            break;
        default:
            log_warning(g_logger,"Operacion desconocida.");
            break;
        }
    }  
}

void proceso_io(){
    int socket_servidor = iniciar_servidor(puerto_escucha, g_logger, "CLIENTE KERNEL");
    atender_clientes(socket_servidor, g_logger, (void*) &procesar_cliente);
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
    sem_init(&g_mutex_cola_new, 0, 1);
    g_cola_ready = queue_create();
    sem_init(&g_mutex_cola_ready, 0,1);
    cola_exec = queue_create(); // No se usa
    sem_init(&g_disponible_exec, 0, 1);
    g_cola_exit = queue_create();
    sem_init(&g_mutex_cola_exit, 0, 1); 

    //Semaforos para io
    sem_init(&g_mutex_acceso_interfaces, 0, 1);

    //Inicializacion de config
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH" );
	puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT" );
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA" );
    ip_memoria = config_get_string_value(config, "IP_MEMORIA" );
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA" );

    g_grado_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    sem_init(&g_tope_multiprogramacion, 0, g_grado_multiprogramacion);


    algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    
    g_quantum = config_get_int_value(config, "QUANTUM");
    
    //INICIAR_PROCESO ../memoria/instrucciones.dummy
    // CONEXIONES DE PRUEBA -- BORRAR LUEGO
    g_conexion_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch, "CPU DISPATCH", g_logger);
    handshake_cliente(g_conexion_cpu_dispatch, g_logger);

    g_conexion_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt, "CPU INTERRUPT", g_logger);
    handshake_cliente(g_conexion_cpu_interrupt, g_logger);

    g_socket_memoria = crear_conexion(ip_memoria, puerto_memoria, "MEMORIA", g_logger);
    sem_init(&g_mutex_socket_memoria,0,1);
    handshake_cliente(g_socket_memoria, g_logger);
    
    //PLANIFICACION
    //Crear Hilo para realizar la ejecucion, que sea bloqueante para esperar respuesta. 
    pthread_t hilo_exit;
    sem_init(&g_hay_elementos_en_exit, 0, 0);
    hilo_exit = pthread_create(&hilo_exit, NULL, (void*)&planificador_exit, NULL);
    
    pthread_t hilo_planificador;
    sem_init(&g_hay_elementos_en_ready, 0, 0);

    if(strcmp(algoritmo_planificacion, "FIFO") == 0){
        hilo_planificador = pthread_create(&hilo_planificador, NULL, (void*)&planificador_fifo, NULL);
        pthread_detach(hilo_planificador);
    } else if(strcmp(algoritmo_planificacion, "RR") == 0){
        hilo_planificador = pthread_create(&hilo_planificador, NULL, (void*)&planificador_RR, NULL);
        pthread_detach(hilo_planificador);
    } else if(strcmp(algoritmo_planificacion, "VRR") == 0){
        //planificador_vrr()
    }

    // IO
    
    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void*)proceso_io, NULL);
    pthread_detach(hilo_io);

    // IO
    
    //CONSOLA INTERACTIVA
    sem_init(&mutex_contador_pid, 0, 1);

    // IO
    // int socket_servidor = iniciar_servidor(puerto_escucha, g_logger, "CLIENTE KERNEL");
    // atender_clientes(socket_servidor, g_logger, (void*) &procesar_cliente);

    g_interfaces = dictionary_create();

    consola_interactiva(g_logger);
    
    free(g_cola_new);
    free(g_cola_ready);
    free(cola_exec);
    close(g_conexion_cpu_dispatch);
    close(g_conexion_cpu_interrupt);
    close(g_socket_memoria);
    config_destroy(config);
    log_destroy(g_logger);
    return 0;
}


