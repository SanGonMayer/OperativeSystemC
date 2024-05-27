#include "io.h"
#include "global_io.h"
#include "utils/buffer.h"
#include "utils/client.h"
#include "utils/codigo_operacion.h"
#include "utils/instrucciones_io.h"

t_instruccion_io* recibir_instruccion_io(int conexion_kernel){

    t_buffer* buffer = recibir_buffer(conexion_kernel);
    t_instruccion_io* instruccion = deserializar_instruccion_io(buffer);

    return instruccion;
}

void atender_instrucciones(int conexion_kernel, ProcesarInstruccion procesar_instruccion){
    while(1){
        t_instruccion_io* instruccion = recibir_instruccion_io(conexion_kernel);
        procesar_instruccion(conexion_kernel, instruccion);
    }
}

int iniciar_conexion_kernel(char* tipo_interfaz, char* nombre_interfaz){

    int conexion = crear_conexion(
    g_config_io->ip_kernel, 
    g_config_io->puerto_kernel, 
    "KERNEL", 
    g_logger);

    handshake_cliente(conexion, g_logger);

    if(conexion == -1){
        log_error(g_logger, "No se pudo conectar con el kernel");
        exit(EXIT_FAILURE);
    }

    t_interfaz interfaz = {
        .tipo = tipo_interfaz,
        .nombre = nombre_interfaz
    };

    t_buffer* buffer = serializar_interfaz(&interfaz);
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = ENVIO_INTERFAZ_CONECTADA;
    agregar_a_paquete(paquete, buffer->stream, buffer->size);
    serializar_y_enviar_paquete(paquete, conexion);

    buffer_destroy(buffer);

    return conexion;
}

int iniciar_conexion_memoria(){

    return crear_conexion(
    g_config_io->ip_memoria,
    g_config_io->puerto_memoria,
    "MEMORIA",
    g_logger);
}



