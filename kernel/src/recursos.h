#ifndef RECURSOS_H_
#define RECURSOS_H_

#include "utils/procesos.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <semaphore.h>

typedef struct {
    char* nombre;
    int instancias;
    t_queue* cola;
    sem_t semaforo_cola;
} t_recurso;


void init_recursos(char** recursos, char** instancias);
t_recurso* crear_recurso(char* nombre, int instancias);
void destruir_recurso(t_recurso* recurso);
void liberar_instancia_recurso(t_recurso* recurso);
void tomar_instancia_recurso(t_recurso* recurso);
void manejar_recurso(int operacion, char* nombre_recurso, t_PCB* pcb);
bool procesar_wait(t_PCB* pcb, char* nombre_recurso);
t_PCB* procesar_signal(char* nombre_recurso);
void liberar_recursos_proceso(t_PCB* pcb);
t_PCB* quitar_proceso_bloqueado(int pid);
t_list* get_procesos_bloqueados_recursos();
#endif