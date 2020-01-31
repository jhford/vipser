#ifndef VIPSER_IO_H
#define VIPSER_IO_H

#define BUFFER_SIZE 4096
RESULT read_all(int fd, size_t *size, char **out);

#endif //VIPSER_IO_H
