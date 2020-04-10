CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LFLAGS = -lpthread -lrt
X = proj2
$(X): $(X).o
	gcc $(CFLAGS) -o $(X) $(X).o $(LFLAGS)

$(X).o: $(X).c
	gcc $(CFLAGS) -c -o $(X).o $(X).c

clean:
	rm -f $(X) $(X).o
