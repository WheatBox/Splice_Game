#include "Application.h"

int main(int argc, char ** argv) {
	printf("main called\n");
	return gApplication->EntryPoint(argc, argv);
}