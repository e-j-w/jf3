GLIB_COMPILE_RESOURCES = `pkg-config --variable=glib_compile_resources gio-2.0`

RESOURCES = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=./data --generate-dependencies data/jf3.gresource.xml)

CFLAGS = -I. -I./src/lin_eq_solver -O2 -Wall -Wshadow -Wunreachable-code -Wpointer-arith -Wcast-align -Wformat-security -Wstack-protector -Wconversion -std=c99

all: lin_eq_solver jf3-resources.c jf3

jf3: src/jf3.c src/jf3.h src/read_data.c src/read_config.c src/fit_data.c src/spectrum_drawing.c src/utils.c jf3-resources.c src/lin_eq_solver/lin_eq_solver.o
	gcc src/jf3.c $(CFLAGS) -lm `pkg-config --cflags --libs gtk+-3.0` -export-dynamic -o jf3 src/lin_eq_solver/lin_eq_solver.o
	rm jf3-resources.c

jf3-resources.c: data/jf3.gresource.xml data/jf3.glade $(RESOURCES)
	$(GLIB_COMPILE_RESOURCES) data/jf3.gresource.xml --target=jf3-resources.c --sourcedir=./data --generate-source

lin_eq_solver: src/lin_eq_solver/lin_eq_solver.c src/lin_eq_solver/lin_eq_solver.h
	gcc $(CFLAGS) -c -o src/lin_eq_solver/lin_eq_solver.o src/lin_eq_solver/lin_eq_solver.c

install:
	@echo "Will install to /usr/bin."
	@echo "Run 'make uninstall' to undo installation."
	@if ! [ "$(shell id -u)" = 0 ]; then \
		echo "This must be run with administrator privileges (eg. with 'sudo')."; \
	else \
		cp jf3 /usr/bin ; \
		cp data/jf3-application-icon.svg /usr/share/icons/hicolor/scalable/apps ; \
		cp data/jf3.desktop /usr/share/applications ; \
		cp data/jf3-mime.xml /usr/share/mime/packages ; \
		update-mime-database /usr/share/mime ; \
		update-desktop-database /usr/share/applications ; \
		echo "Done!" ; \
	fi


uninstall:
	@echo "Will undo changes made from running 'make install'."
	@if ! [ "$(shell id -u)" = 0 ]; then \
		echo "This must be run with administrator privileges (eg. with 'sudo')."; \
	else \
		rm /usr/bin/jf3 ; \
		rm /usr/share/icons/hicolor/scalable/apps/jf3-application-icon.svg ; \
		rm /usr/share/applications/jf3.desktop ; \
		rm /usr/share/mime/packages/jf3-mime.xml ; \
		update-mime-database /usr/share/mime ; \
		update-desktop-database /usr/share/applications ; \
		echo "Done!" ; \
	fi

clean:
	rm -rf *~ *.o */*/*.o jf3-resources.c *# jf3
