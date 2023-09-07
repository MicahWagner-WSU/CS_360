#include <stdio.h>
#include "lineNum.h"

int main(int argc, char **argv) 
{
	printf("%d", lineNum("tiny_9", argv[1], 9));
}