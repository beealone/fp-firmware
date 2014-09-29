#include <string.h>

int FindMax(int *Data, int Count)
{
	int i, m=Data[0], ret=0;
	for(i=1;i<Count;i++)
		if(Data[i]>m)
		{
			ret=i;
			m=Data[i];
		}
	return ret;
}

int FindCenter(int *data, int count, int width)
{
	int i, j, s, dw, mins, result;
  	int sums[1000];

  	memset(sums, 0, sizeof(sums));
  	dw = width / 2;
  	mins = 0xffffffff >> 1;
  	result = count / 2;
  	for(i=dw; i<count-dw; i++)
	{
    		s=0;
		for(j = i-dw; j<=i; j++)
      			s+=data[j];
    		sums[i]=s;
    		s=0;
    		for(j = i; j<=i+dw; j++)
      			s+=data[j];
    		sums[i]=sums[i]-s;
    		if(sums[i]<0) sums[i]=-sums[i];
    		if(mins>sums[i])
    		{
      			mins = sums[i];
      			result = i;
    		}
  	}
  	return result;
}


void SmoothData(int *data, int *cdata, int count, int swidth)
{
	int i,j,s,c;
	for(i=0;i<count;i++)
	{
		s=0;
		c=0;
		for(j= i-swidth/2; j<i-swidth/2+swidth+1; j++)
		if((j>=0) && (j<count))
		{
			c++;
			s+=data[j];
		}
		if(c>0)
	    		cdata[i] = s / c;
	    	else
	    		cdata[i] = data[i];
	}
}

void CalcFCenter(char *Img, int IWidth, int IHeight, int CWidth, int CHeight, int *HCenter, int *VCenter)
{
	int i,j;
	int RData[1000], BData[1000], RCData[1000];
	unsigned char *p;
	
	memset(RData, 0, sizeof(int)*IHeight);
	memset(BData, 0, sizeof(int)*IWidth);
	
	//calc histogram
	p = (unsigned char *)Img;
	for(i= 0; i<IHeight; i++)
	{
		for(j= 0;j<IWidth;j++)
		{
			BData[j]+=*p;
			RData[i]+=*p;
			p++;
		}
	}

	//calc center position
	*HCenter=FindCenter(BData, IWidth, CWidth);
	SmoothData(RData, RCData, IHeight, CHeight);
	*VCenter=FindMax(RCData, IHeight);
	
}

#define GAP 3

static int CalcDPI(int *HData, int Count)
{
	int i, LinePCount, n, MinGap, MaxGap, j, k, TheGap, NewData[1000], res, sum, value, av;
  	
	memset(NewData, 0, sizeof(NewData));
	sum = 0;
  	for(i = GAP; i<Count-GAP; i++)
  	{
    		NewData[i]=(HData[i-GAP]+HData[i+GAP]+HData[i-GAP+1]+HData[i+GAP-1])-HData[i]*4;
    		if(NewData[i]<0) NewData[i]=0;
    		sum+=NewData[i];
  	}
  	av = sum/(Count-GAP*2);
  	SmoothData(NewData, HData, Count, GAP*3);
  	SmoothData(HData, NewData, Count, GAP);
  	MinGap=Count/12;
  	MaxGap=Count/4;
  	i = Count/4 + FindMax(NewData+Count/4, Count/2);
  	j = i+MinGap+FindMax(NewData+i+MinGap, MaxGap-MinGap);
  	if(j-i>MinGap*2)
  	{
  		k = i+MinGap+FindMax(NewData+i+MinGap, (MaxGap-MinGap)/2);
    		if((NewData[j]<NewData[j+1])||
    		((NewData[k]>NewData[j]/2) && (NewData[k]>=NewData[k+1])))
    			j=k;
  	}
  	TheGap = j-i;
  	LinePCount = 1;
  	k = i;
  	while(k-TheGap>=5)
  	{
    		n = k-TheGap-5+FindMax(NewData+k-TheGap-5, 10);
    		if(NewData[n]*3>NewData[k])
    		{
      			k = n;
      			LinePCount++;
    		}
    		else
      			break;
  	}
 	 while(j+TheGap+5<Count)
  	{
    		n = j+TheGap-5+FindMax(NewData+j+TheGap-5, 10);
    		if(NewData[n]*3>NewData[j])
    		{
      			j = n;
      			LinePCount++;
    		}
    		else
      			break;
  	}
  	TheGap = (j-k+LinePCount/2)/LinePCount;
  	res = ((j-k)*254+10*LinePCount)/(20*LinePCount);
  
  	//¼ÆËã¿É¿¿¶È
  	LinePCount = 1;
  	sum = NewData[i];
  	MaxGap = NewData[i];
  	for( j = 1; j<12; j++)
  	{
    		k = i-TheGap*j;
    		if(k>=0)
    		{
      			n = k-5;
      			value = 11;
      			if(n<0)
      			{
        			value = 11+n;
        			n = 0;
      			}
      			k = n+FindMax(NewData+n, value);
      			if(MaxGap<NewData[k]) MaxGap = NewData[k];
      			sum+=NewData[k];
      			LinePCount++;
    		}
    		k = i+TheGap*j;
    		if(k<Count)
    		{
			n = k-5;
      			value = 11;
      			if(n+11>Count)
      			{
        			value = Count-n;
        			n = Count-value;
      			}
      			k = n+FindMax(NewData+n, value);
      			if(MaxGap<NewData[k]) MaxGap = NewData[k];
      			sum+=NewData[k];
      			LinePCount++;
    		}
  	}
  	sum = (sum+LinePCount / 2) / LinePCount;
  	if(sum<1500) return 0;
  	if((av*100+sum / 2) / sum>40) return 0;
  	if((sum*(LinePCount-3) < NewData[i]) || (sum*2>3*NewData[i])) return 0;
  	return res;  
}

int CalcImageDPI(char *Img, int IWidth, int IHeight, int *XDPI, int *YDPI)
{
	int i,j;
	int RData[1000], BData[1000];
	unsigned char *p;
	
	memset(RData, 0, sizeof(int)*IHeight);
	memset(BData, 0, sizeof(int)*IWidth);
	
	//calc histogram
	p = (unsigned char *)Img;
	for(i= 0; i<IHeight; i++)
	{
		for(j= 0;j<IWidth;j++)
		{
			BData[j]+=*p;
			RData[i]+=*p;
			p++;
		}
	}
	*YDPI = CalcDPI(RData, IHeight);
	*XDPI = CalcDPI(BData, IWidth);
	if(!*XDPI || !*YDPI) return 0;
	return (*XDPI+*YDPI)/2;
}

