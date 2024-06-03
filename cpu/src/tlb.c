#include "tlb.h"
#include <commons/collections/list.h>
#include <commons/string.h>
#include <stdlib.h>

static t_tlb *tlb = NULL;

bool tlb_enabled() {
    return tlb != NULL && tlb->capacidad > 0;
}

void tlb_init(int size, char* algoritmo) {
    if(tlb != NULL){
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
    if (string_equals_ignore_case(tlb->algoritmo,"LRU")) {
        t_tlb_entry* entry = list_get(tlb->entradas, 0);
        int index = 0;
        for (int i = 1; i < list_size(tlb->entradas); ++i) {
            t_tlb_entry* current = list_get(tlb->entradas, i);
            if (current->ultimo_uso < entry->ultimo_uso) {
                entry = current;
                index = i;
            }
        }
        entry->pid = pid;
        entry->pagina = pagina;
        entry->marco = marco;
        entry->ultimo_uso = tlb->tiempo++;
    } else {
        t_tlb_entry* entry = list_get(tlb->entradas, 0);
        entry->pid = pid;
        entry->pagina = pagina;
        entry->marco = marco;
    }
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
            return true;
        }
    }
    return false;
}