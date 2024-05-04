#include "cpu.h"

t_log* logger;
t_config* config;

char* ip_memoria;
char* puerto_memoria;
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;
uint32_t cantidad_entradas_tlb;
char* algoritmo_tlb;

void servidor_dispatch(){
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
        case 2:
            t_buffer* buffer = malloc(sizeof(t_buffer));
            buffer->stream = recibir_buffer(&buffer->size, cliente_dispatch_fd);
            buffer->offset = 0;
            t_PCB* pcb = deserializar_pcb(buffer);
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

    pthread_t hilo_dispatch;
    pthread_t hilo_interrupt;

    pthread_create(&hilo_dispatch, NULL, (void*)servidor_dispatch, NULL);
    pthread_create(&hilo_interrupt, NULL, (void*)servidor_interrupt, NULL);

    pthread_join(hilo_dispatch, NULL);
    pthread_join(hilo_interrupt, NULL);
    
    close(conexion_memoria_fd);
    config_destroy(config);
    log_destroy(logger);

    return 0;
}


