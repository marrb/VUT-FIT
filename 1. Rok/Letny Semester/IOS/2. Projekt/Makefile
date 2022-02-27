make: Santa.c main.c Elve.c Reindeer.c Shared_mem.h
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -o Santa Santa.c  -pthread
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -o proj2 main.c -pthread
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -o Elve Elve.c -pthread
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -o Reindeer Reindeer.c -pthread

clean:
	rm Santa ; rm main ; rm Elve ; rm Reindeer
