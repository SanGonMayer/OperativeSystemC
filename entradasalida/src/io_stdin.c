#include "io_stdin.h"
#include "global_io.h"
#include "utils/buffer.h"
#include "utils/codigo_operacion.h"
#include "utils/peticiones_memoria.h"
#include "utils/client.h"
#include "utils/server.h"
#include <commons/string.h>
#include <readline/readline.h>
#include <stdint.h>

char* stdin_leer_texto();
void guardar_en_memoria(char* texto, int direccion_fisica);
bool stdin_soporta_instruccion(char* instruccion);

char instruccion_soportada[14] = "IO_STDIN_READ";

void procesar_instruccion_stdin(int fd, t_instruccion_io* instruccion) {

    if(!stdin_soporta_instruccion(instruccion->instruccion)){
        log_error(g_logger, "Instruccion no soportada: %s", instruccion->instruccion);
        // que hay que hacer en este caso?
        return;
    }

    char* texto = stdin_leer_texto();
    guardar_en_memoria(texto, instruccion->puntero_archivo);

    responder_ok(fd);
}

char* stdin_leer_texto() {
    char* texto = readline("Ingrese texto: ");
    return texto;
}

void guardar_en_memoria(char* texto, int direccion_fisica) {
    t_peticion_acceso_usuario* peticion_escritura = crear_peticion_escritura(direccion_fisica, texto);
    t_buffer* buffer = serializar_peticion_acceso_usuario(peticion_escritura);
    t_paquete* paquete = crear_paquete(ACCEDER_ESPACIO_DE_USUARIO_MEMORIA, buffer);

    int err = enviar_paquete(paquete, g_socket_memoria);

    if(err == -1){
        log_error(g_logger, "Error al enviar paquete a memoria");
    }

    eliminar_paquete(paquete);
    destruir_peticion_acceso_usuario(peticion_escritura);

    bool ok = recibir_ok(g_socket_memoria);

    if(!ok){
        log_error(g_logger, "Error al guardar texto en memoria");
    }
}

bool stdin_soporta_instruccion(char* instruccion){
    return string_equals_ignore_case(instruccion, instruccion_soportada);
}