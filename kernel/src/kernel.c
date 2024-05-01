#include "kernel.h"
#include <string.h>
#include<readline/readline.h>


//aca recibe manda mensaje a memoria y recibe direccion
uint32_t enviar_path_a_memoria(char* path){
    return 5;
}

void iniciar_proceso(char* path, t_queue* cola_new, t_list* lista_paths, int* contadorPID){
    t_PCB* PCB = crear_PCB();
    pcb -> PID = *contadorPID;
    pcb -> estado = NEW;
    cargar_lista_paths(*contadorPID, lista_paths);
    queue_push(cola_new, PCB);
}

void cargar_lista_paths(int contadorPID,t_list* lista_paths, char* path){
    t_listaPID contenido = malloc(sizeof(t_listaPID));
    contenido -> PID = contadorPID;
    contenido -> path = path;
    list_add(lista_paths, contenido);
}

void enviar_proceso_a_ready(t_queue* cola_new, t_queue* cola_ready, t_list* lista_paths){
        t_PCB* pcb = queue_pop(cola_new);
        queue_push(cola_ready, pcb);
        //pidProceso = pcb -> pid;
        //list_find(lista_paths, )
        //la linea de abajo hay que cambiarla
        pcb->registrosMem.codigo = enviar_path_a_memoria(pcb->path);
        pcb-> estado = READY;
}

void ejecutar_cpu_FIFO(t_PCB* pcb, int conexion_cpu_dispatch, t_log* logger){
    t_PCB* pcb_auxiliar = crear_PCB();
    int err;
    
    err = enviar_pcb(conexion_cpu_dispatch, pcb);
    //log_info(logger, "PCB ENVIADA: %d", pcb->PID);
    
    err = recibir_pcb(conexion_cpu_dispatch, pcb_auxiliar);
    
    actualizar_pcb(pcb, pcb_auxiliar);
    //log_info(logger, "PCB RECIBIDA: %d", pcb->PID);
    free(pcb_auxiliar);
}

void consola_interactiva(t_log *logger){
    
    char* linea_leida;
    char * parametro;
    char *funcion;

	linea_leida = readline(">");

	while (strcmp(linea_leida, "q")){

    //falta cortar el string para sacar el path
    funcion = strtok(linea_leida," ");

    parametro = strtok(NULL," "); //tendrá el valor del pid o el path en caso que corresponda, en caso de que lafuncion sea si parametro va a tener el valor null

    int opcion_funciones_consola;

    if (strcmp(linea_leida, "EJECUTAR_SCRIPT") == 0) {
        opcion_funciones_consola = EJECUTAR_SCRIPT;
    } else if (strcmp(linea_leida, "INICIAR_PROCESO") == 0) {
        opcion_funciones_consola = INICIAR_PROCESO;
    } else if (strcmp(linea_leida, "FINALIZAR_PROCESO") == 0) {
        opcion_funciones_consola = FINALIZAR_PROCESO;
    } else if (strcmp(linea_leida, "DETENER_PLANIFICACION") == 0) {
        opcion_funciones_consola = DETENER_PLANIFICACION;
    } else if (strcmp(linea_leida, "INICIAR_PLANIFICACION") == 0) {
        opcion_funciones_consola = INICIAR_PLANIFIACION;
    } else if (strcmp(linea_leida, "MULTIPROGRAMACION") == 0) {
        opcion_funciones_consola = MULTIPROGRAMACION;
    } else if (strcmp(linea_leida, "PROCESO_ESTADO") == 0) {
        opcion_funciones_consola = PROCESO_ESTADO;
    }else 
        log_error(logger, "ingresaste una funcion no valida");

    switch (opcion_funciones_consola) {

        case EJECUTAR_SCRIPT:
            printf("Se seleccionó iniciar  ejecutar script, con el path: %s\n", parametro);
            break;
        case INICIAR_PROCESO:
            printf("Se seleccionó iniciar proceso, con el path: %s\n", parametro);
            break;
        case FINALIZAR_PROCESO:
            printf("Se seleccionó finalizar proceso, con el PID: %s\n", parametro);
            break;
        case DETENER_PLANIFICACION:
            printf("Se seleccionó detener planificacion\n");
            break;
        case INICIAR_PLANIFIACION:
            printf("Se seleccionó iniciar planificacion\n");
            break;
        case MULTIPROGRAMACION:
            printf("Se seleccionó multiprogramacion con el valor: %s\n", parametro);
            break;
        case PROCESO_ESTADO:
            printf("Se seleccionó proceso estado\n");
            break;
    }


        log_info(logger, "me llego la instruccion: %s", linea_leida);
		//log_info(logger, linea_leida);

		linea_leida = readline(">");

        
	}

    log_info(logger, "se termino la terminal XD");

    free(linea_leida);

    return 0;
}