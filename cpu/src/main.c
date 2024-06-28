#include "cpu.h"
#include <commons/log.h>
#include <readline/chardefs.h>
#include <sys/socket.h>

t_config* config;

char* ip_memoria;
char* puerto_memoria;
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;
uint32_t cantidad_entradas_tlb;
char* algoritmo_tlb;

void servidor_dispatch();
void servidor_interrupt();

t_queue* cola_interrupciones;

sem_t mutex_cola_interrupciones;

sem_t g_actualizacion_pcb;


int main(int argc, char* argv[]) {
    
    g_logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
    
    config = config_create("../cpu/cpu.config");
    
    if(config == NULL){
        log_error(g_logger, "Mal el path");
        exit(EXIT_FAILURE);
    }

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    cantidad_entradas_tlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    algoritmo_tlb = config_get_string_value(config, "ALGORITMO_TLB");

    g_socket_memoria = crear_conexion(ip_memoria, puerto_memoria, "MEMORIA", g_logger);
    handshake_cliente(g_socket_memoria, g_logger);

    int op_tam_pagina = OBTENER_TAMANIO_PAGINA;
    send(g_socket_memoria, &op_tam_pagina, sizeof(int), 0);
    t_buffer* tam_pagina_response = recibir_buffer(g_socket_memoria);
    uint32_t tam_pagina = buffer_read_uint32(tam_pagina_response);
    buffer_destroy(tam_pagina_response);
    set_tamanio_pagina(tam_pagina);

    tlb_init(cantidad_entradas_tlb, algoritmo_tlb);

    cola_interrupciones = queue_create();

    sem_init(&mutex_cola_interrupciones, 0, 1);

    pthread_t hilo_dispatch;
    pthread_t hilo_interrupt;

    pthread_create(&hilo_dispatch, NULL, (void*)servidor_dispatch, &g_socket_memoria);
    pthread_create(&hilo_interrupt, NULL, (void*)servidor_interrupt, NULL);

    pthread_join(hilo_dispatch, NULL);
    pthread_join(hilo_interrupt, NULL);
    
    close(g_socket_memoria);
    config_destroy(config);
    log_destroy(g_logger);

    return 0;
}

t_interrupcion_dispatch* check_interrupt(t_PCB* pcb, t_log* logger){
    int hay_interrupcion = 0;

    while(queue_size(cola_interrupciones) > 0 && hay_interrupcion == 0){
        //Hacer mutex
        sem_wait(&mutex_cola_interrupciones);
        t_interrupcion_dispatch *interrupcion = queue_pop(cola_interrupciones);
        sem_post(&mutex_cola_interrupciones);
        if(interrupcion->pid == pcb->PID){
            return interrupcion;
        }
    }
    return NULL;
}

void ciclo_de_ejecucion(int socket_memoria,int socket_dispatch, t_PCB* pcb, t_log* logger, t_dictionary* diccionario){
    char* instruccion;
    log_info(logger, "Se inicia el ciclo de ejecucion");
    instruccion = etapa_fetch(socket_memoria, pcb, logger, diccionario);
    log_info(logger, "Se obtuvo la instruccion %s", instruccion);
    while (instruccion != NULL) {
        //etapa decode
        char** instruccion_separada = string_split(instruccion, " ");
        log_info(logger, "Se ejecuta la instruccion %s", instruccion_separada[0]);
        if (string_equals_ignore_case(instruccion_separada[0], "SET")) {
            char* registro = instruccion_separada[1];
            int valor = atoi(instruccion_separada[2]);
            //etapa execute
            ejecutar_set(registro, valor, pcb, diccionario);
            log_info(logger, "Se ejecuto SET en el registro %s y queda con valor %d", registro, dictionary_get(diccionario,registro));

        }else if(string_equals_ignore_case(instruccion_separada[0], "SUM")){
            char* registroDestino = instruccion_separada[1];
            char* registroValor = instruccion_separada[2];
            //etapa execute
            ejecutar_sum(registroDestino, registroValor, pcb, diccionario);
            log_info(logger, "Se ejecuto SUM quedando el %s con valor %d", registroDestino,dictionary_get(diccionario,registroDestino));

        }else if(string_equals_ignore_case(instruccion_separada[0], "SUB")){
            char* registroDestino = instruccion_separada[1];
            char* registroValor = instruccion_separada[2];
            //etapa execute
            ejecutar_sub(registroDestino, registroValor, pcb, diccionario);
            log_info(logger, "Se ejecuto SUB quedando el %s con valor %d", registroDestino,dictionary_get(diccionario,registroDestino));

        }else if(string_equals_ignore_case(instruccion_separada[0], "JNZ")){
            char* registro = instruccion_separada[1];
            int valorPC = atoi(instruccion_separada[2]);
            //etapa execute
            log_info(logger, "valor del PC antes de jnz %d", dictionary_get(diccionario,"PC"));
            ejecutar_jnz(registro, valorPC-1, pcb, diccionario);
            log_info(logger, "Se ejecuto JNZ %s %d", registro, valorPC);
            log_info(logger, "PC queda con valor %d", dictionary_get(diccionario,"PC"));
        }else if(string_equals_ignore_case(instruccion_separada[0], "IO_GEN_SLEEP")){
            char* dispositivo = instruccion_separada[1];
            int unidadesDeTrabajo = atoi(instruccion_separada[2]);
            desalojar_pcb(socket_dispatch,pcb, (int)IO_GEN_SLEEP, logger, diccionario);
            t_buffer* buffer = ejecutar_io_gen_sleep(dispositivo, unidadesDeTrabajo);
            enviar_buffer(socket_dispatch,buffer, logger);
            log_info(logger, "Se ejecuto IO_GEN_SLEEP %s %d", dispositivo, unidadesDeTrabajo);
            return;
        }else if(string_equals_ignore_case(instruccion_separada[0], "IO_STDIN_READ")){
            char* dispositivo = instruccion_separada[1];
            char* registro_direccion = instruccion_separada[2];
            char* registro_tamanio = instruccion_separada[3];

            desalojar_pcb(socket_dispatch,pcb, (int)IO_STDIN_READ, logger, diccionario);
            // Valores de direcciones
            int direccion_logica = (int)dictionary_get(diccionario, registro_direccion);
            int tamanio = (int)dictionary_get(diccionario, registro_tamanio);
            //Obtener direccion logicas
            int direccion_fisica = traducir_a_direccion_fisica(pcb->PID, direccion_logica);

            t_buffer* buffer = ejecutar_io_stdin_read(dispositivo, direccion_fisica, tamanio);
            enviar_buffer(socket_dispatch,buffer, logger);
            log_info(logger, "Se ejecuto IO_STDIN_READ %s %s %s", dispositivo, registro_direccion, registro_tamanio);

            return;
        }
        else if(string_equals_ignore_case(instruccion_separada[0], "IO_STDOUT_WRITE")){
            char* dispositivo = instruccion_separada[1];
            char* registro_direccion = instruccion_separada[2];
            char* registro_tamanio = instruccion_separada[3];

            desalojar_pcb(socket_dispatch,pcb, (int)IO_STDOUT_WRITE, logger, diccionario);
            // Valores de direcciones
            int direccion_logica = (int)dictionary_get(diccionario, registro_direccion);
            int tamanio = (int)dictionary_get(diccionario, registro_tamanio);
            //Obtener direccion logicas
            int direccion_fisica = traducir_a_direccion_fisica(pcb->PID, direccion_logica);

            t_buffer* buffer = ejecutar_io_stdout_write(dispositivo, direccion_fisica, tamanio);
            enviar_buffer(socket_dispatch,buffer, logger);
            log_info(logger, "Se ejecuto IO_STDOUT_WRITE %s %s %s", dispositivo, registro_direccion, registro_tamanio);
            buffer_destroy(buffer);
            return;
        }else if(string_equals_ignore_case(instruccion_separada[0], "MOV_IN")){
            //(Registro Datos, Registro Dirección)
            char* registro_datos = instruccion_separada[1];
            char* registro_direccion = instruccion_separada[2];

            int direccion_logica = (int)dictionary_get(diccionario, registro_direccion);
            
            ejecutar_mov_in(pcb->PID,registro_datos, direccion_logica, diccionario);
            
        } else if(string_equals_ignore_case(instruccion_separada[0], "MOV_OUT")){
            //(Registro Direccion, Registro Datos)
            char* registro_direccion = instruccion_separada[1];
            char* registro_datos = instruccion_separada[2];

            uint32_t valor = (uint32_t)dictionary_get(diccionario, registro_datos);
            int direccion_logica = (int)dictionary_get(diccionario, registro_direccion);

            ejecutar_mov_out(pcb->PID,direccion_logica, valor, diccionario);

        } else if(string_equals_ignore_case(instruccion_separada[0], "COPY_STRING")){
            //(Tamanio)
            int tamanio = atoi(instruccion_separada[1]);

            ejecutar_copy_string(tamanio, pcb->PID, diccionario);

        } else if(string_equals_ignore_case(instruccion_separada[0], "EXIT")){
            // desalojar_pcb(socket_dispatch,pcb, (int)FINALIZACION, logger, diccionario);
            desalojar_pcb(socket_dispatch,pcb, FINALIZACION, logger, diccionario);
            log_info(logger, "Se ejecuto EXIT");
            return;
        }else if(string_equals_ignore_case(instruccion_separada[0], "RESIZE")){
            
            int tamanio = atoi(instruccion_separada[1]);

            t_peticion_resize* peticion = crear_peticion_resize(pcb->PID, tamanio);
            t_buffer* buffer = serializar_peticion_resize(peticion);
            t_paquete* paquete = crear_paquete(RESIZE_MEMORIA, buffer);
            int err = enviar_paquete(paquete, g_socket_memoria);

            if(err == -1){
                log_error(logger, "Error al enviar la peticion de resize a memoria");
                return;
            }

            eliminar_paquete(paquete);
            destruir_peticion_resize(peticion);

            bool ok = recibir_ok(g_socket_memoria);

            log_info(logger, "Se ejecuto RESIZE %d", tamanio);
            if(!ok){
                t_codigo_error error = recibir_codigo_error(g_socket_memoria);
                if(error == ERROR_OUT_OF_MEMORY){
                    log_error(logger, "Error al hacer el resize, no hay memoria suficiente");
                    desalojar_pcb(socket_dispatch, pcb, (int)ERROR_OUT_OF_MEMORY, logger, diccionario);
                }
                return;
            }
        }else if(string_equals_ignore_case(instruccion_separada[0], "SIGNAL")){
            char* recurso = string_duplicate(instruccion_separada[1]);
            desalojar_pcb(socket_dispatch,pcb, (int)SIGNAL, logger, diccionario);
            
            //este buffer se le manda a kernel con el string del nombre del recurso pedido
            //el kerel solo lo recibe si se trata de un SIGNAL o un WAIT
            t_buffer* buffer = buffer_create(string_length(recurso)+ 1 + sizeof(uint32_t));
            buffer_add_string(buffer, string_length(recurso), recurso);

            enviar_buffer(socket_dispatch, buffer, logger);

            return;
        }
        else if(string_equals_ignore_case(instruccion_separada[0], "WAIT")){
            char* recurso = string_duplicate(instruccion_separada[1]);
            desalojar_pcb(socket_dispatch,pcb, (int)WAIT, logger, diccionario);
            
            //este buffer se le manda a kernel con el string del nombre del recurso pedido
            //el kerel solo lo recibe si se trata de un SIGNAL o un WAIT
            t_buffer* buffer = buffer_create(string_length(recurso) + 1 + sizeof(uint32_t));
            buffer_add_string(buffer, string_length(recurso), recurso);

            enviar_buffer(socket_dispatch, buffer, logger);
            return;
        } else if (string_equals_ignore_case(instruccion_separada[0], "IO_FS_CREATE")) {
            char* interfaz = instruccion_separada[1];
            char* nombre_archivo = instruccion_separada[2];

            desalojar_pcb(socket_dispatch, pcb, (int)IO_FS_CREATE, logger, diccionario);

            t_buffer* buffer = ejecutar_io_fs_create(interfaz, nombre_archivo);
            enviar_buffer(socket_dispatch, buffer, logger);
            log_info(logger, "Se ejecutó IO_FS_CREATE %s %s", interfaz, nombre_archivo);
            buffer_destroy(buffer);
            return;
        } else if (string_equals_ignore_case(instruccion_separada[0], "IO_FS_DELETE")) {
            char* interfaz = instruccion_separada[1];
            char* nombre_archivo = instruccion_separada[2];

            desalojar_pcb(socket_dispatch, pcb, (int)IO_FS_DELETE, logger, diccionario);
            
            t_buffer* buffer = ejecutar_io_fs_delete(interfaz, nombre_archivo);
            enviar_buffer(socket_dispatch, buffer, logger);
            log_info(logger, "Se ejecutó IO_FS_DELETE %s %s", interfaz, nombre_archivo);
            buffer_destroy(buffer);
            return;
        } else if (string_equals_ignore_case(instruccion_separada[0], "IO_FS_TRUNCATE")) {
            char* interfaz = instruccion_separada[1];
            char* nombre_archivo = instruccion_separada[2];
            char* registro_tamanio = instruccion_separada[3];

            desalojar_pcb(socket_dispatch, pcb, (int)IO_FS_TRUNCATE, logger, diccionario);

            int tamanio = (int)dictionary_get(diccionario, registro_tamanio);

            t_buffer* buffer = ejecutar_io_fs_truncate(interfaz, nombre_archivo, tamanio);
            enviar_buffer(socket_dispatch, buffer, logger);
            log_info(logger, "Se ejecutó IO_FS_TRUNCATE %s %s %s", interfaz, nombre_archivo, registro_tamanio);
            buffer_destroy(buffer);
            return;
        } else if (string_equals_ignore_case(instruccion_separada[0], "IO_FS_WRITE")) {
            char* interfaz = instruccion_separada[1];
            char* nombre_archivo = instruccion_separada[2];
            char* registro_direccion = instruccion_separada[3];
            char* registro_tamanio = instruccion_separada[4];
            char* registro_puntero_archivo = instruccion_separada[5];

            desalojar_pcb(socket_dispatch, pcb, (int)IO_FS_WRITE, logger, diccionario);

            int direccion = (int)dictionary_get(diccionario, registro_direccion);
            int tamanio = (int)dictionary_get(diccionario, registro_tamanio);
            int puntero_archivo = (int)dictionary_get(diccionario, registro_puntero_archivo);

            t_buffer* buffer = ejecutar_io_fs_write(interfaz, nombre_archivo, direccion, tamanio, puntero_archivo);
            enviar_buffer(socket_dispatch, buffer, logger);
            log_info(logger, "Se ejecutó IO_FS_WRITE %s %s %s %s %s", interfaz, nombre_archivo, registro_direccion, registro_tamanio, registro_puntero_archivo);
            buffer_destroy(buffer);
        return;
        } else if (string_equals_ignore_case(instruccion_separada[0], "IO_FS_READ")) {
            char* interfaz = instruccion_separada[1];
            char* nombre_archivo = instruccion_separada[2];
            char* registro_direccion = instruccion_separada[3];
            char* registro_tamanio = instruccion_separada[4];
            char* registro_puntero_archivo = instruccion_separada[5];

            desalojar_pcb(socket_dispatch, pcb, (int)IO_FS_READ, logger, diccionario);

            int direccion = (int)dictionary_get(diccionario, registro_direccion);
            int tamanio = (int)dictionary_get(diccionario, registro_tamanio);
            int puntero_archivo = (int)dictionary_get(diccionario, registro_puntero_archivo);

            t_buffer* buffer = ejecutar_io_fs_read(interfaz, nombre_archivo, direccion, tamanio, puntero_archivo);
            enviar_buffer(socket_dispatch, buffer, logger);
            log_info(logger, "Se ejecutó IO_FS_READ %s %s %s %s %s", interfaz, nombre_archivo, registro_direccion, registro_tamanio, registro_puntero_archivo);
            buffer_destroy(buffer);
            return;
        }

        t_interrupcion_dispatch* interrupcion = check_interrupt(pcb, logger);

        if(interrupcion != NULL){
            log_info(g_logger, "Se detecto una interrupcion");
            desalojar_pcb(socket_dispatch, pcb, (int)interrupcion->motivo, logger, diccionario);
            return;
        }
        instruccion = etapa_fetch(socket_memoria, pcb, logger, diccionario);
    }
}

void servidor_dispatch(int* socket_memoria){
    int socket_servidor = iniciar_servidor(puerto_escucha_dispatch, g_logger, "CPU DISPATCH");
    int cliente_dispatch_fd = esperar_nueva_conexion_cliente(socket_servidor, g_logger);
    handshake_server(cliente_dispatch_fd, g_logger);

    int conectado = 1;
    
    while(conectado){
        // agrego logs para ver si se conecta
        int cod_op = recibir_operacion(cliente_dispatch_fd);
        
        log_info(g_logger, "Recibí la operación %d", cod_op);

        switch (cod_op)
        {
            case ENVIO_PCB: // recibir PCB de Kernel para ejecutar
                log_info(g_logger, "Listo para recibir un PCB de Kernel");
                t_PCB* pcb = recibir_pcb(cliente_dispatch_fd);
                log_info(g_logger, "Recibí el PCB con PID %d", pcb->PID);
                t_dictionary* diccionario = dictionary_create();
                registros_cpu_dictionary(pcb->registrosCPU ,diccionario);
                ciclo_de_ejecucion(*socket_memoria,cliente_dispatch_fd, pcb, g_logger, diccionario);
                
                dictionary_destroy(diccionario);
            break;
            
            default:
            log_info(g_logger, "No entiendo el mensaje");
            break;
        }
    }
    close(cliente_dispatch_fd);
}

void servidor_interrupt(){
    int socket_servidor = iniciar_servidor(puerto_escucha_interrupt, g_logger, "CPU INTERRUPT");
    int cliente_interrupt_fd = esperar_nueva_conexion_cliente(socket_servidor, g_logger);
    handshake_server(cliente_interrupt_fd, g_logger);

    int conectado = 1;
    while(conectado){

        int cod_op = recibir_operacion(cliente_interrupt_fd);
        
        switch (cod_op)
        {
        case ENVIO_INTERRUPCION:
            t_interrupcion_dispatch* interrupcion = recibir_interrupcion(cliente_interrupt_fd);
            sem_wait(&mutex_cola_interrupciones);
            queue_push(cola_interrupciones, interrupcion);
            sem_post(&mutex_cola_interrupciones);
            break;
        case -1:
            error_show("cliente desconectado de CPU interrupt");
            close(cliente_interrupt_fd);
            conectado = 0;
            break;
        default:
            log_info(g_logger, "No entiendo el mensaje");
            break;
        }
    }
}