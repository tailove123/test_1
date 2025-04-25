#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <termio.h>                 //getch() 지정
#include <unistd.h>
#include <poll.h> 

//길이관련 상수
#define ARR_SIZE 30                 // 배열 길이
#define MONSTER_EA 6                // 현재 몬스터  수 // 보스랑 이동녀크 추가 하면 8

//맵관련 상수
#define SIZE 50                     //맵 사이즈 지정
#define START_X 0                   //스타트 좌표
#define START_Y SIZE-1               
#define PORTAL_LOCATION 10          //포탈 위치 제한
#define MONSTER_REGEN_RATE 10 
#define BOSS_REGEN_RATE 30  

// 상태(경험치, HP, MP, 전체) UI 함수
void exp_UI(int live_exp, float per_exp);
void hp_UI(int live_hp, float per_hp);
void mp_UI(int live_mp, float per_mp);
void state_check_UI(int gold, int live_exp, int max_exp, int live_hp, int max_hp, int live_mp, int max_mp, int added_stp, int added_str, int added_dex, int added_int_, int live_lv, int START_STR, int START_ATK, int added_atk, float START_EVA, float added_eva, int START_DEX, int START_MTK, int added_mtk, float START_CRI, float added_cri, int START_INT_, int live_def, int return_stp);



// 몬스터능력치 랜덤으로 불러오는 함수
int mon_ran_num;
int mon_ran_;
int mon_name_num;   //넘버링된 (싸우는)몬스터 이름 출력
int war_switch = 0; // 전투 스위치, 전투해야하면 1, 전투가 끝나면 0 
int cri_ran_;


// 스킬 정리
char skill_name[][ARR_SIZE] = {"파이어볼", "워터스피어", "썬더볼트", "스톤샤워", "워터밤", "파이어스피어", "아이스볼", "윈드커터", "블래스트", "블리자드", "메테오","토네이도"}; // 스킬이름
int skill_level[]           = {1, 5, 10, 15, 20, 25, 30, 40, 50, 60, 65, 70}; // 획득레벨
float skill_magic_m[]       = {2, 2.5, 3, 4, 5, 6, 6, 8, 10, 15, 15, 15}; // 마법배율
int skill_mp[]              = {2, 10, 15, 20, 30, 50, 50, 100, 150, 200, 200, 200}; // mp소모
int skill_properties[]      = {2, 1, 3, 3, 1, 2, 1, 3, 2, 1, 2, 3}; // 스킬 속성
int len_skill = 12; // 스킬갯수

// 사용할 수 있는 스킬 저장하는 배열
char user_skill_name[12][ARR_SIZE];
float user_skill_magic_m[12];
int user_skill_mp[12];
int user_skill_properties[12];
int user_skill_num = 0; // 사용할 수 있는 스킬 갯수
int user_war_select; // 전투 중 행동 선택 (물리공격, 마법공격, 아이템사용, 도망가기)
int user_skill_select; // 사용할 스킬 번호 선택

// 몬스터배열    
char monster_korea_name[][ARR_SIZE] =              {"오크전사","좀비","구울","해골","리치","바실리스크","마왕","이동녀크"};
//1.몬스터한국 이름
char monster_english_name[][ARR_SIZE] =            {"ork_warrior","zombie","ghoul","skeleton","rich","basilisk"};
//1.몬스터영어 이름
int monster_fix_hp[ARR_SIZE] =                     {30,        80,    120,  250,   300,   600,   500,   1000       };
//2.몬스터 고정 체력
int monster_random_hp[ARR_SIZE] =                  {50,        40,     80,  150,    50,   300,   3  ,   5       };   //마왕과 보스는 곱하기 user_live_hp
//2.몬스터 랜덤 체력
int monster_fix_ac[ARR_SIZE] =                     {0,          1,      5,   15,    10,    30,     0,     0       };
//3.몬스터 고정 방어력
int monster_random_ac[ARR_SIZE] =                  {0,          2,      5,   15,     5,    20     ,0     ,0       };
//3.몬스터 랜덤 방어력 
int monster_fix_atk[ARR_SIZE] =                    {10,        25,     50,   70,   230,   100,   250,   500       };
//4.몬스터 고정 공격
int monster_random_atk[ARR_SIZE] =                 {11,        15,     30,   40,    70,    30,    50,   100       };
//4. 몬스터 랜덤 공격
int monster_pro_weak[ARR_SIZE] =                   {1,          2,      2,    3,     3,     1,     3,     0       };

char monster_pro_weak_ch[][ARR_SIZE] =             {"약점없음","불","물","자연"};

int monster_pro_stren[ARR_SIZE] =                  {2,          3,      3,    0,     0,     0,     0 ,     0        };

char monster_pro_stren_ch[][ARR_SIZE] =            {"강점없음","불","물","자연"};

char monster_pro_icon[][ARR_SIZE] =                  {"𝐍","🔥","💧","🌿"};

char monster_spc_abil_ch[][ARR_SIZE] =             {"중독","저주","과제내주기"};
//7. 몬스터특수패시브 이름
int monster_spc_abil[ARR_SIZE] =                   {9,          0,      0,    1,     9,     9       };
//8. 몬스터 패시브 속성



// 마법속성이 몬스터 속성이랑 비교해서 약점배율로 바꾸는 함수
float skill_monster_pro_cal()
{
    float weakness_point;

    if (monster_pro_weak[mon_name_num] == 1 && user_skill_properties[user_skill_select] == 3)
    {
       weakness_point == 0.5; 
       return weakness_point;
    }
    else if (monster_pro_weak[mon_name_num] == 3 && user_skill_properties[user_skill_select] == 1)
    {
        weakness_point == 2;
        return weakness_point; 
    }
    else if (monster_pro_weak[mon_name_num]  == 0 || user_skill_properties[user_skill_select] == 0)
    {
        weakness_point == 1;
        return weakness_point;
    }
    else if (monster_pro_weak[mon_name_num] > user_skill_properties[user_skill_select])
    {
        weakness_point == 0.5; 
        return weakness_point;
    } 
    else if (monster_pro_weak[mon_name_num] < user_skill_properties[user_skill_select])
    {
        weakness_point == 2;
        return weakness_point; 
    } 
    else
    {
        weakness_point == 1;
        return weakness_point;
    }

}





//몬스터변수
int monster_max_hp;       ///max가 만날시 몬스터 최대 체력
int monster_max_ac;       ///max가 만날시 몬스터 방어력   
int monster_max_atk;      ///max가 만날시 몬스터 공격력


void mon_determine (); // 몬스터 종류 결정짓는 함수(전투페이즈 맨 첫번째에 넣을 것)


int mon_ran() // 0에서 현재 설정된 몬스터의 수(6)까지 랜덤수 하나추출 / 하나만뽑는경우바로쓰면된다 ex.이름이나 속성값

{
    srand(time(NULL));
    mon_ran_ = rand() % MONSTER_EA;
    return mon_ran_;
}

float cri_ran()
{
    srand(time(NULL));
    cri_ran_ = (rand() % 100) + 1;
    return cri_ran_;

}

// 속성값부여 함수
int enchant_property(int property_num)
{   
    srand(time(NULL));
    property_num = (rand() % 4);

    return property_num;
}

// 인챈트 데미지 설정 함수
int enchant_damage(int after_enchant_damage)

{
    srand(time(NULL));
    after_enchant_damage = -30 + (rand() % 51) ;

    return after_enchant_damage;
}

// 상점 물건 구매 함수
int shop(int *price, int user_cons_select1, char *cons_item_name, char *weapon_name, char *armor_name, char *shoes_name, char *glove_name, char *cloak_name, char *hat_name, int *cons_item_price, int *weapon_price, int *armor_price, int *shoes_price, int *glove_price, int *cloak_price, int *hat_price, int len_cons_item, int len_weapon, int len_armor, int len_shoes, int len_glove, int len_cloak, int len_hat);

// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 6.인챈트(%), 7.인챈트 속성, 특수옵션(나중에 구현)
// 인벤토리 공간
int INVEN[20][10] ={{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}; 
int MY_EQUIP[6][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
int MY_EQUIP_tmp[1][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

// 소모품 공간
int cons_space[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50};    // 소비 아이템 공간, max = 99
int mat_space[5] = {500, 500, 500, 500, 100};                     // 기타 아이템 공간, max = 999

// 좌표관련변수
int map[SIZE][SIZE] = {};                               //전체 맵 저장
char map_input;                                         //방향 입력
int floor = 0;                                          //현재 층 수
int y;                
int x;                                                  //스타트 좌표 초기화
int user_y;                                             //현재 유저 좌표 
int user_x;
int save_point[5][3] = {{0, 0, 0}}; // 층, y, x
int user_position_select;                               // 순간이동주문서 5개 저장된 좌표중 1개 선택
int portal_x, portal_y;                                 //포탈 위치값 저장
int mon_position[MONSTER_REGEN_RATE][2] = {0};          //몬스터 위치값 저장
int boss_x, boss_y;                                     //보스 위치값 저장
int there_is_the_boss = 0;                              //보스가 있으면 켜지는 스위치
int plag_boss_regen = 0;                                //보스 리젠시키는 스위치       //전투 끝날 시 넣어야 함
int plag_mapsave = 0;                                   //맵 저장하는 스위치
int plag_mapout = 0;                                    //맵을 꺼지게 하는 스위치
int plag_mon_regen = 0;                                 //몬스터를 리젠 시키는 스위치
int plag_named = 0;                                     //네임드 발생 스위치
int tp_switch = 0;                                      //텔레포트 스위치

void boss_regen();  //맵 안에 들어가는 함수
void map_print();
void map_change();  //맵이 바뀌어야 할 상황이면 넣어주세요
void map_not_change();  //맵이 안바뀌고 그대로일 상황에 넣어주세요





int main(){
    char properties_list[][20] = {"NONE","FIRE","WATER","NATURE"};

    // 데미지수식 관련변수
    int AD; //총물리데미지
    int AP; //총마법데미지
    float ALLTK_PER = 1; // 일단 장비에서 특수옵션미구현이라 1로

    // 골드
    int START_GOLD = 100000;
    int drop_gold = 0;
    int gold = START_GOLD + drop_gold;
    int price;

    // 기타아이템
    char mat_item_name[][ARR_SIZE] = {"작은 화염 조각", "푸른 화염 조각", "강렬한 화염 조각", "미친 화염 조각", "마력응축석"};

    // 소비아이템
    char cons_item_name[][ARR_SIZE] = {"빨간물약", "주황물약", "맑은물약", "파란물약", "하얀물약", "무지개물약", "엘릭서", "해독제", "축복주문서", "만병통치약", "마을이동주문서", "순간이동주문서"};
    int cons_item_price[]           = {30, 200, 1000, 50, 300, 1500, 500, 2000, 200, 1000};
    int cons_item_tier[]            = {1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 1, 2};
    int cons_item_recov[]           = {30, 60, 80, 30, 60, 80};                                 // 회복력
    int len_cons_item               = 10;                                                       // 상점에서 판매하는 소비아이템 갯수

    // 무기
    char weapon_name[][ARR_SIZE] = {"기본검", "장검", "일본도", "싸울아비장검", "혼돈의검", "천상천하제일무쌍도", "마왕을 멸하는 마검", "운영자검"};
    int weapon_price[]           = {50, 500, 10000};
    int weapon_tier[]            = {0, 1, 2, 3, 4, 5, 5, 5, 6};
    int weapon_ATK[]             = {2, 10, 20, 40, 50, 100, 50, 11111}; // 공격력
    int len_weapon = 3;                                                 // 상점에서 판매하는 무기 갯수

    // 갑옷
    char armor_name[][ARR_SIZE] = {"가죽갑옷", "철갑옷", "강철갑옷", "미스릴갑옷", "진격하는 자의 갑옷", "골렘의 몸 갑옷", "정화의 갑주"};
    int armor_price[]           = {50, 600};
    int armor_tier[]            = {1, 2, 3, 4, 5, 5, 5};
    int armor_DEF[]             = {-2, -6, -12, -20, -20, -20, -20};    // 데미지 감소
    int len_armor               = 2;                                    // 상점에서 판매하는 갑옷 갯수

    // 신발
    char shoes_name[][ARR_SIZE] = {"가죽신발", "철신발", "강철신발", "미스릴신발", "포세이돈의 신발", "헤르메스의 신발", "하데스의 신발"};
    int shoes_price[]           = {50, 600};
    int shoes_tier[]            = {1, 2, 3, 4, 5, 5, 5};
    int shoes_DEF[]             = {-1, -3, -6, -8, -8, -8, -8};     // 데미지 감소
    int len_shoes = 2;                                              // 상점에서 판매하는 신발 갯수

    // 장갑
    char glove_name[][ARR_SIZE] = {"가죽장갑", "철장갑", "강철장갑", "미스릴장갑", "죄악의 마수", "요정의 팔찌", "혼돈의 수갑"};
    int glove_price[]           = {50, 600};
    int glove_tier[]            = {1, 2, 3, 4, 5, 5, 5};
    int glove_DEF[]             = {-1, -3, -6, -8, -8, -8, -8};     // 데미지 감소
    int len_glove               = 2;                                // 상점에서 판매하는 장갑 갯수

    // 망토
    char cloak_name[][ARR_SIZE] = {"천망토", "면망토", "비단망토", "마법망토", "순백의 망토", "용기의 망토", "진홍의 망토"};
    int cloak_price[]           = {50, 600};
    int cloak_tier[]            = {1, 2, 3, 4, 5, 5, 5};
    int cloak_DEF[]             = {-1, -3, -6, -8, -20, -8, -8};    // 데미지 감소
    int len_cloak               = 2;                                // 상점에서 판매하는 망토 갯수

    // 투구
    char hat_name[][ARR_SIZE] = {"가죽투구", "철투구", "강철투구", "미스릴투구", "용사의 투구", "용기사 투구", "기묘한 두건"};
    int hat_price[]           = {50, 600};
    int hat_tier[]            = {1, 2, 3, 4, 5, 5, 5};
    int hat_DEF[]             = {-1, -3, -6, -8, -8, -8, -1};   // 데미지 감소
    int len_hat               = 2;                              // 상점에서 판매하는 투구 갯수

    char equipment_type[][ARR_SIZE][ARR_SIZE] = {{"기본검", "장검", "일본도", "싸울아비장검", "혼돈의검", "천상천하제일무쌍도", "마왕을 멸하는 마검", "운영자검"},
                                                 {"가죽갑옷", "철갑옷", "강철갑옷", "미스릴갑옷", "진격하는 자의 갑옷", "골렘의 몸 갑옷", "정화의 갑주"},
                                                 {"가죽신발", "철신발", "강철신발", "미스릴신발", "포세이돈의 신발", "헤르메스의 신발", "하데스의 신발"},
                                                 {"가죽장갑", "철장갑", "강철장갑", "미스릴장갑", "죄악의 마수", "요정의 팔찌", "혼돈의 수갑"},
                                                 {"천망토", "면망토", "비단망토", "마법망토", "순백의 망토", "용기의 망토", "진홍의 망토"},
                                                 {"가죽투구", "철투구", "강철투구", "미스릴투구", "용사의 투구", "용기사 투구", "기묘한 두건"},
                                                 {"빨간물약", "주황물약", "맑은물약", "파란물약", "하얀물약", "무지개물약", "엘릭서", "해독제", "축복주문서", "만병통치약", "마을이동주문서", "순간이동주문서"}};

    char equipment_type2[][ARR_SIZE][ARR_SIZE] = {{"기본검", "장검", "일본도", "싸울아비검", "혼돈의검", "천상천하도", "마왕멸마검", "운영자검"},
                                                 {"가죽갑옷", "철갑옷", "강철갑옷", "미스릴갑옷", "진격갑옷", "골렘몸갑옷", "정화의 갑주"},
                                                 {"가죽신발", "철신발", "강철신발", "미스릴신발", "포세돈 신발", "헤르메스의 신발", "하데스 신발"},
                                                 {"가죽장갑", "철장갑", "강철장갑", "미스릴장갑", "죄악의 마수", "요정의 팔찌", "혼돈의 수갑"},
                                                 {"천망토", "면망토", "비단망토", "마법망토", "순백의 망토", "용기의 망토", "진홍의 망토"},
                                                 {"가죽투구", "철투구", "강철투구", "미스릴투구", "용사의 투구", "용기사 투구", "기묘한 두건"}};                                             

    
    int equipment_tier[][ARR_SIZE] = {{1, 2, 3, 4, 5, 5, 5, 6},                 // 무기
                                     {1, 2, 3, 4, 5, 5, 5},                     // 갑옷
                                     {1, 2, 3, 4, 5, 5, 5},                     // 신발
                                     {1, 2, 3, 4, 5, 5, 5},                     // 장갑
                                     {1, 2, 3, 4, 5, 5, 5},                     // 망토
                                     {1, 2, 3, 4, 5, 5, 5},                     // 투구
                                     {1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 1, 2}};     // 소비 아이템
    
    int equipment_basic_option[][ARR_SIZE] = {{2, 10, 20, 40, 50, 100, 50, 11111},
                                              {-2, -6, -12, -20, -20, -20, -20},
                                              {-1, -3, -6, -8, -8, -8, -8},
                                              {-1, -3, -6, -8, -8, -8, -8},
                                              {-1, -3, -6, -8, -20, -8, -8},
                                              {-1, -3, -6, -8, -8, -8, -1}};

    // 장비 특수 옵션 정리
    // 0 : 없음
    // 1 : 치명타
    // 2 : 받는 데미지 감소(%)
    // 3 : 공격 회피율
    // 4 : 모든 공격력(%)
    // 5 : 마법배율
    // 6 : 상태이상 안걸림 (opt_stat은 0으로 들어감. 어차피 opt 배열에서 6이면 무조건 상태이상이기 때문)
    int equip_special_opt[][ARR_SIZE] = {{0, 0, 0, 0, 1, 0, 5, 0}, // 무기
                                        {0, 0, 0, 0, 1, 2, 6}, // 갑옷
                                        {0, 0, 0, 0, 6, 3, 4}, // 신발
                                        {0, 0, 0, 0, 4, 2, 1}, // 장갑
                                        {0, 0, 0, 0, 0, 4, 1}, // 망토
                                        {0, 0, 0, 0, 2, 4, 3}}; //투구

    int equip_speical_opt_stat[][ARR_SIZE] = {{0, 0, 0, 0, 30, 0, 5, 0}, // 무기
                                            {0, 0, 0, 0, 20, 20, 0}, // 갑옷
                                            {0, 0, 0, 0, 0, 20, 10}, // 신발
                                            {0, 0, 0, 0, 10, 10, 20}, // 장갑
                                            {0, 0, 0, 0, 0, 10, 20}, // 망토
                                            {0, 0, 0, 0, 10, 10, 30}}; //투구

    char equip_speical_opt_ENG[][ARR_SIZE] = {"   없음    ", "  크리율  ", " 받피감(%) ", "  회피율  ", " 공격력(%) ", " 마법배율(%) ", " 상태이상무효"};   


                            

    int user_cons_select1; // 어떤 종류의 아이템 살지 (소비아이템/무기/갑옷 등등)
    int user_cons_select2; // 그 중에서 어떤거 살지 (빨간물약/주황물약 등등)



    // 레벨
    int START_LV = 1 ;                         // 시작레벨 (상수)
    int MAX_LV = 100 ;                          // 최대 레벨
    int live_lv = START_LV;                     // 현재 레벨 = 시작레벨 + (레벨업당+1)

    // STR는 스텟포인트로 초기화 시 중요//
    int START_STR = 10;                         // 힘
    int plus_str ;                              // 추가힘
    int live_str = START_STR + plus_str;        // 현재 힘
    int added_str = live_str - START_STR ;      // 추가된힘

    // INT는 스텟포인트로 초기화 시 중요//
    int START_INT_ = 10;                        // 지능
    int plus_int ;
    int live_int_ = START_INT_ + plus_int;      // 현재 지능
    int added_int_ = live_int_ - START_INT_;    // 최대 지능 (중요한지 모르겠음)

    // DEX는 스텟포인트로 초기화 시 중요//
    int START_DEX = 10;                          // 민첩
    int plus_dex ;
    int live_dex = START_DEX + plus_dex ;        // 현재 민첩
    int added_dex = live_dex - START_DEX;

    //STP는 스텟포인트로 초기화 시 중요//
    int stp ;                                                   // 스텟 포인트
    int live_stp = live_str + live_dex + live_int_;             // 현재 스텟 포인트
    int added_stp = added_str + added_dex + added_int_;
    int return_stp = live_lv - 1;                                             // 스텟 리턴값 (초기화시 스텟포인트 리턴 )
    
    // 경험치
    int START_EXP = 0;
    int get_exp; 
    int live_exp = START_EXP + get_exp;
    int lv_up_exp = 50;
    int max_exp = 100;
    

    //HP는 스텟포인트로 초기화 시 중요//
    int START_HP = 80;                                                                   // 체력
    int LV_UP_HP = 20;
    int STR_UP_HP = 5;
    int max_hp = START_HP + (LV_UP_HP * live_lv) + (STR_UP_HP * added_str);               // 최대 체력
    int live_hp = max_hp - (AD + AP);                                                     // 최대 체력에서 총물리마법데미지를 뺀값         

    //MP는 스텟포인트로 초기화 시 중요//
    int START_MP = 0;                                                                    // 마나 
    int LV_UP_MP = 10; 
    int INT_UP_MP = 2;
    int max_mp = START_MP + (LV_UP_MP * live_lv) + (INT_UP_MP * added_int_);              // 최대 마나 
    int live_mp = max_mp;                                                                 // 현재 마나 = 최대마나에서 사용된마나빼줘야함

    // ATK는 스텟포인트로 초기화 시 중요//
    int START_ATK = 18;                                                                 // 공격력
    int STR_UP_ATK = 2;
    int added_atk = STR_UP_ATK * added_str;
    int live_atk = START_ATK + added_atk;                                               // 현재 공격력

    // MTK 는 스텟포인트로 초기화 시 중요//
    int START_MTK = 9 ;                                                                // 마법공격력
    int INT_UP_MTK = 1;
    int added_mtk = INT_UP_MTK * added_int_;
    int live_mtk = START_MTK + added_mtk;

    //EVA는 스텟포인트로 초기화 시 중요//
    float START_EVA = 90.0;                                                              // 회피
    float MAX_EVA = 99.9;                                                               // 회피 최대치(100넘기면 안됨.) 
    float DEX_UP_EVA = 0.2;
    float added_eva = (DEX_UP_EVA * added_dex);
    float live_eva = START_EVA + added_eva ;                                            // 현재 회피

    //EVA는 스텟포인트로 초기화 시 중요//
    float START_CRI = 5.0;                                                              // 치명타확률 (영향을 주는 인자가 많음 장비3부위와 스탯값까지 더해야함)
    float CRI_MAX = 100.0;                                                              // 최대 치명타확률(100넘기면 안됨.) 
    float DEX_UP_CRI = 0.5;
    float added_cri = (DEX_UP_CRI * added_dex);
    float live_cri = START_CRI + added_cri;
    float cri_success;

    //여기서 부터는 스텟포인트로 찍을수 없어서 연관이 없다.
    int START_DEF = 0;                                                                  // 방어력
    int live_def = START_DEF;                                                           // 현재 방어력 + 방어구입으면 추가해줘야함

    //재련부분 배열
    int need_gold[] = {100,200,300,1000,2000,3000,10000,20000,30000,100000};            //소모되는골드
    int need_mat[]  = {2,3,4,2,3,4,2,3,4,3};                                            //필요강화재료갯수

    int success_per[]    = {100,100,90,90,80,80,70,60,50,30};                           //성공확률/100(%)
    int down_per[]       = {0,0,0,0,0,0,10,20,20,30};                                   //하락확률/100(%)
    int no_up_down_per[] = {0,0,10,10,20,20,20,20,30,40};                               //변화되지않을확률/100(%)

    int increase_damage_per[] = {5,10,15,20,30,40,50,70,90,150};                       //오르는기본옵션에 곱할값
    int change_enhance_mat[]  = {0,0,0,3,3,3,6,6,6,9};                                 //강화재료가 바뀌는 레벨  
    
    //기타변수
    int i;
    int input;
    int shop_switch = 1;
    

    


    int turn_switch = 0; //전투시 몬스터 턴이 끝나면 =0, 내 턴이 끝나면 =1



   
    
    
    
    int monster_fix_gold[ARR_SIZE] =                   {1,     5,       20,         50,      100,        100  };
    //1.몬스터 처치보상  골드 
    int monster_random_gold[ARR_SIZE] =                {5,     20,      50,         200,     400,        400  };
    //1.몬스터 처치보상  랜덤 골드
    
    int monster_drop_item_tp[ARR_SIZE] =               {10,    10,      20,          20,      0,           0};
    //2.몬스터 처치보상 마을 이동 주문서 (%)
    
    int  monster_drop_item_vil[ARR_SIZE] =             {0,      0,       0,          0,       20,         20};
    //3.순간 이동 주문서(%)
    
    int monster_equip_item_1tier[ARR_SIZE] =           {10,       0,       0,         0,       0,          0};
    //4.몬스터 처치보상 장비아이템 1티어확률(%)
    
    int monster_equip_item_2tier[ARR_SIZE] =            {0,       10,      0,         0,       0,          0};
    //4.몬스터 처치보상  장비 아이템 2티어확률(%)
    
    int monster_equip_item_3tier[ARR_SIZE] =            {0,        0,      10,      20,    0,          0};
    //4.몬스터 처치보상 장비아이템 3티어확률(%)
    //해골 처치보상 장비아이템 3~4티어 소비템 확률20%
    int monster_equip_item_4tier[ARR_SIZE] =            {0,        0,       0,     20,     20,        20};
    //4.몬스터 처치보상  장비아이템 4티어확률(%)
    
    int monster_cons_item_1tier[ARR_SIZE] =         {10,     0,      0,           0,       0,          0};
    //5.몬스터 처치보상  소비아이템 1티어확률(%)
    //오크 처치보상 소비아이템 1~2티어 소비템 확률10%
    
    int monster_cons_item_2tier[ARR_SIZE] =         {10,     20 ,  20,       0,        0,         0};
    //5.몬스터 처치보상  소비아이템 2티어확률(%)
     //구울 처치보상 소비아이템 2~3티어 소비템 확률20%
    
    int monster_cons_item_3tier[ARR_SIZE] =           {0,         0 ,  20,        30,     30,    30 };
    //5.몬스터 처치보상  소비아이템 3티어확률(%)
    //리치 처치보상 소비아이템 3~4티어 소비템 확률30%
    
    int monster_cons_item_4tier[ARR_SIZE] =           {0,         0,        0,          0,     30 ,   30 };
    //5.몬스터 처치보상 소비아이템 4티어확률(%)
     //바실리스크 처치보상 소비아이템 3~4티어 소비템 확률30%
    
    int monster_mat_item_1tier[ARR_SIZE] =                {10,    10,     20,       0,       0,             0  };
    //6.몬스터 처치보상 강화재료아이템 1티어확률(%)
    //좀비 처치보상 강화재료아이템 1~2티어 소비템 확률10%
    int monster_mat_item_2tier[ARR_SIZE] =                {0,     10,     20,       0,       0,             0,  };
    //6.몬스터 처치보상 강화재료아이템 2티어확률(%)
    //구울 처치보상 강화재료아이템 1~3티어 소비템 확률20%
    int monster_mat_item_3tier[ARR_SIZE] =                {0,        20,       20,       20,     20,     20};
    //6.몬스터 처치보상 강화재료아이템 3티어확률(%)  
    //리치 처치보상 강화재료아이템 3~4티어 소비템 확률20%
    int monster_mat_item_4tier[ARR_SIZE] =                {0,          0,          0,         0,     20,      20};
    //6.몬스터 처치보상 강화재료아이템 4티어확률(%)
    //바실리스크 처치보상 강화재료아이템 3~4티어 소비템 확률20%
    int monster_kill_exp[ARR_SIZE] =                {100,          1000,          2000,         3000,     4000,      5000};
    //경험치보상

    
    
    
    int reward_num;
    int equip_tier1[6][2] = {{0, 0}};
    int equip_tier2[6][2] = {{0, 0}};
    int equip_tier3[6][2] = {{0, 0}};
    int equip_tier3_4[12][2] = {{0, 0}};
    int equip_tier4[6][2] = {{0, 0}};

    int cons_tier1_2[7][2] = {{6, 0}, {6, 3}, {6, 10}, {6, 1}, {6, 4}, {6, 7}, {6, 11}};
    int cons_tier2[4][2] = {{6, 1}, {6, 4}, {6, 7}, {6, 11}};
    int cons_tier2_3[7][2] = {{6, 1}, {6, 2}, {6, 4}, {6, 5}, {6, 7}, {6, 8}, {6, 11}};
    int cons_tier3[3][2] = {{6, 2}, {6, 5}, {6, 8}};
    int cons_tier3_4[5][2] = {{6, 2}, {6, 5}, {6, 6}, {6, 8}, {6, 9}};

    int mat_item_tier[] = {1, 2, 3, 4, 3};
    int mat_item_tier1_3[4] = {0, 1, 2, 4};
    int mat_item_tier3[2] = {2, 4};

    int user_select_toss1; // 기존 버릴건지, 보상 버릴건지
    int user_select_toss2; // 기존 아이템중 어떤거 버릴지
    int count;
    





    //상태이상
    char ABN_STATUS[][20] = {"중독","저주","나태"};                                           // 상태이상 이름
    int ABN_STATUS_ONOFF[3] = {0,0,0};                                  // ABN_STATUS_ONOFF[i] = ([0]==0 독이 꺼졌다. || [0]==1 독이 걸렸다)



    while (1)
    {
        map_print();
        
        // 던전 내 행동(메뉴열기) or (물리공격, 마법공격, 아이템사용, 도망가기) ==> 몬스터를 만났을 때 조건 추가

        while(floor != 0)                   ///////////// 던전 메뉴 /////////////
        {
            if (map_input =='1')
            {
                int village_select;
                printf("\n");
                printf("                                           //#.메뉴\n");
                printf("                                                                                                     \n");
                printf("                   // 1. ☆ 상태창 열기   // 2. 인벤토리 열기     3.🚪 메뉴나가기(break)                     \n");
                printf("\n");
                scanf("%d",&village_select);
                system("clear");
                if (village_select == 1) // 상태창 보여주기
                {
                    
                    int state_switch = 1;
                    while (state_switch)
                    {
                        state_check_UI(gold, live_exp, max_exp, live_hp, max_hp, live_mp, max_mp, added_stp, added_str, added_dex, added_int_, live_lv, START_STR, START_ATK, added_atk, START_EVA, added_eva, START_DEX, START_MTK, added_mtk, START_CRI, added_cri, START_INT_, live_def, return_stp);
                        printf("상태이상 : ");
                        for (int i=0;i<3;i++)
                        {
                            if (ABN_STATUS_ONOFF[i]==1)
                                printf("%s  ",ABN_STATUS[i]);
                            else
                                printf("  ");
                        }
                        printf("\n");
                        printf("------------------------------------------------------------------\n");
                        printf("[ 장비창 ]\n");
                        printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t|     7.특수옵션 및 수치\n");
                        for (int i = 0 ; i < 6 ; i++)
                        {
                            if (MY_EQUIP[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                            {
                                printf("%2d. %3d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                            }
                            else
                            {
                                printf("%2d. %3s\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%s\t|%s   %d\n", i+1, equipment_type2[(MY_EQUIP[i][0])-1][(MY_EQUIP[i][1])-1], MY_EQUIP[i][2], MY_EQUIP[i][3], MY_EQUIP[i][4], MY_EQUIP[i][5], MY_EQUIP[i][6], monster_pro_icon[MY_EQUIP[i][7]], equip_speical_opt_ENG[MY_EQUIP[i][8]], MY_EQUIP[i][9]);
                            }
                        }
                        printf("\n");
                        int user_select_stp1;
                        printf("[1. 스탯 올리기 | 2. 좌표 저장 | 3. 상태창 끄기] : ");
                        scanf("%d", &user_select_stp1);
                        switch (user_select_stp1)
                        {
                            case 1: // 스탯 올리기
                            {
                                if (return_stp == 0) // 스탯 포인트 부족한 경우
                                {
                                    system("clear");
                                    break;
                                }
                                int user_select_stp2;
                                printf("어떤 스탯을 올리시고 싶으신가요? [ 1. STR | 2. DEX | 3. INT ] : ");
                                scanf("%d", &user_select_stp2);
                                system("clear");
                                if (user_select_stp2 == 1)
                                {
                                    added_str += 1;
                                    max_hp += 5;
                                    added_atk += 2;
                                    return_stp -= 1;
                                }
                                else if (user_select_stp2 == 2)
                                {
                                    added_dex += 1;
                                    added_eva += 0.2;
                                    added_cri += 0.5;
                                    return_stp -= 1;
                                }
                                else if (user_select_stp2 == 3)
                                {
                                    added_int_ += 1;
                                    max_mp += 2;
                                    added_mtk += 1;
                                    return_stp -= 1;
                                }
                                // else if (user_select_stp2 == 4)
                                // {
                                //     continue;
                                // }
                                break;
                            }
                            case 2: // 순간이동 좌표 저장하기
                            {
                                system("clear");
                                printf("=============================================\n");
                                printf("\t    [ 저장된 위치 목록 ]\n");
                                printf("=============================================\n");
                                printf("      층      |     x좌표     |     y좌표\n");
                                printf("---------------------------------------------\n");
                                for (int i = 0 ; i < 5 ; i++) // 저장된 좌표 출력
                                {  
                                    printf("%d.    %d              %d              %d\n", i+1, save_point[i][0], save_point[i][2], save_point[i][1]);
                                }
                                printf("몇 번 위치에 저장할까요? [0. 나가기]: ");
                                scanf("%d", &user_position_select);
                                if (user_position_select == 0)
                                {
                                    system("clear");
                                    continue;
                                }
                                save_point[user_position_select-1][0] = floor;
                                save_point[user_position_select-1][1] = user_y;
                                save_point[user_position_select-1][2] = user_x;
                                system("clear");
                                break;
                            }
                            case 3: // 상태창 끄기
                            {
                                state_switch = 0;
                                system("clear");
                                break;
                            }
                        }
                    }
        
                }
                else if (village_select ==2)  //인벤토리 열기
                {
                    
                    int inven_switch = 1;
                    while (inven_switch)
                    {
                        int input;
                        printf("1. 인벤토리, 장비창 확인 | 2. 나가기 ");
                        scanf("%d", &input);
                        system("clear");
                        while (input == 1)
                        {
                            // 인벤토리 보여주는 코드
                            // 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                            printf("[ INVENTORY ]\n");
                            printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t|     7.특수옵션 및 수치\n");
                            for (int i = 0 ; i < 20 ; i++)
                            {
                                if (INVEN[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                                {
                                    printf("%2d. %3d\t\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                                }
                                else
                                {
                                    printf("%2d. %3s\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%s\t|%s   %d\n", i+1, equipment_type2[(INVEN[i][0])-1][(INVEN[i][1])-1], INVEN[i][2], INVEN[i][3], INVEN[i][4], INVEN[i][5], INVEN[i][6], monster_pro_icon[INVEN[i][7]], equip_speical_opt_ENG[INVEN[i][8]], INVEN[i][9]);
                                }
                            }
                            printf("===============================================================================================================================\n");
                            // 소비아이템 공간 보기
                            printf("[ 소비 아이템 ]\n");
                            for (int i = 0 ; i < 6 ; i++)
                            {
                                if (i < 4)
                                {
                                    printf("%2d. %s\t\t|%d개\t\t|\t\t%2d. %s\t\t|%d개\n", i+1, cons_item_name[i], cons_space[i], i+7, cons_item_name[i+6], cons_space[i+6]);
                                }
                                else
                                {
                                    printf("%2d. %s\t\t|%d개\t\t|\t\t%2d. %s\t|%d개\n", i+1, cons_item_name[i], cons_space[i], i+7, cons_item_name[i+6], cons_space[i+6]);
                                }
                            }
                            printf("===============================================================================================================================\n");
                            // 기타아이템 공간 보기
                            printf("[ 기타 아이템 ]\n");
                            for (int i = 0 ; i < 5 ; i++)
                            {
                                if (i == 4)
                                {
                                    printf("%d. %s\t\t|%d개\n", i+1, mat_item_name[i], mat_space[i]);
                                }
                                else
                                {
                                    printf("%d. %s\t|%d개\n", i+1, mat_item_name[i], mat_space[i]);
                                }
                            }
                            printf("===============================================================================================================================\n");
                            // 장비창 보기
                            printf("[ 장비창 ]\n");
                            printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t|     7.특수옵션 및 수치\n");
                            for (int i = 0 ; i < 6 ; i++)
                            {
                                if (MY_EQUIP[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                                {
                                    printf("%2d. %3d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                                }
                                else
                                {
                                    printf("%2d. %3s\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%s\t|%s   %d\n", i+1, equipment_type2[(MY_EQUIP[i][0])-1][(MY_EQUIP[i][1])-1], MY_EQUIP[i][2], MY_EQUIP[i][3], MY_EQUIP[i][4], MY_EQUIP[i][5], MY_EQUIP[i][6], monster_pro_icon[MY_EQUIP[i][7]], equip_speical_opt_ENG[MY_EQUIP[i][8]], MY_EQUIP[i][9]);
                                }
                            }
                            // 인벤토리에서 장비창으로 아이템 가져오기    
                            printf("교체할 장비가 있으면 골라주세요. [나가려면 0] : ");
                            int user_select_janbi;
                            scanf("%d", &user_select_janbi);
                            if (user_select_janbi == 0) // 0을 입력하면 나가요 
                            {
                                system("clear");
                                break;
                            }
                            // 인벤토리랑 장비창 교체하는 코드
                            if ((INVEN[user_select_janbi-1][0]) != 0)
                            {
                               for (int i = 0 ; i < 6 ; i++)
                                {
                                    if (MY_EQUIP[i][0] == INVEN[user_select_janbi-1][0]) // 같은 부위의 장비를 이미 장착중인 경우
                                    {
                                        // same_equip_switch = 1; // 같은 장비가 있다는 것을 알려주는 스위치가 0 => 1
                                        // 같은 부위를 선택하면 임시저장소에 내 장비 정보 저장
                                        MY_EQUIP_tmp[0][0] = MY_EQUIP[i][0];
                                        MY_EQUIP_tmp[0][1] = MY_EQUIP[i][1];
                                        MY_EQUIP_tmp[0][2] = MY_EQUIP[i][2];
                                        MY_EQUIP_tmp[0][3] = MY_EQUIP[i][3];
                                        MY_EQUIP_tmp[0][4] = MY_EQUIP[i][4];
                                        MY_EQUIP_tmp[0][5] = MY_EQUIP[i][5];
                                        MY_EQUIP_tmp[0][6] = MY_EQUIP[i][6];
                                        MY_EQUIP_tmp[0][7] = MY_EQUIP[i][7];
                                        MY_EQUIP_tmp[0][8] = MY_EQUIP[i][8];
                                        MY_EQUIP_tmp[0][9] = MY_EQUIP[i][9];
                
                                        // 내 장비에 선택한 인벤토리장비 정보 저장
                                        MY_EQUIP[i][0] = INVEN[user_select_janbi-1][0];
                                        MY_EQUIP[i][1] = INVEN[user_select_janbi-1][1];
                                        MY_EQUIP[i][2] = INVEN[user_select_janbi-1][2];
                                        MY_EQUIP[i][3] = INVEN[user_select_janbi-1][3];
                                        MY_EQUIP[i][4] = INVEN[user_select_janbi-1][4];
                                        MY_EQUIP[i][5] = INVEN[user_select_janbi-1][5];
                                        MY_EQUIP[i][6] = INVEN[user_select_janbi-1][6];
                                        MY_EQUIP[i][7] = INVEN[user_select_janbi-1][7];
                                        MY_EQUIP[i][8] = INVEN[user_select_janbi-1][8];
                                        MY_EQUIP[i][9] = INVEN[user_select_janbi-1][9];
                
                                        // 인벤토리에 임시저장소에 있는 내 장비 정보 저장
                                        INVEN[user_select_janbi-1][0] = MY_EQUIP_tmp[0][0];
                                        INVEN[user_select_janbi-1][1] = MY_EQUIP_tmp[0][1];
                                        INVEN[user_select_janbi-1][2] = MY_EQUIP_tmp[0][2];
                                        INVEN[user_select_janbi-1][3] = MY_EQUIP_tmp[0][3];
                                        INVEN[user_select_janbi-1][4] = MY_EQUIP_tmp[0][4];
                                        INVEN[user_select_janbi-1][5] = MY_EQUIP_tmp[0][5];
                                        INVEN[user_select_janbi-1][6] = MY_EQUIP_tmp[0][6];
                                        INVEN[user_select_janbi-1][7] = MY_EQUIP_tmp[0][7];
                                        INVEN[user_select_janbi-1][8] = MY_EQUIP_tmp[0][8];
                                        INVEN[user_select_janbi-1][9] = MY_EQUIP_tmp[0][9];
                                        break;
                                    }
                                    else if (MY_EQUIP[i][0] != INVEN[user_select_janbi-1][0])// 같은 부위의 장비가 없는 경우
                                    {
                                        if (MY_EQUIP[i][0] == 0) // 내 장비창 빈 칸에 장착
                                        {
                                            printf("선택하신 장비를 장착하였습니다.\n\n");
                                            MY_EQUIP[i][0] = INVEN[user_select_janbi-1][0];
                                            MY_EQUIP[i][1] = INVEN[user_select_janbi-1][1];
                                            MY_EQUIP[i][2] = INVEN[user_select_janbi-1][2];
                                            MY_EQUIP[i][3] = INVEN[user_select_janbi-1][3];
                                            MY_EQUIP[i][4] = INVEN[user_select_janbi-1][4];
                                            MY_EQUIP[i][5] = INVEN[user_select_janbi-1][5];
                                            MY_EQUIP[i][6] = INVEN[user_select_janbi-1][6];
                                            MY_EQUIP[i][7] = INVEN[user_select_janbi-1][7];
                                            MY_EQUIP[i][8] = INVEN[user_select_janbi-1][8];
                                            MY_EQUIP[i][9] = INVEN[user_select_janbi-1][9];
                
                                            // 인벤토리 -> 장비창 ====> 인벤토리에 그 아이템은 지워요 (그냥 꺼내는경우, 교체X)
                                            INVEN[user_select_janbi-1][0] = 0; // 큰 종류
                                            INVEN[user_select_janbi-1][1] = 0; // 작은 종류
                                            INVEN[user_select_janbi-1][2] = 0; // 티어
                                            INVEN[user_select_janbi-1][3] = 0; // 기본옵션(기본공격력/데미지감소)
                                            INVEN[user_select_janbi-1][4] = 0; // 강화레벨
                                            INVEN[user_select_janbi-1][5] = 0; // 강화옵션(%)
                                            INVEN[user_select_janbi-1][6] = 0; // 인챈트(%)
                                            INVEN[user_select_janbi-1][7] = 0; // 인챈트 속성
                                            INVEN[user_select_janbi-1][8] = 0; // 특수옵션
                                            INVEN[user_select_janbi-1][9] = 0; // 특수옵션 수치
                                            break;
                                        }
                                    }
                                }
                            }
                            else // 아이템을 벗고싶은 경우
                            {
                                int user_select_janbi2;
                                printf("인벤토리에 넣을 장비를 선택해주세요. : ");
                                scanf("%d", &user_select_janbi2);
                                INVEN[user_select_janbi-1][0] = MY_EQUIP[user_select_janbi2-1][0]; // 큰 종류
                                INVEN[user_select_janbi-1][1] = MY_EQUIP[user_select_janbi2-1][1]; // 작은 종류
                                INVEN[user_select_janbi-1][2] = MY_EQUIP[user_select_janbi2-1][2]; // 티어
                                INVEN[user_select_janbi-1][3] = MY_EQUIP[user_select_janbi2-1][3]; // 기본옵션(기본공격력/데미지감소)
                                INVEN[user_select_janbi-1][4] = MY_EQUIP[user_select_janbi2-1][4]; // 강화레벨
                                INVEN[user_select_janbi-1][5] = MY_EQUIP[user_select_janbi2-1][5]; // 강화옵션(%)
                                INVEN[user_select_janbi-1][6] = MY_EQUIP[user_select_janbi2-1][6]; // 인챈트(%)
                                INVEN[user_select_janbi-1][7] = MY_EQUIP[user_select_janbi2-1][7]; // 인챈트 속성
                                INVEN[user_select_janbi-1][8] = MY_EQUIP[user_select_janbi2-1][8]; // 특수옵션
                                INVEN[user_select_janbi-1][9] = MY_EQUIP[user_select_janbi2-1][9]; // 특수옵션 수치
                            
                                MY_EQUIP[user_select_janbi2-1][0] = 0; // 큰 종류
                                MY_EQUIP[user_select_janbi2-1][1] = 0; // 작은 종류
                                MY_EQUIP[user_select_janbi2-1][2] = 0; // 티어
                                MY_EQUIP[user_select_janbi2-1][3] = 0; // 기본옵션(기본공격력/데미지감소)
                                MY_EQUIP[user_select_janbi2-1][4] = 0; // 강화레벨
                                MY_EQUIP[user_select_janbi2-1][5] = 0; // 강화옵션(%)
                                MY_EQUIP[user_select_janbi2-1][6] = 0; // 인챈트(%)
                                MY_EQUIP[user_select_janbi2-1][7] = 0; // 인챈트 속성
                                MY_EQUIP[user_select_janbi2-1][8] = 0; // 특수옵션
                                MY_EQUIP[user_select_janbi2-1][9] = 0; // 특수옵션 수치
                            }
                            system("clear");
                        }
                        if (input == 2) // 인벤토리 나가기
                        {
                            inven_switch = 0;
                        }
                    }




                }


                else if (village_select == 3) // 메뉴 나가기
                    map_not_change();
        
            }
            break;
        }

        while(floor == 0 && tp_switch ==0)          //////////마을 메뉴//////////////
        {
            int village_select;
            if (floor == 0)
            {
                printf("\n");
                printf("                        //#.메뉴\n");
                printf("//1.📦 판도라 - 잡화상 // 2.⛪ 성직자 - 성소 // 3.🔨 드워프 - 제련소         // 7. ☆ 상태창 열기 // 8.인벤토리 열기 \n");
                printf("//4.🔮 마법사 - 인챈트 // 5.🏦 은행 - 보관소 // 6.⚔️ 던전입장 - (50,49)      // 0.🚪 메뉴나가기(break)          \n");
                printf("\n");
                scanf("%d",&village_select);
                system("clear");
            }        

            if (village_select == 1) // 잡화점
            {
                while (shop_switch) // 상점에 도착하면?
                {
                    printf("==============================================\n");
                    printf(" 구매할 아이템을 선택하세요. (🪙 %d GOLD)\n", gold);
                    printf("==============================================\n");
                    printf("\t\t1. 소비아이템\n");
                    printf("\t\t2. 무기\n");
                    printf("\t\t3. 갑옷\n");
                    printf("\t\t4. 신발\n");
                    printf("\t\t5. 장갑\n");
                    printf("\t\t6. 망토\n");
                    printf("\t\t7. 투구\n");
                    printf("\t\t8. 상점 나가기\n");
                    printf("==============================================\n");
                    scanf("%d", &user_cons_select1);
                    system("clear");
                    // user_cons_select2 = shop(&GOLD, user_cons_select1, cons_item_name, weapon_name, armor_name, shoes_name, glove_name, cloak_name, hat_name, cons_item_price, weapon_price, armor_price, shoes_price, glove_price, cloak_price, hat_price, len_cons_item, len_weapon, len_armor, len_shoes, len_glove, len_cloak, len_hat);
                    user_cons_select2 = shop(&price, user_cons_select1, cons_item_name, weapon_name, armor_name, shoes_name, glove_name, cloak_name, hat_name, cons_item_price, weapon_price, armor_price, shoes_price, glove_price, cloak_price, hat_price, len_cons_item, len_weapon, len_armor, len_shoes, len_glove, len_cloak, len_hat);
            
                    if (user_cons_select1 == 8) // 상점 나가기를 선택한 경우
                    {
                        system("clear");
                        break;
                    }
                    else if(gold < price)
                    {
                        printf("GOLD가 부족합니다.\n");
                        continue;
                    }
                    else if (user_cons_select2 == 0) // 0번이면 이전메뉴
                    {
                        continue;
                    }
                    else if (user_cons_select1 == 1) // 소비아이템을 구매하면 소비공간에 들어감.
                    {
                        if (cons_space[user_cons_select2-1] == 99) // 선택한 소비 아이템의 소지 갯수가 99개이면 구매불가
                        {
                            printf("소지 가능 갯수를 초과하였습니다.\n"); 
                            continue;
                        }
                        cons_space[user_cons_select2-1] += 1; // 99개 미만이면 1개 구매가능
                    }
                    else if (user_cons_select1 != 1) // 장비를 사면 인벤토리에 들어가는 코드
                    {
                        int count = 0; // 가득 찬 인벤토리 갯수
                        for (int i = 0 ; i < 20 ; i++)
                        {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                            if (INVEN[i][0] == 0)
                            {
                                INVEN[i][0] = user_cons_select1 - 1; // 큰 종류
                                INVEN[i][1] = user_cons_select2; // 작은 종류
                                INVEN[i][2] = equipment_tier[user_cons_select1-2][user_cons_select2-1]; // 티어
                                INVEN[i][3] = equipment_basic_option[user_cons_select1-2][user_cons_select2-1]; // 기본옵션(기본공격력/데미지감소)
                                INVEN[i][4] = 0; // 강화레벨
                                INVEN[i][5] = 0; // 강화옵션(%)
                                INVEN[i][6] = 0; // 인챈트(%)
                                INVEN[i][7] = 0; // 인챈트 속성
                                INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                break;
                            }
                            else
                            {
                                count++;
                            }
                        }
                        if (count == 20) // 인벤토리 20칸이 꽉 차면 구매불가
                        {
                            printf("인벤토리가 가득 찼습니다!\n");
                            continue;
                        }
                    }
                    gold -= price; // 소지한 GOLD에서 구매한 물건 가격 뺌
                    printf("구매 완료!\n");
                }
                continue;
            
               
            
            
            }
            
            else if (village_select == 2) // 성소
            {
                printf("성소입장\n");
                
                int input;
                int temp;
                printf("무슨 일로 오셨습니까?\n");
                printf("1)회복 및 상태이상 치유\n2)스텟 초기화\n3)나가기\n(숫자를 입력하세요)");
                scanf("%d",&input);
                system("clear");
                if(input == 1)
                {   
                    printf("신의축복을 받으시겠습니까?\n맞으면 1번 틀리면 2번을 눌러주세요.\n");
                    printf("HP는 %d/%d\nMP는 %d/%d\n",live_hp,max_hp,live_mp,max_mp);
                    scanf("%d",&input);
                    system("clear");

                    if(input == 1)
                    {     
                        live_hp = max_hp;           //
                        live_mp = max_mp;
                        ABN_STATUS_ONOFF[0] = 0;    //상태이상 치유
                        ABN_STATUS_ONOFF[1] = 0;
                        ABN_STATUS_ONOFF[2] = 0;
                        printf("신의축복을 내립니다.\n");
                        printf("HP는 %d/%d\n MP는 %d/%d\n",live_hp,max_hp,live_mp,max_mp);

                        continue;

                    }
                    else if(input ==2)
                    {
                        continue;
                    }
        
                }
        
                else if (input == 2)
                {
                    printf("스탯을 초기화 하시겠습니까?\n 맞으면 1번 틀리면 2번을 눌러주세요.\n");
                    printf("현재 스탯 STR:%d DEX:%d INT:%d\n",live_str,live_dex,live_int_);
                    scanf("%d",&input);

                    if(input == 1)
                    {   
                        return_stp = added_stp;
                        added_str,added_dex,added_int_,added_stp = 0;
                        
                        printf("반환된 스탯의 값은 \n%d 입니다.\n", return_stp);

                    }
                        continue;
                }   
                else if (input == 3)
                {   
                    system("clear");      
                }
                break;
            }
            
            else if (village_select == 3) // 제련소(강화)
            {
                printf("제련소입장\n");

                while (1)
                {
                    printf("인벤토리를 확인하려면 0번 누르세요 : ");
                    scanf("%d", &input);
                    if (input == 0)
                    {
                        // 인벤토리 보여주는 코드
                        // 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                        printf("[ INVENTORY ]\n");
                        printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t|     7.특수옵션 및 수치\n");
                        for (int i = 0 ; i < 20 ; i++)
                        {
                            if (INVEN[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                            {
                                printf("%2d. %3d\t\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                            }
                            else
                            {
                                printf("%2d. %3s\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%s\t|%s   %d\n", i+1, equipment_type2[(INVEN[i][0])-1][(INVEN[i][1])-1], INVEN[i][2], INVEN[i][3], INVEN[i][4], INVEN[i][5], INVEN[i][6], monster_pro_icon[INVEN[i][7]], equip_speical_opt_ENG[INVEN[i][8]], INVEN[i][9]);
                            }
                        }
                        printf("===============================================================================================================================\n");
                        // 기타아이템 공간 보기
                        printf("[ 기타 아이템 ]\n");
                        for (int i = 0 ; i < 5 ; i++)
                        {
                            printf("%d. %s\t|%d개\n", i+1, mat_item_name[i], mat_space[i]);
                        }
                        printf("소지 GOLD : 🪙 %d\n", gold);
                    }
                    // 강화에 필요한 사전조건
                    // 1. 장비를 가지고있어야하며 - 장비선택
                    printf("몇번장비를 강화 하시겠습니까? : ");
                    scanf("%d",&input);
                    system("clear");
                    // 2. 장비의 강화레벨을 확인하고
                    int enhance_lv_check = INVEN[input-1][4];
                    int mat_space_num[10] = {0,0,0,1,1,1,2,2,2,3};
                    //           [0] = 0,1,2
                    //           [1] = 3,4,5
                    //           [2] = 6,7,8
                    //           [3] = 9
                    int enhance_random_num;
                    
                    // 3. 그에맞는 다음단계강화로 필요한 골드/알맞은재료/재료의갯수 - 조건
                    if (gold >= (need_gold[enhance_lv_check]) && ((mat_space[mat_space_num[enhance_lv_check]]) >= (need_mat[enhance_lv_check])))
                    { 
                        // 4. 확률 로직
                        // (총골드-골드) && (등급에맞는 총재료갯수 - 등급에따른 재료갯수)
                        gold = gold - need_gold[enhance_lv_check];
                        mat_space[mat_space_num[enhance_lv_check]] = mat_space[mat_space_num[enhance_lv_check]] - need_mat[enhance_lv_check];    
                                                    
                        srand(time(NULL));
                        enhance_random_num = (rand() % 100) + 1; // 1부터100까지 랜덤수중에 하나추출

                        if (enhance_random_num <= success_per[enhance_lv_check])
                        {
                        
                            INVEN[input-1][4] += 1; 
                            INVEN[input-1][5] = increase_damage_per[enhance_lv_check+1];
                            // enhance_lv_check += 1;

                            printf("🤩🤩강화성공🤩🤩\n");
                            break;
                        }   
                        else
                        { 
                            printf("😭😭강화실패😭😭\n");
                            srand(time(NULL));
                            enhance_random_num = (rand() % 100) + 1; // 1부터100까지 랜덤수중에 하나추출
                            if (enhance_random_num <= down_per[enhance_lv_check])
                            { 
                                INVEN[input-1][4] -= 1; 
                                INVEN[input-1][5] = increase_damage_per[enhance_lv_check-1];
                                // enhance_lv_check -= 1;
                                printf("강화 하락\n");
                                break;
                            }
                            else if (enhance_random_num <= 10)
                            {
                                printf("강화파괴\n");
                                INVEN[input-1][0] = 0;
                                INVEN[input-1][1] = 0;
                                INVEN[input-1][2] = 0;
                                INVEN[input-1][3] = 0;
                                INVEN[input-1][4] = 0;
                                INVEN[input-1][5] = 0;
                                INVEN[input-1][6] = 0;
                                INVEN[input-1][7] = 0;
                                INVEN[input-1][8] = 0;
                                INVEN[input-1][9] = 0;
                                break;     
                            }
                            else
                                printf("강화등급유지\n");
                        }
                    }    
                }
                
            }
            
            else if (village_select == 4) // 인챈트
            {
                printf("인챈트점입장\n");
                printf("인벤토리를 확인하려면 0번 누르세요 : ");
                scanf("%d", &input);
                if (input == 0)
                {
                    // 인벤토리 보여주는 코드
                    // 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                    printf("[ INVENTORY ]\n");
                    printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t|     7.특수옵션 및 수치\n");
                    for (int i = 0 ; i < 20 ; i++)
                    {
                        if (INVEN[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                        {
                            printf("%2d. %3d\t\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                        }
                        else
                        {
                            printf("%2d. %3s\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%s\t|%s   %d\n", i+1, equipment_type2[(INVEN[i][0])-1][(INVEN[i][1])-1], INVEN[i][2], INVEN[i][3], INVEN[i][4], INVEN[i][5], INVEN[i][6], monster_pro_icon[INVEN[i][7]], equip_speical_opt_ENG[INVEN[i][8]], INVEN[i][9]);
                        }
                    }
                    printf("===============================================================================================================================\n");
                    // 기타아이템 공간 보기
                    printf("[ 기타 아이템 ]\n");
                    for (int i = 0 ; i < 5 ; i++)
                    {
                        printf("%d. %s\t|%d개\n", i+1, mat_item_name[i], mat_space[i]);
                    }
                }
                printf("몇번장비에 인챈트를 하시겠습니까");
                scanf("%d",&input);
                if (INVEN[input-1][0] == 1 && mat_space[4] >= 10)//인챈트필요조건
                {
                    int enchant_property_random_num; // 속성값에 저장
                    int enchant_propertied_num;
                    int enchant_plused_damage_random_num;
                    int enchant_plused_damage;
                    
                    // INVEN[input-1][3] = //선택무기공격력
                    // INVEN[input-1][6] = //저장위치1
                    
                    enchant_propertied_num = enchant_property(enchant_property_random_num);
                    enchant_plused_damage = enchant_damage(enchant_plused_damage_random_num); 

                    mat_space[4] = mat_space[4] - 10;
                    printf("무기기본데미지 : %d / 추가데미지 : %d\n",INVEN[input-1][3],enchant_plused_damage);
                    printf("속성값 : %s\n", properties_list[enchant_propertied_num]);

                    INVEN[input-1][7] = enchant_propertied_num;
                    INVEN[input-1][6] = enchant_plused_damage;

                }
            }
            
            else if (village_select == 5) // 은행
            {
                printf("은행점검시간입니다.\n");
            }
            
            else if (village_select == 6) // 던전입구 치트키
            {   
                system("clear");
                map_not_change();
                map[user_y][user_x]=6;   ///원래 있던자리 빈공간
                user_y =1;               // 좌표 던전앞으로 수정
                user_x =49;
                map[user_y][user_x]=1;   //던전 앞으로 수정된 좌표에 내위치 생성
                printf("던전입구로 이동...\n");
                break;
                //위치값이동
            }
            
            else if (village_select == 0) // 맵으로 나가기
            {   
                map_not_change();
                break;
                //맵출력
            }

            else if (village_select == 7) // 상태창 보여주기
            {
                int state_switch = 1;
                while (state_switch)
                {
                    state_check_UI(gold, live_exp, max_exp, live_hp, max_hp, live_mp, max_mp, added_stp, added_str, added_dex, added_int_, live_lv, START_STR, START_ATK, added_atk, START_EVA, added_eva, START_DEX, START_MTK, added_mtk, START_CRI, added_cri, START_INT_, live_def, return_stp);
                    printf("상태이상 : ");
                    for (int i=0;i<3;i++)
                    {
                        if (ABN_STATUS_ONOFF[i]==1)
                            printf("%s  ",ABN_STATUS[i]);
                        else
                            printf("  ");
                    }
                    printf("\n");
                    printf("------------------------------------------------------------------\n");
                    printf("[ 장비창 ]\n");
                    printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t|     7.특수옵션 및 수치\n");
                    for (int i = 0 ; i < 6 ; i++)
                    {
                        if (MY_EQUIP[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                        {
                            printf("%2d. %3d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                        }
                        else
                        {
                            printf("%2d. %3s\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%s\t|%s   %d\n", i+1, equipment_type2[(MY_EQUIP[i][0])-1][(MY_EQUIP[i][1])-1], MY_EQUIP[i][2], MY_EQUIP[i][3], MY_EQUIP[i][4], MY_EQUIP[i][5], MY_EQUIP[i][6], monster_pro_icon[MY_EQUIP[i][7]], equip_speical_opt_ENG[MY_EQUIP[i][8]], MY_EQUIP[i][9]);
                        }
                    }
                    int user_select_stp1;
                    printf("[1. 스탯 올리기 | 2. 좌표 저장 | 3. 상태창 끄기] : ");
                    scanf("%d", &user_select_stp1);
                    switch (user_select_stp1)
                    {
                        case 1: // 스탯 올리기
                        {
                            if (return_stp == 0) // 스탯 포인트 부족한 경우
                            {
                                system("clear");
                                break;
                            }
                            int user_select_stp2;
                            printf("어떤 스탯을 올리시고 싶으신가요? [1. STR | 2. DEX | 3. INT] : ");
                            scanf("%d", &user_select_stp2);
                            system("clear");
                            if (user_select_stp2 == 1)
                            {
                                added_str += 1;
                                max_hp += 5;
                                added_atk += 2;
                                return_stp -= 1;
                            }
                            else if (user_select_stp2 == 2)
                            {
                                added_dex += 1;
                                added_eva += 0.2;
                                added_cri += 0.5;
                                return_stp -= 1;
                            }
                            else if (user_select_stp2 == 3)
                            {
                                added_int_ += 1;
                                max_mp += 2;
                                added_mtk += 1;
                                return_stp -= 1;
                            }
                            break;
                        }
                        case 2: // 순간이동 좌표 저장하기
                        {
                            system("clear");
                            printf("=============================================\n");
                            printf("\t    [ 저장된 위치 목록 ]\n");
                            printf("=============================================\n");
                            printf("      층      |     x좌표     |     y좌표\n");
                            printf("---------------------------------------------\n");
                            for (int i = 0 ; i < 5 ; i++) // 저장된 좌표 출력
                            {  
                                printf("%d.    %d              %d              %d\n", i+1, save_point[i][0], save_point[i][2], save_point[i][1]);
                            }
                            printf("몇 번 위치에 저장할까요? : ");
                            scanf("%d", &user_position_select);
                            save_point[user_position_select-1][0] = floor;
                            save_point[user_position_select-1][1] = user_y;
                            save_point[user_position_select-1][2] = user_x;
                            system("clear");
                            break;
                        }
                        case 3: // 상태창 끄기
                        {
                            state_switch = 0;
                            system("clear");
                            break;
                        }
                    }
                }
            }
  
            else if (village_select == 8) // 인벤토리 열기
            {
                
                int inven_switch = 1;
                while (inven_switch)
                {
                    int input;
                    printf("1. 인벤토리, 장비창 확인 | 2. 나가기 ");
                    scanf("%d", &input);
                    system("clear");
                    while (input == 1)
                    {
                        // 인벤토리 보여주는 코드
                        // 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                        printf("[ INVENTORY ]\n");
                        printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t|     7.특수옵션 및 수치\n");
                        for (int i = 0 ; i < 20 ; i++)
                        {
                            if (INVEN[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                            {
                                printf("%2d. %3d\t\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                            }
                            else
                            {
                                printf("%2d. %3s\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%s\t|%s   %d\n", i+1, equipment_type2[(INVEN[i][0])-1][(INVEN[i][1])-1], INVEN[i][2], INVEN[i][3], INVEN[i][4], INVEN[i][5], INVEN[i][6], monster_pro_icon[INVEN[i][7]], equip_speical_opt_ENG[INVEN[i][8]], INVEN[i][9]);
                            }
                        }
                        printf("===============================================================================================================================\n");
                        // 소비아이템 공간 보기
                        printf("[ 소비 아이템 ]\n");
                        for (int i = 0 ; i < 6 ; i++)
                        {
                            if (i < 4)
                            {
                                printf("%2d. %s\t\t|%d개\t\t|\t\t%2d. %s\t\t|%d개\n", i+1, cons_item_name[i], cons_space[i], i+7, cons_item_name[i+6], cons_space[i+6]);
                            }
                            else
                            {
                                printf("%2d. %s\t\t|%d개\t\t|\t\t%2d. %s\t|%d개\n", i+1, cons_item_name[i], cons_space[i], i+7, cons_item_name[i+6], cons_space[i+6]);
                            }
                        }
                        printf("===============================================================================================================================\n");
                        // 기타아이템 공간 보기
                        printf("[ 기타 아이템 ]\n");
                        for (int i = 0 ; i < 5 ; i++)
                        {
                            if (i == 4)
                            {
                                printf("%d. %s\t\t|%d개\n", i+1, mat_item_name[i], mat_space[i]);
                            }
                            else
                            {
                                printf("%d. %s\t|%d개\n", i+1, mat_item_name[i], mat_space[i]);
                            }
                        }
                        printf("===============================================================================================================================\n");
                        // 장비창 보기
                        printf("[ 장비창 ]\n");
                        printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t|     7.특수옵션 및 수치\n");
                        for (int i = 0 ; i < 6 ; i++)
                        {
                            if (MY_EQUIP[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                            {
                                printf("%2d. %3d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                            }
                            else
                            {
                                printf("%2d. %3s\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t|\t%s\t|%s   %d\n", i+1, equipment_type2[(MY_EQUIP[i][0])-1][(MY_EQUIP[i][1])-1], MY_EQUIP[i][2], MY_EQUIP[i][3], MY_EQUIP[i][4], MY_EQUIP[i][5], MY_EQUIP[i][6], monster_pro_icon[MY_EQUIP[i][7]], equip_speical_opt_ENG[MY_EQUIP[i][8]], MY_EQUIP[i][9]);
                            }
                        }
                        // 인벤토리에서 장비창으로 아이템 가져오기    
                        printf("교체할 장비가 있으면 골라주세요. [나가려면 0] : ");
                        int user_select_janbi;
                        scanf("%d", &user_select_janbi);
                        if (user_select_janbi == 0) // 0을 입력하면 나가요 
                        {
                            system("clear");
                            break;
                        }
                        // 인벤토리랑 장비창 교체하는 코드
                        if ((INVEN[user_select_janbi-1][0]) != 0)
                        {
                            for (int i = 0 ; i < 6 ; i++)
                            {
                                if (MY_EQUIP[i][0] == INVEN[user_select_janbi-1][0]) // 같은 부위의 장비를 이미 장착중인 경우
                                {
                                    // same_equip_switch = 1; // 같은 장비가 있다는 것을 알려주는 스위치가 0 => 1
                                    // 같은 부위를 선택하면 임시저장소에 내 장비 정보 저장
                                    MY_EQUIP_tmp[0][0] = MY_EQUIP[i][0];
                                    MY_EQUIP_tmp[0][1] = MY_EQUIP[i][1];
                                    MY_EQUIP_tmp[0][2] = MY_EQUIP[i][2];
                                    MY_EQUIP_tmp[0][3] = MY_EQUIP[i][3];
                                    MY_EQUIP_tmp[0][4] = MY_EQUIP[i][4];
                                    MY_EQUIP_tmp[0][5] = MY_EQUIP[i][5];
                                    MY_EQUIP_tmp[0][6] = MY_EQUIP[i][6];
                                    MY_EQUIP_tmp[0][7] = MY_EQUIP[i][7];
                                    MY_EQUIP_tmp[0][8] = MY_EQUIP[i][8];
                                    MY_EQUIP_tmp[0][9] = MY_EQUIP[i][9];

                                    // 내 장비에 선택한 인벤토리장비 정보 저장
                                    MY_EQUIP[i][0] = INVEN[user_select_janbi-1][0];
                                    MY_EQUIP[i][1] = INVEN[user_select_janbi-1][1];
                                    MY_EQUIP[i][2] = INVEN[user_select_janbi-1][2];
                                    MY_EQUIP[i][3] = INVEN[user_select_janbi-1][3];
                                    MY_EQUIP[i][4] = INVEN[user_select_janbi-1][4];
                                    MY_EQUIP[i][5] = INVEN[user_select_janbi-1][5];
                                    MY_EQUIP[i][6] = INVEN[user_select_janbi-1][6];
                                    MY_EQUIP[i][7] = INVEN[user_select_janbi-1][7];
                                    MY_EQUIP[i][8] = INVEN[user_select_janbi-1][8];
                                    MY_EQUIP[i][9] = INVEN[user_select_janbi-1][9];

                                    // 인벤토리에 임시저장소에 있는 내 장비 정보 저장
                                    INVEN[user_select_janbi-1][0] = MY_EQUIP_tmp[0][0];
                                    INVEN[user_select_janbi-1][1] = MY_EQUIP_tmp[0][1];
                                    INVEN[user_select_janbi-1][2] = MY_EQUIP_tmp[0][2];
                                    INVEN[user_select_janbi-1][3] = MY_EQUIP_tmp[0][3];
                                    INVEN[user_select_janbi-1][4] = MY_EQUIP_tmp[0][4];
                                    INVEN[user_select_janbi-1][5] = MY_EQUIP_tmp[0][5];
                                    INVEN[user_select_janbi-1][6] = MY_EQUIP_tmp[0][6];
                                    INVEN[user_select_janbi-1][7] = MY_EQUIP_tmp[0][7];
                                    INVEN[user_select_janbi-1][8] = MY_EQUIP_tmp[0][8];
                                    INVEN[user_select_janbi-1][9] = MY_EQUIP_tmp[0][9];
                                    break;
                                }
                                else if (MY_EQUIP[i][0] != INVEN[user_select_janbi-1][0])// 같은 부위의 장비가 없는 경우
                                {
                                    if (MY_EQUIP[i][0] == 0) // 내 장비창 빈 칸에 장착
                                    {
                                        printf("선택하신 장비를 장착하였습니다.\n\n");
                                        MY_EQUIP[i][0] = INVEN[user_select_janbi-1][0];
                                        MY_EQUIP[i][1] = INVEN[user_select_janbi-1][1];
                                        MY_EQUIP[i][2] = INVEN[user_select_janbi-1][2];
                                        MY_EQUIP[i][3] = INVEN[user_select_janbi-1][3];
                                        MY_EQUIP[i][4] = INVEN[user_select_janbi-1][4];
                                        MY_EQUIP[i][5] = INVEN[user_select_janbi-1][5];
                                        MY_EQUIP[i][6] = INVEN[user_select_janbi-1][6];
                                        MY_EQUIP[i][7] = INVEN[user_select_janbi-1][7];
                                        MY_EQUIP[i][8] = INVEN[user_select_janbi-1][8];
                                        MY_EQUIP[i][9] = INVEN[user_select_janbi-1][9];

                                        // 인벤토리 -> 장비창 ====> 인벤토리에 그 아이템은 지워요 (그냥 꺼내는경우, 교체X)
                                        INVEN[user_select_janbi-1][0] = 0; // 큰 종류
                                        INVEN[user_select_janbi-1][1] = 0; // 작은 종류
                                        INVEN[user_select_janbi-1][2] = 0; // 티어
                                        INVEN[user_select_janbi-1][3] = 0; // 기본옵션(기본공격력/데미지감소)
                                        INVEN[user_select_janbi-1][4] = 0; // 강화레벨
                                        INVEN[user_select_janbi-1][5] = 0; // 강화옵션(%)
                                        INVEN[user_select_janbi-1][6] = 0; // 인챈트(%)
                                        INVEN[user_select_janbi-1][7] = 0; // 인챈트 속성
                                        INVEN[user_select_janbi-1][8] = 0; // 특수옵션
                                        INVEN[user_select_janbi-1][9] = 0; // 특수옵션 수치
                                        break;
                                    }
                                }
                            }
                        }
                        else // 아이템을 벗고싶은 경우
                        {
                            int user_select_janbi2;
                            printf("인벤토리에 넣을 장비를 선택해주세요. : ");
                            scanf("%d", &user_select_janbi2);
                            INVEN[user_select_janbi-1][0] = MY_EQUIP[user_select_janbi2-1][0]; // 큰 종류
                            INVEN[user_select_janbi-1][1] = MY_EQUIP[user_select_janbi2-1][1]; // 작은 종류
                            INVEN[user_select_janbi-1][2] = MY_EQUIP[user_select_janbi2-1][2]; // 티어
                            INVEN[user_select_janbi-1][3] = MY_EQUIP[user_select_janbi2-1][3]; // 기본옵션(기본공격력/데미지감소)
                            INVEN[user_select_janbi-1][4] = MY_EQUIP[user_select_janbi2-1][4]; // 강화레벨
                            INVEN[user_select_janbi-1][5] = MY_EQUIP[user_select_janbi2-1][5]; // 강화옵션(%)
                            INVEN[user_select_janbi-1][6] = MY_EQUIP[user_select_janbi2-1][6]; // 인챈트(%)
                            INVEN[user_select_janbi-1][7] = MY_EQUIP[user_select_janbi2-1][7]; // 인챈트 속성
                            INVEN[user_select_janbi-1][8] = MY_EQUIP[user_select_janbi2-1][8]; // 특수옵션
                            INVEN[user_select_janbi-1][9] = MY_EQUIP[user_select_janbi2-1][9]; // 특수옵션 수치
                        
                            MY_EQUIP[user_select_janbi2-1][0] = 0; // 큰 종류
                            MY_EQUIP[user_select_janbi2-1][1] = 0; // 작은 종류
                            MY_EQUIP[user_select_janbi2-1][2] = 0; // 티어
                            MY_EQUIP[user_select_janbi2-1][3] = 0; // 기본옵션(기본공격력/데미지감소)
                            MY_EQUIP[user_select_janbi2-1][4] = 0; // 강화레벨
                            MY_EQUIP[user_select_janbi2-1][5] = 0; // 강화옵션(%)
                            MY_EQUIP[user_select_janbi2-1][6] = 0; // 인챈트(%)
                            MY_EQUIP[user_select_janbi2-1][7] = 0; // 인챈트 속성
                            MY_EQUIP[user_select_janbi2-1][8] = 0; // 특수옵션
                            MY_EQUIP[user_select_janbi2-1][9] = 0; // 특수옵션 수치
                        }
                        system("clear");
                    }
                    if (input == 2) // 인벤토리 나가기
                    {
                        inven_switch = 0;
                    }
                }
            }
            break;
        }
        
        if (war_switch == 1)         //////////// 전투 ////////////
        {   
            mon_determine(max_hp);
            turn_switch = 0;
            int run_switch = 0;
            int immune = 0;
            int mtk_m = 0;
            while(floor != 0)     
            {
                if(ABN_STATUS_ONOFF[0]==1)
                    live_hp = live_hp-(max_hp*2/100);
                
                for(int i=0;i<6;i++)
                {
                    if (MY_EQUIP[i][0]==1)              //착용 장비 공격력
                    {
                        live_atk+=MY_EQUIP[i][3];
                    }
                    if (MY_EQUIP[i][8]==4)              //착용장비 특수 옵션 모든 공격력%+
                    {
                        ALLTK_PER+=MY_EQUIP[i][9];
                    }
                    if (MY_EQUIP[i][8]==1)              //착용장비 특수 옵션 크리확률
                    {
                        live_cri+=MY_EQUIP[i][9];
                    }
                    if (MY_EQUIP[i][8]==2)              //착용장비 특수 옵션 받피감
                    {
                        live_def+=MY_EQUIP[i][9];
                    }
                    if (MY_EQUIP[i][8]==3)              //착용장비 특수 옵션 회피율
                    {
                        live_eva+=MY_EQUIP[i][9];
                    }
                    if (MY_EQUIP[i][8]==6)              //착용장비 특수 옵션 상태이상
                    {
                        immune =1 ;
                    }
                    if (MY_EQUIP[i][8]==5)              //착용장비 특수 옵션 마법배율
                    {
                        mtk_m =1 ;
                    }
                }
                
                
                //전투 UI
                printf("강점 : %s 약점 : %s\n",monster_pro_icon[monster_pro_stren[mon_name_num]],monster_pro_icon[monster_pro_weak[mon_name_num]]);
                printf("이름:%s 공격력:%d 방어력:%d HP:%d\n",monster_korea_name[mon_name_num],monster_max_atk,monster_max_ac,monster_max_hp);
                printf("용사복이의 공격력:%d 방어력:%d HP:%d/%d MP:%d/%d EXP:%d/%d\n",live_atk,live_def,live_hp,max_hp,live_mp,max_mp,live_exp,max_exp);
                printf("상태이상 : ");
                for (int i=0;i<3;i++)
                {
                    if (ABN_STATUS_ONOFF[i]==1)
                        printf("%s  ",ABN_STATUS[i]);
                    else
                        printf("  ");
                }
                printf("\n");
                printf("=============================================\n");
                printf("\t\t[ 전투 타임 ]\n");
                printf("=============================================\n");
                printf("1. 물리공격\n");
                printf("2. 마법공격\n");
                printf("3. 아이템 사용\n");
                printf("4. 도망가기\n");
                printf("=============================================\n");
                scanf("%d", &user_war_select);

                switch (user_war_select)
                {
                    case 1:                 //물리공격 선택
                    {
                        if (ABN_STATUS_ONOFF[1]==1)
                            ALLTK_PER = ALLTK_PER+(-1*0.2);
                        if (ABN_STATUS_ONOFF[2]==1)
                            ALLTK_PER = ALLTK_PER+(-1*0.5);
                        printf("물리공격 선택\n");
                        cri_ran_ = cri_ran();
                        
                        if (live_cri <= cri_ran_)
                        {
                            cri_success = 2;
                        }
                        else
                            cri_success = 1;

                        float weakness_point;
                        weakness_point = skill_monster_pro_cal(monster_pro_weak,MY_EQUIP[0][6]);
                        AD = ((live_atk + ((MY_EQUIP[0][2] * (MY_EQUIP[0][4] * 0.01))* (MY_EQUIP[0][5] * 0.01))) * (ALLTK_PER*0.01)) * cri_success;

                        printf("%d",AD);
                        monster_max_hp = monster_max_hp - (AD - monster_max_ac);
                        // printf("이름:%s 공격력:%d 방어력:%d HP:%d\n",monster_korea_name[mon_name_num],monster_max_atk,monster_max_ac,monster_max_hp);
                        // printf("1\n");
                        break;
                    }
                    case 2:                  //마법공격 선택
                    {
                        printf("=============================================\n");
                        printf("\t\t[ 마법 공격 ]\n");
                        printf("=============================================\n");
                        printf("    스킬이름    마법배율    소모MP    속성\n");
                        printf("---------------------------------------------\n");
                        // 스킬 고르는거 추가
                        for (int i = 0 ; i < len_skill ; i++)
                        {
                            if (live_lv >= skill_level[i])
                            {
                                user_skill_num += 1;
                                // 내 레벨에 사용할 수 있는 스킬 따로 저장
                                strcpy(user_skill_name[i], skill_name[i]);
                                user_skill_magic_m[i] = skill_magic_m[i];
                                user_skill_mp[i] = skill_mp[i];
                                user_skill_properties[i] = skill_properties[i];
                                printf("%2d. %s\t %.1f\t      %d\t     %s\n", i+1, skill_name[i], skill_magic_m[i], skill_mp[i], properties_list[skill_properties[i]]);
                            }
                        }
                        printf("사용할 스킬을 선택하세요. : ");
                        scanf("%d", &user_skill_select);
                        // printf("이름:%s 공격력:%d 방어력:%d HP:%d\n",monster_korea_name[mon_name_num],monster_max_atk,monster_max_ac,monster_max_hp);
                        float weakness_point;
                        weakness_point = skill_monster_pro_cal(monster_pro_weak,user_skill_properties);
                        int AP;
                        if (mtk_m =1)                      //마법배율 장비 특수옵션 있을 때 계산식
                        {
                            AP = ((( live_mtk * (skill_magic_m[user_skill_select-1]+5)) * (ALLTK_PER * 0.01))* weakness_point) ;
                        }
                        else                               //마법배율 장비 특수옵션 없을 때 계산식
                        {
                            AP = ((( live_mtk * skill_magic_m[user_skill_select-1]) * (ALLTK_PER * 0.01))* weakness_point) ;
                        }
                        printf("%d\n",AP);
                        
                        monster_max_hp = monster_max_hp - (AP - monster_max_ac) ;
                        live_mp = live_mp - user_skill_mp[i];
                        // printf("2\n");
                      
                        if (live_mp < user_skill_mp[user_skill_select-1])
                        {
                            printf("마나가 부족합니다.\n");
                            break;
                        }
                        turn_switch = 1;
                        break;

                    }
                    case 3: // 아이템 사용 - 소비아이템 공간 보기
                    {
                        
                        printf("=============================================\n");
                        printf("\t\t[ 소비 아이템 ]\n");
                        printf("=============================================\n");
                        printf("%2d. 이전 메뉴\n", 0);
                        for (int i = 0 ; i < 12 ; i++)
                        {
                            if (i < 10)
                            {
                                printf("%2d. %s\t\t|%d개\n", i+1, cons_item_name[i], cons_space[i]);
                            }
                            else
                            {
                                printf("%2d. %s\t|%d개\n", i+1, cons_item_name[i], cons_space[i]);
                            }
                        }
                        printf("=============================================\n");
                        
                        int user_war_select_item;
                        
                        printf("사용할 아이템을 선택하세요. : ");
                        scanf("%d", &user_war_select_item);
                        system("clear");
                        if (user_war_select_item == 0) // 0번 입력하면 이전 메뉴
                        {
                            continue;
                        }
        
                        else if (cons_space[user_war_select_item-1] == 0) // 사용하려는 소비 아이템을 가지고 있지 않은 경우 사용불가
                        {
                            printf("해당 아이템이 부족합니다.\n");
                            break;
                        }
                        if ((user_war_select_item >= 1) && (user_war_select_item <= 3)) // 1~3번 아이템 : 체력 회복
                        {
                            printf("현재 HP : %d\n", live_hp);
                            cons_space[user_war_select_item-1] -= 1;
                            live_hp += (max_hp * (cons_item_recov[user_war_select_item-1]*0.01));
                            if (live_hp >= max_hp)
                            {
                                live_hp = max_hp;
                            }
                            printf("회복 HP : %d\n", live_hp);
                            turn_switch =1;
                        }
                        else if ((user_war_select_item >= 4) && (user_war_select_item <= 6)) // 4~6번 아이템 : 마나 회복
                        {
                            printf("현재 MP : %d\n", live_mp);
                            cons_space[user_war_select_item-1] -= 1;
                            live_mp += (max_mp * (cons_item_recov[user_war_select_item-1]*0.01));
                            if (live_mp >= max_mp)
                            {
                                live_mp = max_mp;
                            }
                            printf("회복 HP : %d\n", live_mp);
                            turn_switch =1;
                        }
                        else if (user_war_select_item == 7) // 엘릭서
                        {
                            cons_space[user_war_select_item-1] -= 1;
                            live_hp = max_hp;
                            live_mp = max_mp;
                            turn_switch =1;
                        }
                        else if (user_war_select_item == 8) // 해독제
                        {
                            printf("해독제 사용, 중독상태 회복\n");
                            turn_switch =1;
                        }
                        else if (user_war_select_item == 9) // 축복 주문서
                        {
                            printf("축복 주문서 사용, 저주상태 회복\n");
                            turn_switch =1;
                        }
                        else if (user_war_select_item == 10) // 만병통치약
                        {
                            printf("만병통치약 사용, 모든 상태이상 회복\n");
                            turn_switch =1;
                        }
                        else if (user_war_select_item == 11) // 마을이동주문서
                        {
                            printf("마을이동주문서 사용, 마을로 이동\n");
                            floor = 0; // 0층(마을)로 이동
                            user_x = 0; // 마을에서 초기 좌표로 이동
                            user_y = 49;
                            cons_space[user_war_select_item-1] -= 1;
                            turn_switch =1;
                            war_switch = 0;
                            map_change();
                            break;
                        }
                        else if (user_war_select_item == 12) // 순간이동 주문서
                        {
                            system("clear");
                            printf("=============================================\n");
                            printf("\t    [ 저장된 위치 목록 ]\n");
                            printf("=============================================\n");
                            printf("      층      |     x좌표     |     y좌표\n");
                            printf("---------------------------------------------\n");
                            for (int i = 0 ; i < 5 ; i++) // 저장된 좌표 출력
                            {  
                                printf("%d.    %d              %d              %d\n", i+1, save_point[i][0], save_point[i][2], save_point[i][1]);
                            }
                            printf("%d. 초기 메뉴\n", 0);
                            scanf("%d", &user_position_select);
                            if (user_position_select == 0) // 0 입력하면 이전 메뉴
                            {
                                system("clear");
                                continue;
                            }
                            else // 선택한 층, y, x좌표로 이동
                            {
                                floor = save_point[user_position_select-1][0];
                                user_y = save_point[user_position_select-1][1];
                                user_x = save_point[user_position_select-1][2];
                                map_change();
                            }
                            cons_space[user_war_select_item-1] -= 1;
                            turn_switch =1;
                            break; int user_war_select_item;
                        
                            printf("사용할 아이템을 선택하세요. : ");
                            scanf("%d", &user_war_select_item);
                            system("clear");
                            if (user_war_select_item == 0) // 0번 입력하면 이전 메뉴
                            {
                                continue;
                            }
            
                            else if (cons_space[user_war_select_item-1] == 0) // 사용하려는 소비 아이템을 가지고 있지 않은 경우 사용불가
                            {
                                printf("해당 아이템이 부족합니다.\n");
                                break;
                            }
                            if ((user_war_select_item >= 1) && (user_war_select_item <= 3)) // 1~3번 아이템 : 체력 회복
                            {
                                printf("현재 HP : %d\n", live_hp);
                                cons_space[user_war_select_item-1] -= 1;
                                live_hp += (max_hp * (cons_item_recov[user_war_select_item-1]*0.01));
                                if (live_hp >= max_hp)
                                {
                                    live_hp = max_hp;
                                }
                                printf("회복 HP : %d\n", live_hp);
                                turn_switch =1;
                            }
                            else if ((user_war_select_item >= 4) && (user_war_select_item <= 6)) // 4~6번 아이템 : 마나 회복
                            {
                                printf("현재 MP : %d\n", live_mp);
                                cons_space[user_war_select_item-1] -= 1;
                                live_mp += (max_mp * (cons_item_recov[user_war_select_item-1]*0.01));
                                if (live_mp >= max_mp)
                                {
                                    live_mp = max_mp;
                                }
                                printf("회복 HP : %d\n", live_mp);
                                turn_switch =1;
                            }
                            else if (user_war_select_item == 7) // 엘릭서
                            {
                                cons_space[user_war_select_item-1] -= 1;
                                live_hp = max_hp;
                                live_mp = max_mp;
                                turn_switch =1;
                            }
                            else if (user_war_select_item == 8) // 해독제
                            {
                                printf("해독제 사용, 중독상태 회복\n");
                                turn_switch =1;
                            }
                            else if (user_war_select_item == 9) // 축복 주문서
                            {
                                printf("축복 주문서 사용, 저주상태 회복\n");
                                turn_switch =1;
                            }
                            else if (user_war_select_item == 10) // 만병통치약
                            {
                                printf("만병통치약 사용, 모든 상태이상 회복\n");
                                turn_switch =1;
                            }
                            else if (user_war_select_item == 11) // 마을이동주문서
                            {
                                printf("마을이동주문서 사용, 마을로 이동\n");
                                floor = 0; // 0층(마을)로 이동
                                user_x = 0; // 마을에서 초기 좌표로 이동
                                user_y = 49;
                                cons_space[user_war_select_item-1] -= 1;
                                turn_switch =1;
                                war_switch = 0;
                                map_change();
                                break;
                            }
                            else if (user_war_select_item == 12) // 순간이동 주문서
                            {
                                system("clear");
                                printf("=============================================\n");
                                printf("\t    [ 저장된 위치 목록 ]\n");
                                printf("=============================================\n");
                                printf("      층      |     x좌표     |     y좌표\n");
                                printf("---------------------------------------------\n");
                                for (int i = 0 ; i < 5 ; i++) // 저장된 좌표 출력
                                {  
                                    printf("%d.    %d              %d              %d\n", i+1, save_point[i][0], save_point[i][2], save_point[i][1]);
                                }
                                printf("%d. 초기 메뉴\n", 0);
                                scanf("%d", &user_position_select);
                                if (user_position_select == 0) // 0 입력하면 이전 메뉴
                                {
                                    system("clear");
                                    continue;
                                }
                                else // 선택한 층, y, x좌표로 이동
                                {
                                    floor = save_point[user_position_select-1][0];
                                    user_y = save_point[user_position_select-1][1];
                                    user_x = save_point[user_position_select-1][2];
                                    tp_switch = 1;
                                    map_change();
                                }
                                cons_space[user_war_select_item-1] -= 1;
                                turn_switch =1;
                                break;
                            }
                        }
                        
                        break;
                    }
                    case 4:                 //도망가기 선택
                    {
                        printf("도망가기 선택\n");
                        map_not_change();
                        turn_switch =1;
                        war_switch = 0;
                        run_switch = 1;
                        break;
                    }        
                }
                system("clear");
                if (monster_max_hp <= 0)
                {

                    if (floor ==5)                   //5층에서 몹을 잡으면 보스리젠 스위치 켜짐
                    {
                        plag_boss_regen=1;
                    }
                    
                    if (mon_name_num==7)
                    {
                        return 0;
                    }
                    war_switch = 0;
                    // turn_switch = 1;
                    // 골드 보상

                    ////////////////////보스면 조건 걸고 else if로 나머지것들 묶어주기

                    drop_gold = (monster_fix_gold + (rand() % monster_random_gold[mon_name_num]));//100
                    gold = gold + drop_gold;                                            //골드 증가
                    max_hp = max_hp + LV_UP_HP;max_mp = max_mp + LV_UP_MP;              //만피 증가
                    max_exp = max_exp + lv_up_exp;                                      //경험치 한도값 증가
                    return_stp += 1;                                                    //스탯1포 증가

        
                    // 마을이동주문서 보상
                    if (((rand() % 100) + 1) <= monster_drop_item_tp[mon_name_num])
                    {
                        cons_space[10] += 1;
                        if (cons_space[10] == 99) // 99개 가지고 있는 경우
                        {
                            cons_space[10] -= 1; // 1개 먹은 보상 다시 마이너스
                        }   
                    }
        
                    // 순간이동주문서 보상
                    if (((rand() % 100) + 1) <= monster_drop_item_vil[mon_name_num])
                    {
                        cons_space[11] += 1;
                        if (cons_space[11] == 99) // 99개 가지고 있는 경우
                        {
                            cons_space[11] -= 1; // 1개 먹은 보상 다시 마이너스
                        }   
                    }
        
        
                    // 1티어 장비 아이템 보상
                    if (((rand() % 100) + 1) <= monster_equip_item_1tier[mon_name_num]) 
                    {
                        // 1티어 장비를 뽑는 코드 (1티어 장비 6개)
                        for (int i = 0 ; i < 6 ; i++)
                        {
                            for (int j = 0 ; j < 8 ; j++)
                            {
                                if (equipment_tier[i][j] == 1)
                                {
                                    equip_tier1[i][0] = i;
                                    equip_tier1[i][1] = j;
                                }
                            }
                        }
                        reward_num = (rand() % 6) + 2; // ===> 2~7
                        printf("1티어 장비 %s를 얻었습니다.\n", equipment_type[reward_num][0]);
        
                        // 인벤토리가 가득 찬 경우 처리하는 코드
                        count = 0;
                        for (int i = 0 ; i < 20 ;i++)
                        {
                            if (INVEN[i][0] != 0)
                            {
                                count ++;
                            }
                        }
                        if (count == 20) // 인벤토리가 가득 차있는 경우
                        {
                            printf("인벤토리가 가득 찼습니다.\n");
                            printf("[1. 기존 아이템 버리기 | 2. 보상 아이템 버리기] : ");
                            scanf("%d", &user_select_toss1);
                            switch (user_select_toss1)
                            {
                                case 1: // 기존 아이템 버리기
                                {
                                    printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t| 7.특수옵션\n");
                                    for (int i = 0 ; i < 20 ; i++)
                                    {
                                        if (INVEN[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                                        {
                                            printf("%2d. %3d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                                        }
                                        else
                                        {
                                            printf("%2d. %3s\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\n", i+1, equipment_type[(INVEN[i][0])-1][(INVEN[i][1])-1], INVEN[i][2], INVEN[i][3], INVEN[i][4], INVEN[i][5], INVEN[i][6], INVEN[i][7], INVEN[i][8]);
                                        }
                                    }
                                    printf("\n어떤 아이템을 버리실 건가요?\n");
                                    scanf("%d", &user_select_toss2);
                                    INVEN[user_select_toss2-1][0] = 0;
                                    INVEN[user_select_toss2-1][1] = 0;
                                    INVEN[user_select_toss2-1][2] = 0;
                                    INVEN[user_select_toss2-1][3] = 0;
                                    INVEN[user_select_toss2-1][4] = 0;
                                    INVEN[user_select_toss2-1][5] = 0;
                                    INVEN[user_select_toss2-1][6] = 0;
                                    INVEN[user_select_toss2-1][7] = 0;
                                    INVEN[user_select_toss2-1][8] = 0;
        
                                    // 얻은 장비 인벤토리에 넣음
                                    for (int i = 0 ; i < 20 ; i++)
                                    {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                                        if (INVEN[i][0] == 0)
                                        {
                                            INVEN[i][0] = reward_num-1; // 큰 종류
                                            INVEN[i][1] = 0; // 작은 종류
                                            INVEN[i][2] = 1; // 티어
                                            INVEN[i][3] = equipment_basic_option[reward_num-2][0]; // 기본옵션(기본공격력/데미지감소)
                                            INVEN[i][4] = 0; // 강화레벨
                                            INVEN[i][5] = 0; // 강화옵션(%)
                                            INVEN[i][6] = 0; // 인챈트(%)
                                            INVEN[i][7] = 0; // 인챈트 속성
                                            INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                            break;
                                        }
                                    }
        
                                }
                                case 2: // 보상 아이템 버리기
                                {
                                    printf("보상 아이템을 버립니다.\n");
                                }
                            }
                        }
                        else
                        {
                            // 얻은 장비 인벤토리에 넣음
                            for (int i = 0 ; i < 20 ; i++)
                            {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                                if (INVEN[i][0] == 0)
                                {
                                    INVEN[i][0] = reward_num-1; // 큰 종류
                                    INVEN[i][1] = 0; // 작은 종류
                                    INVEN[i][2] = 1; // 티어
                                    INVEN[i][3] = equipment_basic_option[reward_num-2][0]; // 기본옵션(기본공격력/데미지감소)
                                    INVEN[i][4] = 0; // 강화레벨
                                    INVEN[i][5] = 0; // 강화옵션(%)
                                    INVEN[i][6] = 0; // 인챈트(%)
                                    INVEN[i][7] = 0; // 인챈트 속성
                                    INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                    break;
                                }
                            }
                        }
                    }
    
        
                    // 2티어 장비 아이템 보상
                    if (((rand() % 100) + 1) <= monster_equip_item_2tier[mon_name_num])
                    {
                        for (int i = 0 ; i < 6 ; i++)
                        {
                            for (int j = 0 ; j < 8 ; j++)
                            {
                                if (equipment_tier[i][j] == 2)
                                {
                                    equip_tier2[i][0] = i;
                                    equip_tier2[i][1] = j;
                                }
                            }
                        }
                        reward_num = (rand() % 6) + 2; // ===> 2~7
                        printf("2티어 장비 %s를 얻었습니다.\n", equipment_type[reward_num-2][1]);
                        // 인벤토리가 가득 찬 경우 처리하는 코드
                        count = 0;
                        for (int i = 0 ; i < 20 ;i++)
                        {
                            if (INVEN[i][0] != 0)
                            {
                                count ++;
                            }
                        }
                        if (count == 20) // 인벤토리가 가득 차있는 경우
                        {
                            printf("인벤토리가 가득 찼습니다.\n");
                            printf("[1. 기존 아이템 버리기 | 2. 보상 아이템 버리기] : ");
                            scanf("%d", &user_select_toss1);
                            switch (user_select_toss1)
                            {
                                case 1: // 기존 아이템 버리기
                                {
                                    printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t| 7.특수옵션\n");
                                    for (int i = 0 ; i < 20 ; i++)
                                    {
                                        if (INVEN[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                                        {
                                            printf("%2d. %3d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                                        }
                                        else
                                        {
                                            printf("%2d. %3s\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\n", i+1, equipment_type[(INVEN[i][0])-1][(INVEN[i][1])-1], INVEN[i][2], INVEN[i][3], INVEN[i][4], INVEN[i][5], INVEN[i][6], INVEN[i][7], INVEN[i][8]);
                                        }
                                    }
                                    printf("\n어떤 아이템을 버리실 건가요?\n");
                                    scanf("%d", &user_select_toss2);
                                    INVEN[user_select_toss2-1][0] = 0;
                                    INVEN[user_select_toss2-1][1] = 0;
                                    INVEN[user_select_toss2-1][2] = 0;
                                    INVEN[user_select_toss2-1][3] = 0;
                                    INVEN[user_select_toss2-1][4] = 0;
                                    INVEN[user_select_toss2-1][5] = 0;
                                    INVEN[user_select_toss2-1][6] = 0;
                                    INVEN[user_select_toss2-1][7] = 0;
                                    INVEN[user_select_toss2-1][8] = 0;
        
                                    // 얻은 장비 인벤토리에 넣음
                                    for (int i = 0 ; i < 20 ; i++)
                                    {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                                        if (INVEN[i][0] == 0)
                                        {
                                            INVEN[i][0] = reward_num-1; // 큰 종류
                                            INVEN[i][1] = 1; // 작은 종류
                                            INVEN[i][2] = 2; // 티어
                                            INVEN[i][3] = equipment_basic_option[reward_num-2][1]; // 기본옵션(기본공격력/데미지감소)
                                            INVEN[i][4] = 0; // 강화레벨
                                            INVEN[i][5] = 0; // 강화옵션(%)
                                            INVEN[i][6] = 0; // 인챈트(%)
                                            INVEN[i][7] = 0; // 인챈트 속성
                                            INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                            break;
                                        }
                                    }
        
                                }
                                case 2: // 보상 아이템 버리기
                                {
                                    printf("보상 아이템을 버립니다.\n");
                                }
                            }
                        }
                        else
                        {
                            // 얻은 장비 인벤토리에 넣음
                            for (int i = 0 ; i < 20 ; i++)
                            {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                                if (INVEN[i][0] == 0)
                                {
                                    INVEN[i][0] = reward_num-1; // 큰 종류
                                    INVEN[i][1] = 1; // 작은 종류
                                    INVEN[i][2] = 2; // 티어
                                    INVEN[i][3] = equipment_basic_option[reward_num-2][1]; // 기본옵션(기본공격력/데미지감소)
                                    INVEN[i][4] = 0; // 강화레벨
                                    INVEN[i][5] = 0; // 강화옵션(%)
                                    INVEN[i][6] = 0; // 인챈트(%)
                                    INVEN[i][7] = 0; // 인챈트 속성
                                    INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                    break;
                                }
                            }
                        }
                    }
        
        
                    // 3티어 장비 아이템 보상 (4번째 몬스터가 아닌 경우 => 3티어만)
                    if (mon_name_num != 3 && (((rand() % 100) + 1) <= monster_equip_item_3tier[mon_name_num]))
                    {
                        for (int i = 0 ; i < 6 ; i++)
                        {
                            for (int j = 0 ; j < 8 ; j++)
                            {
                                if (equipment_tier[i][j] == 2)
                                {
                                    equip_tier3[i][0] = i;
                                    equip_tier3[i][1] = j;
                                }
                            }
                        }
                        reward_num = (rand() % 6) + 2; // ===> 2~7
                        printf("2티어 장비 %s를 얻었습니다.\n", equipment_type[reward_num-2][2]);
                        // 인벤토리가 가득 찬 경우 처리하는 코드
                        count = 0;
                        for (int i = 0 ; i < 20 ;i++)
                        {
                            if (INVEN[i][0] != 0)
                            {
                                count ++;
                            }
                        }
                        if (count == 20) // 인벤토리가 가득 차있는 경우
                        {
                            printf("인벤토리가 가득 찼습니다.\n");
                            printf("[1. 기존 아이템 버리기 | 2. 보상 아이템 버리기] : ");
                            scanf("%d", &user_select_toss1);
                            switch (user_select_toss1)
                            {
                                case 1: // 기존 아이템 버리기
                                {
                                    printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t| 7.특수옵션\n");
                                    for (int i = 0 ; i < 20 ; i++)
                                    {
                                        if (INVEN[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                                        {
                                            printf("%2d. %3d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                                        }
                                        else
                                        {
                                            printf("%2d. %3s\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\n", i+1, equipment_type[(INVEN[i][0])-1][(INVEN[i][1])-1], INVEN[i][2], INVEN[i][3], INVEN[i][4], INVEN[i][5], INVEN[i][6], INVEN[i][7], INVEN[i][8]);
                                        }
                                    }
                                    printf("\n어떤 아이템을 버리실 건가요?\n");
                                    scanf("%d", &user_select_toss2);
                                    INVEN[user_select_toss2-1][0] = 0;
                                    INVEN[user_select_toss2-1][1] = 0;
                                    INVEN[user_select_toss2-1][2] = 0;
                                    INVEN[user_select_toss2-1][3] = 0;
                                    INVEN[user_select_toss2-1][4] = 0;
                                    INVEN[user_select_toss2-1][5] = 0;
                                    INVEN[user_select_toss2-1][6] = 0;
                                    INVEN[user_select_toss2-1][7] = 0;
                                    INVEN[user_select_toss2-1][8] = 0;
        
                                    // 얻은 장비 인벤토리에 넣음
                                    for (int i = 0 ; i < 20 ; i++)
                                    {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                                        if (INVEN[i][0] == 0)
                                        {
                                            INVEN[i][0] = reward_num-1; // 큰 종류
                                            INVEN[i][1] = 2; // 작은 종류
                                            INVEN[i][2] = 3; // 티어
                                            INVEN[i][3] = equipment_basic_option[reward_num-2][2]; // 기본옵션(기본공격력/데미지감소)
                                            INVEN[i][4] = 0; // 강화레벨
                                            INVEN[i][5] = 0; // 강화옵션(%)
                                            INVEN[i][6] = 0; // 인챈트(%)
                                            INVEN[i][7] = 0; // 인챈트 속성
                                            INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                            break;
                                        }
                                    }
        
                                }
                                case 2: // 보상 아이템 버리기
                                {
                                    printf("보상 아이템을 버립니다.\n");
                                }
                            }
                        }
                        else
                        {
                            // 얻은 장비 인벤토리에 넣음
                            for (int i = 0 ; i < 20 ; i++)
                            {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                                if (INVEN[i][0] == 0)
                                {
                                    INVEN[i][0] = reward_num-1; // 큰 종류
                                    INVEN[i][1] = 2; // 작은 종류
                                    INVEN[i][2] = 3; // 티어
                                    INVEN[i][3] = equipment_basic_option[reward_num-2][2]; // 기본옵션(기본공격력/데미지감소)
                                    INVEN[i][4] = 0; // 강화레벨
                                    INVEN[i][5] = 0; // 강화옵션(%)
                                    INVEN[i][6] = 0; // 인챈트(%)
                                    INVEN[i][7] = 0; // 인챈트 속성
                                    INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                    break;
                                }
                            }
                        }
                    }
        
        
                    // 3~4티어 장비 아이템 보상 (4번째 몬스터인 경우)
                    if (mon_name_num == 3 && (((rand() % 100) + 1) <= monster_equip_item_3tier[mon_name_num]))
                    {
                        for (int i = 0 ; i < 6 ; i++)
                        {
                            for (int j = 0 ; j < 8 ; j++)
                            {
                                if (equipment_tier[i][j] == 3)
                                {
                                    equip_tier3_4[i][0] = i;
                                    equip_tier3_4[i][1] = j;
                                }
                                else if (equipment_tier[i][j] == 4)
                                {
                                    equip_tier3_4[i+6][0] = i;
                                    equip_tier3_4[i+6][1] = j;
                                }
                            }
                        }
                        reward_num = (rand() % 12) + 2; // ===> 2~13
                        printf("%d티어 장비 %s를 얻었습니다.\n", equipment_tier[equip_tier3_4[reward_num-2][0]][equip_tier3_4[reward_num-2][1]], equipment_type[equip_tier3_4[reward_num-2][0]][equip_tier3_4[reward_num-2][1]]);
                        // 인벤토리가 가득 찬 경우 처리하는 코드
                        count = 0;
                        for (int i = 0 ; i < 20 ;i++)
                        {
                            if (INVEN[i][0] != 0)
                            {
                                count ++;
                            }
                        }
                        if (count == 20) // 인벤토리가 가득 차있는 경우
                        {
                            printf("인벤토리가 가득 찼습니다.\n");
                            printf("[1. 기존 아이템 버리기 | 2. 보상 아이템 버리기] : ");
                            scanf("%d", &user_select_toss1);
                            switch (user_select_toss1)
                            {
                                case 1: // 기존 아이템 버리기
                                {
                                    printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t| 7.특수옵션\n");
                                    for (int i = 0 ; i < 20 ; i++)
                                    {
                                        if (INVEN[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                                        {
                                            printf("%2d. %3d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                                        }
                                        else
                                        {
                                            printf("%2d. %3s\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\n", i+1, equipment_type[(INVEN[i][0])-1][(INVEN[i][1])-1], INVEN[i][2], INVEN[i][3], INVEN[i][4], INVEN[i][5], INVEN[i][6], INVEN[i][7], INVEN[i][8]);
                                        }
                                    }
                                    printf("\n어떤 아이템을 버리실 건가요?\n");
                                    scanf("%d", &user_select_toss2);
                                    INVEN[user_select_toss2-1][0] = 0;
                                    INVEN[user_select_toss2-1][1] = 0;
                                    INVEN[user_select_toss2-1][2] = 0;
                                    INVEN[user_select_toss2-1][3] = 0;
                                    INVEN[user_select_toss2-1][4] = 0;
                                    INVEN[user_select_toss2-1][5] = 0;
                                    INVEN[user_select_toss2-1][6] = 0;
                                    INVEN[user_select_toss2-1][7] = 0;
                                    INVEN[user_select_toss2-1][8] = 0;
                                    // 얻은 장비 인벤토리에 넣음
                                    for (int i = 0 ; i < 20 ; i++)
                                    {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                                        if (INVEN[i][0] == 0)
                                        {
                                            if (reward_num > 7)
                                            {
                                                INVEN[i][0] = reward_num-7; // 큰 종류
                                                INVEN[i][1] = equip_tier3_4[reward_num-2][1]; // 작은 종류
                                                INVEN[i][2] = equipment_tier[equip_tier3_4[reward_num-2][0]][equip_tier3_4[reward_num-2][1]]; // 티어
                                                INVEN[i][3] = equipment_basic_option[equip_tier3_4[reward_num-2][0]][equip_tier3_4[reward_num-2][1]]; // 기본옵션(기본공격력/데미지감소)
                                                INVEN[i][4] = 0; // 강화레벨
                                                INVEN[i][5] = 0; // 강화옵션(%)
                                                INVEN[i][6] = 0; // 인챈트(%)
                                                INVEN[i][7] = 0; // 인챈트 속성
                                                INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                                break;
                                            }
                                            else
                                            {
                                                INVEN[i][0] = reward_num-1; // 큰 종류
                                                INVEN[i][1] = equip_tier3_4[reward_num-2][1]; // 작은 종류
                                                INVEN[i][2] = equipment_tier[equip_tier3_4[reward_num-2][0]][equip_tier3_4[reward_num-2][1]]; // 티어
                                                INVEN[i][3] = equipment_basic_option[equip_tier3_4[reward_num-2][0]][equip_tier3_4[reward_num-2][1]]; // 기본옵션(기본공격력/데미지감소)
                                                INVEN[i][4] = 0; // 강화레벨
                                                INVEN[i][5] = 0; // 강화옵션(%)
                                                INVEN[i][6] = 0; // 인챈트(%)
                                                INVEN[i][7] = 0; // 인챈트 속성
                                                INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                                break;
                                            }
                                        }
                                    }
        
                                }
                                case 2: // 보상 아이템 버리기
                                {
                                    printf("보상 아이템을 버립니다.\n");
                                }
                            }
                        }
                        else
                        {
                            // 얻은 장비 인벤토리에 넣음
                            for (int i = 0 ; i < 20 ; i++)
                            {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                                if (INVEN[i][0] == 0)
                                {
                                    if (reward_num > 7)
                                    {
                                        INVEN[i][0] = reward_num-7; // 큰 종류
                                        INVEN[i][1] = equip_tier3_4[reward_num-2][1]; // 작은 종류
                                        INVEN[i][2] = equipment_tier[equip_tier3_4[reward_num-2][0]][equip_tier3_4[reward_num-2][1]]; // 티어
                                        INVEN[i][3] = equipment_basic_option[equip_tier3_4[reward_num-2][0]][equip_tier3_4[reward_num-2][1]]; // 기본옵션(기본공격력/데미지감소)
                                        INVEN[i][4] = 0; // 강화레벨
                                        INVEN[i][5] = 0; // 강화옵션(%)
                                        INVEN[i][6] = 0; // 인챈트(%)
                                        INVEN[i][7] = 0; // 인챈트 속성
                                        INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                        break;
                                    }
                                    else
                                    {
                                        INVEN[i][0] = reward_num-1; // 큰 종류
                                        INVEN[i][1] = equip_tier3_4[reward_num-2][1]; // 작은 종류
                                        INVEN[i][2] = equipment_tier[equip_tier3_4[reward_num-2][0]][equip_tier3_4[reward_num-2][1]]; // 티어
                                        INVEN[i][3] = equipment_basic_option[equip_tier3_4[reward_num-2][0]][equip_tier3_4[reward_num-2][1]]; // 기본옵션(기본공격력/데미지감소)
                                        INVEN[i][4] = 0; // 강화레벨
                                        INVEN[i][5] = 0; // 강화옵션(%)
                                        INVEN[i][6] = 0; // 인챈트(%)
                                        INVEN[i][7] = 0; // 인챈트 속성
                                        INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                        break;
                                    }
                                }
                            }
                        }
        
        
        
        
        
        
        
        
        
        
        
        
                    }
        
        
                    // 4티어 장비 아이템 보상 (4번째 몬스터가 아닌 경우)
                    if (mon_name_num !=3 && (((rand() % 100) + 1) <= monster_equip_item_4tier[mon_name_num]))
                    {
                        for (int i = 0 ; i < 6 ; i++)
                        {
                            for (int j = 0 ; j < 8 ; j++)
                            {
                                if (equipment_tier[i][j] == 4)
                                {
                                    equip_tier4[i][0] = i;
                                    equip_tier4[i][1] = j;
                                }
                            }
                        }
                        reward_num = (rand() % 6) + 2; // ===> 2~7
                        printf("4티어 장비 %s를 얻었습니다.\n", equipment_type[equip_tier4[reward_num-2][0]][equip_tier4[reward_num-2][1]]);
                        // 인벤토리가 가득 찬 경우 처리하는 코드
                        count = 0;
                        for (int i = 0 ; i < 20 ;i++)
                        {
                            if (INVEN[i][0] != 0)
                            {
                                count ++;
                            }
                        }
                        if (count == 20) // 인벤토리가 가득 차있는 경우
                        {
                            printf("인벤토리가 가득 찼습니다.\n");
                            printf("[1. 기존 아이템 버리기 | 2. 보상 아이템 버리기] : ");
                            scanf("%d", &user_select_toss1);
                            switch (user_select_toss1)
                            {
                                case 1: // 기존 아이템 버리기
                                {
                                    printf(" 0.이름 \t| 1.티어 \t| 2.기본옵션 \t| 3.강화레벨 \t| 4.강화옵션 \t| 5.인챈트(%%) \t| 6.속성 \t| 7.특수옵션\n");
                                    for (int i = 0 ; i < 20 ; i++)
                                    {
                                        if (INVEN[i][0] == 0) // 비어있는 인벤토리 칸에는 0을 넣어주세요
                                        {
                                            printf("%2d. %3d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\n", i+1, 0, 0, 0, 0, 0, 0, 0, 0);
                                        }
                                        else
                                        {
                                            printf("%2d. %3s\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\t|%d\t\n", i+1, equipment_type[(INVEN[i][0])-1][(INVEN[i][1])-1], INVEN[i][2], INVEN[i][3], INVEN[i][4], INVEN[i][5], INVEN[i][6], INVEN[i][7], INVEN[i][8]);
                                        }
                                    }
                                    printf("\n어떤 아이템을 버리실 건가요?\n");
                                    scanf("%d", &user_select_toss2);
                                    INVEN[user_select_toss2-1][0] = 0;
                                    INVEN[user_select_toss2-1][1] = 0;
                                    INVEN[user_select_toss2-1][2] = 0;
                                    INVEN[user_select_toss2-1][3] = 0;
                                    INVEN[user_select_toss2-1][4] = 0;
                                    INVEN[user_select_toss2-1][5] = 0;
                                    INVEN[user_select_toss2-1][6] = 0;
                                    INVEN[user_select_toss2-1][7] = 0;
                                    INVEN[user_select_toss2-1][8] = 0;
        
                                    // 얻은 장비 인벤토리에 넣음
                                    for (int i = 0 ; i < 20 ; i++)
                                    {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                                        if (INVEN[i][0] == 0)
                                        {
                                            INVEN[i][0] = reward_num-1; // 큰 종류
                                            INVEN[i][1] = equip_tier4[reward_num-2][1]; // 작은 종류
                                            INVEN[i][2] = 4; // 티어
                                            INVEN[i][3] = equipment_basic_option[equip_tier4[reward_num-2][0]][equip_tier4[reward_num-2][1]];; // 기본옵션(기본공격력/데미지감소)
                                            INVEN[i][4] = 0; // 강화레벨
                                            INVEN[i][5] = 0; // 강화옵션(%)
                                            INVEN[i][6] = 0; // 인챈트(%)
                                            INVEN[i][7] = 0; // 인챈트 속성
                                            INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                            break;
                                        }
                                    }
                                }
                                case 2: // 보상 아이템 버리기
                                {
                                    printf("보상 아이템을 버립니다.\n");
                                }
                            }
                        }
                        else
                        {
                            // 얻은 장비 인벤토리에 넣음
                            for (int i = 0 ; i < 20 ; i++)
                            {// 장비 카테고리 , 종류 카테고리, 티어, 기본옵션(기본공격력/데미지감소), 강화레벨, 강화옵션(%), 인챈트(%), 인챈트 속성, 특수옵션(나중에 구현)
                                if (INVEN[i][0] == 0)
                                {
                                    INVEN[i][0] = reward_num-1; // 큰 종류
                                    INVEN[i][1] = equip_tier4[reward_num-2][1]; // 작은 종류
                                    INVEN[i][2] = 4; // 티어
                                    INVEN[i][3] = equipment_basic_option[equip_tier4[reward_num-2][0]][equip_tier4[reward_num-2][1]];; // 기본옵션(기본공격력/데미지감소)
                                    INVEN[i][4] = 0; // 강화레벨
                                    INVEN[i][5] = 0; // 강화옵션(%)
                                    INVEN[i][6] = 0; // 인챈트(%)
                                    INVEN[i][7] = 0; // 인챈트 속성
                                    INVEN[i][8] = 0; // 특수옵션(나중에 구현)
                                    break;
                                }
                            }
                        }
                    }
        
        
                    // 1~2티어 소비아이템 보상 (1번째 몬스터의 경우)
                    if (mon_name_num == 0 && (((rand() % 100) + 1) <= monster_cons_item_1tier[mon_name_num]))
                    {
                        for (int i = 0 ; i < 7 ; i++)
                        {
                            for (int j = 0 ; j < 2 ; j++)
                            {
                                printf("%d ", cons_tier1_2[i][j]);
                                
                            }
                            printf("\n");
                        }
                        reward_num = (rand() % 7); // ===> 0~6
                        printf("%d티어 소비아이템 %s를 얻었습니다.\n",equipment_tier[6][cons_tier1_2[reward_num][1]] ,equipment_type[cons_tier1_2[reward_num][0]][cons_tier1_2[reward_num][1]]);
        
                        // 소비아이템 공간이 가득 찬 경우 처리하는 코드
                        if (cons_space[cons_tier1_2[reward_num][1]] == 99)
                        {
                            printf("공간이 가득 찼습니다.\n");
                        }
                        else
                        {
                            cons_space[cons_tier1_2[reward_num][1]] += 1;
                        }
                    }
        
        
                    // 2티어 소비아이템 보상 (2번째 몬스터의 경우)
                    if (mon_name_num == 1 && (((rand() % 100) + 1) <= monster_cons_item_2tier[mon_name_num]))
                    {
                        for (int i = 0 ; i < 4 ; i++)
                        {
                            for (int j = 0 ; j < 2 ; j++)
                            {
                                printf("%d ", cons_tier2[i][j]);
                                
                            }
                            printf("\n");
                        }
                        reward_num = (rand() % 4); // ===> 0~3
                        printf("%d\n", reward_num);
                        printf("%d티어 소비아이템 %s를 얻었습니다.\n", equipment_tier[6][cons_tier2[reward_num][1]] ,equipment_type[cons_tier2[reward_num][0]][cons_tier2[reward_num][1]]);
        
                        // 소비아이템 공간이 가득 찬 경우 처리하는 코드
                        if (cons_space[cons_tier2[reward_num][1]] == 99)
                        {
                            printf("공간이 가득 찼습니다.\n");
                        }
                        else
                        {
                            cons_space[cons_tier2[reward_num][1]] += 1;
                        }
                        // 소비아이템 공간 보기
                        printf("[ 소비 아이템 ]\n");
                        for (int i = 0 ; i < 12 ; i++)
                        {
                            printf("%d. %s\t|%d개\n", i+1, cons_item_name[i], cons_space[i]);
                        }
                    }
                

                    // 2~3티어 소비아이템 보상 (3번째 몬스터의 경우)
                    if (mon_name_num == 2 && (((rand() % 100) + 1) <= monster_cons_item_3tier[mon_name_num]))
                    {
                        reward_num = (rand() % 7); // ===> 0~6
                        printf("%d\n", reward_num);
                        printf("%d티어 소비아이템 %s를 얻었습니다.\n", equipment_tier[6][cons_tier2_3[reward_num][1]] ,equipment_type[cons_tier2_3[reward_num][0]][cons_tier2_3[reward_num][1]]);
                        // 소비아이템 공간이 가득 찬 경우 처리하는 코드
                        if (cons_space[cons_tier2_3[reward_num][1]] == 99)
                        {
                            printf("공간이 가득 찼습니다.\n");
                        }
                        else
                        {
                            cons_space[cons_tier2_3[reward_num][1]] += 1;
                        }
                        // 소비아이템 공간 보기
                        printf("[ 소비 아이템 ]\n");
                        for (int i = 0 ; i < 12 ; i++)
                        {
                            printf("%d. %s\t|%d개\n", i+1, cons_item_name[i], cons_space[i]);
                        }
                    }


                    // 3티어 소비아이템 보상 (4번째 몬스터의 경우)
                    if (mon_name_num == 3 && (((rand() % 100) + 1) <= monster_cons_item_3tier[mon_name_num]))
                    {
                        reward_num = (rand() % 3); // ===> 0~3
                        printf("%d\n", reward_num);
                        printf("%d티어 소비아이템 %s를 얻었습니다.\n", equipment_tier[6][cons_tier3[reward_num][1]] ,equipment_type[cons_tier3[reward_num][0]][cons_tier3[reward_num][1]]);

                        // 소비아이템 공간이 가득 찬 경우 처리하는 코드
                        if (cons_space[cons_tier3[reward_num][1]] == 99)
                        {
                            printf("공간이 가득 찼습니다.\n");
                        }
                        else
                        {
                            cons_space[cons_tier3[reward_num][1]] += 1;
                        }

                        // 소비아이템 공간 보기
                        printf("[ 소비 아이템 ]\n");
                        for (int i = 0 ; i < 12 ; i++)
                        {
                            printf("%d. %s\t|%d개\n", i+1, cons_item_name[i], cons_space[i]);
                        }
                    }


                    // 3~4티어 소비아이템 보상 (5~6번째 몬스터의 경우)
                    if ((mon_name_num >= 5 && mon_name_num <= 6) && (((rand() % 100) + 1) <= monster_cons_item_3tier[mon_name_num]))
                    {
                        reward_num = (rand() % 5); // ===> 0~4
                        printf("%d\n", reward_num);
                        printf("%d티어 소비아이템 %s를 얻었습니다.\n", equipment_tier[6][cons_tier3_4[reward_num][1]] ,equipment_type[cons_tier3_4[reward_num][0]][cons_tier3_4[reward_num][1]]);

                        // 소비아이템 공간이 가득 찬 경우 처리하는 코드
                        if (cons_space[cons_tier3_4[reward_num][1]] == 99)
                        {
                            printf("공간이 가득 찼습니다.\n");
                        }
                        else
                        {
                            cons_space[cons_tier3_4[reward_num][1]] += 1;
                        }
                    }


                    // 1티어 강화재료 보상 (1번째 몬스터의 경우)
                    if (mon_name_num == 0 && (((rand() % 100) + 1) <= monster_mat_item_1tier[mon_name_num]))
                    {
                        printf("%d티어 기타아이템 %s를 얻었습니다.\n", 1, "작은 화염 조각");
                        // 기타아이템 공간이 가득 찬 경우 처리하는 코드
                        if (mat_space[0] == 999)
                        {
                            printf("공간이 가득 찼습니다.\n");
                        }
                        else
                        {
                            mat_space[0] += 1;
                        }
                    }


                    // 1~2티어 강화재료 보상 (2번째 몬스터의 경우)
                    if (mon_name_num == 1 && (((rand() % 100) + 1) <= monster_mat_item_2tier[mon_name_num]))
                    {
                        reward_num = (rand() % 2); // ===> 0~4
                        printf("%d티어 기타아이템 %s를 얻었습니다.\n", mat_item_tier[reward_num], mat_item_name[reward_num]);
                        // 기타아이템 공간이 가득 찬 경우 처리하는 코드
                        if (mat_space[reward_num] == 999)
                        {
                            printf("공간이 가득 찼습니다.\n");
                        }
                        else
                        {
                            mat_space[reward_num] += 1;
                        }
                    }


                    // 1~3티어 강화재료 보상 (3번째 몬스터의 경우)
                    if (mon_name_num == 2 && (((rand() % 100) + 1) <= monster_mat_item_3tier[mon_name_num]))
                    {
                        reward_num = (rand() % 4); // ===> 0~3
                        printf("%d티어 기타아이템 %s를 얻었습니다.\n", mat_item_tier[mat_item_tier1_3[reward_num]], mat_item_name[mat_item_tier1_3[reward_num]]);
                        // 기타아이템 공간이 가득 찬 경우 처리하는 코드
                        if (mat_space[reward_num] == 999)
                        {
                            printf("공간이 가득 찼습니다.\n");
                        }
                        else
                        {
                            mat_space[mat_item_tier1_3[reward_num]] += 1;
                        }
                    }


                    // 3티어 강화재료 보상 (4번째 몬스터의 경우)
                    if (mon_name_num == 3 && (((rand() % 100) + 1) <= monster_mat_item_3tier[mon_name_num]))
                    {
                        reward_num = (rand() % 2); // ===> 0~1
                        printf("%d티어 기타아이템 %s를 얻었습니다.\n", mat_item_tier[mat_item_tier3[reward_num]], mat_item_name[mat_item_tier3[reward_num]]);
                        // 기타아이템 공간이 가득 찬 경우 처리하는 코드
                        if (mat_space[reward_num] == 999)
                        {
                            printf("공간이 가득 찼습니다.\n");
                        }
                        else
                        {
                            mat_space[mat_item_tier3[reward_num]] += 1;
                        }
                    }


                    // 3~4티어 강화재료 보상 (5, 6번째 몬스터의 경우)
                    if ((mon_name_num >= 5 && mon_name_num <= 6) && monster_mat_item_4tier[mon_name_num])
                    {
                        reward_num = (rand() % 3) + 2; // ===> 0 1 2 = > 2, 3, 4
                        printf("%d티어 기타아이템 %s를 얻었습니다.\n", mat_item_tier[reward_num], mat_item_name[reward_num]);
                        // 기타아이템 공간이 가득 찬 경우 처리하는 코드
                        if (mat_space[reward_num] == 999)
                        {
                            printf("공간이 가득 찼습니다.\n");
                        }
                        else
                        {
                            mat_space[reward_num] += 1;
                        }
                    }   

                    //싸운 몬스터 제거
                    if((map_input == 'w')&& (user_y>0) && ((map[user_y-1][user_x] == 2)|| (map[user_y-1][user_x] == 3) || (map[user_y-1][user_x] == 4)))                    
                    // w입력시 ,y가 0보다 크면(맵 밖을 안넘어가면) ,가려할 자리에 몬스터가 있으면
                    {
                        map[user_y-1][user_x] = 6;
                    }
                    else if ((map_input == 's')&& (user_y < SIZE -1)&&((map[user_y+1][user_x] == 2)||(map[user_y+1][user_x] == 3)||(map[user_y+1][user_x] == 4)))
                    //s입력했고 y가 19보다 낮으면(맵 밖을 안넘어가면), 가려할 자리에 몬스터가 있으면
                    {
                        map[user_y+1][user_x] = 6;
                    }
                    else if((map_input == 'a')&&(user_x>0)&&((map[user_y][user_x-1] == 2)||(map[user_y][user_x-1] == 3)||(map[user_y][user_x-1] == 4)))
                    //a입력했고 x가 0보다 크면(맵 밖을 안넘어가면), 가려할 자리에 몬스터가 있으면
                    {
                        map[user_y][user_x-1] = 6;
                    }
                    else if ((map_input == 'd')&&(user_x<SIZE -1)&&((map[user_y][user_x+1] == 2)||(map[user_y][user_x+1] == 3)||(map[user_y][user_x+1] == 4)))
                    //a입력했고 x가 19보다 크면(맵 밖을 안넘어가면), 가려할 자리에 몬스터가 있으면
                    {
                        map[user_y][user_x+1] = 6;
                    }

                    //다시맵으로
                    map_not_change();
                    break;
                }
                else if (war_switch==1)// 몬스터가 죽지X ==> 몬스터가 날 공격 
                {   
                    int i = rand()%100;
                    int j = rand()%1000;    //회피
                    if (immune==0)
                    {
                        if (((mon_name_num==1) || (mon_name_num==6))&& (i>80))       //몹이 좀비나 보스일 시 맞을 때 마다 20%확률로 독에 걸림
                            ABN_STATUS_ONOFF[0]=1;
                        else if ((mon_name_num==2) && (i>60))                        //몹이 구울 일 때 맞을 때 마다 40% 확률로 독
                            ABN_STATUS_ONOFF[0]=1;
                        else if (((mon_name_num==3) || (mon_name_num==6)) && (i>80)) //몹이 해골이나 보스 일 시일 때 맞을 때 마다 20% 확률로 저주
                            ABN_STATUS_ONOFF[1]=1;
                        else if ((mon_name_num==7)&&(i>80))                          //몹이 이동녀크 일 때 맞을 때 마다 20% 확률로 나태
                            ABN_STATUS_ONOFF[2]=1;
                    }
                    
                    if (j>live_eva*10)
                    {
                        live_def = live_def + MY_EQUIP[0][2] + (MY_EQUIP[0][4] * 0.01 * -1);
                        live_hp -= (monster_max_atk - live_def);
                    }
                    else
                    {
                        printf("나는야 럭키가이\n");
                    }
                    if (live_hp <= 0) // 사용자가 죽은 경우
                    {
                        live_exp *= 0.5;
                        live_hp = max_hp;
                        live_mp = max_mp;
                        floor = 0;
                        user_x = 0;
                        user_y = 49;
                        printf("exp : %d\n", live_exp);
                        printf("hp : %d\n", live_hp);
                        printf("mp : %d\n", live_mp);
                        printf("floor : %d\n", floor);
                        printf("y : %d\n", user_y);
                        printf("x : %d\n", user_x);
                        
                        map_change();

                        break;
                    }
                    else // 사용자가 죽지 않은 경우
                    {
                        continue;
                    }
                }
                if (run_switch = 1)
                {
                    run_switch =0;
                    break;
                }
            }        
        }

    }
    return 0;
   
}

int shop(int *price, int user_cons_select1, char *cons_item_name, char *weapon_name, char *armor_name, char *shoes_name, char *glove_name, char *cloak_name, char *hat_name, int *cons_item_price, int *weapon_price, int *armor_price, int *shoes_price, int *glove_price, int *cloak_price, int *hat_price, int len_cons_item, int len_weapon, int len_armor, int len_shoes, int len_glove, int len_cloak, int len_hat)
{
    int user_cons_select2; // 그 중에서 어떤거 살지 (빨간물약/주황물약 등등)

    switch (user_cons_select1)
    {
        case 1:
        {
            printf("[소비아이템]을 선택하였습니다. \n");
            for (int i = 0 ; i < len_cons_item ; i++)
            {
                printf("%2d. %s\t | %d GOLD\n", i+1, cons_item_name + (ARR_SIZE * i), cons_item_price[i]);
            }
            printf("\n 0. 이전메뉴\n");
            scanf("%d", &user_cons_select2); // 그 중에서 어떤거 살지 선택
            system("clear");
            if (user_cons_select2 != 0)
            {
                printf("%s를 선택하였습니다.\n", cons_item_name + (ARR_SIZE * (user_cons_select2-1)));
                *price = cons_item_price[user_cons_select2-1];;
            }
            break;
        }
        case 2:
        {
            printf("[무기]를 선택하였습니다.\n");
            for (int i = 0 ; i < len_weapon ; i++)
            {
                if (i == 1)
                {
                    printf("%d. %s\t\t | %d GOLD\n", i+1, weapon_name + (ARR_SIZE * i), weapon_price[i]);
                }
                else
                {
                    printf("%d. %s\t | %d GOLD\n", i+1, weapon_name + (ARR_SIZE * i), weapon_price[i]);
                }
            }
            printf("\n0. 이전메뉴\n");
            scanf("%d", &user_cons_select2); // 그 중에서 어떤거 살지 선택
            system("clear");
            if (user_cons_select2 != 0)
            {
                printf("%s를 선택하였습니다.\n", weapon_name + (ARR_SIZE * (user_cons_select2-1)));
                *price = weapon_price[user_cons_select2-1];
            }
            break;
        }
        case 3:
        {
            printf("[갑옷]를 선택하였습니다.\n");
            for (int i = 0 ; i < len_armor ; i++)
            {
                printf("%d. %s\t | %d GOLD\n", i+1, armor_name + (ARR_SIZE * i), armor_price[i]);
            }
            printf("\n0. 이전메뉴\n");
            scanf("%d", &user_cons_select2); // 그 중에서 어떤거 살지 선택
            system("clear");
            if (user_cons_select2 != 0)
            {
                printf("%s를 선택하였습니다.\n", armor_name + (ARR_SIZE * (user_cons_select2-1)));
                *price = armor_price[user_cons_select2-1];
            }
            break;
        }
        case 4:
        {
            printf("[신발]를 선택하였습니다.\n");
            // printf("원하시는 아이템을 선택하세요.\n");
            for (int i = 0 ; i < len_shoes ; i++)
            {
                printf("%d. %s\t | %d GOLD\n", i+1, shoes_name + (ARR_SIZE * i), shoes_price[i]);
            }
            printf("\n0. 이전메뉴\n");
            scanf("%d", &user_cons_select2); // 그 중에서 어떤거 살지 선택
            system("clear");
            if (user_cons_select2 != 0)
            {
                printf("%s를 선택하였습니다.\n", shoes_name + (ARR_SIZE * (user_cons_select2-1)));
                *price = shoes_price[user_cons_select2-1];
            }
            break;
        }
        case 5:
        {
            printf("[장갑]를 선택하였습니다.\n");
            // printf("원하시는 아이템을 선택하세요.\n");
            for (int i = 0 ; i < len_glove ; i++)
            {
                printf("%d. %s\t | %d GOLD\n", i+1, glove_name + (ARR_SIZE * i), glove_price[i]);
            }
            printf("\n0. 이전메뉴\n");
            scanf("%d", &user_cons_select2); // 그 중에서 어떤거 살지 선택
            system("clear");
            if (user_cons_select2 != 0)
            {
                printf("%s를 선택하였습니다.\n", glove_name + (ARR_SIZE * (user_cons_select2-1)));
                *price = glove_price[user_cons_select2-1];
            }
            break;
        }
        case 6:
        {
            printf("[망토]를 선택하였습니다.\n");
            // printf("원하시는 아이템을 선택하세요.\n");
            for (int i = 0 ; i < len_cloak ; i++)
            {
                printf("%d. %s\t | %d GOLD\n", i+1, cloak_name + (ARR_SIZE * i), cloak_price[i]);
            }
            printf("\n0. 이전메뉴\n");
            scanf("%d", &user_cons_select2); // 그 중에서 어떤거 살지 선택
            system("clear");
            if (user_cons_select2 != 0)
            {
                printf("%s를 선택하였습니다.\n", cloak_name + (ARR_SIZE * (user_cons_select2-1)));
                *price = cloak_price[user_cons_select2-1];
            }
            break;
        }
        case 7:
        {
            printf("[투구]를 선택하였습니다.\n");
            // printf("원하시는 아이템을 선택하세요.\n");
            for (int i = 0 ; i < len_hat ; i++)
            {
                printf("%d. %s\t | %d GOLD\n", i+1, hat_name + (ARR_SIZE * i), hat_price[i]);
            }
            printf("\n0. 이전메뉴\n");
            scanf("%d", &user_cons_select2); // 그 중에서 어떤거 살지 선택
            system("clear");
            if (user_cons_select2 != 0)
            {
                printf("%s를 선택하였습니다.\n", hat_name + (ARR_SIZE * (user_cons_select2-1)));
                *price = hat_price[user_cons_select2-1];
            }
            break;
        }
    }
    return user_cons_select2;
}

void map_print()
{
    int mon_num = 0;               //남은 몬스터 수
    srand(time(NULL));
    while (plag_mapout == 0)   //전투든 메뉴든 와일문 나가는 플래그
    {
        while (plag_mapsave ==0)     //맵 새로 생성하려면 0 맵 저장하려면 1
        {
            if ((floor == 0) && (tp_switch ==1))    // 마을 일때 (텔포였을 때)
            {
                for (int i = 0;i<SIZE;i++)     //맵 전체 0으로 초기화
                {
                    for(int j =0;j<SIZE;j++)
                        map[i][j]=6;
                }
                tp_switch = 0;
                map[0][SIZE-1] = 5;           //포탈위치
                user_y = save_point[user_position_select-1][1];          //유저 스타트 좌표 초기화
                user_x = save_point[user_position_select-1][2];
                map[save_point[user_position_select-1][1]][save_point[user_position_select-1][2]] = 1;
                portal_y =0;
                portal_x =49;
                break;
            }
            else if (floor == 0 )    // 마을 일때
            {
                for (int i = 0;i<SIZE;i++)     //맵 전체 0으로 초기화
                {
                    for(int j =0;j<SIZE;j++)
                        map[i][j]=6;
                }
                map[0][SIZE-1] = 5;           //포탈위치
                map[START_Y][START_X] = 1;    //map[행][열]  [유저 위치 프린트 위치 초기화]
                user_y = START_Y;          //유저 스타트 좌표 초기화
                user_x = START_X;
                portal_y =0;
                portal_x =49;
                break;
            }
            else if ((floor>=1) && (floor<=4) && tp_switch ==1)   // 1~4층일 때   // (텔포였을 때)
            {   
                for (int i = 0;i<SIZE;i++)       //맵 전체 0으로 초기화
                {
                    for(int j =0;j<SIZE;j++)
                        map[i][j]=6;
                }
                tp_switch = 0;
                //맵에 숫자 5~9 배열 (5개중 하나이면 25% 확률로 벽 생성)
                for (int i=0;i<SIZE;i++) 
                {
                    for (int j=0;j<SIZE;j++)
                    {
                        map[i][j] = rand()%4+6;
                    }
                }
                //포탈 까지 안전 루트 생성
                for(int i=0;i<SIZE;i++)
                {
                    map[i][SIZE-i-1] = 6;
                }
                for (int i=0;i<SIZE-1;i++)
                {
                    map[i+1][SIZE-i-1] = 6;
                }
                //몬스터 위치 배분
                for(int i=0;i<MONSTER_REGEN_RATE;i++) 
                {
                    for(int j=0;j<2;j++)
                    {
                        int tmp;
                        tmp = rand()%50;
                        mon_position[i][j] = tmp;
                        if ((mon_position[i][0]==(SIZE-1)) && (mon_position[i][1]==0))  //주인공 위치면 다시 생성
                        {
                            --i;
                            break;
                        }
                    }
                }
                //몬스터 위치 중복 검사  
                for (int i=0;i<MONSTER_REGEN_RATE-1;i++)
                {
                    for(int j=1;j<MONSTER_REGEN_RATE;j++)
                    {
                        for (int k=0;k<2;k++)
                        {
                            if (mon_position[i][k] == mon_position[j][k])  
                            {
                                int tmp = rand()%SIZE;
                                mon_position[j][k] = tmp;
                            }
                        }
                    }
                }
                //몬스터 도달 길 열기
                for (int i=0;i<MONSTER_REGEN_RATE;i++)
                {
                    for (int j=0;j<SIZE;j++)
                    {
                        map[j][mon_position[i][1]] = 6;  //몬스터 x값 좌표의 세로축은 전부 뚫린길
                    }

                }
                //포탈 주위 클리어
                for(int i=0;i<PORTAL_LOCATION;i++)  //포탈 제한 위치만큼 반복
                {
                    for(int j=0;j<PORTAL_LOCATION;j++)  //포탈 제한 위치만큼 반복
                    {
                        map[i][SIZE-PORTAL_LOCATION+j] = 6;
                    }
                }
                //몬스터 배치
                for (int i=0;i<MONSTER_REGEN_RATE;i++)
                {
                    map[mon_position[i][0]][mon_position[i][1]] = 2;
                }
                // 포탈 생성
                portal_x = ((rand()%PORTAL_LOCATION)+SIZE-PORTAL_LOCATION); 
                portal_y = rand()%PORTAL_LOCATION;
                //포탈 몬스터와 위치 중복 검사
                for (int i=0;i<MONSTER_REGEN_RATE;i++)                  
                {
                    if ((mon_position[i][0] == portal_y)&&(mon_position[i][1]==portal_x))  
                    {
                        portal_x = ((rand()%PORTAL_LOCATION)+SIZE-PORTAL_LOCATION); 
                        portal_y = rand()%PORTAL_LOCATION;
                        --i;
                        break;
                    }

                }
                map [portal_y][portal_x] = 5;
                map[save_point[user_position_select-1][1]][save_point[user_position_select-1][2]] = 1;    //map[행][열]  [유저 위치 프린트 위치 좌표 저장된 값으로 출력
                user_y = save_point[user_position_select-1][1];                                           //유저 스타트 좌표 저장된 값으로 지정
                user_x = save_point[user_position_select-1][2];
                break;
            }
            else if ((floor>=1) && (floor<=4))   // 1~4층일 때
            {   
                for (int i = 0;i<SIZE;i++)       //맵 전체 0으로 초기화
                {
                    for(int j =0;j<SIZE;j++)
                        map[i][j]=6;
                }
                map[START_Y][START_X] = 1;    //map[행][열]  [유저 위치 프린트 위치 초기화]
                user_y = START_Y;          //유저 스타트 좌표 초기화
                user_x = START_X;
                //맵에 숫자 5~9 배열 (5개중 하나이면 25% 확률로 벽 생성)
                for (int i=0;i<SIZE;i++) 
                {
                    for (int j=0;j<SIZE;j++)
                    {
                        map[i][j] = rand()%4+6;
                    }
                }
                //포탈 까지 안전 루트 생성
                for(int i=0;i<SIZE;i++)
                {
                    map[i][SIZE-i-1] = 6;
                }
                for (int i=0;i<SIZE-1;i++)
                {
                    map[i+1][SIZE-i-1] = 6;
                }
                //몬스터 위치 배분
                for(int i=0;i<MONSTER_REGEN_RATE;i++) 
                {
                    for(int j=0;j<2;j++)
                    {
                        int tmp;
                        tmp = rand()%50;
                        mon_position[i][j] = tmp;
                        if ((mon_position[i][0]==(SIZE-1)) && (mon_position[i][1]==0))  //주인공 위치면 다시 생성
                        {
                            --i;
                            break;
                        }
                    }
                }
                //몬스터 위치 중복 검사  
                for (int i=0;i<MONSTER_REGEN_RATE-1;i++)
                {
                    for(int j=1;j<MONSTER_REGEN_RATE;j++)
                    {
                        for (int k=0;k<2;k++)
                        {
                            if (mon_position[i][k] == mon_position[j][k])  
                            {
                                int tmp = rand()%SIZE;
                                mon_position[j][k] = tmp;
                            }
                        }
                    }
                }
                //몬스터 도달 길 열기
                for (int i=0;i<MONSTER_REGEN_RATE;i++)
                {
                    for (int j=0;j<SIZE;j++)
                    {
                        map[j][mon_position[i][1]] = 6;  //몬스터 x값 좌표의 세로축은 전부 뚫린길
                    }

                }
                //포탈 주위 클리어
                for(int i=0;i<PORTAL_LOCATION;i++)  //포탈 제한 위치만큼 반복
                {
                    for(int j=0;j<PORTAL_LOCATION;j++)  //포탈 제한 위치만큼 반복
                    {
                        map[i][SIZE-PORTAL_LOCATION+j] = 6;
                    }
                }
                //몬스터 배치
                for (int i=0;i<MONSTER_REGEN_RATE;i++)
                {
                    map[mon_position[i][0]][mon_position[i][1]] = 2;
                }
                // 포탈 생성
                portal_x = ((rand()%PORTAL_LOCATION)+SIZE-PORTAL_LOCATION); 
                portal_y = rand()%PORTAL_LOCATION;
                //포탈 몬스터와 위치 중복 검사
                for (int i=0;i<MONSTER_REGEN_RATE;i++)                  
                {
                    if ((mon_position[i][0] == portal_y)&&(mon_position[i][1]==portal_x))  
                    {
                        portal_x = ((rand()%PORTAL_LOCATION)+SIZE-PORTAL_LOCATION); 
                        portal_y = rand()%PORTAL_LOCATION;
                        --i;
                        break;
                    }

                }
                map [portal_y][portal_x] = 5;
                map[START_Y][START_X] = 1;    //map[행][열]  [유저 위치 프린트 위치 초기화]
                user_y = START_Y;          //유저 스타트 좌표 초기화
                user_x = START_X;
                break;
            }
            else if(floor ==5 && tp_switch ==1)    // 보스층 일 때
            {
                for (int i = 0;i<SIZE;i++)       //맵 전체 0으로 초기화
                {
                    for(int j =0;j<SIZE;j++)
                        map[i][j]=6;
                }
                tp_switch = 0;
                //맵에 숫자 6~9 배열 (5개중 하나이면 25% 확률로 벽 생성)
                for (int i=0;i<SIZE;i++) 
                {
                    for (int j=0;j<SIZE;j++)
                    {
                        map[i][j] = rand()%4+6;
                    }
                }
                //보스 까지 안전 루트 생성
                for(int i=0;i<SIZE;i++)
                {
                    map[i][SIZE-i-1] = 6;
                }
                for (int i=0;i<SIZE-1;i++)
                {
                    map[i+1][SIZE-i-1] = 6;
                }

                for(int i=0;i<MONSTER_REGEN_RATE;i++)   //몬스터 위치 배분
                {
                    for(int j=0;j<2;j++)
                    {
                        int tmp;
                        tmp = rand()%50;
                        mon_position[i][j] = tmp;
                        if ((mon_position[i][0]==(SIZE-1)) && (mon_position[i][1]==0))  //주인공 위치면 다시 생성
                        {
                            --i;
                            break;
                        }
                    }
                }
                //몬스터 위치 중복 검사  
                for (int i=0;i<MONSTER_REGEN_RATE-1;i++)
                {
                    for(int j=1;j<MONSTER_REGEN_RATE;j++)
                    {
                        for (int k=0;k<2;k++)
                        {
                            if (mon_position[i][k] == mon_position[j][k])  
                            {
                                int tmp = rand()%50;
                                mon_position[j][k] = tmp;
                            }
                        }
                    }
                }
                //몬스터 도달 길 열기
                for (int i=0;i<MONSTER_REGEN_RATE;i++)
                {
                    for (int j=0;j<SIZE;j++)
                    {
                        map[j][mon_position[i][1]] = 6;  //몬스터 x값 좌표의 세로축은 전부 뚫린길
                    }

                }
                //보스 주위 클리어
                for(int i=0;i<PORTAL_LOCATION;i++)  //포탈 제한 위치만큼 반복
                {
                    for(int j=0;j<PORTAL_LOCATION;j++)  //포탈 제한 위치만큼 반복
                    {
                        map[i][SIZE-PORTAL_LOCATION+j] = 6;
                    }
                }
                //몬스터 배치
                for (int i=0;i<MONSTER_REGEN_RATE;i++)
                {
                    map[mon_position[i][0]][mon_position[i][1]] = 2;
                }
                map[save_point[user_position_select-1][1]][save_point[user_position_select-1][2]] = 1;    //map[행][열]  [유저 위치 프린트 위치 좌표 저장된 값으로 출력
                user_y = save_point[user_position_select-1][1];                                           //유저 스타트 좌표 저장된 값으로 지정
                user_x = save_point[user_position_select-1][2];
                break;
            }
            else    // 보스층 일 때
            {
                //맵에 숫자 6~9 배열 (5개중 하나이면 25% 확률로 벽 생성)
                for (int i=0;i<SIZE;i++) 
                {
                    for (int j=0;j<SIZE;j++)
                    {
                        map[i][j] = rand()%4+6;
                    }
                }
                //보스 까지 안전 루트 생성
                for(int i=0;i<SIZE;i++)
                {
                    map[i][SIZE-i-1] = 6;
                }
                for (int i=0;i<SIZE-1;i++)
                {
                    map[i+1][SIZE-i-1] = 6;
                }

                for(int i=0;i<MONSTER_REGEN_RATE;i++)   //몬스터 위치 배분
                {
                    for(int j=0;j<2;j++)
                    {
                        int tmp;
                        tmp = rand()%50;
                        mon_position[i][j] = tmp;
                        if ((mon_position[i][0]==(SIZE-1)) && (mon_position[i][1]==0))  //주인공 위치면 다시 생성
                        {
                            --i;
                            break;
                        }
                    }
                }
                //몬스터 위치 중복 검사  
                for (int i=0;i<MONSTER_REGEN_RATE-1;i++)
                {
                    for(int j=1;j<MONSTER_REGEN_RATE;j++)
                    {
                        for (int k=0;k<2;k++)
                        {
                            if (mon_position[i][k] == mon_position[j][k])  
                            {
                                int tmp = rand()%50;
                                mon_position[j][k] = tmp;
                            }
                        }
                    }
                }
                //몬스터 도달 길 열기
                for (int i=0;i<MONSTER_REGEN_RATE;i++)
                {
                    for (int j=0;j<SIZE;j++)
                    {
                        map[j][mon_position[i][1]] = 6;  //몬스터 x값 좌표의 세로축은 전부 뚫린길
                    }

                }
                //보스 주위 클리어
                for(int i=0;i<PORTAL_LOCATION;i++)  //포탈 제한 위치만큼 반복
                {
                    for(int j=0;j<PORTAL_LOCATION;j++)  //포탈 제한 위치만큼 반복
                    {
                        map[i][SIZE-PORTAL_LOCATION+j] = 6;
                    }
                }
                //몬스터 배치
                for (int i=0;i<MONSTER_REGEN_RATE;i++)
                {
                    map[mon_position[i][0]][mon_position[i][1]] = 2;
                }
                map[START_Y][START_X] = 1;    //map[행][열]  [유저 위치 프린트 위치 초기화]
                user_y = START_Y;          //유저 스타트 좌표 초기화
                user_x = START_X;
                break;
            }
        }

        boss_regen();         //////////////////////////////보스 리젠검사///////////////////////////////


        while(map_input != '1')
        {
            system("clear");                     // 이동 페이즈
            if (floor == 0)
            {
                printf("보은's Village\n");
            }
            else
            {
                printf("AI의 탑 %d층\n",floor);
            }
            //몬스터와 상호작용
            if((map_input == 'w')&& (user_y>0) && ((map[user_y-1][user_x] == 2)|| (map[user_y-1][user_x] == 3) || (map[user_y-1][user_x] == 4)))                    
            // w입력시 ,y가 0보다 크면(맵 밖을 안넘어가면) ,가려할 자리에 몬스터가 있으면
            {
                plag_mapsave = 1;        //맵 저장 플래그
                plag_mapout = 1;         //맵 아웃 플래그
                war_switch = 1;         //전투 페이즈 스위치 온

                break;
            }
            else if ((map_input == 's')&& (user_y < SIZE -1)&&((map[user_y+1][user_x] == 2)||(map[user_y+1][user_x] == 3)||(map[user_y+1][user_x] == 4)))
            //s입력했고 y가 19보다 낮으면(맵 밖을 안넘어가면), 가려할 자리에 몬스터가 있으면
            {
                plag_mapsave = 1;        //맵 저장 플래그
                plag_mapout = 1;         //맵 아웃 플래그
                war_switch = 1;         //전투 페이즈 스위치 온

                break;
            }
            else if((map_input == 'a')&&(user_x>0)&&((map[user_y][user_x-1] == 2)||(map[user_y][user_x-1] == 3)||(map[user_y][user_x-1] == 4)))
            //a입력했고 x가 0보다 크면(맵 밖을 안넘어가면), 가려할 자리에 몬스터가 있으면
            {
                plag_mapsave = 1;        //맵 저장 플래그
                plag_mapout = 1;         //맵 아웃 플래그
                war_switch = 1;         //전투 페이즈 스위치 온

                break;
            }
            else if ((map_input == 'd')&&(user_x<SIZE -1)&&((map[user_y][user_x+1] == 2)||(map[user_y][user_x+1] == 3)||(map[user_y][user_x+1] == 4)))
            //a입력했고 x가 19보다 크면(맵 밖을 안넘어가면), 가려할 자리에 몬스터가 있으면
            {
                plag_mapsave = 1;        //맵 저장 플래그
                plag_mapout = 1;         //맵 아웃 플래그
                war_switch = 1;         //전투 페이즈 스위치 온

                break;
            }
            //벽과 상호작용
            else if((map_input == 'w')&& (user_y>0)&& (map[user_y-1][user_x] != 9))                    
            // w입력시 ,y가 0보다 크면(맵 밖을 안넘어가면) ,유저 현위치 윗자리가 벽이 아니면
            {
                map[user_y][user_x] = 0;                   // 19행 19열은 0이 되고
                user_y--;                             // y는 18이 되며
                map[user_y][user_x] = 1;                   // 18행 19열은 1로 된다.
            }
            else if ((map_input == 's')&& (user_y < SIZE -1)&&(map[user_y+1][user_x] != 9))
            //s입력했고 y가 19보다 낮으면(맵 밖을 안넘어가면), 유저 현위치 밑자리가 벽이 아니면
            {
                map[user_y][user_x] = 0;                   //기존에 있는자리는 0이 되며
                user_y++;                              //밑으로로 한칸 이동하고
                map[user_y][user_x] = 1;                    //그 자리가 1이 된다.
            }
            else if((map_input == 'a')&&(user_x>0)&&(map[user_y][user_x-1] != 9))
            //a입력했고 x가 0보다 크면(맵 밖을 안넘어가면), 유저 현위치 왼쪽이 벽이 아니면
            {
                map[user_y][user_x] = 0;
                user_x--;
                map[user_y][user_x] = 1;
            }
            else if ((map_input == 'd')&&(user_x<SIZE -1)&&(map[user_y][user_x+1] != 9))
            //a입력했고 x가 19보다 크면(맵 밖을 안넘어가면), 유저 현위치 오른쪽이 벽이 아니면
            {
                map[user_y][user_x] = 0;
                user_x++;
                map[user_y][user_x] = 1;
            }
            mon_num = 0;                          //층에 몬스터 마릿수 계산
            for (int i = 0;i<SIZE;i++)
            {
                for(int j = 0;j<SIZE;j++)
                {
                    if(map[i][j]==2)
                        mon_num += 1;
                }
            }

            // 플레이어 좌표와 포탈 좌표가 같다면 혹은 (1층이 아니고 몬스터가 0이면) 몬스터가 없다면 층수 ++ 컨티뉴 후 반복문 나가기
            if ((((user_y==portal_y) && (user_x==portal_x))||((floor!=0)&&(mon_num==0)))&&floor!=5)
            {
                floor += 1;
                map_change();
                break;
            }

            for(int i=0; i<SIZE; i++)         //출력 페이즈
            {                      

                for(int j=0; j<SIZE; j++)
                {
                    if(map[i][j]==1)
                    {                         //주인공 출력
                        printf("🤸");
                    }
                    else if (map[i][j] == 2)
                    {
                        printf("🧟");         //2면 몬스터 출력
                    }
                    else if (map[i][j] == 3)
                    {
                        printf("👹");         //3면  보스 출력
                    }
                    else if (map[i][j] == 4)
                    {
                        printf("👹");         //4면   이동녀크 출력
                    }
                    else if (map[i][j] == 5)
                    {
                        printf("🌀");         //5면 포탈 출력
                    }
                    else if (map[i][j] == 9)
                    {
                        printf("🧱");         //9면 벽 출력
                    }
                    else
                    {
                        printf("  ");         // 6~8여백 출력
                    }
                }
                printf("\n");

            }

            printf("유저의 위치 : %d %d",user_y,user_x);
            printf("포탈의 위치 : %d %d",portal_y,portal_x);
            printf("몬스터 숫자 : %d",mon_num);
            map_input = getch();

            if (map_input == '1')
            {
                plag_mapsave = 1;        //맵 저장 플래그
                plag_mapout = 1;         //맵 아웃 플래그
                break;
            }
        }
        
    }

}

void boss_regen()
{
    while((floor==5)&&(plag_boss_regen==1))       //5층이고 보스리젠 플래그가 켜지면
    {
        for(int i=0;i<SIZE;i++)                  //보스가 있는지 없는지 중복검사. 있으면 보스있다고 1 없으면 없다고 0
        {
            for(int j=0;j<SIZE;j++)
            {
                if(map[i][j] == 3)
                {
                    there_is_the_boss =1;
                }
                else if(map[i][j] == 3)
                {
                    there_is_the_boss =0;
                }

            }
        }
        // 보스 생성
        if (there_is_the_boss == 0)               //보스가 없으면!
        {
            int tmp_boss_regen_roll;        
            tmp_boss_regen_roll = rand()%100;
            if ((BOSS_REGEN_RATE-1)>tmp_boss_regen_roll)    // 확률로 보스 생성
            {
                boss_x = ((rand()%PORTAL_LOCATION)+SIZE-PORTAL_LOCATION); 
                boss_y = rand()%PORTAL_LOCATION;
                for (int i=0;i<MONSTER_REGEN_RATE;i++)                   //포탈 몬스터와 위치 중복 검사
                {
                    if ((mon_position[i][0] == boss_y)&&(mon_position[i][1]==boss_x))  
                    {
                        boss_x = ((rand()%PORTAL_LOCATION)+SIZE-PORTAL_LOCATION); 
                        boss_y = rand()%PORTAL_LOCATION;
                        --i;
                        break;
                    }
                }
                map[boss_y][boss_x] = 3;
            }
        }
        break;
    }
}

// 엔터 없는 입력(동기)
int getch() 
{
    int c;
    struct termios oldattr, newattr;

    tcgetattr(STDIN_FILENO, &oldattr);           // 현재 터미널 설정 읽음
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);         // CANONICAL과 ECHO 끔
    newattr.c_cc[VMIN] = 1;                      // 최소 입력 문자 수를 1로 설정
    newattr.c_cc[VTIME] = 0;                     // 최소 읽기 대기 시간을 0으로 설정
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);  // 터미널에 설정 입력
    c = getchar();                               // 키보드 입력 읽음
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);  // 원래의 설정으로 복구
    return c;
}

void exp_UI(int live_exp, float per_exp)
{
    if (live_exp == 0)
    {
        printf("⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜");
    }
    else if (per_exp < 10)
    {
        printf("🟨⬜⬜⬜⬜⬜⬜⬜⬜⬜");
    }
    else if (per_exp >= 10 && per_exp < 20)
    {
        if (per_exp == 10)
        {
            printf("🟩⬜⬜⬜⬜⬜⬜⬜⬜⬜");
        }
        else
        {
            printf("🟩🟨⬜⬜⬜⬜⬜⬜⬜⬜");
        }
    }
    else if (per_exp >= 20 && per_exp < 30)
    {
        if (per_exp == 20)
        {
            printf("🟩🟩⬜⬜⬜⬜⬜⬜⬜⬜");
        }
        else
        {
            printf("🟩🟩🟨⬜⬜⬜⬜⬜⬜⬜");
        }
    }
    else if (per_exp >= 30 && per_exp < 40)
    {
        if (per_exp == 30)
        {
            printf("🟩🟩🟩⬜⬜⬜⬜⬜⬜⬜");
        }
        else
        {
            printf("🟩🟩🟩🟨⬜⬜⬜⬜⬜⬜");
        }
    }
    else if (per_exp >= 40 && per_exp < 50)
    {
        if (per_exp == 40)
        {
            printf("🟩🟩🟩🟩⬜⬜⬜⬜⬜⬜");
        }
        else
        {
            printf("🟩🟩🟩🟩🟨⬜⬜⬜⬜⬜");
        }
    }
    else if (per_exp >= 50 && per_exp < 60)
    {
        if (per_exp == 50)
        {
            printf("🟩🟩🟩🟩🟩⬜⬜⬜⬜⬜");
        }
        else
        {
            printf("🟩🟩🟩🟩🟩🟨⬜⬜⬜⬜");
        }
        
    }
    else if (per_exp >= 60 && per_exp < 70)
    {
        if (per_exp == 60)
        {
            printf("🟩🟩🟩🟩🟩🟩⬜⬜⬜⬜");
        }
        else
        {
            printf("🟩🟩🟩🟩🟩🟩🟨⬜⬜⬜");
        }
    }
    else if (per_exp >= 70 && per_exp < 80)
    {
        if (per_exp == 70)
        {
            printf("🟩🟩🟩🟩🟩🟩🟩⬜⬜⬜");
        }
        else
        {
            printf("🟩🟩🟩🟩🟩🟩🟩🟨⬜⬜");
        }
    }
    else if (per_exp >= 80 && per_exp < 90)
    {
        if (per_exp == 80)
        {
            printf("🟩🟩🟩🟩🟩🟩🟩🟩⬜⬜");
        }
        else
        {
            printf("🟩🟩🟩🟩🟩🟩🟩🟩🟨⬜");
        }
    }
    else if (per_exp >= 90 && per_exp < 100)
    {
        if (per_exp == 90)
        {
            printf("🟩🟩🟩🟩🟩🟩🟩🟩🟩⬜");
        }
        else
        {
            printf("🟩🟩🟩🟩🟩🟩🟩🟩🟩🟨");
        }
    }
    printf("\n\n");
}


void hp_UI(int live_hp, float per_hp)
{
    
    if (live_hp == 0)
    {
        printf("⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜");
    }
    else if (per_hp <= 10)
    {
        printf("🟥⬜⬜⬜⬜⬜⬜⬜⬜⬜");
    }
    else if (per_hp <= 20)
    {
        printf("🟥🟥⬜⬜⬜⬜⬜⬜⬜⬜");
    }
    else if (per_hp <= 30)
    {
        printf("🟥🟥🟥⬜⬜⬜⬜⬜⬜⬜");
    }
    else if (per_hp <= 40)
    {
        printf("🟥🟥🟥🟥⬜⬜⬜⬜⬜⬜");
    }
    else if (per_hp <= 50)
    {
        printf("🟥🟥🟥🟥🟥⬜⬜⬜⬜⬜");
    }
    else if (per_hp <= 60)
    {
        printf("🟥🟥🟥🟥🟥🟥⬜⬜⬜⬜");
    }
    else if (per_hp <= 70)
    {
        printf("🟥🟥🟥🟥🟥🟥🟥⬜⬜⬜");
    }
    else if (per_hp <= 80)
    {
        printf("🟥🟥🟥🟥🟥🟥🟥🟥⬜⬜");
    }
    else if (per_hp <= 90)
    {
        printf("🟥🟥🟥🟥🟥🟥🟥🟥🟥⬜");
    }
    else if (per_hp <= 100)
    {
        printf("🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥");
    }
    printf("\n\n");
}

void mp_UI(int live_mp, float per_mp)
{
    
    if (live_mp == 0)
    {
        printf("⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜");
    }
    else if (per_mp <= 10)
    {
        printf("🟦⬜⬜⬜⬜⬜⬜⬜⬜⬜");
    }
    else if (per_mp <= 20)
    {
        printf("🟦🟦⬜⬜⬜⬜⬜⬜⬜⬜");
    }
    else if (per_mp <= 30)
    {
        printf("🟦🟦🟦⬜⬜⬜⬜⬜⬜⬜");
    }
    else if (per_mp <= 40)
    {
        printf("🟦🟦🟦🟦⬜⬜⬜⬜⬜⬜");
    }
    else if (per_mp <= 50)
    {
        printf("🟦🟦🟦🟦🟦⬜⬜⬜⬜⬜");
    }
    else if (per_mp <= 60)
    {
        printf("🟦🟦🟦🟦🟦🟦⬜⬜⬜⬜");
    }
    else if (per_mp <= 70)
    {
        printf("🟦🟦🟦🟦🟦🟦🟦⬜⬜⬜");
    }
    else if (per_mp <= 80)
    {
        printf("🟦🟦🟦🟦🟦🟦🟦🟦⬜⬜");
    }
    else if (per_mp <= 90)
    {
        printf("🟦🟦🟦🟦🟦🟦🟦🟦🟦⬜");
    }
    else if (per_mp <= 100)
    {
        printf("🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦");
    }
    printf("\n\n");
}


void state_check_UI(int gold, int live_exp, int max_exp, int live_hp, int max_hp, int live_mp, int max_mp, int added_stp, int added_str, int added_dex, int added_int_, int live_lv, int START_STR, int START_ATK, int added_atk, float START_EVA, float added_eva, int START_DEX, int START_MTK, int added_mtk, float START_CRI, float added_cri, int START_INT_, int live_def, int return_stp)
{
    float per_exp = ((live_exp*0.1) / max_exp) * 1000;
    float per_hp = ((live_hp*0.1) / max_hp) * 1000;
    float per_mp = ((live_mp*0.1) / max_mp) * 1000;
    added_stp = added_str + added_dex + added_int_;
    printf("=================================================================\n");
    printf("\t\t\t    [ 상태 ]\t\t %d[GOLD]\n", gold);
    printf("=================================================================\n");
    printf("Lv. %d\n\n", live_lv);
    printf("EXP %.2f %% (%d / %d)\t", per_exp, live_exp, max_exp);
    exp_UI(live_exp, per_exp);

    printf(" HP %.2f %% (%d / %d)\t", per_hp, live_hp, max_hp);
    hp_UI(live_hp, per_hp);

    printf(" MP %.2f %% (%d / %d)\t", per_mp, live_mp, max_mp);
    mp_UI(live_mp, per_mp);
    printf("------------------------------------------------------------------\n");
    printf("STR %d + %d\t|\tATK %d + %d\t|\tEVA %.2f + %.2f\n\n", START_STR, added_str, START_ATK ,added_atk, START_EVA, added_eva);
    printf("DEX %d + %d\t|\tMKT %d + %d\t|\tCRI %.2f + %.2f\n\n", START_DEX, added_dex, START_MTK ,added_mtk, START_CRI, added_cri);
    printf("INT %d + %d\t|\tDEF %d\t\t|\n\n", START_INT_, added_int_, live_def);
    printf("STP %d\t\t|\t\t\t|\n\n", return_stp);
    printf("------------------------------------------------------------------\n");
}


void mon_determine (int max_hp)
{   
    srand(time(NULL));

    int draw_2 = rand()%2;
    int draw_3 = rand()%3;
    int draw_10 = rand()%10;
    if (draw_10>7)
    {
        plag_named = 1;
    }
    if ((floor == 1)&&(plag_named==1))           //1층에 있으면
    {   
        mon_name_num = 0;
        monster_max_hp = (monster_fix_hp[0] + (rand() % monster_random_hp[0]))*2;    //오크전사 체력결정
        monster_max_ac = 0;   //오크전사 방어력 결정rand() %
        monster_max_atk = (monster_fix_atk[0] + (rand() % monster_random_atk[0]))*2;
    }
    else if ((floor ==2)&&(plag_named==1))          //3층에 있으면
    {
        mon_name_num = draw_2;
        if (mon_name_num ==0)
        {
            monster_max_hp = monster_fix_hp[0] + (rand() % monster_random_hp[0]);    //오크전사 체력결정
            monster_max_ac = 0;   //오크전사 방어력 결정rand() %
            monster_max_atk = monster_fix_atk[0] + (rand() % monster_random_atk[0]);
        }
        else
        {
            monster_max_hp = monster_fix_hp[draw_2] + (rand() % monster_random_hp[draw_2]);    //몬스터 체력결정
            monster_max_ac = monster_fix_ac[draw_2] + (rand() % monster_random_ac[draw_2]);   //몬스터 방어력 결정
            monster_max_atk = monster_fix_atk[draw_2] + (rand() % monster_random_atk[draw_2]);    //몬스터 공격력 결정
        }
    }
    else if (floor ==3&&(plag_named==1))          //3층에 있으면
    {
        mon_name_num = draw_2+1;
        monster_max_hp = monster_fix_hp[draw_2+1] + (rand() % monster_random_hp[draw_2+1]);    //몬스터 체력결정
        monster_max_ac = monster_fix_ac[draw_2+1] + (rand() % monster_random_ac[draw_2+1]);   //몬스터 방어력 결정
        monster_max_atk = monster_fix_atk[draw_2+1] + (rand() % monster_random_atk[draw_2+1]);    //몬스터공격력 결정
    }
    else if (floor ==4&&(plag_named==1))              //4층에 있으면
    {
        mon_name_num = draw_2+2;
        monster_max_hp = monster_fix_hp[draw_2+2] + (rand() % monster_random_hp[draw_2+2]);    //몬스터 체력결정
        monster_max_ac = monster_fix_ac[draw_2+2] + (rand() % monster_random_ac[draw_2+2]);   //몬스터 방어력 결정
        monster_max_atk = monster_fix_atk[draw_2+2] + (rand() % monster_random_atk[draw_2+2]);    //몬스터 공격력 결정
    }
    else if (floor ==5&&(plag_named==1))             //5층에 있으면
    {
        mon_name_num = draw_3+3;
        monster_max_hp = monster_fix_hp[draw_3+3] + (rand() % monster_random_hp[draw_3+3]);    //몬스터 체력결정
        monster_max_ac = monster_fix_ac[draw_3+3] + (rand() % monster_random_ac[draw_3+3]);   //몬스터 방어력 결정
        monster_max_atk = monster_fix_atk[draw_3+3] + (rand() % monster_random_atk[draw_3+3]);    //몬스터 공격력 결정
    }
    else if (floor == 1)             //1층에 있으면
    {   
        mon_name_num = 0;
        monster_max_hp = monster_fix_hp[0] + (rand() % monster_random_hp[0]);    //오크전사 체력결정
        monster_max_ac = 0;   //오크전사 방어력 결정rand() %
        monster_max_atk = monster_fix_atk[0] + (rand() % monster_random_atk[0]);
    }
    else if (floor ==2)          //3층에 있으면
    {
        mon_name_num = draw_2;
        if (mon_name_num ==0)
        {
            monster_max_hp = monster_fix_hp[0] + (rand() % monster_random_hp[0]);    //오크전사 체력결정
            monster_max_ac = 0;   //오크전사 방어력 결정rand() %
            monster_max_atk = monster_fix_atk[0] + (rand() % monster_random_atk[0]);
        }
        else
        {
            monster_max_hp = monster_fix_hp[draw_2] + (rand() % monster_random_hp[draw_2]);    //몬스터 체력결정
            monster_max_ac = monster_fix_ac[draw_2] + (rand() % monster_random_ac[draw_2]);   //몬스터 방어력 결정
            monster_max_atk = monster_fix_atk[draw_2] + (rand() % monster_random_atk[draw_2]);    //몬스터 공격력 결정
        }
    }
    else if (floor ==3)          //3층에 있으면
    {
        mon_name_num = draw_2+1;
        monster_max_hp = monster_fix_hp[draw_2+1] + (rand() % monster_random_hp[draw_2+1]);    //몬스터 체력결정
        monster_max_ac = monster_fix_ac[draw_2+1] + (rand() % monster_random_ac[draw_2+1]);   //몬스터 방어력 결정
        monster_max_atk = monster_fix_atk[draw_2+1] + (rand() % monster_random_atk[draw_2+1]);    //몬스터공격력 결정
    }
    else if (floor ==4)              //4층에 있으면
    {
        mon_name_num = draw_2+2;
        monster_max_hp = monster_fix_hp[draw_2+2] + (rand() % monster_random_hp[draw_2+2]);    //몬스터 체력결정
        monster_max_ac = monster_fix_ac[draw_2+2] + (rand() % monster_random_ac[draw_2+2]);   //몬스터 방어력 결정
        monster_max_atk = monster_fix_atk[draw_2+2] + (rand() % monster_random_atk[draw_2+2]);    //몬스터 공격력 결정
    }
    else if (floor ==5)             //5층에 있으면
    {
        if (map[user_y-1][user_x]==3 || map[user_y+1][user_x]==3 || map[user_y][user_x-1]==3 || map[user_y][user_x+1]==3)            //캐릭터 주위에 보스가 있을 시
        {
            mon_name_num = 6;
            monster_max_hp = monster_fix_hp[6] + (max_hp*monster_random_hp[6]);    //몬스터 체력결정
            monster_max_ac = monster_fix_ac[6] + (monster_random_ac[6]);   //몬스터 방어력 결정
            monster_max_atk = monster_fix_atk[6] + (rand() % monster_random_atk[6]);    //몬스터 공격력 결정
        }
        else if (map[user_y-1][user_x]==4 || map[user_y+1][user_x]==4 || map[user_y][user_x-1]==4 || map[user_y][user_x+1]==4)       //캐릭터 주위에 이동녀크가 있을 시
        {
            mon_name_num = 7;
            monster_max_hp = monster_fix_hp[7] + (max_hp*monster_random_hp[7]);    //몬스터 체력결정
            monster_max_ac = monster_fix_ac[7] + (monster_random_ac[7]);   //몬스터 방어력 결정
            monster_max_atk = monster_fix_atk[7] + (rand() % monster_random_atk[7]);    //몬스터 공격력 결정
        }
        else                    //나머지 일반몹
        {
            mon_name_num = draw_3+3;
            monster_max_hp = monster_fix_hp[draw_3+3] + (rand() % monster_random_hp[draw_3+3]);    //몬스터 체력결정
            monster_max_ac = monster_fix_ac[draw_3+3] + (rand() % monster_random_ac[draw_3+3]);   //몬스터 방어력 결정
            monster_max_atk = monster_fix_atk[draw_3+3] + (rand() % monster_random_atk[draw_3+3]);    //몬스터 공격력 결정
        }

    }
}

void map_change()    //맵이 바뀌어야 할 상황이면 넣어주세요
{
    plag_mapsave = 0;        //맵 저장하는 스위치
    plag_mapout = 0;         //맵을 꺼지게 하는 스위치
    map_input = '0';
}

void map_not_change()   //맵이 안바뀌고 그대로일 상황에 넣어주세요
{
    plag_mapsave = 1;        //맵 저장하는 스위치
    plag_mapout = 0;         //맵을 꺼지게 하는 스위치
    map_input = '0';
}