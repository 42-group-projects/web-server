NAME		=	webserver
INCLUDES	=	-I include

CXXFLAGS 	= 	-std=c++98 $(INCLUDES) -Wall -Werror -Wextra

OBJ_DIR 	=	obj
SRC_DIR 	=	src
SRCS		=	$(shell find $(SRC_DIR) -name '*.cpp')
OBJS		=	$(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
RM			=	rm -rf
CXX			=	c++

# Colors
DEF_COLOR = \033[0;39m
GRAY = \033[0;90m
RED = \033[0;91m
GREEN = \033[0;92m
YELLOW = \033[0;93m
BLUE = \033[0;94m
MAGENTA = \033[0;95m
CYAN = \033[0;96m
WHITE = \033[0;97m


all:$(NAME)

$(NAME): $(OBJS)
	@printf "\r$(GREEN)[%2d/%2d]$(DEF_COLOR) All files compiled! Linking $(CYAN)$(NAME)$(DEF_COLOR)...\\n\033[K" \
		"$$(find $(SRCS) -type f | wc -l)" "$$(find $(SRCS) -type f | wc -l)"
	@$(CXX) $(OBJS) -o $(NAME)
	@echo "\n$(GREEN)<-----------------$(NAME) compiled!----------------->$(DEF_COLOR)\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	@printf "\r$(GREEN)[%2d/%2d]$(DEF_COLOR) Compiling $(NAME): $(CYAN)%s$(DEF_COLOR)\033[K" \
		"$$(find $(OBJ_DIR) -type f | wc -l)" "$$(find $(SRCS) -type f | wc -l)" "$<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(RM) $(OBJ_DIR)
	@echo "$(CYAN)<-----------$(NAME) object files cleaned!----------->$(DEF_COLOR)\n"

fclean:
	@$(RM) $(OBJ_DIR) $(NAME)
	@echo "\n$(CYAN)<---------$(NAME) executable files cleaned!--------->$(DEF_COLOR)\n"

re: fclean all

.PHONY: all clean fclean re