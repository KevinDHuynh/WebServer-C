/////////////////////////////////////////////////////////////////////////
//
// To compile: 			gcc -o my_httpd my_httpd.c -lnsl -lsocket
//
// To start your server:	./my_httpd 8080 ./www
//
// To Kill your server:		kill_my_httpd
//
/////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include <stdlib.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include <signal.h>

#define	ERROR_FILE	0
#define REG_FILE  	1
#define DIRECTORY 	2

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void GetMyHomeDir(char *myhome, char **environ) {
        int i=0;
	while( environ[i] != NULL ) {
        	if( ! strncmp(environ[i], "HOME=", 5) ){
			strcpy(myhome, &environ[i][5]);
			return;
		}
		i++;
        }
	fprintf(stderr, "[FATAL] SHOULD NEVER COME HERE\n");
	fflush(stderr);
	exit(-1);
}

////////////////////////////////////////////////////////////////////
// Tells us if the request is a directory or a regular file
////////////////////////////////////////////////////////////////////
int TypeOfFile(char *fullPathToFile) {
	struct stat buf;	/* to stat the file/dir */

        if( stat(fullPathToFile, &buf) != 0 ) {
		perror("stat()");
		fprintf(stderr, "[ERROR] stat() on file: |%s|\n",
						fullPathToFile);
		fflush(stderr);
                exit(-1);
        }


        if( S_ISREG(buf.st_mode) )
		return(REG_FILE);
        else if( S_ISDIR(buf.st_mode) )
		return(DIRECTORY);

	return(ERROR_FILE);
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void SendDataBin(char *fileToSend, int sock, char *home, char *content) {
  int f;
	char fullPathToFile[256];
  size_t BUFFER_SIZE = 1024;
	char Header[BUFFER_SIZE];
  int s;
  char buffer[BUFFER_SIZE];
	int fret;		/* return value of TypeOfFile() */

  //used to avoid data corruption when sending files
  memset(Header, 0, BUFFER_SIZE);
	memset(buffer, 0, BUFFER_SIZE);

	/*
	 * Build the header
	 */
	strcpy(Header, "HTTP/1.0 200 OK\nContent-type: text/html\n\n");

	/*
	 * Build the full path to the file
	 */
	sprintf(fullPathToFile, "%s/%s/%s", home, content, fileToSend);

	/*
	 * - If the requested file is a directory, append the 'index.html'
	 *   file to the end of the fullPathToFile
	 *   (Use TypeOfFile(fullPathToFile))
	 * - If the requested file is a regular file, do nothing and proceed
	 * - else your client requested something other than a directory
	 *   or a reqular file
	 */
	/* TODO 5 */
  fret = TypeOfFile(fullPathToFile);
	if (fret == DIRECTORY) {
    char *temp =malloc(strlen(fullPathToFile)+strlen("index.html")+1);
		strcpy(temp, fullPathToFile);
		strcat(temp, "index.html");
		strcpy(fullPathToFile, temp);
	}
  //END TODO 5//
	/*
	 * 1. Send the header (use write())
	 * 2. open the requested file (use open())
	 * 3. now send the requested file (use write())
	 * 4. close the file (use close())
	 */

	 /* TODO 6 */

	write(sock,Header,sizeof(Header));

	//open requested file

	int c;
	FILE *file;
	const char *dot = strrchr(fullPathToFile,'.');
  //check for CGI Request
	if((dot != NULL ) && (strcmp(dot,".cgi") == 0) ) {
    //popen should be used for shell
		file = popen(fullPathToFile, "r");
		int  i = 0;
		if (file) {
			while ((c = getc(file)) != EOF) {
			buffer[i] = c;
			i++;
			}
		}
		pclose(file);
	  }
  //should handle any other type of file
	else {
		file = fopen(fullPathToFile, "r");
		if (file) {
			while ((c = fread(buffer,sizeof(size_t),BUFFER_SIZE,file)) != EOF) {
			}
		}
		fclose(file);
	}

	//send file
	write(sock,buffer,BUFFER_SIZE);
  //close file
	close(sock);

}


////////////////////////////////////////////////////////////////////
// Extract the file request from the request lines the client sent
// to us.  Make sure you NULL terminate your result.
////////////////////////////////////////////////////////////////////
void ExtractFileRequest(char *req, char *buff) {

	/* TODO 4  */
  int i=0;
  while(buff[5+i]!=' '){
    req[i] = buff[5+i];
    i++;
  }
  req[i] = '\0';
	//END TODO 4
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
int main(int argc, char **argv, char **environ) {
  pid_t pid;		/* pid of child */
	int sockid;		/* our initial socket */
	int PORT;		/* Port number, used by 'bind' */
	char content[128];	/* Your directory that contains your web
				   content such as .www in
				   your home directory */
	char myhome[128];	/* Your home directory */
				/* (gets filled in by GetMyHomeDir() */
	/*
	 * structs used for bind, accept..
	 */
  	struct sockaddr_in server_addr, client_addr;

	char file_request[256];	/* where we store the requested file name */
        int one=1;		/* used to set socket options */


	/*
	 * Get my home directory from the environment
	 */
	GetMyHomeDir(myhome, environ);

	if( argc != 3 ) {
		fprintf(stderr, "USAGE: %s <port number> <content directory>\n",
								argv[0]);
		exit(-1);
	}

	PORT = atoi(argv[1]);		/* Get the port number */
	strcpy(content, argv[2]);	/* Get the content directory */


	if ( (pid = fork()) < 0) {
		perror("Cannot fork (for deamon)");
		exit(0);
  	}
	else if (pid != 0) {
		/*
	  	 * I am the parent
		 */
		char t[128];
		sprintf(t, "echo %d > %s.pid\n", pid, argv[0]);
		system(t);
    		exit(0);
  	}

  	// setsid();
  	// chdir("/");
  	// umask(0);

	/*
	 * Create our socket, bind it, listen
	 */

	/* TODO 1 */
	if ((sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Could not create socket");
		exit(0);
	}

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT);

  if (bind(sockid, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
      perror("Could not bind socket");
      exit(0);
  }
  if( (listen(sockid, 5)) < 0){
      perror("Could not listen");
      exit(0);
  }

	//END TODO 1

	signal(SIGCHLD, SIG_IGN);


	/*
	 * - accept a new connection and fork.
	 * - If you are the child process,  process the request and exit.
	 * - If you are the parent close the socket and come back to
         *   accept another connection
	 */
  	while (1) {
		/*
		 * socket that will be used for communication
		 * between the client and this server (the child)
		 */
		int newsock;

		/*
		 * Get the size of this structure, could pass NULL if we
		 * don't care about who the client is.
		 */
   	int client_len = sizeof(client_addr);

		/*
		 * Accept a connection from a client (a web browser)
		 * accept the new connection. newsock will be used for the
		 * child to communicate to the client (browser)
		 */
		 /* TODO 2 */
		newsock = accept(sockid,(struct sockaddr *)&client_addr,&client_len);
		//END TODO 2
    	if (newsock < 0) {
			perror("accept");
			exit(-1);
		}

		if ( (pid = fork()) < 0) {
			perror("Cannot fork");
			exit(0);
  	}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
		else if( pid == 0 ) {
			/*
			 * I am the Child
			 */
			int r;
      char buff[1024];
			int read_so_far = 0;
			char ref[1024], rowRef[1024];

			memset(buff, 0, 1024);

			/*
			 * Read client request into buff
			 * 'use a while loop'
			 */
			/* TODO 3 */
		    while(read_so_far = recv(newsock,buff,1024,0) > 0) {
		    	break;
		    }
		    if(read_so_far < 0){
		        perror("Could not Recv");
		    }
			//END TODO
//
// What you may get from the client:
//			GET / HTTP/1.0
//			Connection: Keep-Alive
//			User-Agent: Mozilla/3.0 (X11; I; SunOS 5.5 sun4m)
//			Host: spiff:6789
//			Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, * /*
//


// Write to client
//
//			You should write to the client an HTTP response and then the
//			requested file, if appropriate. A response may look like:
//
//			HTTP/1.0 200 OK
//			Content-length: 2032
//			Content-type: text/html
//			[single blank line necessary here]
//			[document follows]
//



			ExtractFileRequest(file_request, buff);

			printf("** File Requested: |%s|\n", file_request);
			fflush(stdout);

			SendDataBin(file_request, newsock, myhome, content);
			shutdown(newsock, 1);
			close(newsock);
			exit(0);
    		}
		/*
		 * I am the Parent
		 */
		close(newsock);	/* Parent handed off this connection to its child,
			           doesn't care about this socket */
  	}
}
