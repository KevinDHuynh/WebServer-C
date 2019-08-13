#include <stdio.h>
#include <stdlib.h>
//
// Compile with:  gcc -o myCGI myCGI.c
//

main(int argc, char **argv) {
	FILE *fd =  NULL;
	int p=0;
	char buf[1024];

	printf( "<HTML>\n");
	printf( "<HEAD>\n");
	printf( "<TITLE>Example CGI Script</TITLE>\n");
	printf( "</HEAD>\n");
	printf( "<BODY>\n");

	printf( "<H1>This is a working CGI Script!</H1>\n");


	printf( "<table border=1>\n");
	printf( "<tr><td>\n");
	printf( "<font color=red><B>");

	if( (fd = popen("/bin/date", "r")) == NULL) {
		perror("popen");
		exit(-1);
	}

	while ( fgets(buf, 1024, fd) != NULL) {
		printf("%s", buf);
	}

	pclose(fd);
	printf("</B></font>\n");
	printf( "</td></tr>\n");

	printf( "</table>\n");



	printf("</BODY>\n");
	printf("</HTML>\n");
}

