#ifndef IO_DIALFS_H
#define IO_DIALFS_H

void initialize_fs();
void finalize_fs();
void io_fs_create(const char* filename);
void io_fs_delete(const char* filename);
void io_fs_truncate(const char* filename, int new_size);
void io_fs_write(const char* filename, const char* data, int size, int offset);
void io_fs_read(const char* filename, char* buffer, int size, int offset);

#endif // IO_DIALFS_H