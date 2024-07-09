#include "cpu.h"
#include <commons/log.h>
#include <stdint.h>

char* etapa_fetch(int socket, t_PCB* pcb, t_log* logger, t_dictionary* diccionario){
    char* instruccion;
    //Envio posicion de memoria + program counter

    instruccion = pedir_instruccion(socket, pcb, logger, diccionario);
    
    //Sumar program counter
    uint32_t valorPCanterior = (uint32_t)dictionary_get(diccionario, "PC");
    log_info(logger, "PC: %s", string_itoa(valorPCanterior));
    dictionary_put(diccionario, "PC", (void*) (valorPCanterior + 1));
    return instruccion;
}

char* recibir_instruccion(int socket){
    t_buffer* buffer = recibir_buffer(socket);

    uint32_t size_instruccion;
    char* instruccion = buffer_read_string(buffer, &size_instruccion);

    buffer_destroy(buffer);
    return instruccion;
}

t_interrupcion_dispatch* recibir_interrupcion(int socket){
    t_buffer* buffer = recibir_buffer(socket);

    t_interrupcion_dispatch* interrupcion = malloc(sizeof(t_interrupcion_dispatch));

    interrupcion->pid = buffer_read_uint32(buffer);
    interrupcion->motivo = buffer_read_uint32(buffer);

    return interrupcion;
}

char* pedir_instruccion(int socket, t_PCB* pcb, t_log* logger, t_dictionary* diccionario){
    log_info(logger, "Pidiendo instruccion a memoria");
    t_paquete_instruccion* paquete_instruccion = crear_paquete_instruccion(pcb->PID, (uint32_t)dictionary_get(diccionario, "PC"));
    
    t_paquete* paquete = crear_paquete(ENVIO_PID_PC, serializar_paquete_instruccion(paquete_instruccion));
    int err = enviar_paquete(paquete, socket);
    
    if(err < 0){
        log_error(logger, "Error al enviar paquete");
        return NULL;
    }

    eliminar_paquete(paquete);

    return recibir_instruccion(socket);
}

void ejecutar_set(char* registro, int valor, t_PCB* pcb, t_dictionary* diccionario){
    dictionary_put(diccionario, registro, (void*)valor);
}

void ejecutar_sum(char* registroDestino, char* registroValor, t_PCB* pcb, t_dictionary* diccionario){
    //Chequear tipos de dato
    uint32_t valorASumar = (uint32_t) dictionary_get(diccionario, registroValor);
    uint32_t valorDestino = (uint32_t)dictionary_get(diccionario, registroDestino);
    uint32_t suma = valorASumar + valorDestino;
    dictionary_put(diccionario, registroDestino, (void*)suma);
}

void ejecutar_sub(char* registroDestino, char* registroValor, t_PCB* pcb, t_dictionary* diccionario){
    //Chequear tipos de dato
    uint32_t valorARestar = (uint32_t)dictionary_get(diccionario, registroValor);
    uint32_t valorDestino = (uint32_t)dictionary_get(diccionario, registroDestino);
    uint32_t resta = valorDestino - valorARestar;
    dictionary_put(diccionario, registroDestino, (void*)resta);

    // free(valorARestar);
    // free(valorDestino);
}

void ejecutar_jnz(char* registro, int valorPC, t_PCB* pcb, t_dictionary* diccionario){
    uint32_t valor = (uint32_t)dictionary_get(diccionario, registro);
    if (valor != 0){
        dictionary_put(diccionario, "PC", (void*)valorPC);
    }
}

t_buffer* ejecutar_io_gen_sleep(char* dispositivo, int unidadesDeTrabajo){
    uint32_t length = strlen(dispositivo) + 1;
    t_buffer* buffer = buffer_create(sizeof(int) + sizeof(uint32_t) + length);
    buffer_add_int(buffer, unidadesDeTrabajo);
    buffer_add_string(buffer, length, dispositivo);
    return buffer;
}

int sizeTotalDeLista(t_list *peticiones){
    int sizeLista = list_size(peticiones);

    int size_por_peticion = sizeof(uint32_t) + sizeof(int) + sizeof(t_tipo_acceso) + sizeof(uint32_t);

    int sizeTotal =  sizeof(int) 
        + sizeLista * size_por_peticion;

    for (int i = 0; i < sizeLista; i++) {
        t_peticion_acceso_usuario* peticion = list_get(peticiones, i);
        sizeTotal += string_length(peticion->string);
    }

    return sizeTotal;
}

int sizeTotalIo(uint32_t length_dispositivo, t_list* peticiones){
    int sizeLista = sizeTotalDeLista(peticiones);

    int sizeTotal =  sizeof(int)
        + sizeLista 
        + sizeof(uint32_t) 
        + length_dispositivo;

    return sizeTotal;
}

t_buffer* ejecutar_io_stdin_read(uint32_t pid,char* dispositivo, int direccion_logica, int registro_tamanio){
    uint32_t length = strlen(dispositivo) + 1;
    t_list* peticiones = obtener_direcciones_logicas_escritura_stdin(pid, direccion_logica, registro_tamanio);

    int sizeTotal = sizeTotalIo(length, peticiones);
    int sizeLista = list_size(peticiones);

    t_buffer* buffer = buffer_create(sizeTotal);

    buffer_add_int(buffer, registro_tamanio);

    buffer_add_lista(buffer, sizeLista, peticiones);
    
    buffer_add_string(buffer, length, dispositivo);

    return buffer;

}

t_buffer* ejecutar_io_stdout_write(uint32_t pid,char* dispositivo, int direccion_logica, int registro_tamanio){
uint32_t length = strlen(dispositivo) + 1;
    t_list* peticiones = obtener_direcciones_logicas_lectura(pid, direccion_logica, registro_tamanio);

    int sizeLista = list_size(peticiones);

    int sizeTotal = sizeTotalIo(length, peticiones);

    t_buffer* buffer = buffer_create(sizeTotal);

    buffer_add_int(buffer, registro_tamanio);

    buffer_add_lista(buffer,sizeLista,peticiones);

    buffer_add_string(buffer, length, dispositivo);

    return buffer;
}

void ejecutar_mov_in(uint32_t pid,char* registro_datos, int direccion_logica, t_dictionary* diccionario){
    uint32_t tamanio;
    if(string_starts_with(registro_datos, "E")){
        tamanio = sizeof(uint32_t);
    } else {
        tamanio = sizeof(uint8_t);
    }
    t_list* peticiones = obtener_direcciones_logicas_lectura(pid, direccion_logica, tamanio);
    char* mensaje = string_new();
    for(int i = 0; i < list_size(peticiones); i++){
        t_peticion_acceso_usuario * peticion = list_get(peticiones, i);
        t_buffer* buffer =  serializar_peticion_acceso_usuario(peticion);
        t_paquete* paquete = crear_paquete(ACCEDER_ESPACIO_DE_USUARIO_MEMORIA, buffer);
        enviar_paquete(paquete, g_socket_memoria);
        t_buffer* buffer_respuesta = recibir_buffer(g_socket_memoria);
        uint32_t length;
        char* respuesta = buffer_read_string(buffer_respuesta, &length);
        string_append(&mensaje, respuesta);

        free(respuesta);
        eliminar_paquete(paquete);
        buffer_destroy(buffer_respuesta);
    }

    uint32_t mensaje_diccionario;

    uint32_t valor = (uint32_t)mensaje[0];

    memcpy(&mensaje_diccionario, &valor, sizeof(uint32_t));

    dictionary_put(diccionario, registro_datos, (void*)mensaje_diccionario);
    list_destroy_and_destroy_elements(peticiones, (void*)destruir_peticion_acceso_usuario);

    return;
}

void ejecutar_mov_out(uint32_t pid, int direccion_logica, uint32_t valor, t_dictionary* diccionario){

    //char* valor_string = string_itoa(valor);
    char* valor_string = string_new();
    //Convertir un uint32 en un string, a traves de memcpy no necesita casteo segun Issue pero hay que probarlo
    memcpy(valor_string, &valor, sizeof(uint32_t));

    t_list* peticiones = obtener_direcciones_logicas_escritura(pid, direccion_logica, valor_string);

    for(int i = 0; i < list_size(peticiones); i++){
        t_peticion_acceso_usuario* peticion = list_get(peticiones, i);
        t_buffer* buffer = serializar_peticion_acceso_usuario(peticion);
        t_paquete* paquete = crear_paquete(ACCEDER_ESPACIO_DE_USUARIO_MEMORIA, buffer);
        enviar_paquete(paquete, g_socket_memoria);
        bool ok = recibir_ok(g_socket_memoria);
        if(ok){
            log_info(g_logger, "Se escribio correctamente en memoria con MOV_OUT");
        } else {
            log_error(g_logger, "No se pudo escribir en memoria con MOV_OUT");
        }
        eliminar_paquete(paquete);
    }
    list_destroy_and_destroy_elements(peticiones, (void*)destruir_peticion_acceso_usuario);
    return;
}   

void ejecutar_copy_string(int tamanio, uint32_t pid, t_dictionary* diccionario){
    int direccion_logica_si = (int)dictionary_get(diccionario, "SI");
    int direccion_logica_di = (int)dictionary_get(diccionario, "DI");

    t_list* peticiones_lectura = obtener_direcciones_logicas_lectura(pid, direccion_logica_si, tamanio);

    char* data = leer_de_memoria(g_socket_memoria, tamanio, peticiones_lectura, g_logger);

    list_destroy_and_destroy_elements(peticiones_lectura, (void*)destruir_peticion_acceso_usuario);

    // Ahora escribir los datos en la dirección lógica DI
    t_list* peticiones_escritura = obtener_direcciones_logicas_escritura(pid, direccion_logica_di, data);

    guardar_en_memoria(g_socket_memoria, data, peticiones_escritura, g_logger);

    list_destroy_and_destroy_elements(peticiones_escritura, (void*)destruir_peticion_acceso_usuario);

    free(data);
}

t_buffer* ejecutar_io_fs_create(char* interfaz, char* nombre_archivo) {
    uint32_t length_interfaz = strlen(interfaz) + 1;
    uint32_t length_nombre_archivo = strlen(nombre_archivo) + 1;
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + length_interfaz + sizeof(uint32_t) + length_nombre_archivo);
    buffer_add_string(buffer, length_interfaz, interfaz);
    buffer_add_string(buffer, length_nombre_archivo, nombre_archivo);
    return buffer;
}

t_buffer* ejecutar_io_fs_delete(char* interfaz, char* nombre_archivo) {
    uint32_t length_interfaz = strlen(interfaz) + 1;
    uint32_t length_nombre_archivo = strlen(nombre_archivo) + 1;
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + length_interfaz + sizeof(uint32_t) + length_nombre_archivo);
    buffer_add_string(buffer, length_interfaz, interfaz);
    buffer_add_string(buffer, length_nombre_archivo, nombre_archivo);
    return buffer;
}

t_buffer* ejecutar_io_fs_truncate(char* interfaz, char* nombre_archivo, int registro_tamanio) {
    uint32_t length_interfaz = strlen(interfaz) + 1;
    uint32_t length_nombre_archivo = strlen(nombre_archivo) + 1;
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + length_interfaz + sizeof(uint32_t) + length_nombre_archivo + sizeof(int));
    buffer_add_string(buffer, length_interfaz, interfaz);
    buffer_add_string(buffer, length_nombre_archivo, nombre_archivo);
    buffer_add_int(buffer, registro_tamanio);
    return buffer;
}

t_buffer* ejecutar_io_fs_write(uint32_t pid, char* interfaz, char* nombre_archivo, int direccion_logica, int registro_tamanio, int registro_puntero_archivo) {
    uint32_t length_interfaz = strlen(interfaz) + 1;
    uint32_t length_nombre_archivo = strlen(nombre_archivo) + 1;

    t_list* peticiones = obtener_direcciones_logicas_lectura(pid, direccion_logica, registro_tamanio);
    int sizeTotalLista = sizeTotalDeLista(peticiones);
    int sizeLista = list_size(peticiones);
    t_buffer* buffer = buffer_create(sizeof(uint32_t) 
    + length_interfaz 
    + sizeof(uint32_t) 
    + length_nombre_archivo 
    + 3 * sizeof(int)
    + sizeTotalLista);

    buffer_add_string(buffer, length_interfaz, interfaz);
    buffer_add_string(buffer, length_nombre_archivo, nombre_archivo);
    buffer_add_int(buffer, direccion_logica);
    buffer_add_int(buffer, registro_tamanio);
    buffer_add_int(buffer, registro_puntero_archivo);
    buffer_add_lista(buffer, sizeLista, peticiones);

    return buffer;
}

t_buffer* ejecutar_io_fs_read(uint32_t pid,char* interfaz, char* nombre_archivo, int direccion_logica, int registro_tamanio, int registro_puntero_archivo) {
    uint32_t length_interfaz = strlen(interfaz) + 1;
    uint32_t length_nombre_archivo = strlen(nombre_archivo) + 1;

    t_list* peticiones = obtener_direcciones_logicas_escritura_stdin(pid, direccion_logica, registro_tamanio);
    int listaSizeTotal = sizeTotalDeLista(peticiones);
    int listaSize = list_size(peticiones);

    t_buffer* buffer = buffer_create(sizeof(uint32_t) 
    + length_interfaz 
    + sizeof(uint32_t) 
    + length_nombre_archivo 
    + 3 * sizeof(int)
    + listaSizeTotal);

    buffer_add_string(buffer, length_interfaz, interfaz);
    buffer_add_string(buffer, length_nombre_archivo, nombre_archivo);
    buffer_add_int(buffer, direccion_logica);
    buffer_add_int(buffer, registro_tamanio);
    buffer_add_int(buffer, registro_puntero_archivo);
    buffer_add_lista(buffer, listaSize, peticiones);

    return buffer;
}


void desalojar_pcb(int socket_dispatch, t_PCB* pcb, int motivo, t_log* logger, t_dictionary* diccionario){
    pcb->registrosCPU = registros_cpu_from_dictionary(diccionario);
    responder_pcb(socket_dispatch, pcb, logger);
    send(socket_dispatch, &motivo, sizeof(int), 0);
}

void registros_cpu_dictionary(t_registrosCPU registros, t_dictionary* dictionary){
    dictionary_put(dictionary, "AX", (void*)(registros.ax));
    dictionary_put(dictionary, "BX", (void*)(registros.bx));
    dictionary_put(dictionary, "CX", (void*)(registros.cx));
    dictionary_put(dictionary, "DX", (void*)(registros.dx));
    dictionary_put(dictionary, "DI", (void*)(registros.di));
    dictionary_put(dictionary, "SI", (void*)(registros.si));
    dictionary_put(dictionary, "PC", (void*)(registros.pc));
    dictionary_put(dictionary, "EAX", (void*)(registros.eax));
    dictionary_put(dictionary, "EBX", (void*)(registros.ebx));
    dictionary_put(dictionary, "ECX", (void*)(registros.ecx));
    dictionary_put(dictionary, "EDX", (void*)(registros.edx));
}

t_registrosCPU registros_cpu_from_dictionary(t_dictionary* dictionary){
    t_registrosCPU registros = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    registros.ax = (uint8_t)dictionary_get(dictionary, "AX");
    registros.bx = (uint8_t)dictionary_get(dictionary, "BX");
    registros.cx = (uint8_t)dictionary_get(dictionary, "CX");
    registros.dx = (uint8_t)dictionary_get(dictionary, "DX");
    registros.di = (uint32_t)dictionary_get(dictionary, "DI");
    registros.si = (uint32_t)dictionary_get(dictionary, "SI");
    registros.pc = (uint32_t)dictionary_get(dictionary, "PC");
    registros.eax = (uint32_t)dictionary_get(dictionary, "EAX");
    registros.ebx = (uint32_t)dictionary_get(dictionary, "EBX");
    registros.ecx = (uint32_t)dictionary_get(dictionary, "ECX");
    registros.edx = (uint32_t)dictionary_get(dictionary, "EDX");

    return registros;
}


int obtener_direccion_fisica(int pid, int direccion_logica){
    bool tlb_hit = false;
    int direccion_fisica;

    if(tlb_enabled())
        tlb_hit = tlb_get_marco(pid, direccion_logica, &direccion_fisica);

    if(!tlb_hit){
        direccion_fisica = traducir_a_direccion_fisica(pid, direccion_logica);

        if(tlb_enabled())
            tlb_add(pid, direccion_logica, direccion_fisica);
    }

    return direccion_fisica;
}