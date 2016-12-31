compressRworkermake: compressR_LOLS.c compressR_LOLS.h compressR_worker_LOLS.c compressT_LOLS.c compressT_LOLS.h
	gcc -Wall compressR_LOLS.c -o compressR_LOLS
	gcc -Wall compressR_worker_LOLS.c -o compressRworker
	gcc -Wall compressT_LOLS.c -o compressT_LOLS -pthread

clean :
	rm compressR_LOLS
	rm compressRworker
	rm compressT_LOLS