CC=clang
CFLAGS=-c -O2 -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
LDFLAGS=
SOURCES=main.c map_file.c parse_time.c file_scan.c console_output.c
OBJECTS=$(SOURCES:.c=.o)
OUTFILE=tgrep

all: $(SOURCES) $(OUTFILE)
	
$(OUTFILE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o $(OUTFILE)
