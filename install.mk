#!/usr/bin/make -f

#DEST = $(HOME)/CoursInfo/Algo_S1/tp/graph
DEST = $(HOME)/public_html/enseignement/Permanent/AlgoProg/tp/graph

SRC = DrawingWindow.h DrawingWindow.cpp
SRC += exemple.cpp exemple.pro
#SRC += index.html

.PHONY: help generate install install_www

help:
	@echo "Usage: $(MAKEFILE_LIST) generate | install | install_www"

generate:
	rm -fr html/
	doxygen Doxyfile
	sed -i '/^pre\.fragment {/,/}/{/font-size/d}' html/doxygen.css

install: generate
	rm -fr $(DEST)
	install -m 755 -d $(DEST)
	install -m 644 $(SRC) $(DEST)
	install -m 644 html/* $(DEST)
	install -m 644 htaccess $(DEST)/.htaccess

install_www: install
	update-www
