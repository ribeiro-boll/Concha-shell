CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lreadline
TARGET = concha
DESKTOP_FILE = concha.desktop
DESKTOP_PATH = /usr/share/applications
SHELL_PATH = /usr/local/bin/concha

all: $(TARGET)

$(TARGET): concha.c
	$(CC) $(CFLAGS) concha.c -o $(TARGET) $(LIBS)

install-desktop:
	cp $(DESKTOP_FILE) $(DESKTOP_PATH)/
	update-desktop-database

install: 
	cp $(TARGET) /usr/local/bin
	cp $(DESKTOP_FILE) $(DESKTOP_PATH)/
	@if ! grep -qx "$(SHELL_PATH)" /etc/shells; then \
		echo "Adicionando $(SHELL_PATH) em /etc/shells..."; \
		echo $(SHELL_PATH) | sudo tee -a /etc/shells; \
	else \
		echo "$(SHELL_PATH) já está presente em /etc/shells."; \
	fi; \
	update-desktop-database 
	@printf "\n\npara usar como shell padrão, digite chsh -s /usr/local/bin/concha !\n"

clean:
	rm -f $(TARGET)

help:
	@echo "Uso:"
	@echo "  make           : Compila o programa"
	@echo "  sudo make install : Instala o programa com o .desktop"
	@echo "  make clean     : Remove o executável"
