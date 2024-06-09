#include "mmu.h"
#include "global_cpu.h"
#include "utils/buffer.h"
#include "utils/client.h"
#include "utils/codigo_operacion.h"
#include "utils/peticiones_memoria.h"

static int tamanio_pagina;

void set_tamanio_pagina(int tamanio) {
    tamanio_pagina = tamanio;
}

int calcular_paginas(int direccion_logica, uint32_t tamanio){
    int pagina_inicial = direccion_logica / tamanio_pagina;
    int pagina_final = (direccion_logica + tamanio) / tamanio_pagina;

    if((direccion_logica + tamanio) % tamanio_pagina != 0){
        pagina_final++;
    }

    return pagina_final - pagina_inicial;
}

t_list* obtener_direcciones_logicas(uint32_t pid, int direccion_logica,uint32_t tamanio_total_lectura){
    t_list* peticiones = list_create();
    int pagina = direccion_logica / tamanio_pagina;
    int offset = direccion_logica % tamanio_pagina;

    uint32_t tamanio_disponible = tamanio_pagina - offset;
    int direccion_fisica = traducir_a_direccion_fisica(pid, direccion_logica);
    
    while(tamanio_total_lectura > 0){
        if(tamanio_disponible < tamanio_total_lectura){
            t_peticion_acceso_usuario* peticion = crear_peticion_lectura(tamanio_disponible, direccion_fisica);
            list_add(peticiones, peticion);
            tamanio_total_lectura -= tamanio_disponible;
            direccion_logica += tamanio_disponible;
            pagina = direccion_logica / tamanio_pagina;
            offset = direccion_logica % tamanio_pagina;
            tamanio_disponible = tamanio_pagina - offset;
            direccion_fisica = traducir_a_direccion_fisica(pid, direccion_logica);
            //Calcular direccion logica siguiente
        } else {
            t_peticion_acceso_usuario* peticion = crear_peticion_lectura(tamanio_total_lectura, direccion_fisica);
            list_add(peticiones, peticion);
            tamanio_total_lectura = 0;
        }        
    }
    
    return peticiones;
}

int traducir_a_direccion_fisica(uint32_t pid, int direccion_logica) {
    int nro_pagina = direccion_logica / tamanio_pagina;
    int offset = direccion_logica % tamanio_pagina;

    int nro_marco = pedir_marco_a_memoria(pid, nro_pagina);

    int direccion_fisica = nro_marco * tamanio_pagina + offset;

    return direccion_fisica;
}

int pedir_marco_a_memoria(uint32_t pid, int nro_pagina) {

    t_peticion_marco* peticion = crear_peticion_marco(pid, nro_pagina);
    t_buffer* buffer = serializar_peticion_marco(peticion);
    t_paquete* paquete = crear_paquete(OBTENER_MARCO_MEMORIA, buffer);
    int result = enviar_paquete(paquete, g_socket_memoria);

    if (result == -1) {
        log_error(g_logger, "Error al enviar la peticion de marco a memoria");
        return -1;
    }

    eliminar_paquete(paquete);
    destruir_peticion_marco(peticion);

    t_buffer* respuesta = recibir_buffer(g_socket_memoria);
    int nro_marco = buffer_read_int(respuesta);

    buffer_destroy(respuesta);

    return nro_marco;
}