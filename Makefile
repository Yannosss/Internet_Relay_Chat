##########   BASE   ##########
# File name
NAME = ircserv

# Sources
SRCS += ./src/main.cpp
SRCS += ./src/10_tools/utils.cpp
SRCS += ./src/20_class/client/client.cpp
SRCS += ./src/20_class/client/client_command.cpp
SRCS += ./src/20_class/client/cmdJOIN.cpp
SRCS += ./src/20_class/client/cmdCAP.cpp
SRCS += ./src/20_class/client/cmdMODE.cpp
SRCS += ./src/20_class/client/cmdNOTICE.cpp
SRCS += ./src/20_class/client/cmdPART.cpp
SRCS += ./src/20_class/client/cmdPASS.cpp
SRCS += ./src/20_class/client/cmdPONG.cpp
SRCS += ./src/20_class/client/cmdPRIVMSG.cpp
SRCS += ./src/20_class/client/cmdLIST.cpp
SRCS += ./src/20_class/client/cmdNAMES.cpp
SRCS += ./src/20_class/client/cmdNICK.cpp
SRCS += ./src/20_class/client/cmdUSER.cpp
SRCS += ./src/20_class/client/cmdKICK.cpp
SRCS += ./src/20_class/client/cmdINVITE.cpp
SRCS += ./src/20_class/client/cmdOPER.cpp
SRCS += ./src/20_class/client/cmdTOPIC.cpp
SRCS += ./src/20_class/client/cmdQUIT.cpp
SRCS += ./src/20_class/client/cmdPING.cpp
SRCS += ./src/20_class/client/cmdWHOIS.cpp
SRCS += ./src/20_class/server.cpp
SRCS += ./src/20_class/channel.cpp
SRCS += ./src/30_Parsing_argv/parsing.cpp


# Header/s
HEADER += ./src/10_tools/colors.hpp
HEADER += ./src/10_tools/utils.hpp
HEADER += ./src/20_class/server.hpp
HEADER += ./src/20_class/client/client.hpp
HEADER += ./src/20_class/channel.hpp
HEADER += ./src/10_tools/errors.hpp
HEADER += ./src/main.hpp


OBJS = ${SRCS:.cpp=.o}

# Compilateurs
CC = c++

CFLAGS = -g -Wall -Wextra -Werror -std=c++98
#CFLAGS = -g3 -Wall -Wextra -std=c++98

##########   RULES   ##########
all: $(NAME)

$(NAME): $(OBJS) $(HEADER)
	${CC} ${SRCS} -o ${NAME} ${CFLAGS}

%.o: %.cpp $(HEADER)
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f ${NAME}

re: fclean all

.PHONY: all clean fclean re
