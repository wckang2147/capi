GSList* list = NULL; // 선언, 생성 
list = g_slist_append(list, "first"); // 아이템을 뒤에 추가  

list = g_slist_prepend(list, "first"); // 아이템을 맨 처음에 추가  
list = g_slist_remove(list, "first"); // 아이템 한 개삭제 
list = g_slist_remove_all(list, "third"); // 동일한 아이템 모두 삭제 
g_slist_length(list); // 리스트 길이 구하기 
list->data; // 첫 번째 아이템 가져오기 
g_slist_last(list)->data; // 마지막 아이템 가져오기 
g_slist_nth_data(list, 1); // n번째 아이템 가져오기, 여기서는 두 번째 아이템(0 base) 
g_slist_next(list)->data; // 두 번째 아이템 가져오기 
g_slist_free(list); // 메모리 해제 

typedef struct {
    char* name;
    int shoe_size;
} Person;
Person* fred = g_new(Person, 1); // Person스트럭처 1개 메모리 할당 Person* fred = (Person*)malloc(sizeof(Person)); 

GSList* both = g_slist_concat(list1, list2); // GSList 2개를 합친다 
GSList* reversed = g_slist_reverse(both); // 리스트를 뒤집는다. 원래 리스트의 마지막 아이템이 처음(reversed->data)으로 간다.  

[Simple Iteration]
GSList* list = NULL, * iterator = NULL;
for (iterator = list; iterator; iterator = iterator->next) { // 리스트 탐색 
    printf("Current item is '%s'\n", iterator->data);
}

[sort]
gint my_comparator(gconstpointer item1, gconstpointer item2) {
    return g_ascii_strcasecmp(item1, item2);
}
list = g_slist_sort(list, (GCompareFunc)my_comparator);

[Find Element]
1.
GSList * item = g_slist_find(list, "second");
printf("This should be the 'second' item: '%s'\n", item->data);

2.
gint my_finder(gconstpointer item) {
    return g_ascii_strcasecmp(item, "second");
}
item = g_slist_find_custom(list, NULL, (GCompareFunc)my_finder);
printf("Again, this should be the 'second' item: '%s'\n", item->data);

[insert]
g_slist_insert(list, "Boston ", 1);
list = g_slist_insert_before(list, g_slist_nth(list, 2), "Chicago ");
list = g_slist_insert_sorted(list, "Denver ", (GCompareFunc)g_ascii_strcasecmp);


#include <gmodule.h>



// List 사용법 (기본 입출력)

static void itr_print(gpointer value, gpointer user_data) {  // 반복출력 함수
    printf("%s\n", (char*)value);
}

void main()
{
    int i;
    char* pTemp;

    GPtrArray* al = g_ptr_array_new(); // GPterArray 포인터 할당

    g_ptr_array_add(al, g_strdup("MMMM MMMM"));  // 데이터 입력
    g_ptr_array_add(al, g_strdup("BBB BBB"));
    g_ptr_array_add(al, g_strdup("CC CC"));
    g_ptr_array_add(al, g_strdup("BBB DDD"));
    g_ptr_array_add(al, g_strdup("KKK KKK"));


    g_ptr_array_foreach(al, (GFunc)itr_print, NULL); // 출력 방식(전체출력)
    printf("\n");

    g_ptr_array_remove_index(al, 2); // 3번째 (Index 2) 데이터 삭제


    for (i = 0; i < al->len; i++) // 출력 방식(개별출력)
    {
        pTemp = g_ptr_array_index(al, i);
        printf("%s\n", pTemp);
    }

    g_ptr_array_free(al, TRUE); // 메모리 해제 
}


----List 정렬----------------------------------------------------------------------------

static int compare_name(gpointer a, gpointer b) { // 정렬 비교(오름차순)
    int* x = (int*)a; int* y = (int*)b;
    return strcmp((char*)*x, (char*)*y);
}

static int compare_name_reverse(gpointer a, gpointer b) { // 정렬 비교(내림차순)
    int* x = (int*)a; int* y = (int*)b;
    return strcmp((char*)*y, (char*)*x);
}

static void itr_print(gpointer value, gpointer user_data) {  // 반복출력 함수
    printf("%s\n", (char*)value);
}

void main(void)
{
    GPtrArray* al = g_ptr_array_new(); // GPterArray 포인터 할당

    g_ptr_array_add(al, g_strdup("MMMM MMMM"));  // 데이터 입력
    g_ptr_array_add(al, g_strdup("BBB BBB"));
    g_ptr_array_add(al, g_strdup("CC CC"));
    g_ptr_array_add(al, g_strdup("BBB DDD"));
    g_ptr_array_add(al, g_strdup("KKK KKK"));



    g_ptr_array_sort(al, (gpointer)compare_name); // 정렬시 비교방법 선택
    g_ptr_array_foreach(al, (GFunc)itr_print, NULL); // 출력 방식(전체출력)
    printf("\n");



    g_ptr_array_sort(al, (gpointer)compare_name_reverse); // 정렬시 내림차순 선택
    g_ptr_array_foreach(al, (GFunc)itr_print, NULL); // 출력 방식(전체출력)

    g_ptr_array_free(al, TRUE);
}



---- - 여러 데이터 정렬----------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <glib.h>

typedef struct _DATA {
    int item;
    char* name;
    int price;
    double value;
} DATA;

int compare_items(gpointer, gpointer);
void printAll(gpointer, gpointer);

DATA data[] = { {7143, "aaa", 1231234, 235.5},
                {5231, "bbb", 532458, 238.75},
                {8751, "ccc", 872654, 125.47},
                {2354, "ddd", 87542, 564.4} };

int main(void)
{
    GPtrArray* a = g_ptr_array_new();
    g_ptr_array_add(a, &data[0]);
    g_ptr_array_add(a, &data[1]);
    g_ptr_array_add(a, &data[2]);
    g_ptr_array_add(a, &data[3]);

    g_ptr_array_sort(a, (GCompareFunc)compare_items);

    g_ptr_array_foreach(a, printAll, (gpointer)NULL);

    return 0;
}

gint compare_items(gpointer a, gpointer b) {

    DATA* alpha = *(DATA**)a;
    DATA* beta = *(DATA**)b;

    return (gint)(alpha->item - beta->item);
}

void printAll(gpointer a, gpointer b) {
    DATA* alpha = (DATA*)a;
    printf("item : %d, name : %s, price : %d, value : %lf\n", alpha->item, alpha->name, alpha->price, alpha->value);
    return;
}