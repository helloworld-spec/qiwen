CC = gcc
CFLAGS = -c -Wall 
LDFLAGS =
CC_INCLUDE := -I../../smac/hal/ssv6006/

SRV_SOURCES=icomm_osif_hal_main.c
SRV_OBJECTS=$(SRV_SOURCES:.c=.o)
SRV_EXECUTABLE=icomm_osif_hal_main

CLI_SOURCES:=icomm_osif_hal_util.c 
CLI_OBJECTS=$(CLI_SOURCES:.c=.o)
CLI_EXECUTABLE=icomm_osif_hal_util
    

do_all:
	make all 2>&1 |tee test.log

all:  client server

server: $(SRV_EXECUTABLE)

$(SRV_EXECUTABLE): $(SRV_OBJECTS) 
	$(CC) $(LDFLAGS) $(SRV_SOURCES) -lpthread -o $@

client: $(CLI_SOURCES) $(CLI_EXECUTABLE)

$(CLI_EXECUTABLE): $(CLI_OBJECTS) 
	$(CC) $(CC_INCLUDE) $(LDFLAGS) $(CLI_OBJECTS) -lpthread -o $(CLI_EXECUTABLE) -ldl

.c.o:
	$(CC) $(CFLAGS) $(CC_INCLUDE) $(LDFLAGS) $< -o $@

clean:
	rm *o icomm_osif_hal_main icomm_osif_hal_util test.log



