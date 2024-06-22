#ifndef GLOBAL_KERNEL_H_
#define GLOBAL_KERNEL_H_

#include <commons/collections/queue.h>
#include <commons/log.h>
#include <semaphore.h>
#include <utils/procesos.h>
#include <commons/temporal.h>

extern int g_contador_pid;

extern t_queue* g_cola_new;
extern t_queue* g_cola_ready;
extern t_queue* g_cola_exit;
extern t_queue* g_cola_auxiliar;

extern t_log* g_logger;
extern int g_grado_multiprogramacion;
extern int g_socket_memoria;
extern sem_t g_mutex_multiprogramacion;
extern int g_conexion_cpu_dispatch;
extern int g_quantum;
extern t_PCB* g_exec;
extern int g_conexion_cpu_interrupt;
extern char * algoritmo_planificacion;

//Semaforos para corto plazo
extern sem_t g_hay_elementos_en_ready;
extern sem_t g_mutex_cola_ready;
extern sem_t g_disponible_exec;
//Semaforos largo plazo
extern sem_t g_tope_multiprogramacion;
extern sem_t mutex_contador_pid;
extern sem_t g_mutex_cola_new;
extern sem_t g_mutex_socket_memoria;
extern sem_t g_mutex_cola_exit;
extern sem_t g_hay_elementos_en_exit;
extern sem_t g_mutex_cola_auxiliar;

// Semaforos para interfaces
extern sem_t g_mutex_acceso_interfaces;

extern t_dictionary* g_interfaces;

extern sem_t g_notif_corto_plazo;
extern sem_t g_notif_largo_plazo;
extern int g_planificacion_pausada;

//VRR
extern t_temporal* timer;
extern int g_ms_transcurridos;
extern sem_t g_tiempo_calculado;
extern sem_t g_hay_elementos_para_ejecutar;

extern t_dictionary* g_diccionario_recursos;

//lista bloqueado gral
extern t_list* g_lista_blocked_gral;
extern sem_t g_mutex_lista_blocked_gral;

extern t_list* g_lista_procesos_gral;
extern sem_t g_mutex_lista_procesos_gral;


extern char** g_recursos_instancias;
extern char** g_recursos;
#endif