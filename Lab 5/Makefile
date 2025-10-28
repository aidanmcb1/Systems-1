lab5:main.o quicksort_unoptimized.o quicksort_optimized_CMOV.o quicksort_optimized_XOR.o
	gcc -o3 -o lab5-gcc main.o quicksort_unoptimized.o quicksort_optimized_CMOV.o quicksort_optimized_XOR.o
	clang -o3 -o lab5-clang main.o quicksort_unoptimized.o quicksort_optimized_CMOV.o quicksort_optimized_XOR.o

main.o:main.c quicksort_unoptimized.h
	gcc -o3 -c main.c
	clang -o3 -c main.c

quicksort_unoptimized.o:quicksort_unoptimized.c quicksort_unoptimized.h
	gcc -o3 -c quicksort_unoptimized.c
	clang -o3 -c quicksort_unoptimized.c

quicksort_optimized_CMOV.o:quicksort_optimized_CMOV.c quicksort_optimized_CMOV.h
	gcc -o3 -c quicksort_optimized_CMOV.c
	clang -o3 -c quicksort_optimized_CMOV.c

quicksort_optimized_XOR.o:quicksort_optimized_XOR.c quicksort_optimized_XOR.h
	gcc -o3 -c quicksort_optimized_XOR.c
	clang -o3 -c quicksort_optimized_XOR.c