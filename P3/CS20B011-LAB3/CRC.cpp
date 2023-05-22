#include <bits/stdc++.h>
#include <stdlib.h>
#include <fstream>

using namespace std;

char * Remainder(char tmp[])
{
    char crc[9] = {'1','0','0','0','0','0','1','1','1'};

    char* R =new char[8]; 
    int j = 9;
    char div[9];

    for(int i=0;i<9;i++)
    {
        div[i]=tmp[i];
    }

    while(j<137)
    {
        if(div[0]=='1')
        {
            for(int i=0;i<8;i++)
            {
                if((div[i+1]=='0' && crc[i+1]=='0') || (div[i+1]=='1' && crc[i+1]=='1'))
                {
                    R[i] = '0';
                }
                else
                {
                    R[i] = '1';
                }
            }
        }
        else
        {
            for(int i=0;i<8;i++)
            {
                R[i] = div[i+1];
            }
        }

        for(int i=0;i<8;i++)
        {
            div[i] = R[i];
        }
        div[8] = tmp[j];
        j++;
    }

    return R;
}

int main(int argc, char* argv[])
{
    if(argc!=3)
    {
        cout<<"Incorrect arguments\n";
        return 0;
    }

    char* file = argv[1];
    FILE* inputFilePtr = fopen(file,"r");
    if(inputFilePtr==NULL)
    {
        printf("Failed to open input file\n");
    }

    char * outfile = argv[2];
    FILE* outputFilePtr = fopen(outfile,"w");
    if(outputFilePtr==NULL)
    {
        printf("Failed to open output file\n");
    }

    char crc[9] = {'1','0','0','0','0','0','1','1','1'};

    srand(time(NULL));

    char inp[256];
    while(fgets(inp,256,inputFilePtr)!=NULL)
    {
        inp[128] = '\0';
        //fgets(inp,128,inputFilePtr);
        fprintf(outputFilePtr,"Input: %s\n",inp);

        char cinp[137];
        char tmp[136];          //x^8 Mx
        for(int i=0;i<128;i++)
        {
            tmp[i] = inp[i];
            cinp[i] = inp[i];
        }
        for(int i=128;i<136;i++)
        {
            tmp[i] = '0';
        }
        //get crc string x^8 M(x) - R(x) where R(x) is x^8 M(x) % G(x)
        
        char *R = Remainder(tmp);

        for(int i=128;i<136;i++)
        {
            cinp[i] = R[i-128];
        }

        cinp[136] = '\0';
        fprintf(outputFilePtr,"Original string with CRC: %s\n",cinp);

        for(int i=0;i<10;i++)
        {
            int errors = 2*(rand()%60)+3;
            bool flag[136];
            for(int j=0;j<136;j++)
            {
                flag[j] = false;
            }
            
            char corr[137];
            for(int j=0;j<136;j++)
            {
                corr[j]=cinp[j];
            }
            int j = 0;
            while(j!=errors)
            {
                int pos = rand()%136;
                if(flag[pos]==false)
                {
                    if(corr[pos]=='0')
                    {
                        corr[pos] = '1';
                    }
                    else
                    {
                        corr[pos] = '0';
                    }
                    flag[pos] = true;
                    //fprintf(outputFilePtr,"%d ",pos);
                    j++;
                }
            }
            corr[136] = '\0';
            fprintf(outputFilePtr,"Corrupted String: %s\n",corr);
            fprintf(outputFilePtr,"Number of Errors introduced: %d\n",errors);

            // calculate remainder[T'(x)/G(x)] if not zero than error is detected
            char* Rprime = Remainder(corr);
            bool chk = false;
            for(int j=0;j<8;j++)
            {
                if(Rprime[j]=='1')
                {
                    chk = true;
                    break;
                }
            }             
            if(chk)
            {
                fprintf(outputFilePtr,"CRC check : Failed\n");
            } 
            else
            {
                fprintf(outputFilePtr,"CRC check : Passed\n");
            }
        
        }

        bool check[11];
        for(int j=0;j<11;j++)
        {
            check[j] = false;
        }
        int i = 0;
        while(i<5)                                         //bursty errors
        {
            int pos = rand()%11;
            if(check[pos]==true)
            {
                continue;
            }
            char corr[137];
            for(int j=0;j<136;j++)
            {
                corr[j]=cinp[j];
            }
            check[pos]=true;
            for(int j=0;j<6;j++)
            {
                if(corr[100+pos+j]=='0')
                {
                    corr[100+pos+j] = '1';
                }
                else
                {
                    corr[100+pos+j] = '0';
                }
            }
            corr[136] = '\0';
            fprintf(outputFilePtr,"Corrupted String: %s\n",corr);
            fprintf(outputFilePtr,"Bursty error of length 6 introduced at position : %d\n",pos+100);

            char *Rprime2 = Remainder(corr);
            bool chk = false;
            for(int k=0;k<8;k++)
            {
                if(Rprime2[k]=='1')
                {
                    chk = true;
                    break;
                }
            }             
            if(chk)
            {
                fprintf(outputFilePtr,"CRC check : Failed\n");
            } 
            else
            {
                fprintf(outputFilePtr,"CRC check : Passed\n");
            }
            i++;
        }

        fprintf(outputFilePtr,"\n\n\n");
    }

    fclose(inputFilePtr);
    fclose(outputFilePtr);
}