CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++11 -I/usr/include/freetype2
LDFLAGS = -lglfw -lGLEW -lGL -lm -lglut -lGLU -lstdc++ stb_image.cpp

SRCS = ExercicioM6_AtividadeVivencial.cpp
OBJS = $(SRCS:.cpp=.o)
EXECUTABLE = main

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXECUTABLE)

reload: $(EXECUTABLE)
	pgrep -x $(EXECUTABLE) > /dev/null && pkill -x $(EXECUTABLE) || true
	sleep 1
	./$(EXECUTABLE) &
	@make watch

watch:
	fswatch -o $(SRCS) | xargs -n1 -I{} make reload
