#ifndef MEMORIA_H
#define MEMORIA_H

#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <stdint.h>

// Estructura para representar la memoria.
typedef struct {
    uint32_t tam_memoria;      // Tamaño total de la memoria.
    uint32_t tam_pagina;       // Tamaño de cada página.
    uint32_t cantidad_marcos;  // Número de marcos disponibles en la memoria.
    void* espacio_contiguo;    // Espacio de memoria contiguo.
    t_dictionary* tablas_de_paginas; // Diccionario de tablas de páginas por proceso.
    char* bitmap;              // Bitmap para gestionar marcos libres/ocupados.
    t_bitarray* bitarray;      // Estructura para manipular el bitmap.
} t_memoria;

/**
 * @brief Calcula los bits necesarios para representar la cantidad de marcos.
 * 
 * @param cantidad_marcos Número de marcos.
 * @return int Número de bytes necesarios para el bitmap.
 */
int calcular_bits_necesarios(uint32_t cantidad_marcos);

/**
 * @brief Inicializa la memoria con el tamaño y la página especificados.
 * 
 * @param tam_memoria Tamaño total de la memoria.
 * @param tam_pagina Tamaño de cada página.
 */
void inicializar_memoria(uint32_t tam_memoria, uint32_t tam_pagina);

/**
 * @brief Inicializa la tabla de páginas para un proceso dado.
 * 
 * @param pid Identificador del proceso.
 * @return t_list* Puntero a la lista que representa la tabla de páginas.
 */
t_list* inicializar_tabla_paginas(uint32_t pid);

/**
 * @brief Libera la tabla de páginas de un proceso dado.
 * 
 * @param pid Identificador del proceso.
 */
void liberar_tabla_paginas(uint32_t pid);

/**
 * @brief Lee el número de marco de una página específica de un proceso.
 * 
 * @param pid Identificador del proceso.
 * @param nro_pagina Número de la página.
 * @return int Número de marco correspondiente.
 */
int leer_nro_marco(uint32_t pid, uint32_t nro_pagina);

/**
 * @brief Obtiene el primer marco libre disponible.
 * 
 * @return int Número de marco libre o -1 si no hay marcos libres.
 */
int obtener_marco_libre();

/**
 * @brief Libera un marco especificado.
 * 
 * @param nro_marco Número del marco a liberar.
 */
void liberar_marco(uint32_t nro_marco);

/**
 * @brief Ajusta el tamaño de la memoria asignada a un proceso.
 * 
 * @param pid Identificador del proceso.
 * @param nuevo_tamanio Nuevo tamaño de memoria requerido.
 */
void ajustar_tamanio_proceso(uint32_t pid, uint32_t nuevo_tamanio);

/**
 * @brief Lee datos desde la memoria.
 * 
 * @param pid Identificador del proceso.
 * @param direccion_fisica Dirección física de inicio de la lectura.
 * @param tamanio Tamaño de los datos a leer.
 * @param buffer Puntero al buffer donde se almacenarán los datos leídos.
 */
void leer_de_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio, void* buffer);

/**
 * @brief Escribe datos en la memoria.
 * 
 * @param pid Identificador del proceso.
 * @param direccion_fisica Dirección física de inicio de la escritura.
 * @param tamanio Tamaño de los datos a escribir.
 * @param buffer Puntero al buffer que contiene los datos a escribir.
 */
void escribir_en_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio, void* buffer);

#endif // MEMORIA_H