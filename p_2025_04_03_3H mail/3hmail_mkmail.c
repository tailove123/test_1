#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#define MAX_LINE 100  // 최대 줄 길이
#define MAX_USERS 100  // 최대 저장할 회원 수
#define SIZE 100

//////////////////////새로 추가 된 것들//////////////
//5번째 줄 25번째 줄 29~31, 37~48, !!!!!57~62는 기존 코드 바꾸기!!!!, 109줄, 118~188


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

void mk_txt(FILE *fp, char *login, char *cwd_path, struct tm *tm);

int main()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    struct tm *ptr_tm = &tm;


    //현재 로그인된 유저 아이디
    char login_u_ID[SIZE]="asdf1";    

    //현재 유저 작업 디렉토리 경로
    char cwd_path[SIZE];
    char csv_path[SIZE];                        
    if ( getcwd(cwd_path, SIZE) == NULL )
    {
        fprintf(stderr, "Error: getcwd() cannot execute\n") ;
        exit(1); 
    }
    strcpy(csv_path,cwd_path);  //cwd주소를 scv주소에 복사
    strcat(csv_path,"/DB/test_u_DB.csv");   //csv주소 지정  "DB파일명 바뀌면 이걸 건들것!"

    printf("%s\n",csv_path);
   

    //유저 정보 구조체에 넣는 기능
    FILE *fp;
    char buffer[MAX_LINE];  //한줄을 저장할 버퍼
    users users_info[SIZE];  // 구조체 배열선언
    int count = 0; // 현재 저장된 회원 수(처음엔 0)

    // CSV 파일 열기
    fp = fopen(csv_path, "r");
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

    mk_txt(fp, login_u_ID, cwd_path, ptr_tm);

    return 0;
}

// 텍스트 파일 만들어서 해당유저 아이디/ 경로에 저장
void mk_txt(FILE *fp, char *login_u_ID, char *cwd_path, struct tm *tm)
{
    //내가 만든 파일 경로  ==  현재작업위치/ID/u_id/1_시간_제목.txt
    char tmp_cwd_path[SIZE];
    strcpy(tmp_cwd_path,cwd_path);  //현재 작업 위치를 tmp_cwd_path에 저장
    strcat(tmp_cwd_path,"/ID"); //현재 경로와 ID 디렉토리로 경로 재설정
    char u_id[SIZE];
    strcpy(u_id,login_u_ID);   //로그인된 아이디를 u_id배열에 저장
    strcat(tmp_cwd_path, "/");
    strcat(tmp_cwd_path,u_id); //현재작업위치/ID/u_id 문자열이 tmp_cwd_path 에 저장됨
    char text_inside[SIZE];

    char mailtype_time[SIZE];   //메일타입(받은메일, 저장된메일)과 시간정보 섞인 문자열
    char mon[SIZE];
    char mday[SIZE];
    char hour[SIZE];
    char min [SIZE];
    char sec [SIZE];
    char input_m_name[SIZE];  //유저가 입력한 메일 제목
    
    //시간정보에 일의 자리면 0붙이는 코드
    if ((tm->tm_mon+1)<10)
        sprintf(mon, "0%d", tm->tm_mon+1);
    else
        sprintf(mon, "%d", tm->tm_mon+1);  
    if ((tm->tm_mday)<10)
        sprintf(mday, "0%d", tm->tm_mday);
    else
        sprintf(mday, "%d", tm->tm_mday);
    if ((tm->tm_hour)<10)
        sprintf(hour, "0%d", tm->tm_hour);
    else
        sprintf(hour, "%d", tm->tm_hour);
    if ((tm->tm_min)<10)
        sprintf(min, "0%d", tm->tm_min);
    else
        sprintf(min, "%d", tm->tm_min);
    if ((tm->tm_sec)<10)
        sprintf(sec, "0%d", tm->tm_sec);
    else
        sprintf(sec, "%d", tm->tm_sec);

    sprintf (mailtype_time, "/1_%d_%s_%s_%s_%s_%s_", tm->tm_year+1900, mon, mday, hour, min, sec);
    
    printf("메일 제목을 입력해 주세요.\n");
    scanf("%s",input_m_name);
    strcat(tmp_cwd_path, mailtype_time);
    strcat(tmp_cwd_path, input_m_name);
    strcat(tmp_cwd_path, ".txt");
    printf("%s\n",tmp_cwd_path);

    fp = fopen(tmp_cwd_path, "a");

    if (fp == NULL)
    {
        perror("파일을 열 수 없습니다");
        return 1;
    }
    printf("문자열을 입력하시오. 입력을 끝내려면 'end'를 누르시오. \n");
    gets(text_inside); // 문자열 입력
    while(strcmp(text_inside, "end")) 
    { // 입력된 문자열이 end가 아니면 loop 반복
        strcat(text_inside, "\n"); // 문자열에 "\n" 추가
        fputs(text_inside, fp); // 3. 파일 입출력 : 문자열 fp가 가리키는 파일에 출력
        gets(text_inside);
    }
    fclose(fp);
}