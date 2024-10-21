#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <elf.h>

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *program_header = (Elf32_Phdr *)(map_start + elf_header->e_phoff);
    int phnum = elf_header->e_phnum;

    for (int i = 0; i < phnum; i++) {
        func(&program_header[i], arg);
    }

    return 0;
}

void print_phdr(Elf32_Phdr *phdr, int arg) {
    printf("Program Header:\n");
    printf("  Type: %d\n", phdr->p_type);
    printf("  Offset: 0x%x\n", phdr->p_offset);
    printf("  Virtual Address: 0x%x\n", phdr->p_vaddr);
    printf("  Physical Address: 0x%x\n", phdr->p_paddr);
    printf("  File Size: %d\n", phdr->p_filesz);
    printf("  Memory Size: %d\n", phdr->p_memsz);
    printf("  Flags: %d\n", phdr->p_flags);
    printf("  Align: %d\n", phdr->p_align);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ELF file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    void *map_start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    foreach_phdr(map_start, print_phdr, 0);

    munmap(map_start, file_size);
    close(fd);

    return 0;
}
