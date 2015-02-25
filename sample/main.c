#include <simple_png_to_jpeg.h>

#include <stdio.h>

int main(int argc, char *argv[])
{
	int ret = png_to_jpeg(argv[1], argv[2], 75);
	if (ret != 0)
		fprintf(stderr, "Returned error %d\n", ret);
	return ret;
}