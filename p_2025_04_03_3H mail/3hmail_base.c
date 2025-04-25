#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#define MAX_LINE 100  // 최대 줄 길이
#define MAX_USERS 100  // 최대 저장할 회원 수
#define SIZE 100

typedef struct{
    int u_num;
    char u_ID[SIZE];
    char u_PW[SIZE];
    char u_name[SIZE];
    int u_age;
    char u_phone_1[SIZE];
    char u_phone_2[SIZE];
    char u_phone_3[SIZE];
} users;



int main()
{
    char cwd_path[SIZE];               //현재 유저 작업 디렉토리 경로
    char login_u_ID[SIZE]="asdf1";     //현재 로그인된 유저 아이디

    if ( getcwd(cwd_path, SIZE) == NULL )
    {
        fprintf(stderr, "Error: getcwd() cannot execute\n") ;
        exit(1); 
    } 
      


    //유저 정보 구조체에 넣는 기능
    FILE *fp;
    char buffer[MAX_LINE];  //한줄을 저장할 버퍼
    users users_info[SIZE];  // 구조체 배열선언
    int count = 0; // 현재 저장된 회원 수(처음엔 0)

    // CSV 파일 열기
    fp = fopen("/home/tail/Desktop/3H_mail/DB/test_u_DB.csv", "r");
    if (fp == NULL) {
        perror("파일을 열 수 없습니다");
        return 1;
    }

    // 한 줄씩 읽어서 구조체에 저장
    //fgets() 한줄씩읽기
    //strtok() → 쉼표 , 기준으로 데이터 분리
    //atoi(), strcpy() → 데이터 변환 및 저장
    while (fgets(buffer, sizeof(buffer), fp) != NULL && count < MAX_USERS) //10���� ������ ����
    {
        // 쉼표(,)를 기준으로 데이터 분리
        char *token = strtok(buffer, ","); 

        users_info[count].u_num = atoi(token); // 첫 번째 값 : 사용자 번호

        token = strtok(NULL, ",");
        strcpy(users_info[count].u_ID, token);  // 두 번째 값 : ID
        
        token = strtok(NULL, ",");
        strcpy(users_info[count].u_PW, token);  // 세 번째 값 : PW

        token = strtok(NULL, ",");
        strcpy(users_info[count].u_name, token);  // 네 번째 값 : 이름

        token = strtok(NULL, ",");
        users_info[count].u_age = atoi(token); // 다섯번째 값 : 나이

        token = strtok(NULL, ",");
        strcpy(users_info[count].u_phone_1, token); // 여섯번째 값 : 핸드폰 번호1

        token = strtok(NULL, ",");
        strcpy(users_info[count].u_phone_2, token); // 일곱섯번째 값 : 핸드폰 번호2

        token = strtok(NULL, ",");
        strcpy(users_info[count].u_phone_3, token); // 여덟번째 값 : 핸드폰 번호3


        if (token != NULL) 
        {
            token[strcspn(token, "\n")] = '\0'; 
            //    strcspn(token, "\n") → \n(개행 문자) 위치 찾기
            //    token[strcspn(token, "\n")] = '\0'; → 개행 문자를 \0(문자열 끝)로 바꿈
            strcpy(users_info[count].u_phone_3, token);  // 여덟번째 값 (전화번호3)
        }
        count++;  // 회원 수 증가
    }
    fclose(fp);  // 파일 닫기
    // 구조체에 저장된 데이터 출력
    printf("[ 회원 정보 ]\n");
    for (int i = 0; i < count; i++) 
    {
        printf("user_num : %d, user_id : %s, user_pw : %s, user_name : %s, user_age : %d, user_phone : %s-%s-%s\n", users_info[i].u_num, users_info[i].u_ID, users_info[i].u_PW, users_info[i].u_name, users_info[i].u_age, users_info[i].u_phone_1, users_info[i].u_phone_2, users_info[i].u_phone_3);
    }




    return 0;
}





