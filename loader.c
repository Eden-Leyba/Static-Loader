#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <elf.h>
#include <errno.h>

//2c
extern int startup(int argc, char **argv, void (*start)());

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
    const char *type;
    switch (phdr->p_type) {
        case PT_NULL: type = "NULL"; break;
        case PT_LOAD: type = "LOAD"; break;
        case PT_DYNAMIC: type = "DYNAMIC"; break;
        case PT_INTERP: type = "INTERP"; break;
        case PT_NOTE: type = "NOTE"; break;
        case PT_SHLIB: type = "SHLIB"; break;
        case PT_PHDR: type = "PHDR"; break;
        default: type = "UNKNOWN"; break;
    }

    //protection flags
    int prot = 0;
    if (phdr->p_flags & PF_R) 
        prot |= PROT_READ;
    if(phdr->p_flags & PF_W)
        prot |= PROT_WRITE;
    if(phdr->p_flags & PF_X)
        prot |= PROT_EXEC;
    
    int flags = MAP_PRIVATE;
    if(phdr->p_type == PT_LOAD)
        flags |= MAP_FIXED;

    printf("%-8s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %c%c%c 0x%x\n",
           type,
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz,
           (phdr->p_flags & PF_R) ? 'R' : ' ',
           (phdr->p_flags & PF_W) ? 'W' : ' ',
           (phdr->p_flags & PF_X) ? 'E' : ' ',
           phdr->p_align
           );
}

//2b
void load_phdr(Elf32_Phdr *phdr, int fd) {
    if(phdr->p_type != PT_LOAD)
        return;

    int prot = PROT_NONE;
    if (phdr->p_flags & PF_R) prot |= PROT_READ;
    if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot |= PROT_EXEC;

    int flags;
    
    if(phdr->p_flags & 0b110)
        flags = MAP_PRIVATE;
    else
        flags = MAP_FIXED;
        
    
    void* vaddr = (void*) (phdr->p_vaddr&0xfffff000);
    int offset = phdr->p_offset&0xfffff000;
    int padding = phdr->p_vaddr & 0xfff;

    void *addr = mmap(vaddr, phdr->p_memsz+padding, prot, flags, fd, offset);

    if(addr == MAP_FAILED) {
        printf("Error: mmap");
    
        switch (errno) {
            case EACCES:
                printf("EACCES\n");
                break;
            case EAGAIN:
                printf("EAGAIN\n");
                break;
            case EBADF:
                printf("EBADF\n");
                break;
            case EINVAL:
                printf("EINVAL\n");
                break;
            case ENFILE:
                printf("ENFILE\n");
                break;
            case ENODEV:
                printf("ENODEV\n");
                break;
            case ENOMEM:
                printf("ENOMEM\n");
                break;
            case EPERM:
                printf("EPERM\n");
                break;
            case ETXTBSY:
                printf("ETXTBSY\n");
                break;
            default:
                printf("Unknown error\n");
                break;
        }
        exit(EXIT_FAILURE);
    }

    print_phdr(phdr, 0);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ELF file> <arguments>\n", argv[0]);
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
        perror("mmap error!");
        close(fd);
        return 1;
    }

    //2c
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    void (*entry_point)() = (void (*)())ehdr->e_entry;
    
    printf("Type     Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align \n");
    foreach_phdr(map_start, print_phdr, fd);
    foreach_phdr(map_start, load_phdr, fd);
    

    int result = startup(argc-1, argv+1, (void *)(ehdr->e_entry));
    printf("startup returned: %d\n", result);
    munmap(map_start, file_size);
    
    close(fd);

    return 0;
    
}