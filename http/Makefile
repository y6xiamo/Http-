.PHONY:clean

WORK_PATH=$(shell pwd)
BIN=httpd
SRC=httpd.c
CC= gcc
LDFLAGS=-lpthread -DDEBUG

.PHONY:all
all:$(BIN) cgi


$(BIN):$(SRC)
	$(CC) -o $@ $^ $(LDFLAGS)
cgi:
	@cd wwwRoot/connectSql; make clean; make; cd -
#	cd wwwRoot/tetris; make clean; make; cd 
#	cd wwwRoot/Tetris; make clean; make; cd 

.PHONY:output
output:
	@mkdir -p output/wwwRoot/cgi
	@cp $(BIN) output
	@cp -rf lib/lib output
	@cp start.sh output
	@cp -rf wwwRoot/*.html output/wwwRoot
	@cp -rf wwwRoot/imag output/wwwRoot
	@cp wwwRoot/cgi/netCal output/wwwRoot/cgi
clean:
	@rm -f $(BIN)
	@cd wwwRoot/connectSql; make clean; make; cd -
#	@cd wwwRoot/tetris; make clean; make; cd -

