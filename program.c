#include <stdio.h>

int main(int argc, char const *argv[]) {
    if(argc < 2)
        printf("You didn't say anything\n %s", argv[0]);
    else
        printf("You said: %s", argv[1]); 
    return 0;
}
