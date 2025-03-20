BIN = philo
BIN_BONUS = philo_bonus

CC = gcc
C_FLAGS = -Werror -Wextra -Wall -g3

ifeq ($(SANITIZE), 1)
  C_FLAGS += -fsanitize=address
endif

V_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose

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

gdb:: SANITIZE = 1
gdb:: $(BIN)
	gdb ./$(BIN)

valgrind:: SANITIZE = 0
valgrind:: $(BIN)
	@bash -c 'read -p "Enter philo arguments: " args; \
	echo "Running: valgrind $(V_FLAGS) ./$(BIN) $$args"; \
	valgrind $(V_FLAGS) ./$(BIN) $$args'
clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(BIN) $(BIN_BONUS)

re: fclean all

.PHONY: all bonus clean fclean re gdb valgrind
