#include "Application.h"

int main() {
	printf("main called\n");
	return gApplication->EntryPoint("Splice", { 1280, 720 });
}