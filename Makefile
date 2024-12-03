# Program name
NAME		= webserver

# Compiler
CC			= c++

# Compilation flags
CFLAGS		= -Wall -Wextra -Werror -std=c++11

# Directories
SRC_DIR		= src
OBJ_DIR		= obj

# Source files
SRCS		=	$(SRC_DIR)/Config.cpp \
				$(SRC_DIR)/ServerBlock.cpp \
				$(SRC_DIR)/HTTPRequest.cpp \
				$(SRC_DIR)/Server.cpp \
				$(SRC_DIR)/methods.cpp \
				$(SRC_DIR)/main.cpp

# Objects
OBJS		= $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

# Build project
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(NAME) built successfully"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Make obj directory
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Clean object files
clean:
	@rm -f $(OBJS)
	@rmdir $(OBJ_DIR) 2>/dev/null || true

# Clean object and executable files
fclean: clean
	@rm -f $(NAME)

# Clean and rebuild executable
re: fclean all

.PHONY: all clean fclean re bonus