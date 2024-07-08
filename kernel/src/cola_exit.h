#ifndef COLA_EXIT_H_
#define COLA_EXIT_H_

#include "utils/procesos.h"
void init_cola_exit();
void agregar_a_cola_exit(t_PCB* pcb);
bool esta_en_cola_exit(int pid);
t_PCB* obtener_de_cola_exit(int pid);
void procesar_cola_exit();
void crear_hilo_cola_exit();
t_list* pids_exit();
#endif