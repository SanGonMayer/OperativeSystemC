#include "instrucciones_memoria.h"


void guardar_en_memoria(uint32_t pid,int socket_memoria, char* texto, t_list* peticionesMemoria, t_log* g_logger) {

    for(int i = 0; i < list_size(peticionesMemoria); i++){
        t_peticion_acceso_usuario* peticion = list_get(peticionesMemoria, i);
        t_buffer* buffer = serializar_peticion_acceso_usuario(peticion);
        t_paquete* paquete = crear_paquete(ACCEDER_ESPACIO_DE_USUARIO_MEMORIA, buffer);
        enviar_paquete(paquete, socket_memoria);
        bool ok = recibir_ok(socket_memoria);
        if(ok){
            log_info(g_logger, "Se escribio correctamente en memoria");
        } else {
            log_error(g_logger, "No se pudo escribir en memoria");
        }
        eliminar_paquete(paquete);
    }

    t_peticion_acceso_usuario* peticion = list_get(peticionesMemoria, 0);
    int direccion_fisica = peticion->direccion_fisica;

    log_info(g_logger, "PID %d - Acccion: ESCRIBIR - Direccion fisica: %d - Valor: %s", pid, direccion_fisica, texto);

}

char* leer_de_memoria(uint32_t pid,int socket_memoria, int tamanio, t_list* peticionesMemoria, t_log* logger) {

    char* mensaje = string_new();
    for(int i = 0; i < list_size(peticionesMemoria); i++){
        t_peticion_acceso_usuario * peticion = list_get(peticionesMemoria, i);
        t_buffer* buffer =  serializar_peticion_acceso_usuario(peticion);
        t_paquete* paquete = crear_paquete(ACCEDER_ESPACIO_DE_USUARIO_MEMORIA, buffer);
        enviar_paquete(paquete, socket_memoria);
        t_buffer* buffer_respuesta = recibir_buffer(socket_memoria);
        uint32_t length;
        char* respuesta = buffer_read_string(buffer_respuesta, &length);
        string_append(&mensaje, respuesta);

        free(respuesta);
        eliminar_paquete(paquete);
        buffer_destroy(buffer_respuesta);
    }

    uint32_t length = string_length(mensaje);

    mensaje[length] = '\0';

    t_peticion_acceso_usuario* peticion = list_get(peticionesMemoria, 0);
    int direccion_fisica = peticion->direccion_fisica;

    log_info(logger, "PID %d - Acccion: Leido - Direccion fisica: %d - Valor: %s", pid, direccion_fisica, mensaje);

    return mensaje;
}

void actualizar_peticiones_con_valor(t_list* peticionesMemoria, char* valor){
    int tamanioCopiado = 0;
    for(int i = 0; i < list_size(peticionesMemoria); i++){
        t_peticion_acceso_usuario* peticion = list_get(peticionesMemoria, i);
        peticion->string = string_substring(valor, tamanioCopiado, peticion->tamanio_a_leer);
        tamanioCopiado+= peticion->tamanio_a_leer;
    }
}