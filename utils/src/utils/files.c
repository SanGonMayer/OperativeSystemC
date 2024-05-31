#include "files.h"

bool abrir_archivo_txt(char* path, FILE** archivo){
    *archivo = fopen(path, "r+");
    if(archivo == NULL){
        return false;
    }
    return true;
}

char* leer_archivo_txt(char* path){
    FILE* archivo;
    if(!abrir_archivo_txt(path, &archivo)){
        return NULL;
    }


    fseek(archivo, 0, SEEK_END);
    long int tamanio = ftell(archivo);
    rewind(archivo);
    char* buffer = malloc(tamanio + 1);
    fread(buffer, tamanio, 1, archivo);
    buffer[tamanio] = '\0';
    fclose(archivo);
    return buffer;
}