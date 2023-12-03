// C program to read and print 
// all files in a zip file 
// uses library libzip 
#include <stdlib.h> 

#include "cap.h"
#include "tools.h"

// this is run from the command line with the zip file 
// passed in example usage: ./program zipfile.zip 
int main(int argc, char* argv[]) 
{
	char* deafault_name = "/Users/alexey/Documents/S/lpa/rsp.cap";
	char* name;

	printf(" ZIP info application\n");

	if (argc > 1)
		name = argv[1];
	else name = deafault_name;

	print_cap_info(name);
	
	upload_cap(name);

	return 0; 
}
