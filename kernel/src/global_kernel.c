
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <semaphore.h>
#include <utils/procesos.h>
#include <commons/temporal.h>

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
t_queue* g_cola_auxiliar;
char * algoritmo_planificacion;

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
sem_t g_mutex_cola_auxiliar;

sem_t g_mutex_acceso_interfaces;

sem_t g_notif_corto_plazo;
sem_t g_notif_largo_plazo;
int g_planificacion_pausada;

t_temporal* timer;
int g_ms_transcurridos;
sem_t g_tiempo_calculado;
sem_t g_hay_elementos_para_ejecutar;

t_dictionary* g_diccionario_recursos;
t_dictionary* g_diccionario_recursos_colas_blocked;

t_list* g_lista_blocked_gral;
sem_t g_mutex_lista_blocked_gral;