
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <semaphore.h>
int g_contador_pid = 0;
t_queue* g_cola_new;
t_queue* g_cola_ready;
t_log* g_logger;
int g_grado_multiprogramacion;
sem_t g_mutex_multiprogramacion;
int g_socket_memoria;
int g_conexion_cpu_dispatch;

sem_t g_hay_elementos_en_ready;
sem_t g_mutex_cola_ready;
sem_t g_disponible_exec;

