#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <time.h>
// 픽셀 값의 분산을 계산하는 함수
double var(IplImage* img, int y, int x)
{
	int sum = 0;
	double avg, var = 0;
	// 50x50 픽셀 합을 계산
	for (int i = 0; i < 50; i++)
		for (int j = 0; j < 50; j++)
			sum += cvGet2D(img, y + i, x + j).val[0];
	avg = sum / 2500.0;// 평균 계산
	// 분산 계산
	for (int i = 0; i < 50; i++)
		for (int j = 0; j < 50; j++)
			var += pow((cvGet2D(img, y + i, x + j).val[0] - avg), 2);
	return var / 2500.0;
}
int main() {
	clock_t start, end;
	double result;
	char str[100];

	printf("Input File Name: ");
	scanf("%s", str);
	IplImage* src = cvLoadImage(str);	//원본 이미지
	if (src == NULL)	return -1;

	start = clock();// 시간 측정 시작
	CvSize src_size = cvGetSize(src);	//원본 이미지의 사이즈
	int w = src_size.width - 40;
	int h = (src_size.height - 40) / 3;
	CvSize size = cvSize(w, h);	//자른 이미지의 사이즈

	IplImage* dst = cvCreateImage(size, 8, 3);	//최종 이미지
	IplImage* img1 = cvCreateImage(size, 8, 3);	//blue
	IplImage* img2 = cvCreateImage(size, 8, 3);	//green
	IplImage* img3 = cvCreateImage(size, 8, 3);	//red

	// 각각의 채널로 분리
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			CvScalar c = cvGet2D(src, y + 20, x + 20);
			cvSet2D(img1, y, x, c);
			c = cvGet2D(src, y + h + 20, x + 20);
			cvSet2D(img2, y, x, c);
			c = cvGet2D(src, y + 2 * h + 20, x + 20);
			cvSet2D(img3, y, x, c);
		}
	}
	// 임의의 세 범위의 분산 계산
	double var1 = var(img1, h / 2, w / 2);
	double var2 = var(img1, h / 3, w / 3);
	double var3 = var(img1, h / 3 * 2, w / 3 * 2);
	int a, b;
	//분산이 가장 큰 값 찾기
	if (var1 >= var2 && var1 >= var3) {
		a = h / 2; b = w / 2;
	}
	if (var2 >= var1 && var2 >= var3) {
		a = h / 3; b = w / 3;
	}
	if (var3 >= var2 && var3 >= var1) {
		a = h / 3 * 2; b = w / 3 * 2;
	}

	int min_u1 = 0;
	int min_v1 = 0;
	int min_u2 = 0;
	int min_v2 = 0;
	float min_err1 = FLT_MAX;
	float min_err2 = FLT_MAX;
	// 이미지 정렬
	for (int v = -30; v <= 30; v++)
		for (int u = -10; u <= 10; u++)
		{
			float err1 = 0.0f;
			int ct = 0;
			float err2 = 0.0f;
			for (int y = a; y < a + 50; y++) {
				int y1 = y;
				int y2 = y + v;

				if (y2 < 0 || y2 >= h) continue;
				for (int x = b; x < b + 50; x++)
				{
					int x1 = x;
					int x2 = x + u;
					if (x2 < 0 || x2 >= w) continue;
					CvScalar f1 = cvGet2D(img1, y1, x1);
					CvScalar f2 = cvGet2D(img2, y2, x2);
					CvScalar f3 = cvGet2D(img3, y2, x2);
					//색 차이 계산
					err1 += (f1.val[0] - f2.val[0]) * (f1.val[0] - f2.val[0]);
					err2 += (f1.val[0] - f3.val[0]) * (f1.val[0] - f3.val[0]);
					ct++;
				}

			}
			//색 차이가 적은 부분 찾기
			err1 /= ct;
			err2 /= ct;
			if (err1 < min_err1)
			{
				min_err1 = err1;
				min_u1 = u;
				min_v1 = v;
			}
			if (err2 < min_err2)
			{
				min_err2 = err2;
				min_u2 = u;
				min_v2 = v;
			}
		}
	// 이미지 정렬 및 결합
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++) {
			int x1 = x;
			int y1 = y;
			int x2 = x + min_u1;
			int y2 = y + min_v1;
			int x3 = x + min_u2;
			int y3 = y + min_v2;

			if (x2 < 0 || x2 >= w || y2 < 0 || y2 >= h) continue;
			if (x3 < 0 || x3 >= w || y3 < 0 || y3 >= h) continue;

			CvScalar f1 = cvGet2D(img1, y1, x1);
			CvScalar f2 = cvGet2D(img2, y2, x2);
			CvScalar f3 = cvGet2D(img3, y3, x3);

			CvScalar g;
			g.val[0] = f1.val[0];
			g.val[1] = f2.val[1];
			g.val[2] = f3.val[2];
			cvSet2D(dst, y, x, g);
		}

	cvShowImage("dst", dst);
	end = clock(); //시간 측정 끝
	result = (double)(end - start) / CLOCKS_PER_SEC;
	printf("%f", result);
	cvWaitKey();
	return 0;
}