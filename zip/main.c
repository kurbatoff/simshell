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
	printf(" ZIP info application\n");

	print_cap_info(argv[1]);
	
	upload_cap(argv[1]);

	return 0; 
}
