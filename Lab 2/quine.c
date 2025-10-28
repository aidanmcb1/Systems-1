#include <stdio.h>
char program[]="#include <stdio.h>%cchar program[]=%c%s%c;%c%cint main(){%cprintf(program, 10, 34, program, 34, 10, 10, 10, 10);%c}";

int main(){
printf(program, 10, 34, program, 34, 10, 10, 10, 10);
}