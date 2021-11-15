setup:
	gcc -std=gnu99 -pthread -o line_processor main.c

clean:
	rm line_processor