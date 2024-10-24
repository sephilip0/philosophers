NAME = philo
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

OBJ_PATH = obj/

SRC = main.c aux.c

OBJ = $(SRC:.c=.o)
OBJS = $(addprefix $(OBJ_PATH), $(OBJ))

all: $(OBJ_PATH) $(NAME) 

$(OBJ_PATH):
	mkdir $(OBJ_PATH)

$(OBJ_PATH)%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

#	PUTTING THE OBJECTS IN THEIR FOLDERS
#	-c ==> don't link sorce file to executable, yet.
#	$< ==> first prerequisite file, something.c
#	$@ ==> the target, objects.o


$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)
#	we link them here

clean:
	rm -rf $(OBJ_PATH)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
