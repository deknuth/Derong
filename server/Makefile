ORACLE_INCLUDES = /opt/oracle/oracle/product/11.2.0/rdbms/public
ORACLE_LIBS     = /opt/oracle/oracle/product/11.2.0/lib
CC = gcc
XX = g++
CFLAGS = -Wall -O -I$(ORACLE_INCLUDES)  -L$(ORACLE_LIBS)
CTD = -I$(ORACLE_INCLUDES)  -L$(ORACLE_LIBS)
TARGET = robot
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 
%.o:%.cpp
	$(XX) $(CFLAGS) -c $< -o $@ 
SOURCES = $(wildcard *.c ./src/*.c)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
$(TARGET) : $(OBJS)
	$(XX) $(OBJS) $(CTD) -o $(TARGET) -lpthread -lm -g -lc -rdynamic 
	chmod a+x $(TARGET)
clean:
	rm -rf $(wildcard demo *.log *.o ./src/*.o)
