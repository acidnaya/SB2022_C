SOURCES += main.c

NAME = ff

all: $(NAME)

$(NAME):
	@gcc -o $(NAME) $(SOURCES)

clean:
	@rm -f $(NAME)

re: clean all