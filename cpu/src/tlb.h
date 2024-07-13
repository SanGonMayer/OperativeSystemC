#ifndef TLB_H_
#define TLB_H_

#include <commons/collections/list.h>

typedef struct {
    int pid;
    int pagina;
    int marco;
    int ultimo_uso;
} t_tlb_entry;

typedef struct {
    t_list* entradas;
    int capacidad;
    int tiempo;
    char* algoritmo;
} t_tlb;
bool tlb_enabled();
void tlb_init(int size, char* algoritmo);
t_tlb* tlb_create(int capacidad);
void tlb_destroy();
void tlb_add(int pid, int pagina, int marco);
bool tlb_get_marco(int pid, int pagina, int* marco, int pagina);

#endif