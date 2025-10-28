#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* cpuinfo() {
    char* string = malloc(61);
    int eax, ebx, edx, ecx;

    asm volatile (
        "mov $0x0, %%eax\n\t"
        "cpuid"
        :"=b" (ebx),
         "=d" (edx),
         "=c" (ecx)
        :
        :"cc"
    );
    memcpy(string, &ebx, 4);
    memcpy(string + 4, &edx, 4);
    memcpy(string + 8, &ecx, 4);

    int offset = 12;
    unsigned int address = 0x80000002;

    for (int i = 0; i < 3; i++) {
        asm volatile (
        "mov %[input], %%eax\n\t"
        "cpuid"
        :"=a" (eax),
         "=b" (ebx),
         "=d" (edx),
         "=c" (ecx)
        :[input] "r" (address)
        :"cc"
        );
        address++;

        memcpy(string + offset, &eax, 4);
        offset = offset + 4;
        memcpy(string + offset, &ebx, 4);
        offset = offset + 4;
        memcpy(string + offset, &ecx, 4);
        offset = offset + 4;
        memcpy(string + offset, &edx, 4);
        offset = offset + 4;

    }
    
    string[60] = 0;
    return string;
}

int main() {
    char* output = cpuinfo();
    printf("%.12s\n", output);
    printf("%s\n", output + 12);
    free(output);
}