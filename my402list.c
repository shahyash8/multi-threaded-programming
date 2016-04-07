#include"my402list.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

/*
typedef struct data
{
char type[2];
unsigned long int timestamp;
unsigned long int amt;
char *desc;

//int num;
}data;
*/
//int My402ListTraverse(My402List*);
//void processfile(FILE *,My402List*);
//void ListDispTransac(My402List*);

//static void BubbleForward(My402List *, My402ListElem **, My402ListElem **);
//static void BubbleSortForwardList(My402List *pList, int num_items);


/*
int i=0;
int cnt;
printf("\nEnter the number of elements you want ");
scanf("%d",&cnt);

if(cnt!=0)
for(i=0;i<cnt;i++)
{
	data *dataptr=(struct data *)malloc(sizeof(struct data));
	printf("\nPlease Enter the element to be added to the list\n");	
	scanf("%d",&dataptr->num);
	My402ListPrepend(list,&dataptr->num);
	//My402ListAppend(list,&dataptr->num);
}

//My402ListTraverse(list);


My402ListElem *elem;

//First Element
elem=My402ListFirst(list);
if(elem!=NULL)
{
	int *numptr;
	numptr=(int *)elem->obj;
	printf("\nThe first element is %d\n",*numptr);
}

//Last Element
elem=My402ListLast(list);
if(elem!=NULL)
{
	int *numptr1;
	numptr1=(int *)elem->obj;
	if(*numptr1!=0)
	printf("\nThe last element is %d\n",*numptr1);
}

//Length
int len=0;
len=My402ListLength(list);
printf("\nThe length of the list is %d\n",len);

//EMPTY LIST
int flag=0;
flag=My402ListEmpty(list);
if(flag==1)
	printf("The List is empty..!!\n");
else
	printf("The list is not empty..!!\n");

*/
//Find Element
/*
int srch=0;
printf("\nEnter the element to be searched");
scanf("%d",&srch);

int *srchptr;
srchptr=&srch;


elem=My402ListFind(list,&list->anchor.next);
if(elem!=NULL)
{
int *numptr2;
numptr2=(int *)elem->obj;
printf("\nThe element found is %d\n",*numptr2);
}
*/

/*
//UNLINK ALL
My402ListUnlinkAll(list);
printf("All elements deleted successfully...\n");
My402ListTraverse(list);

*/
/*
//Delete Specific
My402ListUnlink(list,list->anchor.next->next);
printf("\nThe second element has been deleted succesffully...\n");
My402ListTraverse(list);

*/
/*
//Insert After
int n=100;
My402ListInsertAfter(list,&n,list->anchor.next);
printf("\n 100 inserted after the 1st element....\n");
My402ListTraverse(list);


//Insert Before
n=100;
My402ListInsertBefore(list,&n,list->anchor.next);
printf("\n 100 inserted before the 1st element....\n");
My402ListTraverse(list);
*/

/*
return 0;
}
*/
int My402ListInit(My402List* list)
{
	//list = malloc(sizeof(struct tagMy402List));

	if(list)
	{
		list->num_members=0;

		//My402ListElem* anchor=(struct tagMy402ListElem *)malloc(sizeof(struct tagMy402ListElem));
		
		//if(anchor)
		//{
			//list->anchor=*anchor;
			list->anchor.next=&(list->anchor);
			list->anchor.prev=&(list->anchor);
			list->anchor.obj=NULL;

			//printf("Init successful\n");
		//}
		//else 
		//	return FALSE;
	}
	else
		return FALSE;

	return TRUE;
}


//Original works for prepend
/*
int My402ListTraverse(My402List* list)
{
	My402ListElem *elem=NULL;
	int i=0;
	for(elem=list->anchor.next;i<5; elem=elem->next)
	{
		i++;
		int *data=NULL;
		data=(int *)elem->obj;
		printf("%d -> \t",*data);
	}
	return TRUE;
}
*/
/*
int My402ListTraverse(My402List* list)
{
	My402ListElem *elem=NULL;
	//int i=0;
	
	for(elem=My402ListFirst(list);elem!=NULL; elem=My402ListNext(list,elem))
	{
		//i++;
		int *data=NULL;
		data=(int *)elem->obj;
		printf("%d -> \t",*data);
	}
	return TRUE;
}
*/
/*
int  My402ListLength(My402List* list)
{
	int len=0;
	My402ListElem *elem=NULL;

        for (elem=My402ListFirst(list);elem != NULL;elem=My402ListNext(list, elem)) 
        {
         	len++;
            Foo *foo=(Foo*)(elem->obj);
        }

        return len;
}
*/

My402ListElem *My402ListFirst(My402List* list)
{
	My402ListElem *elem=NULL;

	if(list->anchor.next==&list->anchor)
		return NULL;
	else
	{
		elem=list->anchor.next;
		return elem;
	}	
}

My402ListElem *My402ListLast(My402List* list)
{
	
	My402ListElem *elem=NULL;
	
	if(list->anchor.prev==&list->anchor)
		return NULL;
	else
	{
		elem=list->anchor.prev;
		return elem;	
	}
}

int  My402ListLength(My402List* list)
{
	return list->num_members;
}


int  My402ListEmpty(My402List* list)
{
	if(list->anchor.next==&list->anchor && list->anchor.prev==&list->anchor)
		return TRUE;
	else
		return FALSE;
}

int  My402ListPrepend(My402List* list, void* obj)
{
	My402ListElem *elem=(struct tagMy402ListElem *)malloc(sizeof(struct tagMy402ListElem));

	if(list->anchor.next==NULL)
	{
		list->anchor.next=elem;
		list->anchor.prev=elem;
		elem->next=&list->anchor;
		elem->prev=&list->anchor;
		elem->obj=obj;	
	}
	else	
	{
		//list->anchor.next=elem;
		My402ListElem *elem1;
		elem1=list->anchor.next;

		elem->next=elem1;
		elem->prev=&list->anchor;
		elem->obj=obj;
		
		list->anchor.next=elem;
		elem1->prev=elem;
	}
	list->num_members++;
	return TRUE;
}

int  My402ListAppend(My402List* list, void* obj)
{
	My402ListElem *elem=(struct tagMy402ListElem *)malloc(sizeof(struct tagMy402ListElem));

	if(list->anchor.prev==NULL)
	{
		list->anchor.prev=elem;
		list->anchor.next=elem;
		
		elem->next=&list->anchor;
		elem->prev=&list->anchor;
		elem->obj=obj;	
	}

	else
	{
		My402ListElem *elem1=list->anchor.prev;
		
		elem->prev=elem1;
		elem->next=&list->anchor;
		elem->obj=obj;

		list->anchor.prev=elem;
		elem1->next=elem;
	}

	list->num_members++;
	return TRUE;
}

My402ListElem *My402ListNext(My402List* list, My402ListElem* elem)
{
	if(elem->next != &list->anchor)
		return elem->next;
	else
		return NULL;	
}

My402ListElem *My402ListPrev(My402List* list, My402ListElem* elem)
{
	if(elem->prev != &list->anchor)
		return elem->prev;
	else
		return NULL;	
}

My402ListElem *My402ListFind(My402List* list, void* obj)
{
	int flag=0;
	My402ListElem *elem=NULL;
	for(elem=My402ListFirst(list);elem!=NULL; elem=My402ListNext(list,elem))
	{
		if(elem->obj==obj)
		{
			flag=1;
			break;
		}
	}

	if(flag==1)
		return elem;
	else
		return NULL;
}

void My402ListUnlinkAll(My402List* list)
{
	My402ListElem *elem=NULL;
	My402ListElem *elem1=NULL;
	
	elem=My402ListFirst(list);
	while(elem!=NULL)
	{
		elem1=My402ListNext(list,elem);
		free(elem);		
		elem=elem1;
	}	
	
	list->anchor.next=NULL;
	list->anchor.prev=NULL;
	
	list->num_members=0;
}

void My402ListUnlink(My402List* list, My402ListElem* elem)
{
	//My402ListElem *cur=NULL;
	My402ListElem *prev=NULL;

	//cur=My402ListFirst(list);
	//while(cur!=elem)
	//	cur=My402ListNext(list,cur);		
	
	prev=elem->prev;
	prev->next=elem->next;
	elem->next->prev=prev;

	list->num_members--;
	free(elem);
}

int  My402ListInsertAfter(My402List* list, void* obj, My402ListElem* elem)
{
	My402ListElem *cur=(struct tagMy402ListElem *)malloc(sizeof(struct tagMy402ListElem));
	if(elem==NULL)
	{
		My402ListAppend(list,obj);
		return TRUE;
	}
	else
	{	
		cur->next=elem->next;
		cur->prev=elem;
		cur->obj=obj;
		elem->next->prev=cur;
		elem->next=cur;
		list->num_members++;
		return TRUE;
	}	
	return FALSE;
}

int  My402ListInsertBefore(My402List* list, void* obj, My402ListElem* elem)
{
	My402ListElem *cur=(struct tagMy402ListElem *)malloc(sizeof(struct tagMy402ListElem));
	if(elem==NULL)
	{
		My402ListPrepend(list,obj);
		return TRUE;		
	}	
	else
	{	
		cur->next=elem;
		cur->prev=elem->prev;
		cur->obj=obj;
		elem->prev->next=cur;
		elem->prev=cur;

		list->num_members++;
		return TRUE;
	}	
	return FALSE;
}




