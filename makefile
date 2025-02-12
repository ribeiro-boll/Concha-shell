CC = gcc
CFLAGS = -Wall -Wextra
TARGET = concha
DESKTOP_FILE = concha.desktop
DESKTOP_PATH = /usr/share/applications

all: $(TARGET)

$(TARGET): concha.c
	$(CC) $(CFLAGS) concha.c -o $(TARGET)

install-desktop:
	cp $(DESKTOP_FILE) $(DESKTOP_PATH)/
	update-desktop-database

install: 
	cp $(TARGET) /usr/local/bin
	cp $(DESKTOP_FILE) $(DESKTOP_PATH)/
	update-desktop-database

clean:
	rm -f $(TARGET)

help:
	@echo "Uso:"
	@echo "  make           : Compila o programa"
	@echo "  sudo make install : Instala o programa com o .desktop"
	@echo "  sudo make install-all : Instala o app + ícone no menu"
	@echo "  make clean     : Remove o executável"