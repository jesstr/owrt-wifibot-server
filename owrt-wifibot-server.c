#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string.h>  /* Объявления строковых функций */
#include <unistd.h>  /* Объявления стандартных функций UNIX */
#include <fcntl.h>   /* Объявления управления файлами */
#include <termios.h> /* Объявления управления POSIX-терминалом */


#define COMMAND_LENGTH 16 	// 16 byte


int fd; 					/* Файловый дескриптор для порта */
char buf[COMMAND_LENGTH]; 	/*размер зависит от размера строки принимаемых данных*/
char *command;

/* Open UART port*/
int open_port(void)
{
	fd = open("/dev/ttyATH0", O_RDWR | O_NOCTTY | O_NDELAY); 	/*'open_port()' - Открывает последовательный порт */
	if (fd < 0) {
		printf("Couldn't open port.\n");
		return -1;
    }
    else {
        struct termios options; 			/*структура для установки порта*/
        tcgetattr(fd, &options); 			/*читает пораметры порта*/

        cfsetispeed(&options, B9600); 		/*установка скорости порта*/
        cfsetospeed(&options, B9600); 		/*установка скорости порта*/

         options.c_cflag &= ~PARENB; 		/*выкл проверка четности*/
         options.c_cflag &= ~CSTOPB; 		/*выкл 2-х стобит, вкл 1 стопбит*/
         options.c_cflag &= ~CSIZE; 		/*выкл битовой маски*/
         options.c_cflag |= (CLOCAL | CREAD);
         options.c_cflag |= CS8; 			/*вкл 8бит*/
         tcsetattr(fd, TCSANOW, &options); 	/*сохронения параметров порта*/
         //printf("UART port opened.\n");	//debug
         return fd;
	}
}

int main()
{
    int sock, listener;
    struct sockaddr_in addr;
    int bytes_read;

	open_port();

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0) {
        printf("socket error\n");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 	/* INADDR_ANY - get connections from any net interface */
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("bind error\n");
        return 2;
    }
    listen(listener, 1); /* queue length = 1 */
    printf("listening..\n");

    while(1) {
        sock = accept(listener, NULL, NULL); 	/* we do not interest in addr of client, so (.., NULL, NULL) */
        printf("accepted!\n");
        if(sock < 0) {
            printf("accept error\n");
            return 3;
        }
		/* Get buf from socket */
		while((bytes_read = recv(sock, buf, COMMAND_LENGTH, 0)) > 0 ) {
			//printf("%d\n",bytes_read); // debug
         	if(bytes_read <= 0) break;
         	command = buf;
            printf("%.*s", bytes_read, command); // debug
            /* Send command to UART port */
			/*
			if (write(fd, command, bytes_read) < 0) {
				  printf("UART port write error!\n");
			}
			*/
        }
        close(sock);
        printf("disconnected\n");
    }
    return 0;
}

