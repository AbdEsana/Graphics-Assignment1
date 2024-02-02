#include "InputManager.h"
#include "../DisplayGLFW/display.h"
#include "game.h"
#include "../res/includes/glm/glm.hpp"
#include <stb_image.h>

unsigned char* averageRGB(unsigned char* data, int size) {
	unsigned char* average = (unsigned char*)malloc(size / 4);
	for (int i = 0; i < size / 4; i++) {
		average[i] = (data[4 * i] + data[4 * i + 1] + data[4 * i + 2]) / 3;
	}
	return average;
}

void saveToFile(char* fileName, unsigned char* data, int size, int floyd) {
	FILE* file = fopen(fileName, "w");
	unsigned char* dataNoAlpha = averageRGB(data, size);
	for (int i = 0; i < size / 4 - 1; i++) {
		if (!floyd) (dataNoAlpha[i] == 0) ? fprintf(file, "0,") : fprintf(file, "1,");
		else fprintf(file, "%d,", dataNoAlpha[i] / 16);
	}
	(dataNoAlpha[size / 4 - 2] == 0) ? fprintf(file, "0") : fprintf(file, "1");
	fclose(file);
}

unsigned char* grayIt(unsigned char* data) {
	unsigned char* grayScale = (unsigned char*)malloc(512 * 512);
	for (int i = 0, counter = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++, counter += 4) {
			unsigned char val = (unsigned char)(data[4 * (i * 256 + j)] * 0.3 + data[4 * (i * 256 + j) + 1] * 0.58 + data[4 * (i * 256 + j) + 2] * 0.11);
			grayScale[counter] = val;
			grayScale[counter + 1] = val;
			grayScale[counter + 2] = val;
			grayScale[counter + 3] = val;
		}
	}
	return grayScale;
}

unsigned char* grayIt(unsigned char* data, int width, int height) {
	
	unsigned char* grayScale = (unsigned char*)malloc(width * height);
	
	
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			unsigned char val = (unsigned char)(
				data[i * width + j] * 0.3 +
				data[i * width + j] * 0.58 +
				data[i * width + j] * 0.11);

			grayScale[i * width + j] = val;
		}
	}
	return grayScale;
}

unsigned char* finalTouch(unsigned char* data, unsigned char* original) {
	unsigned char* finalTouch = (unsigned char*)calloc(512 * 512, 1);
	unsigned char(*finalTouch2D)[1024] = (unsigned char(*)[1024])finalTouch;
	unsigned char(*data2D)[256] = (unsigned char(*)[256])data;
	unsigned char(*original2D)[1024] = (unsigned char(*)[1024])original;

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			if (data2D[i][j] == 75) {
				if ((data2D[(i - 1)][j - 1] == 255) ||
					(data2D[(i - 1)][j] == 255) ||
					(data2D[(i - 1)][j + 1] == 255) ||
					(data2D[i][j - 1] == 255) ||
					(data2D[i][j + 1] == 255) ||
					(data2D[(i + 1)][j - 1] == 255) ||
					(data2D[(i + 1)][j] == 255) ||
					(data2D[(i + 1)][j + 1] == 255)) {
					finalTouch2D[i][4 * j] = 255; finalTouch2D[i][4 * j + 1] = 255; finalTouch2D[i][4 * j + 2] = 255; finalTouch2D[i][4 * j + 3] = original2D[i][4 * j + 3];
				}
				else {
					finalTouch2D[i][4 * j] = 0; finalTouch2D[i][4 * j + 1] = 0; finalTouch2D[i][4 * j + 2] = 0; finalTouch2D[i][4 * j + 3] = original2D[i][4 * j + 3];
				}
			}
			else {
				finalTouch2D[i][4 * j] = data2D[i][j]; finalTouch2D[i][4 * j + 1] = data2D[i][j]; finalTouch2D[i][4 * j + 2] = data2D[i][j]; finalTouch2D[i][4 * j + 3] = original2D[i][4 * j + 3];
			}
		}
	}
	saveToFile("../img4.txt", finalTouch, 512 * 512, 0);
	return finalTouch;
}

unsigned char* threshold(unsigned char* data, unsigned char* original) {
	unsigned char* threshold = (unsigned char*)calloc(256 * 256, 1);
	unsigned char(*threshold2D)[256] = (unsigned char(*)[256])threshold;
	unsigned char(*data2D)[256] = (unsigned char(*)[256])data;
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			// Adjusted thresholding values for showing more edges
			if (data2D[i][j] >= 120)
				threshold2D[i][j] = 255;  // Strong edge
			else if (data2D[i][j] <= 10)
				threshold2D[i][j] = 0;    // Non-edge
			else
				threshold2D[i][j] = 75;   // Weak edge
		}
	}
	return finalTouch(threshold, original);
}


unsigned char* non_max_suppression(unsigned char* data, unsigned char* original) {
	unsigned char* suppressed = (unsigned char*)calloc(256 * 256 * 4, 1);
	unsigned char* suppression = (unsigned char*)calloc(256 * 256, 1);
	unsigned char* angle = (unsigned char*)calloc(256 * 256, 1);
	unsigned char(*suppression2D)[256] = (unsigned char(*)[256])suppression;
	unsigned char(*angle2D)[256] = (unsigned char(*)[256])angle;
	unsigned char(*data2D)[256] = (unsigned char(*)[256])data;

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			unsigned char val = data2D[i][j] * (unsigned char)(180 / 3.14);
			angle2D[i][j] = (val < 0) ? val + 180 : val;
		}
	}

	for (int i = 1; i < 255; i++) {
		for (int j = 1; j < 255; j++) {
			try {
				int q = 255;
				int r = 255;

				//angle 0
				if ((0 <= angle2D[i][j] < 22.5) | (157.5 <= angle2D[i][j] <= 180))
				{
					q = data2D[i][j + 1];
					r = data2D[i][j - 1];
				}
				else if (22.5 <= angle2D[i][j] < 67.5) {

					q = data2D[i + 1][j - 1];
					r = data2D[i - 1][j + 1];
					//angle 90
				}
				else if (67.5 <= angle2D[i][j] < 112.5) {
					q = data2D[i + 1][j];
					r = data2D[i - 1][j];
					//angle2D 135
				}
				else if (112.5 <= angle2D[i][j] < 157.5) {
					q = data2D[i - 1][j - 1];
					r = data2D[i + 1][j + 1];
				}
				if ((data2D[i][j] >= q) & (data2D[i][j] >= r)) {
					suppression2D[i][j] = data2D[i][j];
				}
				else suppression2D[i][j] = 0;
			}
			catch (const std::exception&) {}
		}
	}

	for (int i = 0, counter = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++, counter += 4) {
			suppressed[counter] = suppression2D[i][j];
			suppressed[counter + 1] = suppression2D[i][j];
			suppressed[counter + 2] = suppression2D[i][j];
			suppressed[counter + 3] = original[counter + 3];
		}
	}
	return threshold(suppression, original);
}


unsigned char* edgeIt(unsigned char* data) {
	char* dataNoAlpha = (char*)averageRGB(data, 512*512);
	unsigned char* edge = (unsigned char*)malloc(256 * 256);
	printf("here 1");
	int counter = 0;
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++, counter++) {
			char gx = dataNoAlpha[(i - 1) * 256 + j - 1] + 2 * dataNoAlpha[(i - 1) * 256 + j] + dataNoAlpha[(i - 1) * 256 + j + 1]
				- dataNoAlpha[(i + 1) * 256 + j - 1] - 2 * dataNoAlpha[(i + 1) * 256 + j] - dataNoAlpha[(i + 1) * 256 + j + 1];
			char gy = -dataNoAlpha[(i + 1) * 256 + j - 1] - 2 * dataNoAlpha[i * 256 + j - 1] - dataNoAlpha[(i - 1) * 256 + j - 1]
				+ dataNoAlpha[(i + 1) * 256 + j + 1] + 2 * dataNoAlpha[i * 256 + j + 1] + dataNoAlpha[(i - 1) * 256 + j + 1];
			unsigned char val = std::abs(gx) + std::abs(gy);
			edge[counter] = val;
		}
	}
	printf("here 2");
	return non_max_suppression(edge, data);
}

unsigned char* halfToneIt(unsigned char* data) {
	unsigned char* halftone = (unsigned char*)calloc(512 * 512 * 4, 1);
	unsigned char* dataNoAlpha = averageRGB(data, 512 * 512);
	for (int i = 0; i < 256; i++) {//4096
		for (int j = 0; j < 256; j++) {
			int val = dataNoAlpha[i * 256 + j] / 51 - 1;
			halftone[4096 * (i + 1) + j * 8 + 3] = data[512 * i + j];
			halftone[4096 * i + j * 8 + 4 + 3] = data[512 * i + j];
			halftone[4096 * (i + 1) + j * 8 + 4 + 3] = data[512 * i + j];
			halftone[4096 * i + j * 8 + 3] = data[512 * i + j];
			if (val > 0) { halftone[4096 * (i + 1) + j * 8] = 255; halftone[4096 * (i + 1) + j * 8 + 1] = 255; halftone[4096 * (i + 1) + j * 8 + 2] = 255; val--; }
			if (val > 0) { halftone[4096 * i + j * 8 + 4] = 255; halftone[4096 * i + j * 8 + 1 + 4] = 255; halftone[4096 * i + j * 8 + 2 + 4] = 255; val--; }
			if (val > 0) { halftone[4096 * (i + 1) + j * 8 + 4] = 255; halftone[4096 * (i + 1) + j * 8 + 1 + 4] = 255; halftone[4096 * (i + 1) + j * 8 + 2 + 4] = 255; val--; }
			if (val > 0) { halftone[4096 * i + j * 8] = 255; halftone[4096 * i + j * 8 + 1] = 255; halftone[4096 * i + j * 8 + 2] = 255; val--; }
		}
	}
	saveToFile("../img5.txt", halftone, 512 * 512 * 4, 0);
	return halftone;
}




unsigned char* floydIt(unsigned char* data) {
	unsigned char floyd[512][512];
	char* dataNoAlpha = (char*)averageRGB(data, 256 * 256 * 4);
	char(*data2D)[256] = (char(*)[256])dataNoAlpha;

	for (int y = 256; y >= 0; y--) {
		for (int x = 0; x < 256; x++) {
			int oldPixel = data2D[x][y];
			int newPixel = oldPixel / 255;
			floyd[x][y] = newPixel;
			int quant_error = oldPixel - newPixel;
			floyd[x + 1][y] = data2D[x + 1][y] + quant_error * 7 / 8;   // Increase scaling factor
			floyd[x - 1][y + 1] = data2D[x - 1][y + 1] + quant_error * 1 / 16;   // Increase scaling factor
			floyd[x][y + 1] = data2D[x][y + 1] + quant_error * 15 / 16;   // Increase scaling factor
			floyd[x + 1][y + 1] = data2D[x + 1][y + 1] + quant_error * 13 / 16;   // Increase scaling factor
		}
	}
	unsigned char* floyded = (unsigned char*)malloc(256 * 256*4);
	for (int i = 0, counter = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++, counter += 4) {
			floyded[counter] = floyd[i][j]; floyded[counter + 1] = floyd[i][j]; floyded[counter + 2] = floyd[i][j]; floyded[counter + 3] = data[counter + 3];
		}
	}
	saveToFile("../img6.txt", floyded, 256 * 256*4, 1);
	return floyded;
}

void printData(unsigned char* data, int width, int height, int numComponents) {
	printf("Image Data:\n");
	

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width * numComponents; j += numComponents) {
			printf("(%3d, %3d, %3d, %3d) ", data[i * width * numComponents + j],
				data[i * width * numComponents + j + 1],
				data[i * width * numComponents + j + 2],
				data[i * width * numComponents + j + 3]);
		}
		printf("\n");
	}
	printf("Width: %d\n", width);
	printf("Height: %d\n", height);
	printf("Number of Components: %d\n", numComponents);
}

int main(int argc,char *argv[])
{
	const int DISPLAY_WIDTH = 800;
	const int DISPLAY_HEIGHT = 800;
	const float CAMERA_ANGLE = 0.0f;
	const float NEAR = 1.0f;
	const float FAR = 100.0f;

	Game *scn = new Game(CAMERA_ANGLE,(float)DISPLAY_WIDTH/DISPLAY_HEIGHT,NEAR,FAR);
	
	Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");
	
	Init(display);
	
	scn->Init();

	display.SetScene(scn);

	int width, height, numComponents;
	unsigned char* data = stbi_load("../res/textures/lena256.jpg", &width, &height, &numComponents, 4);
	
	printData(data, width, height, numComponents); //there is data

	unsigned char* data1 = grayIt(data);
	scn->AddTexture(width, height, data1);
	scn->SetShapeTex(0, 0);
	scn->CustomDraw(1, 0, scn->BACK, true, false, 0);

	
	unsigned char* data2 = edgeIt(data);
	scn->AddTexture(256, 256, data2);
	scn->SetShapeTex(0, 1);
	scn->CustomDraw(1, 0, scn->BACK, false, false, 1);

	
	unsigned char* data3 = halfToneIt(data);
	scn->AddTexture(512, 512, data3);
	scn->SetShapeTex(0, 2);
	scn->CustomDraw(1, 0, scn->BACK, false, false, 2);

	
	unsigned char* data4 = floydIt(data);
	scn->AddTexture(256, 256, data4);
	scn->SetShapeTex(0, 3);
	scn->CustomDraw(1, 0, scn->BACK, false, false, 3);

	scn->Motion();
	display.SwapBuffers();

	while(!display.CloseWindow())
	{
		//scn->Draw(1,0,scn->BACK,true,false);
		//scn->Motion();
		//display.SwapBuffers();
		//display.PollEvents();	
		
		/*scn->SetShapeTex(0, 0);//gray
		glViewport(0, 256, 256, 256);
		scn->Draw(1, 0, scn->BACK, true, false);
		scn->SetShapeTex(0, 1);//edges
		glViewport(256, 256, 256, 256);
		scn->Draw(1, 0, scn->BACK, false, false);
		scn->SetShapeTex(0, 2);//halftone
		glViewport(0, 0, 256, 256);
		scn->Draw(1, 0, scn->BACK, false, false);
		scn->SetShapeTex(0, 3);//floyd
		glViewport(256, 0, 256, 256);
		scn->Draw(1, 0, scn->BACK, false, false);
		scn->Motion();
		display.SwapBuffers();*/
		display.PollEvents();
			
	}
	delete scn;
	return 0;
}
