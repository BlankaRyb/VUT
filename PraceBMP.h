#pragma once

//#include "resource.h"
//#include "atlstr.h"
//#include "Global.h"
//#include "cislice_pismena.h"

const int POCET_SEDYCH_BAREV = 256;

//RECT rect;
//rect.top = 0;

//CRect rect(0, 0, 100, 50)
BYTE **PrevodCB(BYTE **Sedy, RECT rec = RECT());
void histogramy(long occurrence[256], long int sum_pixels, double* histogram, double* kumlHistogram, int konec);
int otsu_method(double *histogram, double *kumlHistogram, int konec);
double otsu_rozptyl(double* histogram, double* kumlHistogram, double dataRozptyl[256], int konec);
//int *calculateHist(BYTE **Sedy, int all_pixel_count);
//int calculateIntensitySum(BYTE **Sedy);
int Seda(__int16 R, __int16 G, __int16 B);
BYTE **SedaBMP(LPSTR pDIB, HBITMAP hBitmap);
void UlozNovyBMP(LPBITMAPINFOHEADER	lpbmi, BYTE** bPixel, RECT rec);
void UlozNovyBMPSedy(LPBITMAPINFOHEADER	lpbmi, BYTE** bPixel, RECT rec);
double PrevedNaSedouBMP(BITMAP BMP_z, HBITMAP hBitmap, long dataHis[256], double dataRozptyl[256], int konec);
void UrctiTXT(BITMAP BMP, HBITMAP hBitmap, bool bTxt);
