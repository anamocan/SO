CC = gcc
CFLAGS = -Wall -g

# Targets
TM_TARGET = treasure_manager
MONITOR_TARGET = treasure_monitor
HUB_TARGET = treasure_hub
SCORECALC_TARGET = calculate_score

# Object files
TM_OBJS = treasure_manager.o
MONITOR_OBJS = treasure_monitor.o treasure_manager.o
HUB_OBJS = treasure_hub.o
SCORECALC_OBJS = calculate_score.o treasure_manager.o

.PHONY: all clean rebuild

all: $(TM_TARGET) $(MONITOR_TARGET) $(HUB_TARGET) $(SCORECALC_TARGET)

$(TM_TARGET): main.o $(TM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(MONITOR_TARGET): $(MONITOR_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(HUB_TARGET): $(HUB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SCORECALC_TARGET): $(SCORECALC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c treasure_manager.h
	$(CC) $(CFLAGS) -c main.c

treasure_manager.o: treasure_manager.c treasure_manager.h
	$(CC) $(CFLAGS) -c treasure_manager.c

treasure_monitor.o: treasure_monitor.c treasure_manager.h
	$(CC) $(CFLAGS) -c treasure_monitor.c

treasure_hub.o: treasure_hub.c hub.h
	$(CC) $(CFLAGS) -c treasure_hub.c

calculate_score.o: calculate_score.c treasure_manager.h
	$(CC) $(CFLAGS) -c calculate_score.c

clean:
	rm -f *.o $(TM_TARGET) $(MONITOR_TARGET) $(HUB_TARGET) $(SCORECALC_TARGET)

rebuild: clean all