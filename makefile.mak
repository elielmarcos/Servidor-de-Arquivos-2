all: servidor cliente

servidor: ./Servidor/servidor.c
	gcc -o ./Servidor/servidor.exe ./Servidor/servidor.c
cliente: ./Cliente/cliente.c
	gcc -o ./Cliente/cliente.exe ./Cliente/cliente.c
	./Servidor/servidor.exe