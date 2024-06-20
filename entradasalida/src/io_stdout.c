#include "io_stdout.h"
#include "global_io.h"
#include "utils/codigo_operacion.h"
#include "utils/peticiones_memoria.h"
#include "utils/client.h"
#include <commons/string.h>
#include <readline/readline.h>

void leer_de_memoria_stdout(int tamanio, int direccion_fisica);
bool stdout_soporta_instruccion(char* instruccion);

char instruccion_soportada_stdout[15] = "IO_STDOUT_WRITE";

void procesar_instruccion_stdout(int fd, t_instruccion_io* instruccion){
    
    if(!stdout_soporta_instruccion(instruccion->instruccion)){
        log_error(g_logger, "Instruccion no soportada: %s", instruccion->instruccion);
        // que hay que hacer en este caso?
        return;
    }
    
    int direccion = instruccion->puntero_archivo;
    int tamanio = instruccion->tamanio;
    char *valor_a_mostrar = malloc(tamanio);
    
    leer_de_memoria_stdout(tamanio, direccion);

    printf("%s\n", valor_a_mostrar); //esto es el putero archivo
    
    free(valor_a_mostrar);
    responder_ok(fd);
}

void leer_de_memoria_stdout(int tamanio, int direccion_fisica) {
    t_peticion_acceso_usuario* peticion_lectura = crear_peticion_lectura(tamanio, direccion_fisica);
    t_buffer* buffer = serializar_peticion_acceso_usuario(peticion_lectura);
    t_paquete* paquete = crear_paquete(ACCEDER_ESPACIO_DE_USUARIO_MEMORIA, buffer);

    int err = enviar_paquete(paquete, g_socket_memoria);

    if(err == -1){
        log_error(g_logger, "Error al enviar paquete a memoria");
    }

    eliminar_paquete(paquete);
    destruir_peticion_acceso_usuario(peticion_lectura);

    t_buffer* buffer_lectura = recibir_buffer(g_socket_memoria);
    
    uint32_t* length = malloc(sizeof(uint32_t));

    char *texto_leido = buffer_read_string(buffer_lectura, length);
    
    log_info(g_logger, "Texto leido de memoria: %s", texto_leido);

    buffer_destroy(buffer_lectura);
    
}

bool stdout_soporta_instruccion(char* instruccion){
    return string_equals_ignore_case(instruccion, instruccion_soportada_stdout);
}