all: servidor cliente file_manager

servidor: ./Servidor/servidor.c
	gcc -o ./Servidor/servidor.exe ./Servidor/servidor.c
cliente: ./Cliente/cliente.c
	gcc -o ./Cliente/cliente.exe ./Cliente/cliente.c
file_manager: ./Servidor/file_manager.c
	gcc -o ./Servidor/file_manager.exe ./Servidor/file_manager.c
	cd Servidor
	./servidor.exe