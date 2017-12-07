#include "bmpReader.h"
#include "bmpReader.cpp"
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#define THREAD_CNT 6
#define MYRED	2
#define MYGREEN 1
#define MYBLUE	0
using namespace std;
int imgWidth, imgHeight, onethread_height;
int FILTER_SIZE;
int FILTER_SCALE;
int *filter_G;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

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

unsigned char sobel_filter(int w, int h)
{
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}
//multithread image processing

void* onethread_process_grey(void* args)
{
	long cur_thread=(long)args;

	int upper_bound=cur_thread+1;
	//last one, uneven divide
	if(cur_thread==THREAD_CNT-1)
	{
		//printf("cur thread last %ld\n",cur_thread);
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
			//printf("cur thread %ld\n",cur_thread);
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
	int onethread_height = imgHeight / THREAD_CNT; //split by row
	pthread_t thread_id[THREAD_CNT];
	for(long cur_thread=0;cur_thread<THREAD_CNT;cur_thread++)
	{
		pthread_create(&thread_id[cur_thread],NULL,onethread_process_grey,(void *) cur_thread);
		//printf("Grey cur thread id created %u\n",&thread_id[cur_thread]);
	}
	for(long  cur_thread=0;cur_thread<THREAD_CNT;cur_thread++)
	{
		//printf("Grey cur thread id pass in %u\n",&thread_id[cur_thread]);
		pthread_join(thread_id[cur_thread],NULL);
	}

}
void* onethread_process_sobel(void* args)
{
	long cur_thread=(long)args;
	int upper_bound=cur_thread+1;
	//last one, uneven divide
	register int tmp;
	if(cur_thread==THREAD_CNT-1)
	{
		for(int i=cur_thread*onethread_height;i<imgHeight;i++)
		{
			tmp=i*imgWidth;
			for(int j=0;j<imgWidth;j++)
			{
				pic_blur[tmp + j]=sobel_filter(j/*col*/,i/*row*/);
			}
		}
	}
	else
	{
		for(int i=cur_thread*onethread_height;i<onethread_height*upper_bound;i++)
		{
			tmp=i*imgWidth;
			for(int j=0;j<imgWidth;j++)
			{
				pic_blur[tmp + j]=sobel_filter(j/*col*/,i/*row*/);
			}
		}
	}
	pthread_exit(EXIT_SUCCESS);
}
void multithread_gaussian()
{
	int onethread_height = imgHeight / THREAD_CNT; //split by row
	pthread_t thread_id[THREAD_CNT];

	for(long  cur_thread=0;cur_thread<THREAD_CNT;cur_thread++)
	{
		pthread_create(&thread_id[cur_thread],NULL,onethread_process_gaussian,(void *) cur_thread);
		//printf("Gaussian cur thread id created%u\n",&thread_id[cur_thread]);
	}
	for(long  cur_thread=0;cur_thread<THREAD_CNT;cur_thread++)
	{
		//printf("Gaussian cur thread id pass in%u \n",&thread_id[cur_thread]);
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
	for (register int k = 0; k<5; k++)
	{
		// read input BMP file
		pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth, &imgHeight);
		// allocate space for output image
		pic_grey = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_blur = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_final = (unsigned char*)malloc(3 * imgWidth*imgHeight*sizeof(unsigned char));
		pthread_mutex_lock(&mutex1);
		multithread_grey();
		multithread_gaussian();
		pthread_mutex_unlock(&mutex1);
		for (register int j = 0; j<imgHeight; j++)
		{
			for (register int i = 0; i<imgWidth; i++)
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
