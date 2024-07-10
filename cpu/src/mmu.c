#include "mmu.h"
#include "cpu.h"
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

t_list* obtener_direcciones_logicas_lectura(uint32_t pid, int direccion_logica,uint32_t tamanio_total_lectura){
    t_list* peticiones = list_create();
    int pagina = direccion_logica / tamanio_pagina;
    int offset = direccion_logica % tamanio_pagina;

    uint32_t tamanio_disponible = tamanio_pagina - offset;
    int direccion_fisica = obtener_direccion_fisica(pid, direccion_logica);
    
    while(tamanio_total_lectura > 0){
        if(tamanio_disponible < tamanio_total_lectura){
            t_peticion_acceso_usuario* peticion = crear_peticion_lectura(pid,tamanio_disponible, direccion_fisica);
            list_add(peticiones, peticion);
            tamanio_total_lectura -= tamanio_disponible;
            direccion_logica += tamanio_disponible;
            pagina = direccion_logica / tamanio_pagina;
            offset = direccion_logica % tamanio_pagina;
            tamanio_disponible = tamanio_pagina - offset;
            direccion_fisica = obtener_direccion_fisica(pid, direccion_logica);
            //Calcular direccion logica siguiente
        } else {
            t_peticion_acceso_usuario* peticion = crear_peticion_lectura(pid,tamanio_total_lectura, direccion_fisica);
            list_add(peticiones, peticion);
            tamanio_total_lectura = 0;
        }        
    }
    
    return peticiones;
}

char* crear_string_de_tamanio(int tamanio){
    char* valor = malloc(tamanio);
    for(int i = 0; i < tamanio; i++){
        valor[i] = 'a';
    }
    return valor;
}

t_list* obtener_direcciones_logicas_escritura_stdin(uint32_t pid, int direccion_logica, int tamanio){
    t_list* peticiones = list_create();
    int pagina = direccion_logica / tamanio_pagina;
    int offset = direccion_logica % tamanio_pagina;
    char* valor = crear_string_de_tamanio(tamanio); 
    
    int tamanio_restante = tamanio;
    char* ptr_valor = valor;

    while (tamanio_restante > 0) {
        uint32_t tamanio_disponible = tamanio_pagina - offset;
        uint32_t tamanio_escritura = tamanio_disponible < tamanio_restante ? tamanio_disponible : tamanio_restante;

        char* fragmento_valor = strndup(ptr_valor, tamanio_escritura);
        int direccion_fisica = obtener_direccion_fisica(pid, direccion_logica);
        t_peticion_acceso_usuario* peticion = crear_peticion_escritura_stdin(pid,direccion_fisica, tamanio_escritura);

        list_add(peticiones, peticion);

        free(fragmento_valor);

        tamanio_restante -= tamanio_escritura;
        ptr_valor += tamanio_escritura;
        direccion_logica += tamanio_escritura;
        pagina = direccion_logica / tamanio_pagina;
        offset = direccion_logica % tamanio_pagina;
    }

    return peticiones;
}

t_list* obtener_direcciones_logicas_escritura(uint32_t pid, int direccion_logica, char* valor) {
    t_list* peticiones = list_create();
    int pagina = direccion_logica / tamanio_pagina;
    int offset = direccion_logica % tamanio_pagina;
    int tamanio = string_length(valor);
    if(valor[0] == '\0'){
        tamanio = 1;
    }
    
    int tamanio_restante = tamanio;
    char* ptr_valor = valor;

    while (tamanio_restante > 0) {
        uint32_t tamanio_disponible = tamanio_pagina - offset;
        uint32_t tamanio_escritura = tamanio_disponible < tamanio_restante ? tamanio_disponible : tamanio_restante;

        char* fragmento_valor = strndup(ptr_valor, tamanio_escritura);
        int direccion_fisica = obtener_direccion_fisica(pid, direccion_logica);
        t_peticion_acceso_usuario* peticion = crear_peticion_escritura(pid,direccion_fisica, fragmento_valor);

        list_add(peticiones, peticion);

        free(fragmento_valor);

        tamanio_restante -= tamanio_escritura;
        ptr_valor += tamanio_escritura;
        direccion_logica += tamanio_escritura;
        pagina = direccion_logica / tamanio_pagina;
        offset = direccion_logica % tamanio_pagina;
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

    log_info(g_logger, "PID: %d - OBTENER MARCO - Pagina: %d - Marco: %d", pid, nro_pagina, nro_marco);

    buffer_destroy(respuesta);

    return nro_marco;
}