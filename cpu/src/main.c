#include "cpu.h"

t_log* logger;
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

int main(int argc, char* argv[]) {
    
    logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
    
    config = config_create("../cpu/cpu.config");
    
    if(config == NULL){
        log_error(logger, "Mal el path");
        exit(EXIT_FAILURE);
    }

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    cantidad_entradas_tlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    algoritmo_tlb = config_get_string_value(config, "ALGORITMO_TLB");

    int conexion_memoria_fd = crear_conexion(ip_memoria, puerto_memoria, "MEMORIA", logger);
    handshake_cliente(conexion_memoria_fd, logger);
    enviar_mensaje("Mensaje CPU a MEMORIA", conexion_memoria_fd);

    cola_interrupciones = queue_create();

    pthread_t hilo_dispatch;
    pthread_t hilo_interrupt;

    pthread_create(&hilo_dispatch, NULL, (void*)servidor_dispatch, conexion_memoria_fd);
    pthread_create(&hilo_interrupt, NULL, (void*)servidor_interrupt, NULL);

    pthread_join(hilo_dispatch, NULL);
    pthread_join(hilo_interrupt, NULL);
    
    close(conexion_memoria_fd);
    config_destroy(config);
    log_destroy(logger);

    return 0;
}

void servidor_dispatch(int socket_memoria){
    int socket_servidor = iniciar_servidor(puerto_escucha_dispatch, logger, "CPU DISPATCH");
    int cliente_dispatch_fd = esperar_cliente(socket_servidor, logger);
    handshake_server(cliente_dispatch_fd, logger);

    int conectado = 1;
    
    while(conectado){
        // agrego logs para ver si se conecta
        int cod_op = recibir_operacion(cliente_dispatch_fd);
        
        log_info(logger, "Recibí la operación %d", cod_op);

        switch (cod_op)
        {
        case 1:
            recibir_mensaje_logger(cliente_dispatch_fd, logger);
            break;
        case 2: // recibir PCB de Kernel para ejecutar
            t_PCB* pcb = recibir_pcb(cliente_dispatch_fd);
            t_dictionary* diccionario = dictionary_create();
            registros_cpu_dictionary(diccionario);
            ciclo_de_ejecucion(socket_memoria, pcb, logger, diccionario);
            
            break;
        default:
            log_info(logger, "No entiendo el mensaje");
            break;
        }
    }
    close(cliente_dispatch_fd);
}

void servidor_interrupt(){
    int socket_servidor = iniciar_servidor(puerto_escucha_interrupt, logger, "CPU INTERRUPT");
    int cliente_interrupt_fd = esperar_cliente(socket_servidor, logger);
    handshake_server(cliente_interrupt_fd, logger);

    int conectado = 1;
    while(conectado){

        int cod_op = recibir_operacion(cliente_interrupt_fd);
        
        switch (cod_op)
        {
        case 1:
            recibir_mensaje_logger(cliente_interrupt_fd, logger);
            break;
        case 2:
            uint32_t pidInterrupcion = recibir_interrupcion(cliente_interrupt_fd); //TODO
            queue_push(cola_interrupciones, pidInterrupcion);
            break;
        case -1:
            error_show("cliente desconectado de CPU interrupt");
            close(cliente_interrupt_fd);
            conectado = 0;
            break;
        default:
            log_info(logger, "No entiendo el mensaje");
            break;
        }
    }
}

void ciclo_de_ejecucion (int socket_dispatch, t_PCB* pcb, t_log* logger, t_dictionary* diccionario){
    char* instruccion;

    instruccion = etapa_fetch(socket_dispatch, pcb, logger);

    while (instruccion != NULL) {
        //etapa decode
        char** instruccion_separada = string_split(instruccion, " ");
        switch (instruccion_separada[0])
        {
        case "SET":
            char* registro = instruccion_separada[1];
            int valor = atoi(instruccion_separada[2]);
            //etapa execute
            ejecutar_set(registro, valor, pcb);
            break;
        case "SUM":
            char* registroDestino = instruccion_separada[1];
            char* registroValor = instruccion_separada[2];
            //etapa execute
            ejecutar_sum(registroDestino, registroValor, pcb);
            break;
        case "SUB":
            char* registroDestino = instruccion_separada[1];
            char* registroValor = instruccion_separada[2];
            //etapa execute
            ejecutar_sub(registroDestino, registroValor, pcb);
            break;
        case "JNZ":
            char* registro = instruccion_separada[1];
            int valorPC = atoi(instruccion_separada[2]);
            //etapa execute
            ejecutar_jnz(registro, valorPC, pcb);
            break;
        case "IO_GEN_SLEEP":
            char* dispositivo = instruccion_separada[1];
            int unidadesDeTrabajo = atoi(instruccion_separada[2]);
            desalojar_pcb(pcb, (int)IO_GEN_SLEEP, logger, diccionario);
            t_buffer* buffer = ejecutar_io_gen_sleep(dispositivo, unidadesDeTrabajo);
            enviar_buffer(socket_dispatch, buffer, logger);
            return;
        case "EXIT":
            desalojar_pcb(socket_dispatch, pcb, (int)FINALIZACION, logger, diccionario);
            return;
        default:
            break;
        }
        if(check_interrupt(pcb, logger) == 1){
            desalojar_pcb(socket_dispatch, pcb, (int)INTERRUPCION, logger, diccionario);
            return;
        }
        instruccion = etapa_fetch(socket, pcb, logger);
    }
}
