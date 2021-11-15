setup:
	gcc -std=gnu99 -pthread -o line_processor line_processor.c

clean:
	rm line_processor