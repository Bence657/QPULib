#include <stdlib.h>
#include "QPULib.h"

struct Cursor {
  Ptr<Float> addr;
  Float prev, current, next;

  void init(Ptr<Float> p) {
    gather(p);
    current = 0;
    addr = p+16;
  }

  void prime() {
    receive(next);
    gather(addr);
  }

  void advance() {
    addr = addr+16;
    prev = current;
    gather(addr);
    current = next;
    receive(next);
  }

  void finish() {
    receive(next);
  }

  void shiftLeft(Float& result) {
    result = rotate(current, 15);
    Float nextRot = rotate(next, 15);
    Where (index() == 15)
      result = nextRot;
    End
  }

  void shiftRight(Float& result) {
    result = rotate(current, 1);
    Float prevRot = rotate(prev, 1);
    Where (index() == 0)
      result = prevRot;
    End
  }
};

void calculate(Ptr<Float> image, Ptr<Float> result, Int width, Int height) {
	Cursor row[3];

	image = image + width*me() + index();

	// ?
	result = result + width;

	For (Int y = me(), y < height, y = y + numQPUs())

		// Point p to output row
		Ptr<Float> p = result + y*width;

		// Initialise three cursors
		for (int i = 0; i < 3; i++) row[i].init(image + i*width);
		for (int i = 0; i < 3; i++) row[i].prime();

		// Compute one output row
		For (Int x = 0, x < width, x = x + 16)

			for (int i = 0; i < 3; i++) row[i].advance();

			Float left[3], right[3];
			for (int i = 0; i < 3; i++) {
				row[i].shiftLeft(right[i]);
				row[i].shiftRight(left[i]);
			}

			Float sum = left[0] + row[0].current +  right[0] +
						left[1] + row[1].current +	right[1] +
						left[2] + row[2].current + 	right[2];
			
			store(sum, p);
			p = p + 16;
		End

		for (int i = 0; i < 3; i++) row[i].finish();
		
		image = image + width*numQPUs();
	End
}

int main() {
	
	float image[16][16] = {
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
		{3, 3, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3},
	 };

	const int NQPUS = 8;
	const int WIDTH = 512;
	const int HEIGHT = 512;

	srand(0);
	const int MAX_VALUE = 10.0;
	SharedArray<float> imageA(WIDTH*HEIGHT), imageB(WIDTH*HEIGHT);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			// printf("%f ", image[y][x]);
			imageA[y*WIDTH + x] = (float)rand()/((float)RAND_MAX/MAX_VALUE);
			imageB[y*WIDTH + x] = 0;			
		}
	}
	printf("\n");

	auto k = compile(calculate);
	k.setNumQPUs(NQPUS);


#if 0
	printf("Input:\n");
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			printf("%3.2f ", (float) imageA[y * WIDTH + x]);
		}
		printf("\n");
	}
#endif

	k(&imageA, &imageB, WIDTH, HEIGHT);


#if 1
	printf("\nOutput:\n");
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			printf("%3.2f ", (float) imageB[y * WIDTH + x]);
		}
		printf("\n");
	}
#endif	
	return 0;
}
