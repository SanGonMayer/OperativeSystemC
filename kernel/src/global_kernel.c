
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <semaphore.h>
#include <utils/procesos.h>
int g_contador_pid = 0;
t_queue* g_cola_new;
t_queue* g_cola_ready;
t_queue* g_cola_exit;
t_log* g_logger;
int g_grado_multiprogramacion;
sem_t g_mutex_multiprogramacion;
int g_socket_memoria;
int g_conexion_cpu_dispatch;
int g_quantum;
t_PCB* g_exec;
int g_conexion_cpu_interrupt;

t_dictionary* g_interfaces;

sem_t g_hay_elementos_en_ready;
sem_t g_mutex_cola_ready;
sem_t g_disponible_exec;
sem_t g_tope_multiprogramacion;
sem_t mutex_contador_pid;
sem_t g_mutex_cola_new;
sem_t g_mutex_socket_memoria;
sem_t g_mutex_cola_exit;
sem_t g_hay_elementos_en_exit;

sem_t g_mutex_acceso_interfaces;
