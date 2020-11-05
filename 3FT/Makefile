#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3
# Author: Alex Baroody
#--------------------------------------------------------------------

all: ft_client

ft_client: ft_client.o ft.o dynarray.o node.o
	gcc217 -g ft_client.o ft.o checker.o node.o dynarray.o -o ft_client

ft_client.o: ft_client.c ft.h
	gcc217 -c ft_client.c

ft.o: ft.c ft.h
	gcc217 -c ft.c

node.o: node.c node.h
	gcc217 -c node.c

dynarray.o: dynarray.c dynarray.h
	gcc217 -c dynarray.c

checker.o: checker.c checker.h
	gcc217 -c checker.c
