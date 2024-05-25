#ifndef GLOBAL_KERNEL_H_
#define GLOBAL_KERNEL_H_

#include <commons/collections/queue.h>
#include <commons/log.h>
#include <semaphore.h>
#include <utils/procesos.h>

extern int g_contador_pid;
extern t_queue* g_cola_new;
extern t_queue* g_cola_ready;
extern t_queue* g_cola_exit;
extern t_log* g_logger;
extern int g_grado_multiprogramacion;
extern int g_socket_memoria;
extern sem_t g_mutex_multiprogramacion;
extern int g_conexion_cpu_dispatch;
extern int g_quantum;
extern t_PCB* g_exec;
extern int g_conexion_cpu_interrupt;

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

extern t_dictionary* g_interfaces;
#endif