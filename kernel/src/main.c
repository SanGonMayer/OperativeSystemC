#include "cola_exit.h"
#include "consola_interactiva.h"
#include "global_kernel.h"
#include "kernel.h"
#include "recursos.h"
#include "utils/buffer.h"
#include "utils/instrucciones_io.h"
#include "utils/server.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <pthread.h>
#include <semaphore.h>
#include <threads.h>

char * puerto_escucha;
char * ip_memoria;
char * puerto_memoria;
char * ip_cpu;
char * puerto_cpu_dispatch;
char * puerto_cpu_interrupt;
char ** recursos;
char ** recursos_instancias;

int quantum;

t_config* config;
t_queue* cola_exec;

void procesar_cliente(int* fd){

    handshake_server(*fd, g_logger);

    int cod_op = recibir_operacion(*fd);

    if(cod_op != ENVIO_INTERFAZ_CONECTADA){
        log_error(g_logger, "Se esperaba un primer mensaje de tipo ENVIO_INTERFAZ_CONECTADA. Se recibió otro tipo de mensaje.");
        return;
    }
    t_buffer* buffer = recibir_buffer(*fd);
    t_interfaz* interfaz = deserializar_interfaz(buffer);
    t_interfaz_conectada* interfaz_conectada = malloc(sizeof(t_interfaz_conectada));

    *interfaz_conectada = (t_interfaz_conectada) {
        .fd = *fd,
        .cola = queue_create(),
        .interfaz = interfaz
    };
    
    sem_init(&interfaz_conectada->semaforo, 0, 0);
    sem_init(&interfaz_conectada->mutex, 0, 1);
    

    dictionary_put(g_interfaces, interfaz->nombre, interfaz_conectada);

    while(1){
        sem_wait(&interfaz_conectada->semaforo);

        
        sem_wait(&interfaz_conectada->mutex);
        if(queue_is_empty(interfaz_conectada->cola)){
            sem_post(&interfaz_conectada->mutex);
            continue;
        }
        t_parametro_cola_interfaz* instruccion = queue_pop(interfaz_conectada->cola);
        sem_post(&interfaz_conectada->mutex);
        
        t_buffer* buffer = serializar_instruccion_io(instruccion->instruccion);
 
        enviar_buffer(interfaz_conectada->fd, buffer, g_logger);

        bool result = recibir_ok(interfaz_conectada->fd);

        //eliminar_de_lista_blocked_gral(instruccion->pcb->PID);

        if(result){
            log_info(g_logger, "Se ejecuto correctamente la instrucción enviada a la interfaz %s", interfaz->nombre);
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(instruccion->pcb->readyplus == 1){
                    agregar_a_cola_auxiliar(instruccion->pcb);
                    sem_post(&g_hay_elementos_para_ejecutar);
                }
            } else {
                enviar_proceso_a_ready(instruccion->pcb);
            }
            
        }else{
            uint32_t error = recibir_codigo_error(interfaz_conectada->fd);
            log_error(g_logger, "No se pudo ejecutar la instrucción enviada a la interfaz %s. Error: %d", interfaz->nombre, error);
            finalizar_proceso(instruccion->pcb);
        }
    }
}

void proceso_io(){
    int socket_servidor = iniciar_servidor(puerto_escucha, g_logger, "CLIENTE KERNEL");
    atender_clientes(socket_servidor, g_logger, (void*) &procesar_cliente);
}

int main(void){
	
    //semaforos para pausar o reanudar planificadores
    sem_init(&g_notif_corto_plazo,0,1); 
    sem_init(&g_notif_largo_plazo,0,1);
    sem_init(&g_mutex_cola_signal, 0, 1);
    g_cola_signal = queue_create();
    g_lista_blocked_gral = list_create();
    sem_init(&g_mutex_lista_blocked_gral, 0, 1);

    g_lista_procesos_gral = list_create();
    sem_init(&g_mutex_lista_procesos_gral, 0, 1);

    //VRR
    
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

    init_cola_exit();

    g_cola_auxiliar = queue_create();
    sem_init(&g_mutex_cola_auxiliar, 0, 1);


    //Semaforos para io
    sem_init(&g_mutex_acceso_interfaces, 0, 1);

    //Inicializacion de config
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH" );
	puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT" );
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA" );
    ip_memoria = config_get_string_value(config, "IP_MEMORIA" );
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA" );
    g_recursos = config_get_array_value(config, "RECURSOS");
    g_recursos_instancias = config_get_array_value(config, "INSTANCIAS_RECURSOS");

    init_recursos(g_recursos, g_recursos_instancias); //carga en la variable global g_diccionario_recursos un diccionarios con los recursos y sus instancias

    g_grado_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    g_grado_multiprogramacion_inicial = g_grado_multiprogramacion;
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
    
    
    crear_hilo_cola_exit();
    
    pthread_t hilo_planificador;
    sem_init(&g_hay_elementos_en_ready, 0, 0);

    if(string_equals_ignore_case(algoritmo_planificacion, "FIFO")){
        hilo_planificador = pthread_create(&hilo_planificador, NULL, (void*)&planificador_fifo, NULL);
        pthread_detach(hilo_planificador);
    }
    else if(string_equals_ignore_case(algoritmo_planificacion, "RR")){
        hilo_planificador = pthread_create(&hilo_planificador, NULL, (void*)&planificador_RR, NULL);
        pthread_detach(hilo_planificador);
    } 
    else if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
        sem_init(&g_tiempo_calculado, 0,0);
        sem_init(&g_hay_elementos_para_ejecutar, 0, 0);
        hilo_planificador = pthread_create(&hilo_planificador, NULL, (void*)&planificador_VRR, NULL);
        pthread_detach(hilo_planificador);
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

    consola_interactiva();
    
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


