/********************************************
 *
 *             prog2
 *
 *  This program reads the secret key information
 *  from the secret file and uses it to decode
 *  the message in the message file.
 *
 *********************************************/
 
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <sys/random.h>
 #include <stdio.h>
 #include <openssl/conf.h>
 #include <openssl/evp.h>
 #include <openssl/err.h>
 #include <string.h>
 #include <unistd.h>
 
 /*
  *  Structure to store the key and the initialization vector
  */
  struct secretStruct {
 	unsigned char key[32];
 	unsigned char iv[16];
 } secret;
 
 /*
  * Standard error handling procedure.
  */
 void handleErrors() {
 	ERR_print_errors_fp(stderr);
 	abort();
 }
 
 /*
  *  Read the key and initialization vector from the secret file
  */
 void read_key() {
 	int fin;
 	
 	fin = open("secret", O_RDONLY, 0600);
 	read(fin, &secret, sizeof(secret));
 	close(fin);
 	
 }
 
 /*
  *  Procedure that decodes the encrypted message.  The first parameter
  *  is the cipher text.  The second parameter is the length of the
  *  cipher text.  The third parameter is a pointer to a block of
  *  memory where the plain text will be stored.
  */
 int decode(unsigned char *cipher, int length, unsigned char *plain) {
 	EVP_CIPHER_CTX *ctx;
 	int len;
 	int plaintext_len;
 	
 	/*
 	 *  Create a context for the decrypt operation
 	 */
 	if(!(ctx = EVP_CIPHER_CTX_new()))
 		handleErrors();
 	
 	/*
 	 *  Initialize the decryption operation.  The second parameter is
 	 *  the decryption algorithm to be used.  The NULL third parameter
 	 *  specifies that the default implementation for this algorithm
 	 *  will be used.
 	 */
 	if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret.key, secret.iv))
 		handleErrors();
 	
 	/*
 	 *  Start descrypting the message.
 	 */	
 	if(1 != EVP_DecryptUpdate(ctx, plain, &len, cipher, length))
 		handleErrors();
 		
 	plaintext_len = len;

	/*
	 *  Finish decrypting the message, make sure that the last
	 *  block of the message is processed.
	 */
 	if(1 != EVP_DecryptFinal_ex(ctx, plain+len, &len))
 		handleErrors();
 		
 	plaintext_len += len;
 	
 	/*
 	 *  Free the memory used by the cipher context.
 	 */
 	EVP_CIPHER_CTX_free(ctx);
 	
 	return(plaintext_len);
 	
 }
 
 int main(int argc, char ** argv) {
 	unsigned char buffer[512];
 	unsigned char *plain;
 	int len;
 	int n;
 	int fin;
 
 	read_key();
 	
 	/*
 	 *  Read the encrypted message from the message file.
 	 */
 	fin = open("message", O_RDONLY, 0644);
 	n = read(fin, buffer, 512);
 	close(fin);
 	
 	/*
 	 *  Allocate memory for the plain text based on the size
 	 *  of the encrypted message.
 	 */
 	plain = (unsigned char *) malloc(n);
 	
 	/*
 	 *  Decode the message and print the result
 	 */
 	len = decode(buffer, n, plain);
 	
 	printf("Plain text: %s\n",plain);
 	
 }
