lab7:main.o test.o editDistance.o
	clang -O3 -pthread -Wall -Wextra -Werror -mavx2 main.o test.o editDistance.o -o lab7

main.o:main.c editDistance.h
	clang -O3 -pthread -Wall -Wextra -Werror -c main.c

test.o:test.c editDistance.h
	clang -O3 -pthread -Wall -Wextra -Werror -c test.c

editDistance.o:editDistance.h editDistance.c
	clang -O3 -pthread -Wall -Wextra -Werror -mavx2 -c editDistance.c