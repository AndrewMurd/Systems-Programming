 /****************************************
 *             lib.h
 *
 *  Declarations of the procedures in our
 *  library of useful procedures for
 *  reliable network programs.
 *************************************/

int readn(int fd, char *buffer, int count);
int writen(int fd, char *buffer, int count);
char *readString(int fd);
int writeString(int fd, char *string);


int sendData(int sock, int arg1, int arg2);
int receiveData(int sock, int *result);


struct data {
    int arg1;
    int arg2;
    int result;
};