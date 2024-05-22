#ifndef GLOBAL_KERNEL_H_
#define GLOBAL_KERNEL_H_

#include <commons/collections/queue.h>
#include <commons/log.h>
#include <semaphore.h>
extern int g_contador_pid;
extern t_queue* g_cola_new;
extern t_queue* g_cola_ready;
extern t_log* g_logger;
extern int g_grado_multiprogramacion;
extern int g_socket_memoria;


extern sem_t g_mutex_multiprogramacion;
extern sem_t g_actualizacion_pcb;
#endif