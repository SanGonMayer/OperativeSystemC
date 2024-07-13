#include "recursos.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <semaphore.h>
#include <stdlib.h>
#include "global_kernel.h"
#include "utils/procesos.h"

t_dictionary* diccionario_recursos;

t_dictionary* recursos_proceso = NULL;

t_list* procesos_bloqueados = NULL;

void init_recursos(char** recursos, char** instancias) {

    procesos_bloqueados = list_create();

    if(recursos_proceso == NULL){
        recursos_proceso = dictionary_create();
    }

    diccionario_recursos = dictionary_create();
    int i = 0;
    
    while(recursos[i] != NULL){
        t_recurso* recurso = crear_recurso(recursos[i], atoi(instancias[i]));
        dictionary_put(diccionario_recursos, recurso->nombre, recurso);
        i++;
    }
}

t_recurso* crear_recurso(char* nombre, int instancias) {
    t_recurso *recurso = malloc(sizeof(t_recurso));
    recurso->nombre = nombre;
    recurso->instancias = instancias;
    recurso->cola = queue_create();
    sem_init(&recurso->semaforo_cola, 0, instancias);
    return recurso;
}

void destruir_recurso(t_recurso* recurso) {
    free(recurso->nombre);
    queue_destroy(recurso->cola);
    sem_destroy(&recurso->semaforo_cola);
    free(recurso);
}

void liberar_instancia_recurso(t_recurso* recurso) {
    sem_post(&recurso->semaforo_cola);
    recurso->instancias++;
}

void tomar_instancia_recurso(t_recurso* recurso) {
    sem_wait(&recurso->semaforo_cola);
    recurso->instancias--;
}

bool esta_bloqueado(t_PCB* pcb){
    bool esProceso(void* proceso){
        return ((t_PCB*)proceso)->PID == pcb->PID;
    }
    return list_any_satisfy(procesos_bloqueados, esProceso);
}

bool procesar_wait(t_PCB* pcb, char* nombre_recurso){
    if (!dictionary_has_key(diccionario_recursos, nombre_recurso)){
        log_error(g_logger, "El recurso: %s no existe", nombre_recurso);
        return false;
    }

    t_recurso* recurso = dictionary_get(diccionario_recursos, nombre_recurso);

    list_add(procesos_bloqueados, pcb);
    log_info(g_logger, "Esperando una instancia del recurso: %s", nombre_recurso);
    tomar_instancia_recurso(recurso);
    log_info(g_logger, "Se tomo una instancia del recurso: %s", nombre_recurso);

    if(esta_bloqueado(pcb)){

        bool esProcesoAeliminar(void* proceso){
            return ((t_PCB*)proceso)->PID == pcb->PID;
        }   
        list_remove_by_condition(procesos_bloqueados, esProcesoAeliminar);

        if(dictionary_has_key(recursos_proceso, string_itoa(pcb->PID))){
            t_list* lista_recursos_proceso = dictionary_get(recursos_proceso, string_itoa(pcb->PID));
            list_add(lista_recursos_proceso, nombre_recurso);
        }else{
            t_list* lista_recursos_proceso = list_create();
            list_add(lista_recursos_proceso, nombre_recurso);
            dictionary_put(recursos_proceso, string_itoa(pcb->PID), lista_recursos_proceso);
        }

    }else{
        liberar_instancia_recurso(recurso);
        return false;
    }

    return true;    
}

t_PCB* procesar_signal(char* nombre_recurso){
    if (!dictionary_has_key(diccionario_recursos, nombre_recurso)){
        log_error(g_logger, "El recurso: %s no existe", nombre_recurso);
        abort();
    }

    t_recurso* recurso = dictionary_get(diccionario_recursos, nombre_recurso);
    log_info(g_logger, "Liberando una instancia del recurso: %s", nombre_recurso);
    liberar_instancia_recurso(recurso);
    
    return NULL;
}

void liberar_recursos_proceso(t_PCB* pcb){
    char* pid_str = string_itoa(pcb->PID);

    if(dictionary_has_key(recursos_proceso, pid_str)){
        t_list* lista_recursos_proceso = dictionary_get(recursos_proceso, string_itoa(pcb->PID));
        
        list_iterate(lista_recursos_proceso, (void*) procesar_signal);
        list_destroy(lista_recursos_proceso);
        dictionary_remove(recursos_proceso, string_itoa(pcb->PID));
    }
    free(pid_str);
}

t_PCB* quitar_proceso_bloqueado(int pid){



    bool esProcesoAeliminar(void* proceso){
        return ((t_PCB*)proceso)->PID == pid;
    }

    if(!list_any_satisfy(procesos_bloqueados, esProcesoAeliminar)){
        return NULL;
    }

    t_PCB* pcb = list_remove_by_condition(procesos_bloqueados, esProcesoAeliminar);
    return pcb;
}

t_list* get_procesos_bloqueados_recursos(){
    return procesos_bloqueados;
}