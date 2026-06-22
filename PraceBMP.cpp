#include "stdafx.h"
#include "PraceBMP.h"
#include "atlstr.h"

__int16 svetly, tmavy;  //hodnoty v sede nejtamvsiho a nejsvetlejsiho bodu obrazku
BITMAP BMP;
CString FileNew;  //jmeno noveho souboru

const int konec = 104;//190; //kde konci histogram

struct {
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[256];
} pbmi;


/********************************************************************************
*
* Bitmapu prevede na sede odstiny
*
* BMP - informace z BMP
* hBitmap - HBITMAP
* dataHis [256] vraci - jsou tam absolutni hodnoty histogramu
* dataRozptyl [256] vraci - hodnoty rozptylu v histogramu
* konec kde konci histogram standartne je histogram do 255, ale muze byt oriznut
*
* Vraci maximalni rozptyl
********************************************************************************/
//long *PrevedNaSedouBMP(BITMAP BMP_z, HBITMAP hBitmap)
double PrevedNaSedouBMP(BITMAP BMP_z, HBITMAP hBitmap, long dataHis[256], double dataRozptyl[256], int konec)
{
	//Bitmap* bitmap; //MFC nebo ATL

	LPSTR    pDIB;
	BYTE    **Sedy;//sedy obr
	int i, j;
	long int sum_pixels; //pocet pixelu v obraze
	double histogram[256];
	double kumlHistogram[256];
	double maxRozptyl;

	//nacteni pbmi
	HDC hDC = GetDC(NULL);  // any DC will work

	// inicializace
	for (int i = 0; i <= 255; i++) dataHis[i] = 0;

	memset(&pbmi, 0, sizeof(pbmi));
	pbmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(hDC, hBitmap, 0, 1, NULL, (BITMAPINFO *)&pbmi,
		DIB_RGB_COLORS);

	if (pbmi.bmiHeader.biBitCount <= 8)
	{
		GetDIBits(hDC, hBitmap, 0, 1, NULL, (BITMAPINFO *)&pbmi,
			DIB_RGB_COLORS);
	}

	BMP = BMP_z;
	//pole s informacemi o pixelech
	pDIB = (LPSTR)BMP.bmBits;

	//prevod na sede odstiny
	Sedy = SedaBMP(pDIB, hBitmap);

	RECT rect;
	rect.bottom = (LONG)(0 + BMP.bmHeight);
	rect.top = 0;
	rect.left = 0;
	rect.right = (LONG)(0 + BMP.bmWidth);

	sum_pixels = 0;
	// data dana do histogramu
	for (i = rect.top; i < rect.bottom; i++)
	{
		for (j = rect.left; j < rect.right; j++)
		{
			if ((__int64)((BYTE)(Sedy[i][j])) < konec)
			{
				//pocet vyskytu sede //histogram
				dataHis[(__int64)((BYTE)(Sedy[i][j]))] += 1;// 0x1f;//++;
				sum_pixels++;
			}
		}
	}

	histogramy(dataHis, sum_pixels, histogram, kumlHistogram, konec);
	maxRozptyl = otsu_rozptyl(histogram, kumlHistogram, dataRozptyl, konec);

	//	UlozNovyBMP(hBitmap, (BYTE)Sedy, rect);
	//	UlozNovyBMP(BMP, (BYTE**)Sedy, rect);
	UlozNovyBMPSedy(&pbmi.bmiHeader, Sedy, rect);
	free(Sedy);
	return maxRozptyl;
}

/********************************************************************************
*
* Z hBitmap urci TXT
*
* BMP - informace z BMP
* hBitmap - HBITMAP
* bTxt - true bitmapa obsahuje text s SPZ, hleda SPZ v bitmape, ale byt tam nemusi 
*      - false bitmapa obsahuje jen jeden znak, MUSI tam byt jen jeden znak (slouzi pro otestovani metody)
*
********************************************************************************/
void UrctiTXT(BITMAP BMP_z, HBITMAP hBitmap, bool bTxt)
{

	//Bitmap* bitmap; //MFC nebo ATL

	LPSTR    pDIB;
	//LPBITMAPFILEHEADER lpbmf; //BITMAPFILEHEADER
	//LPBITMAPINFOHEADER lpbmi; // BITMAPINFOHEADER
//	HPALETTE hPal;
//	LPPALETTEENTRY palBarvy; //paleta barev z puvodniho obrazku
//	LOGPALETTE logpal;
	BYTE    **Sedy;//sedy obr
	BYTE    **CB;//cerno bily obr

	//nacteni pbmi
	HDC hDC = GetDC(NULL);  // any DC will work

	memset(&pbmi, 0, sizeof(pbmi));
	pbmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(hDC, hBitmap, 0, 1, NULL, (BITMAPINFO *)&pbmi,
		DIB_RGB_COLORS);

	if (pbmi.bmiHeader.biBitCount <= 8)
	{
		GetDIBits(hDC, hBitmap, 0, 1, NULL, (BITMAPINFO *)&pbmi,
			DIB_RGB_COLORS);
	}

	//paleta
/*	hPal = NULL;
	//hPal = CreatePalette((LOGPALETTE)pbmi.bmiColors[0]);
	if (hPal == NULL)
	{
		// neni paleta
		//2^24=16777216 nema pelatu
		delete hPal;
		hPal = NULL;
	}

	//paleta berev puvodniho obrazku
	//palBarvy = BarvyPalety(lpbmi, hPal);

	//paleta
	hPal = NULL;*/

	BMP = BMP_z;
	//pole s informacemi o pixelech
	pDIB = (LPSTR)BMP.bmBits;

	//prevod na sede odstiny
	Sedy = SedaBMP(pDIB, hBitmap);
	RECT rect;
	rect.bottom = 0;
	rect.top = (LONG)(0 + BMP.bmHeight);
	rect.left = 0;
	rect.right = (LONG)(0 + BMP.bmWidth);

	//prahovani //prevod na cerno-bily
/*	RECT r1;
	r1.bottom = 0;
	r1.top = BMP.bmHeight / 4;
	r1.left = 0;
	r1.right = BMP.bmWidth / 4;*/
	//prevod pouze na cerno-bily snimek
	CB = PrevodCB(Sedy);

	//	UlozNovyBMP(hBitmap, (BYTE)Sedy, rect);
	//	UlozNovyBMP(BMP, (BYTE**)Sedy, rect);
	UlozNovyBMP(&pbmi.bmiHeader, CB, rect);
	free(Sedy);
	free(CB);
}

/*******************************************************************
*
* Prevede pole Sedy na cernobile pole
*
* Sedy - bit mapa v sedych odstinech
* rect - obdelnik ve kterem se pocita ostrost (pouzije se pro histogram v metode OTSU)
*        vetsinou obdelnik neni zadan a ma hodnoty (0, 0, 0, 0). pak se pocita pres celou BMP
*
*******************************************************************/
BYTE **PrevodCB(BYTE **Sedy, RECT rect)
{
	BYTE  **bCB; //obr v cernobile
	int i, j;
	double histogram[256];
	double kumlHistogram[256];
	long int sum_pixels; //pocet pixelu v obraze
	long occurrence[256]; //pro hystogram nenormovana/absolutni data
	int rozdeleni_his;
	long Height1, Height2, Width1, Width2; //pro obelnik ve ketrem se pocita hranice podle OTSU pro pixely, ktere maji byt cerne a ktere bile
					//ale hranice se pak puzije na cely obelnik

	if (rect.top == 0 && rect.bottom == 0 && rect.left == 0 && rect.right == 0) //obenik neni zadan
	{
		Height1 = 0;
		Height2 = BMP.bmHeight;
		Width1 = 0;
		Width2 = BMP.bmWidth;
	}
	else
	{
		Height1 = rect.bottom;
		Height2 = rect.top;
		Width1 = rect.left;
		Width2 = rect.right;
	}

	//OTSU
	////////////////////////////////////////////////////////////////////////////////////////////////
	//https://en.wikipedia.org/wiki/Otsu%27s_method
	
	sum_pixels = 0;

	// inicializace
	for (int i = 0; i <= 255; i++) occurrence[i] = 0;
	for (int i = 0; i <= 255; i++) histogram[i] = 0;

	// data dana do histogramu
	for (i = Height1; i < Height2; i++)
	{
		for (j = Width1; j < Width2; j++)
		{
			if ((__int64)((BYTE)(Sedy[i][j])) < konec)
			{
				//pocet vyskytu sede //histogram
				occurrence[(__int64)((BYTE)(Sedy[i][j]))] += 1;// 0x1f;//++;
				//occurrence[(__int64)((BYTE)(Sedy[i][j]))] = occurrence[(__int64)((BYTE)(Sedy[i][j]))] + 1;
				sum_pixels++;
			}
		}
	}

	/*******************************************************************
	* Histogramy pro metody otsu
	*******************************************************************/
	histogramy(occurrence, sum_pixels, histogram, kumlHistogram, konec);

	rozdeleni_his = otsu_method(histogram, kumlHistogram, konec);

	/*******************************************************************
	* Prevod na cerno bily obr
	*******************************************************************/
	bCB = (BYTE**)calloc(BMP.bmHeight, sizeof(BYTE*));
	for (i = 0; i < BMP.bmHeight; i++)
	{
		bCB[i] = (BYTE*)calloc(BMP.bmWidth, sizeof(BYTE));
	}

	for (i = 0; i < BMP.bmHeight; i++)
	{
		for (j = 0; j < BMP.bmWidth; j++)
		{
			if ((__int64)((BYTE)(Sedy[i][j])) <= rozdeleni_his)
			{
				bCB[i][j] = 0;
			}
			else
			{
				bCB[i][j] = (BYTE)255;
			}

		}
	}

	return bCB;
}

/*******************************************************************
*
* Funkce spocte histogramy, normalni histogram a kumulovany histogram pro mwetou OTSU
*
* occurrence[256] - pro hystogram nenormovana/absolutni data
* sum_pixels - pocet pixelu v obraze (pro orezany histogram jsou zmenseny o pixely lezici mimo)
* histogram - Normovany histogram
* kumlHistogram - Kumulovany histogram
* konec - pokud se pocita histogram
*
*******************************************************************/
void histogramy(long occurrence[256], long int sum_pixels, double* histogram, double* kumlHistogram, int konec)
{
	if ((konec < 1) || (konec > 256))
	{
		konec = 256;
	}

	//Normovany histogram
	for (int i = 0; i < konec; i++)
	{
		/* Vezme pocet vyskytu konkretniho pixelu
		* a vydeli jej celkovym poctem pixelu */
		histogram[i] = (double)occurrence[i] / (double)sum_pixels;
	}
	//Kumulovany histogram
	kumlHistogram[0] = histogram[0];
	for (int i = 1; i < konec; i++)
	{
		kumlHistogram[i] = histogram[i];
		kumlHistogram[i] += kumlHistogram[i - 1];
	} // for
}
/*******************************************************************
*
* metoda Otsu - na zaklade histogramu a poctu bodu urci rozdeleni v histogramu - dolni body cerne, horni bile
* prevede sedy obrazek na cernobily, podle hitogramu (cetnosti bodu).
*
* Hleda maximální rozptyl
*
* histogram - Normovany histogram
* kumlHistogram - Kumulovany histogram
* konec kde konci histogram standartne je histogram do 255, ale muze byt oriznut
*
* vraci max rozptyl
*******************************************************************/
//Hledani maximalniho rozptylu
int otsu_method(double *histogram, double *kumlHistogram, int konec)
{
	double omega0 = 0;
	double omega1 = 0;
	double mi0 = 0;
	double mi1 = 0;
	double mit = 0;
	int ix;
	double mik = 0;
	int ixKMaxRozptyl = 0;
	double maxRozptyl = 0;
	double Rozptyl = 0;
	int i;

	if ((konec < 1) || (konec > 256))
	{
		konec = 256;
	}

	//Koeficient mit
	for (i = 0; i < konec; i++)
		mit += (i + 1) * histogram[i];
 
    for (ix = 1; ix < konec; ix++)
	{
		omega0 = kumlHistogram[ix - 1];
		omega1 = 1 - omega0;
        mik = 0;
		for (int j = 0; j < ix; j++)
		{
			mik += (j + 1) * histogram[j];
		} // for
		mi0 = mik / omega0;
		mi1 = (mit - mik) / omega1;
		Rozptyl = omega0 * omega1 * ((mi1 - mi0) * (mi1 - mi0));
		if (Rozptyl >= maxRozptyl)
		{
			maxRozptyl = Rozptyl;
			ixKMaxRozptyl = ix;
		} // if
    } // for

	return ixKMaxRozptyl;
}

/*******************************************************************
*
* otsu_rozptyl - vrati 256 hodnot rozptylu. (Pro kresleni rozptylove krivky v hitogramu)
*
* histogram - Normovany histogram
* kumlHistogram - Kumulovany histogram
* dataRozptyl - hodnoty rozptylu v histogramu
* konec - pokud se pocita histogram
*
* vraci maximalni rozptyl
******************************************************************/
//Hledani maximalniho rozptylu
double otsu_rozptyl(double* histogram, double* kumlHistogram, double dataRozptyl[256], int konec)
{
	double omega0 = 0;
	double omega1 = 0;
	double mi0 = 0;
	double mi1 = 0;
	double mit = 0;
	int ix;
	double mik = 0;
//	double Rozptyl = 0;
	double maxRozptyl = 0;
	int i;

	maxRozptyl = 0;

	if ((konec < 1) || (konec > 256))
	{
		konec = 256;
	}

	//Koeficient mit
	for (i = 0; i < konec; i++)
		mit += (i + 1) * histogram[i];

	for (ix = 1; ix < konec; ix++)
	{
		omega0 = kumlHistogram[ix - 1];
		omega1 = 1 - omega0;
		mik = 0;
		for (int j = 0; j < ix; j++)
		{
			mik += (j + 1) * histogram[j];
		} // for
		mi0 = mik / omega0;
		mi1 = (mit - mik) / omega1;
		dataRozptyl[ix-1] = omega0 * omega1 * ((mi1 - mi0) * (mi1 - mi0));
		if (dataRozptyl[ix - 1] >= maxRozptyl)
		{
			maxRozptyl = dataRozptyl[ix - 1];
		} // if
	} // for

	return maxRozptyl;
}

/*******************************************************************
vrati podle RGB, sedou barvu (pro vsechny 3 slozky barvy stejnou hodnotu)
R,G,B - baravne hodnoty jedne barvy
*******************************************************************/
int Seda(__int16 R, __int16 G, __int16 B)
{
	int seda;

	seda = (int)(0.299*R + 0.587*G + 0.114*B + .5);

	return seda;
}

/*******************************************************************
vrati podle RGB, sedou barvu (pro vsechny 3 slozky barvy stejnou hodnotu)
barva - z palety, ktere se priradi hodnota v sede od 0..255
*******************************************************************/
int Seda(RGBQUAD barva)
{
	int seda;

	seda = (int)(0.299*barva.rgbRed + 0.587*barva.rgbGreen + 0.114*barva.rgbBlue + .5);

	return seda;
}

/*******************************************************************
prevede BMP na odstiny sede
pDIB - retez prvku, kde za sebou jsou pixely bitmapy
hBitmap - puvodni BMP
return - vraci retezec prku, kde za sebou jsou pixely bitmapy (sede)
*******************************************************************/
BYTE **SedaBMP(LPSTR pDIB, HBITMAP hBitmap)
{
	BYTE  **bSeda; //obr v sede
	int i, j, ix, jy;
	__int16 ii, i1, iir, iig, iib;    //pro vypocet barev
	__int64 iPoradi;   //Poradi v pDIB
	int iZarovnat; //byty na zarovnani radku na 32 bitovy okraj
	double  d;

	iZarovnat = 0;
	//pro 256 stupnu sedi. Pri pruchodu BMP najde hodnotu nejsvìtlejsi a nejtmavsi, pro start jsou hodnoty nastaveny opacne
	tmavy = 255; //nejtmasi bod
	svetly = 0;   //nejsvetlejsi bod

	bSeda = (BYTE**)calloc(BMP.bmHeight, sizeof(BYTE*));
	for (i = 0; i < BMP.bmHeight; i++)
	{
		bSeda[i] = (BYTE*)calloc(BMP.bmWidth, sizeof(BYTE));
	}

	//Pro rychlejsi praci se data nactou do bSeda, ale pouze ve stupnich sede,
	iPoradi = 0;
	//rozdeleni podle poctu brev
	switch (BMP.bmBitsPixel)//(lpbmi->biBitCount)
	{
	case 1:  //2^1=2 barvy
		//vypocet zbytku radku upraveneho na 32 bitovy okraj
		d = ((double)BMP.bmWidth / 32) - floor((double)BMP.bmWidth / 32);
		//if ((0.78125 <= d)||(d == 0   )) iZarovnat = 0;
		if ((0.03125 <= d) && (d <= 0.25)) iZarovnat = 3;
		if ((0.28125 <= d) && (d <= 0.5)) iZarovnat = 2;
		if ((0.53125 <= d) && (d <= 0.75)) iZarovnat = 1;
		ix = 0;
		for (i = 0; i < BMP.bmHeight; i++)
		{
			jy = 0;
			for (j = 0; j < BMP.bmWidth; j++)
			{
				for (i1 = 0; i1 < 8; i1++)
				{
					switch (i1)
					{
					case 0:
						ii = (__int16)(pDIB[iPoradi] & 0x80);
						break;
					case 1:
						ii = (__int16)(pDIB[iPoradi] & 0x40);
						j++;
						break;
					case 2:
						ii = (__int16)(pDIB[iPoradi] & 0x20);
						j++;
						break;
					case 3:
						ii = (__int16)(pDIB[iPoradi] & 0x10);
						j++;
						break;
					case 4:
						ii = (__int16)(pDIB[iPoradi] & 0x8);
						j++;
						break;
					case 5:
						ii = (__int16)(pDIB[iPoradi] & 0x4);
						j++;
						break;
					case 6:
						ii = (__int16)(pDIB[iPoradi] & 0x2);
						j++;
						break;
					case 7:
						ii = (__int16)(pDIB[iPoradi] & 0x1);
						j++;
						break;
					default:
						ii = 0;
						break;
					}
					if (j < BMP.bmWidth)
					{
						//zjisteni cisla barvy (tady jen 0 nebo 1)
						ii = (__int16)(ii >> (7 - i1));

						bSeda[i][j] = Seda(pbmi.bmiColors[ii]);
						if (tmavy > (BYTE)bSeda[i][j])
						{
							tmavy = (BYTE)bSeda[i][j];
						}
						if (svetly < (BYTE)bSeda[i][j])
						{
							svetly = (BYTE)bSeda[i][j];
						}

						jy++;
					}
					else //je za rozmerem obrazku
					{
						i1 = 7;
					}
				}
				iPoradi++;
			}
			if (jy > 0)
			{
				ix++;
			}
			iPoradi += iZarovnat;
		}
		break;
	case 4:  //2^4=16 barev
		//vypocet zbytku radku upraveneho na 32 bitovy okraj
		d = ((double)BMP.bmWidth / 8) - floor((double)BMP.bmWidth / 8);
		//if ((d == 0    )||(d == 0.875)) iZarovnat = 0;
		if ((d == 0.125) || (d == 0.25)) iZarovnat = 3;
		if ((d == 0.375) || (d == 0.5)) iZarovnat = 2;
		if ((d == 0.625) || (d == 0.75)) iZarovnat = 1;
		ix = 0;
		for (i = 0; i < BMP.bmHeight; i++)
		{
			jy = 0;
			for (j = 0; j < BMP.bmWidth; j++)
			{
				ii = (__int16)(pDIB[iPoradi] & 0xF0);
				ii = (__int16)(ii >> 4);
				bSeda[i][j] = Seda(pbmi.bmiColors[ii]);
				if (tmavy > (BYTE)bSeda[i][j])
				{
					tmavy = (BYTE)bSeda[i][j];
				}
				if (svetly < (BYTE)bSeda[i][j])
				{
					svetly = (BYTE)bSeda[i][j];
				}

				jy++;
				j++;

				if (j < BMP.bmWidth)
				{
					ii = (__int16)(pDIB[iPoradi] & 0xF);
					bSeda[i][j] = Seda(pbmi.bmiColors[ii]);
					if (tmavy > (BYTE)bSeda[i][j])
					{
						tmavy = (BYTE)bSeda[i][j];
					}
					if (svetly < (BYTE)bSeda[i][j])
					{
						svetly = (BYTE)bSeda[i][j];
					}
					jy++;
				}
				iPoradi++;
			}
			if (jy > 0)
			{
				ix++;
			}
			iPoradi += iZarovnat;
		}
		break;
	case 8:  //2^8=256 barev
		//vypocet zbytku radku upraveneho na 32 bitovy okraj
		d = ((double)BMP.bmWidth / 4) - floor((double)BMP.bmWidth / 4);
		//if (d == 0   ) iZarovnat = 0;
		if (d == 0.25) iZarovnat = 3;
		if (d == 0.5) iZarovnat = 2;
		if (d == 0.75) iZarovnat = 1;
		ix = 0;
		for (i = 0; i < BMP.bmHeight; i++)
		{
			jy = 0;
			for (j = 0; j < BMP.bmWidth; j++)
			{
				ii = (__int16)(pDIB[iPoradi] & 0xff);
				//				bPuvodni[i][j] = Seda(palBarvy[lpDIBBits[iPoradi] & 0xff]);
				bSeda[i][j] = Seda(pbmi.bmiColors[ii]);
				if (tmavy > (BYTE)bSeda[i][j])
				{
					tmavy = (BYTE)bSeda[i][j];
				}
				if (svetly < (BYTE)bSeda[i][j])
				{
					svetly = (BYTE)bSeda[i][j];
				}
/*				if ((BYTE)bSeda[i][j] <= 80)
				{
					bSeda[i][j] = 0;
				}
				else
				{
					bSeda[i][j] = (LPSTR)255;
				}*/
				jy++;
				iPoradi++;
			}
			if (jy > 0)
			{
				ix++;
			}
			iPoradi += iZarovnat;
		}
		break;
	case 24: //2^24 = 16777216 barev
		//vypocet zbytku radku upraveneho na 32 bitovy okraj
		d = ((double)BMP.bmWidth / 4) - floor((double)BMP.bmWidth / 4);
		//if (d == 0   ) iZarovnat = 0;
		if (d == 0.25) iZarovnat = 1;
		if (d == 0.5) iZarovnat = 2;
		if (d == 0.75) iZarovnat = 3;
		ix = 0;
		for (i = 0; i < BMP.bmHeight; i++)
		{
			jy = 0;
			for (j = 0; j < BMP.bmWidth; j++)
			{
				iir = (__int16)(pDIB[iPoradi] & 0xff);
				iig = (__int16)(pDIB[iPoradi + 1] & 0xff);
				iib = (__int16)(pDIB[iPoradi + 2] & 0xff);
				//				bSeda[ix][jy] = Seda(lpDIBBits[iPoradi] & 0xff, lpDIBBits[iPoradi + 1] & 0xff, lpDIBBits[iPoradi + 2] & 0xff);
				bSeda[ix][jy] = Seda(iir, iig, iib);
				if (tmavy > (BYTE)bSeda[i][j])
				{
					tmavy = (BYTE)bSeda[i][j];
				}
				if (svetly < (BYTE)bSeda[i][j])
				{
					svetly = (BYTE)bSeda[i][j];
				}
				jy++;
				iPoradi += 3;
			}
			if (jy > 0)
			{
				ix++;
			}
			iPoradi += iZarovnat;
		}
		break;
	default:
		for (i = 0; i < BMP.bmHeight; i++)
		{
			for (j = 0; j < BMP.bmWidth; j++)
			{
				bSeda[i][j] = 0;
				iPoradi++;
			}
		}
	}


	return bSeda;
}

/*******************************************************************
Ulozi obrazek do a.bmp
zde BMP je jen cerno-bila
lpbmi - LPBITMAPINFOHEADER - informace o obrazku
**bPixel - matice pixelu
rec - RECT ktery se ma z BMP ulozit
*******************************************************************/
void UlozNovyBMP(LPBITMAPINFOHEADER lpbmi, BYTE **bPixel, RECT rec)
{
	BOOL bRead;
	LPSTR pDIB;
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bmiHeader;
	PALETTEENTRY palBarvy[2]; //paleta barev jen cerna a bila
	__int32 i, j;
	__int16 ii, i1;
	__int32 iPoradi;   //Poradi v pDIB
	long    Width, Height;

	DWORD dwBytesRead = 0;
	__int16 iZarovnat; //byty na zarovnani radku na 32 bitovy okraj
	double  d;

	iZarovnat = 0;

	Width = abs(rec.right - rec.left);
	Height = abs(rec.bottom - rec.top);

	//	LPCTSTR szFile = "a.bmp";
	// vytvori a.bmp
	if (FileNew == "")
	{
		FileNew = "a.bmp";
	}
	HANDLE hFile = CreateFile(FileNew, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	_ASSERTE(INVALID_HANDLE_VALUE != hFile);

	/////////////// BITMAPFILEHEADER
	bmfHeader.bfType = 0x4d42;
	bmfHeader.bfSize = 0x3e + (DWORD)ceil((double)Width / 32) * 4 * Height;
	bmfHeader.bfReserved1 = 0;
	bmfHeader.bfReserved2 = 0;
	bmfHeader.bfOffBits = 0x3e;

	bRead = WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesRead, NULL);
	_ASSERTE(FALSE != bRead);

	//BITMAPINFOHEADER
	bmiHeader.biSize = lpbmi->biSize;
	bmiHeader.biWidth = Width;
	bmiHeader.biHeight = Height;
	bmiHeader.biPlanes = lpbmi->biPlanes;
	bmiHeader.biBitCount = 1;
	bmiHeader.biCompression = lpbmi->biCompression;
	bmiHeader.biSizeImage = (DWORD)ceil((double)Width / 32) * 4 * Height;
	bmiHeader.biXPelsPerMeter = lpbmi->biXPelsPerMeter;
	bmiHeader.biYPelsPerMeter = lpbmi->biYPelsPerMeter;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;
	bRead = WriteFile(hFile, (LPSTR)&bmiHeader, sizeof(BITMAPINFOHEADER), &dwBytesRead, NULL);
	_ASSERTE(FALSE != bRead);

	//PALETA - 2 barvy
	palBarvy[0].peRed = 0;
	palBarvy[0].peGreen = 0;
	palBarvy[0].peBlue = 0;
	palBarvy[0].peFlags = 0;
	palBarvy[1].peRed = 0xff;
	palBarvy[1].peGreen = 0xff;
	palBarvy[1].peBlue = 0xff;
	palBarvy[1].peFlags = 0;
	bRead = WriteFile(hFile, (LPSTR)&palBarvy, 2 * sizeof(PALETTEENTRY), &dwBytesRead, NULL);
	_ASSERTE(FALSE != bRead);

	//PIXELY

	pDIB = (LPSTR)calloc(bmiHeader.biSizeImage, sizeof(BYTE));

	//vypocet zbytku radku upraveneho na 32 bitovy okraj
	d = ((double)Width / 32) - floor((double)Width / 32);
	//if ((0.78125 <= d)||(d == 0   )) iZarovnat = 0;
	if ((0.03125 <= d) && (d <= 0.25)) iZarovnat = 3;
	if ((0.28125 <= d) && (d <= 0.5)) iZarovnat = 2;
	if ((0.53125 <= d) && (d <= 0.75)) iZarovnat = 1;

	iPoradi = 0;
	for (i = 0; i < Height; i++)
	{
		ii = 0;
		i1 = 0;
		for (j = 0; j < Width; j++)
		{
			switch (i1)
			{
			case 0:
				if (bPixel[i][j] == 255)
				{
					ii = 0x80;
				}
				break;
			case 1:
				if (bPixel[i][j] == 255)
				{
					ii += 0x40;
				}
				break;
			case 2:
				if (bPixel[i][j] == 255)
				{
					ii += 0x20;
				}
				break;
			case 3:
				if (bPixel[i][j] == 255)
				{
					ii += 0x10;
				}
				break;
			case 4:
				if (bPixel[i][j] == 255)
				{
					ii += 0x8;
				}
				break;
			case 5:
				if (bPixel[i][j] == 255)
				{
					ii += 0x4;
				}
				break;
			case 6:
				if (bPixel[i][j] == 255)
				{
					ii += 0x2;
				}
				break;
			case 7:
				if (bPixel[i][j] == 255)
				{
					ii += 0x1;
				}
				pDIB[iPoradi] = (char)ii;
				iPoradi++;
				ii = 0;
				i1 = -1;
				break;
			default:
				ii += 0;
				break;
			}
			i1++;
		}
		if (i1 != 0) //nepropbehlo zapamotovani pixelu
		{
			pDIB[iPoradi] = (CHAR)ii;
			iPoradi++;
		}
		for (i1 = 0; i1 < iZarovnat; i1++)
		{
			pDIB[iPoradi] = 0;
			iPoradi++;
		}
	}

	bRead = WriteFile(hFile, pDIB, iPoradi, &dwBytesRead, NULL);
	_ASSERTE(FALSE != bRead);

	CloseHandle(hFile);

}

/*******************************************************************
Ulozi obrazek do a.bmp
zde BMP je jen seda 256 barev
lpbmi - LPBITMAPINFOHEADER - informace o obrazku
**bPixel - matice pixelu
rec - RECT ktery se ma z BMP ulozit
*******************************************************************/
void UlozNovyBMPSedy(LPBITMAPINFOHEADER lpbmi, BYTE** bPixel, RECT rec)
{
	BOOL bRead;
	LPSTR pDIB;
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bmiHeader;
	PALETTEENTRY palBarvy[256]; //paleta barev 256 barev
	__int32 i, j;
	__int16 ii, i1;
	__int32 iPoradi;   //Poradi v pDIB
	long    Width, Height;

	DWORD dwBytesRead = 0;
	__int16 iZarovnat; //byty na zarovnani radku na 32 bitovy okraj
	double  d;

	iZarovnat = 0;

	Width = abs(rec.right - rec.left);
	Height = abs(rec.bottom - rec.top);

	//	LPCTSTR szFile = "a.bmp";
	// vytvori a.bmp
	if (FileNew == "")
	{
		FileNew = "a.bmp";
	}
	HANDLE hFile = CreateFile(FileNew, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	_ASSERTE(INVALID_HANDLE_VALUE != hFile);

	/////////////// BITMAPFILEHEADER
	bmfHeader.bfType = 0x4d42;
	bmfHeader.bfSize = 0x36 + 256 * 4 + (DWORD)ceil((double)Width / 4) * 4 * Height;// 36 hlavicky, 256*4 paleta, pak pixely //32/8=4
	bmfHeader.bfReserved1 = 0;
	bmfHeader.bfReserved2 = 0;
	bmfHeader.bfOffBits = 0x36 + 256 * 4;

	bRead = WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesRead, NULL);
	_ASSERTE(FALSE != bRead);

	//BITMAPINFOHEADER
	bmiHeader.biSize = lpbmi->biSize;
	bmiHeader.biWidth = Width;
	bmiHeader.biHeight = Height;
	bmiHeader.biPlanes = lpbmi->biPlanes;
	bmiHeader.biBitCount = 8;//256 barev
	bmiHeader.biCompression = lpbmi->biCompression;
	bmiHeader.biSizeImage = (DWORD)ceil((double)Width / 4) * 4 * Height;
	bmiHeader.biXPelsPerMeter = lpbmi->biXPelsPerMeter;
	bmiHeader.biYPelsPerMeter = lpbmi->biYPelsPerMeter;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;
	bRead = WriteFile(hFile, (LPSTR)&bmiHeader, sizeof(BITMAPINFOHEADER), &dwBytesRead, NULL);
	_ASSERTE(FALSE != bRead);

	//PALETA - 256 barev
	for (i = 0; i < 256; i++)
	{
		palBarvy[i].peRed = i;
		palBarvy[i].peGreen = i;
		palBarvy[i].peBlue = i;
		palBarvy[i].peFlags = 0;
	}
	bRead = WriteFile(hFile, (LPSTR)&palBarvy, 256 * sizeof(PALETTEENTRY), &dwBytesRead, NULL);
	_ASSERTE(FALSE != bRead);

	//PIXELY

	pDIB = (LPSTR)calloc(bmiHeader.biSizeImage, sizeof(BYTE));

	//vypocet zbytku radku upraveneho na 32 bitovy okraj
	d = ((double)BMP.bmWidth / 4) - floor((double)BMP.bmWidth / 4);
	//if (d == 0   ) iZarovnat = 0;
	if (d == 0.25) iZarovnat = 3;
	if (d == 0.5)  iZarovnat = 2;
	if (d == 0.75) iZarovnat = 1;

	iPoradi = 0;
	for (i = 0; i < Height; i++)
	{
		ii = 0;
		i1 = 0;
		for (j = 0; j < Width; j++)
		{
			pDIB[iPoradi] = (CHAR)bPixel[i][j];
			iPoradi++;
		}

		for (i1 = 0; i1 < iZarovnat; i1++)
		{
			pDIB[iPoradi] = 0;
			iPoradi++;
		}
	}

	bRead = WriteFile(hFile, pDIB, iPoradi, &dwBytesRead, NULL);
	_ASSERTE(FALSE != bRead);

	CloseHandle(hFile);

}
