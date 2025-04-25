void login(users *users_info, int *count, char *login_u_ID,  char *cwd_path, int *user_input)
{
    char user_id[SIZE];
    char user_pw[SIZE];
    int plag_login=0;
    while(1)
    {
        system("clear");
        printf("███████╗██╗    ██╗    ███╗   ███╗ █████╗ ██╗██╗     \n");
        printf("╚════██║██║    ██║    ████╗ ████║██╔══██╗██║██║     \n");
        printf("███████║█████████║    ██╔████╔██║███████║██║██║     \n");
        printf("╚════██║██╔════██║    ██║╚██╔╝██║██╔══██║██║██║     \n");
        printf("███████║██║    ██║    ██║ ╚═╝ ██║██║  ██║██║███████╗\n");
        printf("╚══════╝╚═╝    ╚═╝    ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝╚══════╝\n\n\n");
        printf("             1. 로그인     2. 회원가입\n");

        scanf("%d",user_input);
        if(*user_input == 1);;
        {
            printf("아이디를 입력해 주세요 : \n");
            scanf("%s",user_id);
            while (getchar() != '\n');
            for (int i = 0 ; i < *count ; i++) // ID 중복 검사 코드
            {
                if (strcmp(user_id, users_info[i].u_ID) == 0)
                {
                    printf("비밀번호를 입력해 주세요. : \n");
                    scanf("%s",user_pw);
                    if (strcmp(user_pw, users_info[i].u_PW) == 0)
                    {
                        printf("반갑습니다. %s 님.\n",user_id);
                        strcpy(login_u_ID,user_id);
                        printf("%s",login_u_ID);
                        plag_login = 1;
                        getchar();
                        break;
                    }
                    else
                    {
                        i--;
                        continue;
                    }
                }
            }
            if (plag_login == 0)
            {
                printf("아이디를 확인해 주세요.\n");
                getchar();
                continue;
            }
        }
        break; 
    }
}
