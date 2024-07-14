#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <commons/collections/list.h>
void set_tamanio_pagina(int tamanio);
int traducir_a_direccion_fisica(uint32_t pid, int direccion_logica);
int pedir_marco_a_memoria(uint32_t pid, int nro_pagina);
int calcular_paginas(int direccion_logica, uint32_t tamanio);
char* crear_string_de_tamanio(int tamanio);
t_list* obtener_direcciones_logicas_lectura(uint32_t pid,int direccion_logica, uint32_t tamanio);

t_list* obtener_direcciones_logicas_escritura(uint32_t pid,int direccion_logica, char* valor);

t_list* obtener_direcciones_logicas_escritura_stdin(uint32_t pid, int direccion_logica, int tamanio);
int get_tamanio_pagina();

#endif