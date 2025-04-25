#include <stdio.h>

void map_change();  //맵이 바뀌어야 할 상황이면 넣어주세요
void map_not_change();  //맵이 안바뀌고 그대로일 상황에 넣어주세요

int main()
{
    


    return 0;
}

void map_change()
{
    plag_mapsave = 0;        //맵 저장하는 스위치
    plag_mapout = 0;         //맵을 꺼지게 하는 스위치
    map_input = '0';
}

void map_not_change()
{
    plag_mapout = 0;         //맵을 꺼지게 하는 스위치
    map_input = '0';
}