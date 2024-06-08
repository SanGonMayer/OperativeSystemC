#include "tlb.h"
#include "global_cpu.h"
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stdlib.h>

static t_tlb *tlb = NULL;

bool tlb_enabled() {
    return tlb != NULL && tlb->capacidad > 0;
}

void tlb_init(int size, char* algoritmo) {
    if(tlb == NULL){
        tlb = tlb_create(size);
    }

    tlb->algoritmo = string_duplicate(algoritmo);
    tlb->tiempo = 0;
}

void tlb_entry_destroyer(void* entry){
        free(entry);
}

void tlb_destroy() {
    list_destroy_and_destroy_elements(tlb->entradas, &tlb_entry_destroyer);
    free(tlb->algoritmo);
    free(tlb);
}

t_tlb* tlb_create(int capacidad) {
    t_tlb* new_tlb = malloc(sizeof(t_tlb));
    new_tlb->entradas = malloc(sizeof(t_tlb_entry) * capacidad);
    new_tlb->capacidad = capacidad;
    return new_tlb;
}

void tlb_replace_entry(t_tlb* tlb, int pid, int pagina, int marco) {
    int index = 0;
    if (string_equals_ignore_case(tlb->algoritmo, "FIFO")) {

        list_remove_and_destroy_element(tlb->entradas, 0, free);
    } else if (string_equals_ignore_case(tlb->algoritmo, "LRU")) {

        int ultimo_uso_mas_antiguo = ((t_tlb_entry*)list_get(tlb->entradas, 0))->ultimo_uso;
        for (int i = 1; i < list_size(tlb->entradas); ++i) {
            t_tlb_entry *entry = list_get(tlb->entradas, i);
            if (entry->ultimo_uso < ultimo_uso_mas_antiguo) {
                ultimo_uso_mas_antiguo = entry->ultimo_uso;
                index = i;
            }
        }
        list_remove_and_destroy_element(tlb->entradas, index, &tlb_entry_destroyer);
    }else{
        log_error(g_logger, "Algoritmo de reemplazo de TLB no soportado: %s", tlb->algoritmo);
        exit(EXIT_FAILURE);
    }
    
    tlb_add(pid, pagina, marco);
}

void tlb_add(int pid, int pagina, int marco) {
    if (list_size(tlb->entradas) < tlb->capacidad) {
        t_tlb_entry* new_entry = malloc(sizeof(t_tlb_entry));
        new_entry->pid = pid;
        new_entry->pagina = pagina;
        new_entry->marco = marco;
        new_entry->ultimo_uso = tlb->tiempo++;
    } else {
        tlb_replace_entry(tlb, pid, pagina, marco);
    }
}

bool tlb_get_marco(int pid, int pagina, int* marco) {
    for (int i = 0; i < list_size(tlb->entradas); ++i) {
        t_tlb_entry *entry = list_get(tlb->entradas, i);
        if (entry->pid == pid && entry->pagina == pagina) {
            *marco = entry->marco;
            if (string_equals_ignore_case(tlb->algoritmo,"LRU")) {
                entry->ultimo_uso = tlb->tiempo++;
            }
            log_info(g_logger, "PID: %d - TLB HIT - Pagina: %d", pid, pagina);
            return true;
        }
    }

    log_info(g_logger, "PID: %d - TLB MISS - Pagina: %d", pid, pagina);
    return false;
}