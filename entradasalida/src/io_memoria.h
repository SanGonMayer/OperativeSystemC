#ifndef IO_MEMORIA_H_
#define IO_MEMORIA_H_ 

char* leer_de_memoria(int socket_memoria,int tamanio, t_list* peticionesMemoria);
void guardar_en_memoria(int socket_memoria, char* texto, t_list* peticionesMemoria);

#endif