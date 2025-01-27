# Program name
NAME		= webserver

# Compiler
CC			= c++

# Compilation flags
CFLAGS		= -Wall -Wextra -Werror -std=c++17

# Directories
SRC_DIR		= src
OBJ_DIR		= obj

# Debug Flags
DEBUG=-DDEBUG=1 -g -fsanitize=address 

# Source files
SRCS		=	$(SRC_DIR)/Config.cpp \
				$(SRC_DIR)/ServerBlock.cpp \
				$(SRC_DIR)/HTTPRequest.cpp \
				$(SRC_DIR)/HTTPResponse.cpp \
				$(SRC_DIR)/Server.cpp \
				$(SRC_DIR)/main.cpp \
				$(SRC_DIR)/log.cpp \
				$(SRC_DIR)/ServerMsg.cpp \
				$(SRC_DIR)/Client.cpp \
				$(SRC_DIR)/ProcessRequest.cpp \
				$(SRC_DIR)/Utils.cpp

# Objects
OBJS		= $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DOBJS		= $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%_debug.o)

all: $(NAME)

# Build project
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(NAME) built successfully"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%_debug.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG) -c $< -o $@

# Make obj directory
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Clean object files
clean:
	@rm -f $(OBJS)
	@rm -f $(DOBJS)
	@rmdir $(OBJ_DIR) 2>/dev/null || true

# Clean object and executable files
fclean: clean
	@rm -f $(NAME)

# Clean and rebuild executable
re: fclean all

debug: fclean $(DOBJS)
	$(CC) $(CFLAGS) $(DEBUG) $(DOBJS) -o $(NAME)
	@echo "$(NAME) built successfully"

.PHONY: all clean fclean re debug
