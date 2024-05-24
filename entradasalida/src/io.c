#include "io.h"
#include "global_io.h"
#include "utils/buffer.h"
#include "utils/client.h"
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

int iniciar_conexion_kernel(){

    return crear_conexion(
    g_config_io->ip_kernel, 
    g_config_io->puerto_kernel, 
    "KERNEL", 
    g_logger);
}

int iniciar_conexion_memoria(){

    return crear_conexion(
    g_config_io->ip_memoria,
    g_config_io->puerto_memoria,
    "MEMORIA",
    g_logger);
}



