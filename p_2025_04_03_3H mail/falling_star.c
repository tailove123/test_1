#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define SIZE 50
int main()
{
    int time_limit = 0;
    int field[50][50];
    for (int i = 0;i<SIZE;i++)     //맵 전체 0으로 초기화
    {
        for(int j =0;j<SIZE;j++)
            field[i][j]=0;
    }

    while(time_limit<100)
    {
        system("clear");
        for(int i=0; i<100;i++)
        {
            srand(time(NULL));
            int rand_50;
            rand_50 = rand()%50;
            if(i%3==0)
            {
                field[0][rand_50] = 1;
            }
            for(int j=0;j<SIZE;j++)
            {
                for(int k=0;k<SIZE;k++)
                {
                    if(field[j][k]==1)
                    {
                        field[j][k]=2;
                    }
                    else if(field[j][k]==2)
                    {
                        field[j][k]=3;
                    }
                    else if(field[j][k]==3)
                    {
                        field[j][k]=4;
                    }
                    else if(field[j][k]==4)
                    {
                        field[j][k]=5;
                    }
                    else if(field[j][k]==5)
                    {
                        field[j][k]=6;
                    }
                    else if(field[j][k]==6)
                    {
                        field[j][k]=7;
                    }
                    else if(field[j][k]==7)
                    {
                        field[j][k]=8;
                    }
                    else if(field[j][k]==8)
                    {
                        field[j][k]=9;
                    }
                    else if(field[j][k]==9)
                    {
                        field[j][k]=0;
                    }
                    if(((j-1)>=0)&&(field[j-1][k]==1)&&(field[j-2][k]!=1))
                    {
                        field[j][k]=1;
                    }
                }
            }  
        }
        for(int i=0; i<SIZE; i++)         //출력 페이즈
        {                      

            for(int j=0; j<SIZE; j++)
            {
                if(field[i][j]==2)
                {                         //주인공 출력
                    printf("█");
                }
                else if (field[i][j] == 3)
                {
                    printf("▓");         //2면 몬스터 출력
                }
                else if (field[i][j] == 4)
                {
                    printf("▒");         //3면  보스 출력
                }
                else if (field[i][j] == 5)
                {
                    printf("░");         //4면   이동녀크 출력
                }
                else if (field[i][j] == 6)
                {
                    printf("⁛");         //5면 포탈 출력
                }
                else if (field[i][j] == 7)
                {
                    printf(":");         //9면 벽 출력
                }
                else if (field[i][j] == 8 || field[i][j] == 9)
                {
                    printf("·");         //9면 벽 출력
                }
                else
                {
                    printf(" ");         // 6~8여백 출력
                }
            }
            printf("\n");
        }
        usleep(100000);
        time_limit+=1;
    }


    return 0;
}