BIN = philo
BIN_BONUS = philo_bonus

CC = gcc
C_FLAGS = -Werror -Wextra -Wall 

ifeq ($(SANITIZE), 1)
  C_FLAGS += -fsanitize=address -g3
endif

V_FLAGS = --tool=helgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --log-file=logs --track-origins=yes --verbose


SRC_FILES = main.c		\
	    routines.c		\
	    monitor.c		\
	    init-free.c		\
	    print.c		\
	    utils.c		\

SOURCES = $(addprefix Src/, $(SRC_FILES))
OBJECTS = $(SOURCES:.c=.o)
INCLUDES = -IInc/

all: $(BIN)

$(BIN): $(SOURCES)
	$(CC) $(C_FLAGS) $(INCLUDES) $(SOURCES) -o $(BIN)

bonus: $(BIN_BONUS)

$(BIN_BONUS): $(SOURCES)
	$(CC) $(C_FLAGS) $(INCLUDES) $(SOURCES) -o $(BIN_BONUS)

test: $(BIN)
	python3 philo_checker.py ./$(BIN)

gdb: fclean
	$(MAKE) SANITIZE=1 $(BIN)
	gdb ./$(BIN)

valgrind: fclean
	$(MAKE) SANITIZE=0 $(BIN)
	@bash -c 'read -p "Enter philo arguments: " args; \
	valgrind $(V_FLAGS) ./$(BIN) $$args'

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(BIN) $(BIN_BONUS)

re: fclean all

.PHONY: all bonus clean fclean re gdb valgrind
