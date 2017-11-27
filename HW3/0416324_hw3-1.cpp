#include "bmpReader.h"
#include "bmpReader.cpp"
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

#define THREAD_CNT 6
#define MYRED	2
#define MYGREEN 1
#define MYBLUE	0

int imgWidth, imgHeight, onethread_height;
int FILTER_SIZE;
int FILTER_SCALE;
int *filter_G;

const char *inputfile_name[5] =
{
	"input1.bmp",
	"input2.bmp",
	"input3.bmp",
	"input4.bmp",
	"input5.bmp"
};
const char *outputBlur_name[5] =
{
	"Blur1.bmp",
	"Blur2.bmp",
	"Blur3.bmp",
	"Blur4.bmp",
	"Blur5.bmp"
};

unsigned char *pic_in, *pic_grey, *pic_blur, *pic_final;
unsigned char RGB2grey(int w, int h)
{
	int tmp =(pic_in[3 * (h*imgWidth + w) + MYRED] +pic_in[3 * (h*imgWidth + w) + MYGREEN] +pic_in[3 * (h*imgWidth + w) + MYBLUE])/3;
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}
/*for (int i = 0; i<ws; i++)
	{
		a = w + i - (ws / 2);
		b = h + j - (ws / 2);

		// detect for borders of the image
		if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;

		tmp += filter_G[j*ws + i] * pic_grey[b*imgWidth + a];
	}*/
unsigned char GaussianFilter(int w, int h)
{
	//w col, h row
	int tmp = 0;
	int a, b;
	int ws = (int)sqrt((float)FILTER_SIZE);
	for (int j = 0; j<ws; j++)
	{
		switch(ws)
		{
			case 3:
			{
				a = w - (ws / 2);
				b = h + j - (ws / 2);
				if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
				tmp += filter_G[j*ws + 0] * pic_grey[b*imgWidth + a];

				a = w + 1 - (ws / 2);
				b = h + j - (ws / 2);
				if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
				tmp += filter_G[j*ws + 1] * pic_grey[b*imgWidth + a];

				a = w + 2 - (ws / 2);
				b = h + j - (ws / 2);
				if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
				tmp += filter_G[j*ws + 2] * pic_grey[b*imgWidth + a];
				break;
			}

			case 5:
			{
				a = w - (ws / 2);
				b = h + j - (ws / 2);
				if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
				tmp += filter_G[j*ws + 0] * pic_grey[b*imgWidth + a];

				a = w + 1 - (ws / 2);
				b = h + j - (ws / 2);
				if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
				tmp += filter_G[j*ws + 1] * pic_grey[b*imgWidth + a];

				a = w + 2 - (ws / 2);
				b = h + j - (ws / 2);
				if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
				tmp += filter_G[j*ws + 2] * pic_grey[b*imgWidth + a];

				a = w + 3 - (ws / 2);
				b = h + j - (ws / 2);
				if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
				tmp += filter_G[j*ws + 3] * pic_grey[b*imgWidth + a];

				a = w + 4 - (ws / 2);
				b = h + j - (ws / 2);
				if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;
				tmp += filter_G[j*ws + 4] * pic_grey[b*imgWidth + a];
				break;
			}
		}

	}

	tmp /= FILTER_SCALE;
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}
//multithread image processing

void* onethread_process_grey(void* args)
{
	int cur_thread=*((int *)args);

	int upper_bound=cur_thread+1;
	//last one, uneven divide
	if(cur_thread==THREAD_CNT-1)
	{
		printf("cur thread last %d\n",cur_thread);
		for(int i=cur_thread*onethread_height;i<imgHeight;i++)
		{
			for(int j=0;j<imgWidth;j++)
			{
				pic_grey[i*imgWidth + j] = RGB2grey(j, i);
			}
		}
	}
	else
	{
			printf("cur thread %d\n",cur_thread);
		for(int i=cur_thread*onethread_height;i<onethread_height*upper_bound;i++)
		{
			for(int j=0;j<imgWidth;j++)
			{
				pic_grey[i*imgWidth + j] = RGB2grey(j, i);
			}
		}
	}
	pthread_exit(EXIT_SUCCESS);
}
void multithread_grey()
{
	int /*onethread_width = imgWidth / THREAD_CNT, */onethread_height = imgHeight / THREAD_CNT; //split by row
	printf("OTH %d", onethread_height);
	pthread_t thread_id[THREAD_CNT];
	/* pthread_create usage
	int pthread_create(pthread_t *thread,
				  const pthread_attr_t *restrict_attr,
				  void*(*start_rtn)(void*),
				  void *restrict arg);
	*/
	for(int cur_thread=0;cur_thread<THREAD_CNT;cur_thread++)
	{
		pthread_create(&thread_id[cur_thread],NULL,onethread_process_grey,&cur_thread);
	}
	for(int cur_thread=0;cur_thread<THREAD_CNT;cur_thread++)
	{
		pthread_join(thread_id[cur_thread],NULL);
	}

}
void* onethread_process_gaussian(void* args)
{
	int cur_thread=*((int *)args);

	int upper_bound=cur_thread+1;
	//last one, uneven divide
	if(cur_thread==THREAD_CNT-1)
	{
		printf("cur thread last %d\n",cur_thread);
		for(int i=cur_thread*onethread_height;i<imgHeight;i++)
		{
			for(int j=0;j<imgWidth;j++)
			{
				pic_blur[i*imgWidth + j]=GaussianFilter(j/*col*/,i/*row*/);
			}
		}
	}
	else
	{
		printf("cur thread %d\n",cur_thread);
		for(int i=cur_thread*onethread_height;i<onethread_height*upper_bound;i++)
		{
			for(int j=0;j<imgWidth;j++)
			{
				pic_blur[i*imgWidth + j]=GaussianFilter(j/*col*/,i/*row*/);
			}
		}
	}
	pthread_exit(EXIT_SUCCESS);
}
void multithread_gaussian(unsigned char* pic_grey, unsigned char* pic_blur)
{
	int /*onethread_width = imgWidth / THREAD_CNT, */onethread_height = imgHeight / THREAD_CNT; //split by row
	printf("OTH %d", onethread_height);
	pthread_t thread_id[THREAD_CNT];
	/* pthread_create usage
	int pthread_create(pthread_t *thread,
				  const pthread_attr_t *restrict_attr,
				  void*(*start_rtn)(void*),
				  void *restrict arg);
	*/
	for(int cur_thread=0;cur_thread<THREAD_CNT;cur_thread++)
	{
		pthread_create(&thread_id[cur_thread],NULL,onethread_process_gaussian,&cur_thread);
	}
	for(int cur_thread=0;cur_thread<THREAD_CNT;cur_thread++)
	{
		pthread_join(thread_id[cur_thread],NULL);
	}

}
int main()
{
	// read mask file
	FILE* mask;
	mask = fopen("mask_Gaussian.txt", "r");
	fscanf(mask, "%d", &FILTER_SIZE);
	fscanf(mask, "%d", &FILTER_SCALE);

	filter_G = new int[FILTER_SIZE];
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_G[i]);
	fclose(mask);


	BmpReader* bmpReader = new BmpReader();
	for (int k = 0; k<5; k++)
	{
		// read input BMP file
		pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth, &imgHeight);
		// allocate space for output image
		pic_grey = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_blur = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_final = (unsigned char*)malloc(3 * imgWidth*imgHeight*sizeof(unsigned char));

		multithread_grey();
		multithread_gaussian(pic_grey,pic_blur);

		for (int j = 0; j<imgHeight; j++)
		{
			for (int i = 0; i<imgWidth; i++)
			{
				pic_final[3 * (j*imgWidth + i) + MYRED] = pic_blur[j*imgWidth + i];
				pic_final[3 * (j*imgWidth + i) + MYGREEN] = pic_blur[j*imgWidth + i];
				pic_final[3 * (j*imgWidth + i) + MYBLUE] = pic_blur[j*imgWidth + i];
			}
		}

		// write output BMP file
		bmpReader->WriteBMP(outputBlur_name[k], imgWidth, imgHeight, pic_final);

		//free memory space
		free(pic_in);
		free(pic_grey);
		free(pic_blur);
		free(pic_final);
	}

	return 0;
}