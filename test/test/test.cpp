#include <iostream>
#include <Windows.h>
#include <thread>
#include <math.h>
#include <vector>
#include <mutex>
#include <random>
#include <shlobj.h>
#include <signal.h>
#include <tchar.h>
using namespace std;
#pragma comment(lib, "version.lib")

enum FLAT_TYPE
	{
		// 등록 후 아래 정각에 따라 발동 ( 밀리초 )
		EVERY_1_SECOND	 = 1 * 1000,
		EVERY_5_SECOND	 = EVERY_1_SECOND*5,
		EVERY_10_SECOND  = EVERY_1_SECOND*10,
		EVERY_15_SECOND  = EVERY_1_SECOND*15,
		EVERY_20_SECOND  = EVERY_1_SECOND*20,
		EVERY_30_SECOND  = EVERY_1_SECOND*30,

		EVERY_1_MINUTE	 = EVERY_1_SECOND*60,
		EVERY_5_MINUTE	 = EVERY_1_MINUTE*5,
		EVERY_10_MINUTE  = EVERY_1_MINUTE*10,
		EVERY_15_MINUTE  = EVERY_1_MINUTE*15,
		EVERY_20_MINUTE  = EVERY_1_MINUTE*20,
		EVERY_30_MINUTE  = EVERY_1_MINUTE*30,

		EVERY_1_HOUR	 = EVERY_1_MINUTE*60,
		EVERY_2_HOUR	 = EVERY_1_HOUR*2,
		EVERY_3_HOUR	 = EVERY_1_HOUR*3,
		EVERY_4_HOUR	 = EVERY_1_HOUR*4,
		EVERY_6_HOUR	 = EVERY_1_HOUR*6,
		EVERY_8_HOUR	 = EVERY_1_HOUR*8,
		EVERY_12_HOUR	 = EVERY_1_HOUR*12,
		EVERY_24_HOUR	 = EVERY_1_HOUR*24,
	};

DWORD CalcFirstRemainMSec( const FLAT_TYPE flat_type )
{
	SYSTEMTIME t;

	GetLocalTime( &t );

	WORD cur_msec = t.wMilliseconds;
	WORD cur_sec  = t.wSecond;
	WORD cur_min  = t.wMinute;
	WORD cur_hour = t.wHour;

	DWORD remain_msec = 0;
	DWORD remain_sec = 0;
	DWORD remain_min = 0;
	DWORD remain_hour = 0;

	switch( flat_type )
	{
	case EVERY_1_SECOND:
	case EVERY_5_SECOND:
	case EVERY_10_SECOND:
	case EVERY_15_SECOND:
	case EVERY_20_SECOND:
	case EVERY_30_SECOND:
		{
			remain_sec = (flat_type/1000 - (cur_sec % (flat_type/1000) )) - 1;
			remain_msec = 1000 - cur_msec;			
		}
		break;

	case EVERY_1_MINUTE:
	case EVERY_5_MINUTE:
	case EVERY_10_MINUTE:
	case EVERY_15_MINUTE:
	case EVERY_20_MINUTE:
	case EVERY_30_MINUTE:
		{
			remain_min = (flat_type/60000 - ( cur_min % (flat_type/60000) )) - 1;
			remain_sec = 60 - cur_sec - 1;
			remain_msec = 1000 - cur_msec;
		}
		break;

	case EVERY_1_HOUR:
	case EVERY_2_HOUR:
	case EVERY_3_HOUR:
	case EVERY_4_HOUR:
	case EVERY_6_HOUR:
	case EVERY_8_HOUR:
	case EVERY_12_HOUR:
	case EVERY_24_HOUR:
		{
			remain_hour = (flat_type/3600000 - ( cur_hour % (flat_type/3600000) )) - 1;
			remain_min = 60 - cur_min - 1;
			remain_sec = 60 - cur_sec - 1;
			remain_msec = 1000 - cur_msec;
		}
		break;
	}

	return (remain_msec + remain_sec*1000 + remain_min*60000 + remain_hour*3600000);
}


void thread1 ()
{
    for(int i = 0; i < 30; i++)
    {
        cout << "Thread1" << endl;
    }
}
void thread2 ()
{
    for(int i = 0; i < 30; i++)
    {
        cout << "Thread2" << endl;
    }
}

INT64 GetAbsDay( SYSTEMTIME st )
{
	INT64 n64Day;
	FILETIME fst;

	st.wHour = st.wMinute = st.wSecond = st.wMilliseconds = st.wDayOfWeek = 0;
	SystemTimeToFileTime( &st, &fst );

	n64Day = ( ( (INT64)fst.dwHighDateTime ) << 32 ) + fst.dwLowDateTime;

	n64Day = n64Day / 10000000 - (INT64)145731 * 86400;

	return n64Day;
}

int GetTimeLeft( const char* szDate )
{
	char cpYear[8] = "";
	memcpy( cpYear, szDate, 4 );

	char cpMonth[8] = "";
	memcpy( cpMonth, szDate + 4, 2 );

	char cpDay[8] = "";
	memcpy( cpDay, szDate + 6, 2 );

	SYSTEMTIME st;
	GetLocalTime( &st );
	DWORD dwNowSec = (DWORD)GetAbsDay( st );

	st.wYear = static_cast<WORD>( atoi( cpYear ) );
	st.wMonth = static_cast<WORD>( atoi( cpMonth ) );
	st.wDay = static_cast<WORD>( atoi( cpDay ) );	 

	return (int)( ( GetAbsDay( st ) - dwNowSec) / 86400 );		
}

struct userInfo {
    int left;
    int right;
};
 
int fork[5] = { 1,1,1,1,1 };
userInfo user[5] = { -1, }; 
mutex myMutex;

void func(int id) {
    while (1) {
        myMutex.lock();
        if (fork[id] && user[id].left == -1) {
            printf("id :: %d이(가) 왼손에 포크 :: %d 잡음\n", id, id);
            fork[id] = 0;
            user[id].left = id;
        }
        myMutex.unlock();
        myMutex.lock();
        if (fork[(id + 1) % 5] && user[id].right == -1) {
            printf("id :: %d이(가) 오른손에 포크 :: %d 잡음\n", id, (id + 1) % 5);
            fork[(id + 1) % 5] = 0;
            user[id].right = (id + 1) % 5;
            if (user[id].left >= 0 && user[id].right >= 0) {
                printf("id :: %d이(가) 포크 %d, %d 반납\n", id, user[id].left, user[id].right);
                fork[user[id].left] = 1;
                fork[user[id].right] = 1;
                user[id].left = user[id].right = -1;
            }
        }
        myMutex.unlock();
    }
}

void test(int*& c, int* a)
{
    c = a;
}

//typedef struct Node
//{
//    int data;
//    Node* prev;
//    Node* next;
//}Node;
//typedef struct List
//{
//    Node* head;
//    Node* tail;
//    int count;
//}List;
//
//void InitList(List **list)
//{
//    (*list) = (List*)malloc(sizeof(list));
//    (*list)->head = NULL;
//    (*list)->tail = NULL;
//    (*list)->count = 0;
//}
//
//Node* CreateNode(int data)
//{
//    Node* node = (Node*)malloc(sizeof(Node));
//    node->data = data;
//    node->prev = nullptr;
//    node->next = nullptr;
//    return node;
//}
//
//void AddNode(List **list, Node* node)
//{
//    if (!(*list) || !node) return;
//    cout << "Add Node - data: " << node->data << endl;
//
//    if ((*list)->count == 0)
//    {
//        (*list)->head = (*list)->tail = node;
//        node->next = node->prev = node;
//    }
//    else
//    {
//        node->prev = (*list)->tail;
//        node->next = (*list)->head;
//    }
//}

template<class T>
struct Node
{
    T  data;
    Node<T>* next;
};

template<class T>
struct List
{
    Node<T>* head;
    Node<T>* tail;
    int count;
};

template<class T>
void Initlist(List<T> **list)
{
    *list = new List<T>;
    (*list)->head = nullptr;
    (*list)->tail = nullptr;
    (*list)->count = 0;
}

template<class T>
Node<T>* CreateNode(T data)
{
    Node<T>* node = new Node<T>;
    node->data = data;
    node->next = nullptr;

    return node;
}

template<class T>
void AddNode(List<T> **list, Node<T>* node)
{
    if(!(*list) || !node) return;

    if((*list)->count == 0)
    {
        (*list)->head = node;
        node->next = nullptr;
        (*list)->tail = node;
    }
    else
    {
        (*list)->tail->next = node;
        node->next = nullptr;
        (*list)->tail = node;
        if((*list)->count == 1)
            (*list)->head->next = node;
    }
    (*list)->count++;
}

int main()
{
    List<int>* list;
    Initlist(&list);
    for(int i = 0; i< 5; i++)
        AddNode(&list, CreateNode(i));
    
    while(list->count != 0)
    {
        Node<int>* temp = list->head->next;
        delete list->head;
        list->head = nullptr;
        list->head= temp;
        list->count--;
    }

    if(list->count == 0)
    {
        list->tail = nullptr;
        delete list;
        list = nullptr;
    }

    std::string strSourceIn = "df[/dkjsggrg67567/hn5dfd,reward_rid_rid]";
    vector<std::string> strVec;
    std::string::size_type HeadPos = 0;
    std::string::size_type NextPos = 0;
    std::string::size_type retval = 0;

    while(retval != std::string::npos)
    {
        retval = strSourceIn.find_first_of(']', HeadPos);
        if(std::string::npos == retval)
        {
            NextPos = strSourceIn.size();
        }
        else
        {
            NextPos = retval;
        }
        std::string strTemp(strSourceIn.substr(HeadPos, NextPos - HeadPos));

        HeadPos = retval +1;

        if(strTemp.empty())
            continue;
        strVec.push_back(strTemp);        
    }
    



    Sleep(5000);
    return 0;
}

    //LARGE_INTEGER counter[2], frequency;
    //QueryPerformanceFrequency(&frequency);
    //QueryPerformanceCounter(&counter[0]);
    //Sleep(1520);
    //QueryPerformanceCounter(&counter[1]);
    //float time = (float)(counter[1].QuadPart - counter[0].QuadPart) / (float)frequency.QuadPart;
    //printf("time: %f", time);
    //Sleep(2000);

    //LARGE_INTEGER counter[2];
    //vector<int> vec;
    //vec.push_back(5);
    //vec.push_back(200);
    //vec.push_back(10000);
    //std::random_device rd;
    //std::mt19937_64 rng{ rd()};
    ////std::uniform_int_distribution<unsigned int> rng2(1, 10205);
    //int a[3] = {0,};
    //vector<long long> vecdb;
    //vecdb.reserve(1000);
    //for (int l = 0; l < 1000; l++)
    //{
    //    unsigned int j = rng2(rd);
    //    unsigned int acc = 0;
    //    QueryPerformanceCounter(&counter[0]);
    //    for (auto& per : vec)
    //    {
    //        acc += per;
    //        if (j <= acc)
    //        {
    //            if(5 == per)
    //                ++a[0];
    //            else if(200 == per)
    //                ++a[1];
    //            else if(10000 == per)
    //                ++a[2];
    //            QueryPerformanceCounter(&counter[1]);                    
    //            vecdb.push_back(counter[1].QuadPart - counter[0].QuadPart);
    //            break;
    //        }
    //    }
    //}    
    //std::discrete_distribution<size_t> dist = {vec.cbegin(), vec.cend()};
    //int e = 0;
    //for(int i = 0; i< 1000; i++)
    //{
    //    QueryPerformanceCounter(&counter[0]);
    //    e = dist(rng);
    //    if(0 == e)
    //    {
    //        ++a[0];
    //        QueryPerformanceCounter(&counter[1]);                    
    //        vecdb.push_back(counter[1].QuadPart - counter[0].QuadPart);
    //    }
    //    else if(1 == e)
    //    {
    //        ++a[1];
    //        QueryPerformanceCounter(&counter[1]);                    
    //        vecdb.push_back(counter[1].QuadPart - counter[0].QuadPart);
    //    }
    //    else
    //    {
    //        ++a[2];
    //        QueryPerformanceCounter(&counter[1]);                    
    //        vecdb.push_back(counter[1].QuadPart - counter[0].QuadPart);
    //    }
    //}
    //long long dfdf = 0;
    //for(auto per : vecdb)
    //    dfdf += per;
    //dfdf = dfdf / vecdb.size();
    //printf("avg times: %d", dfdf);
    //printf("select a, %d times\nselect b, %d times\nselect c, %d tiems\n", a[0], a[1], a[2]);    
    //int d = 0;
    //scanf_s("%d", &d);