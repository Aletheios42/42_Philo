BIN = philo
BIN_BONUS = philo_bonus

CC = gcc
C_FLAGS = -Werror -Wextra -Wall -g3
# C_FLAGS += -fsanitize=address


SRC_FILES = main.c		\
	    routines.c		\
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

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(BIN)

