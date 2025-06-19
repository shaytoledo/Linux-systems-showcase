CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
LDFLAGS = -lmta_crypt -lmta_rand -lcrypto
TARGET = bruteforce
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
