#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <ctype.h>

#include "cs402.h"
#include "my402list.h"

typedef struct data
{
int id;
double lambda;
int P;
double mu;
char arr_time[20];

char q1_arr_time[20];
char q1_exit_time[20];

char q2_arr_time[20];
char q2_exit_time[20];

char s_arr_time[20];
char s_exit_time[20];

}data;

pthread_t id_arr;
pthread_t id_token;
pthread_t id_server1;
pthread_t id_server2;
pthread_t id_sighandler;

char prev_arr_time[20];

int dropped_packets=0;
int dropped_tokens=0;

int tokens=0;

double avg_interval_time,avg_service_time,avg_packets_in_q1,avg_packets_in_q2,avg_packets_in_s1,avg_packets_in_s2;
double avg_time_in_system,avg_sq_time_in_sys,std_deviation;

double token_drop_probability;
double packet_drop_probability;

int nopacket=0;

int tokenthreadexit=0;

int server1dead=0;
int server2dead=0; 

double lambda;
double mu;
int P;
double r=(double)1.5;
int num=20;
int B=10;

int q2empty=0;
//mode=0 deterministic mode
//mode=1 trace/file mode
int mode=0;

int shutdown=0;

char curtime[20];
struct timeval tv,tv1,tv2,tv3,tv4;

double start_time,end_time;

sigset_t set;

//q1 list
My402List *q1list;

//q2 list
My402List *q2list;

//mutex
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

//condition variable
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

//no_of tokens
int num_of_tokens=0;

//packets produced
int num_of_packets=0;

int server1_processing=0;
int server2_processing=0;

int signalactive=0;

int completed_packets=0;

void getpacketparameters(int cnt,FILE *fp)
{
	int i=0,j=0;
	char buff[1024];
	char templambda[30];
	char tempmu[30];
	char tempP[10];


	//set lambda depending on mode
	if(mode==1 && fp!=NULL)
	{
		if(cnt<=num)
		{
			//printf("In trace mode..!!\n");
			if(fgets(buff, sizeof(buff), fp) != NULL)
			{
				i=0;
				j=0;
				while(buff[i]!='\t' && buff[i]!=' ')
					templambda[j++]=buff[i++];

				templambda[j]='\0';

				//printf("templambda - %s\n",templambda);

				while(buff[i]=='\t' || buff[i]==' ')
					i++;

				j=0;
				while(buff[i]!='\t' && buff[i]!=' ')
					tempP[j++]=buff[i++];

				tempP[j]='\0';

				//printf("tempP - %s\n",tempP);

				while(buff[i]=='\t' || buff[i]==' ')
					i++;

				j=0;
				while(buff[i]!='\t' && buff[i]!=' ' && buff[i]!='\n')
					tempmu[j++]=buff[i++];

				tempmu[j]='\0';

				//printf("tempmu - %s\n",tempmu);
				
				sscanf(templambda,"%lf",&lambda);
				sscanf(tempP,"%d",&P);
				sscanf(tempmu,"%lf",&mu);

				//printf("templambda - %lf\n",lambda);
				//printf("tempP - %d\n",P);
				//printf("tempmu - %lf\n",mu);

			}
		}
	}
}


void gettime()
{
	gettimeofday(&tv1,NULL);

	//printf("\nTime end: tv_sec %ld\n",tv1.tv_sec);
	//printf("\nTime end: tv_usec %ld\n",tv1.tv_usec);

	double elapsed = ((tv1.tv_sec-tv.tv_sec)*(double)1000);

	elapsed +=((tv1.tv_usec-tv.tv_usec)/(double)1000);

	//printf("\nThe elapsed time is : %.3lf\n",elapsed);

	sprintf(curtime,"%lf",elapsed);

	int len=strlen(curtime);
	curtime[len-3]='\0';

	len=strlen(curtime);

	int z=0;
	int i=12-len;

	char temptime[20];
	while(z<i)
		temptime[z++]='0';

	temptime[z]='\0';

	//printf("\nThe elapsed time is : %sms:\n",temptime);

	strcat(temptime,curtime);
	strcpy(curtime,temptime);

	//printf("\nThe elapsed time is : %sms:\n",curtime);
}


//Arrival Thread
void *arrival_thread(void *argv)
{
	
	//printf("Arrival thread started..\n");
	
	FILE *fp=argv;
	int cnt=1;
	char arrival_time[20];
	char time_difference[20];
	

	//set packet parameters 
	double templambda;
	//while loop
	while(1)
	{

		getpacketparameters(cnt,fp);

		if(mode==1)
		{
			templambda=lambda;
			templambda/=1000;
		}
		else
			templambda=1/lambda;


		if(templambda>10)
			templambda=10;

		long l=templambda;
		//printf("\nl %d\n",l);
		long ul=(templambda-l)*1000000;

		//printf("%lf\n",templambda);
		//printf("%ld\n",l);
		//printf("%ld\n",ul);

		tv3.tv_sec=l;
		tv3.tv_usec=ul;

		//arrival time adjustment
		double z1,z2,z3;

		gettime();
		sscanf(prev_arr_time,"%lf",&z1);
		sscanf(curtime,"%lf",&z2);

		z3=z2-z1;
		z3/=1000;

		if(z3<templambda)
		{
		long z=(templambda-z3);
		//printf("\nl %d\n",l);
		long uz=((templambda-z3)-z)*1000000;

		tv3.tv_sec=z;
		tv3.tv_usec=uz;
		
		//sleep till packet is ready to arrive
		select(FD_SETSIZE,NULL,NULL,NULL,&tv3);
		}

		//if(z3>)
	
		pthread_mutex_lock(&mutex);
		//printf("\nMutex lock obtained by packet thread\n");


		//if(!My402ListEmpty(q1list))
		//{
			gettime();

			double t1,t2;
			sscanf(prev_arr_time,"%lf",&t1);
			sscanf(curtime,"%lf",&t2);

			t1=t2-t1;

			avg_interval_time+=t1;
			sprintf(arrival_time,"%.3lf",t1);
		//}	
		//else
		/*{
			gettime();

			double tme;
			sscanf(curtime,"%lf",&tme);
			avg_interval_time+=tme;
			sprintf(arrival_time,"%.3lf",tme);		
		}*/
	
	if(P>B)
	{
		printf("%sms: p%d arrives, needs %d tokens, inter-arrival time = %sms, dropped\n",curtime,cnt,P,arrival_time);
		dropped_packets++;
		cnt++;
		num_of_packets++;
	}

	else
	{
	
	printf("%sms: p%d arrives, needs %d tokens, inter-arrival time = %sms\n",curtime,cnt,P,arrival_time);
	

	strcpy(prev_arr_time,curtime);

	data *packet=(struct data *)malloc(sizeof(struct data));
	packet->id=cnt;
	packet->lambda=lambda;
	packet->P=P;
	packet->mu=mu;

	//timestamp packet arrival time
	strcpy(packet->arr_time,curtime);

	//critical section

	gettime();
	My402ListAppend(q1list,packet);

	printf("%sms: p%d enters Q1\n",curtime,cnt);
	strcpy(packet->q1_arr_time,curtime);
	cnt++;
	num_of_packets++;

	if(!My402ListEmpty(q1list))
	{
		My402ListElem *firstpacket=NULL;
		firstpacket=My402ListFirst(q1list);

		//printf("The q1 status check is %d\n",q1status);

		data *firstq1packet=firstpacket->obj;
	
		if(firstq1packet->P <= num_of_tokens)
		{
			num_of_tokens-=firstq1packet->P;

			gettime();
			strcpy(firstq1packet->q1_exit_time,curtime);

			double t3,t4;
			sscanf(firstq1packet->q1_arr_time,"%lf",&t3);
			sscanf(firstq1packet->q1_exit_time,"%lf",&t4);

			t4=t4-t3;
			avg_packets_in_q1+=t4;

			sprintf(time_difference,"%0.3lf",t4);

			My402ListUnlink(q1list,firstpacket);
			printf("%sms: p%d leaves Q1, time in Q1 = %sms, token bucket now has %d tokens\n",curtime,firstq1packet->id,time_difference,num_of_tokens);

			gettime();
			strcpy(firstq1packet->q2_arr_time,curtime);

			if(My402ListEmpty(q2list))
				q2empty=1;

			My402ListAppend(q2list,firstq1packet);
			
			if(q2empty==1)
					pthread_cond_broadcast(&cv);

			q2empty=0;


			printf("%sms: p%d enters Q2\n",curtime,firstq1packet->id);

		}
	
		
			}
	}

	//printf("\nMutex unlocked by packet thread\n");

	pthread_mutex_unlock(&mutex);

	if(cnt>num)
	{
		nopacket=1;
		pthread_exit(NULL);
	}	
}	
	pthread_exit(NULL);
	//return 0;
}


//Token deposit thread
void *token_deposit_thread()
{
	//printf("Token_deposit_thread started\n");

	//token count
	int cnt=0;
	char time_difference[20];

	double tempr=1/r;
	
	if(tempr>10)
		tempr=10;

	long rate=tempr;
	//printf("\nl %d\n",l);


	long urate=(tempr-rate)*1000000;

	//printf("%lf\n",tempr);
	//printf("%ld\n",rate);
	//printf("%ld\n",urate);

	
	while(1)
	{
		tv2.tv_sec=rate;
		tv2.tv_usec=urate;

		//printf("sleeping\n");
		//sleep till packet is ready to arrive
		select(FD_SETSIZE,NULL,NULL,NULL,&tv2);
		//printf("awake\n");

		pthread_mutex_lock(&mutex);
		//printf("\nMutex lock obtained by token thread\n");

		//critical section code
		if(num_of_tokens<B)
		{
			gettime();
			cnt++;
			tokens++;
			
			num_of_tokens++;

			
			printf("%sms: token t%d arrives, token bucket now has %d tokens\n",curtime,cnt,num_of_tokens);
		}
		else
		{
			gettime();
			cnt++;
			tokens++;

			dropped_tokens++;
			printf("%sms: token t%d arrives, dropped\n",curtime,cnt);
		}

		if(!My402ListEmpty(q1list))
		{
			My402ListElem *firstpacket=NULL;
			firstpacket=My402ListFirst(q1list);

			//printf("The q1 status check is %d\n",q1status);

			data *firstq1packet=firstpacket->obj;

			//printf("\n\nthe packet->p is %d\n\n",firstq1packet->P);
			if(firstq1packet->P <= num_of_tokens)
			{
				num_of_tokens-=firstq1packet->P;

				gettime();
				strcpy(firstq1packet->q1_exit_time,curtime);

				double t3,t4;
				sscanf(firstq1packet->q1_arr_time,"%lf",&t3);
				sscanf(firstq1packet->q1_exit_time,"%lf",&t4);

				t4=t4-t3;
				avg_packets_in_q1+=t4;

				sprintf(time_difference,"%0.3lf",t4);

				My402ListUnlink(q1list,firstpacket);
				printf("%sms: p%d leaves Q1, time in Q1 = %sms, token bucket now has %d tokens\n",curtime,firstq1packet->id,time_difference,num_of_tokens);

				gettime();
				strcpy(firstq1packet->q2_arr_time,curtime);
				
				if(My402ListEmpty(q2list))
						q2empty=1;                       

				My402ListAppend(q2list,firstq1packet);
				
				if(q2empty==1)
				{
					//printf("broadcast to servers\n");
					pthread_cond_broadcast(&cv);
				}

				q2empty=0;

				printf("%sms: p%d enters Q2\n",curtime,firstq1packet->id);
			}
		}
	
		//printf("\nMutex unlocked by token thread\n");

		pthread_mutex_unlock(&mutex);

		if(My402ListEmpty(q1list) && nopacket==1)
		{

			//tokens=cnt;
			tokenthreadexit=1;

			pthread_mutex_lock(&mutex);
			if(My402ListEmpty(q2list))
			{
				shutdown=1;
				pthread_cond_broadcast(&cv);
			}
			pthread_mutex_unlock(&mutex);
			//if()
			//shutdown=1;
			//pthread_cond_broadcast(&cv);
			//printf("\n\nTokens - %d\n",tokens);
			//printf("\n Dropped-tokens - %d\n",dropped_tokens);
			pthread_exit(NULL);
		}
	}
	pthread_exit(NULL);
}


//cleanup function
void cleanup(void * arg)
{
	//printf("\n\nserver cleanup thread\n");
}

//server1 thread
void *server1_thread()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	//pthread_cleanup_push(cleanup, NULL);

	char time_difference[20];
	char time_in_system[20];

	double tempmu;

	long serv_time;
	//printf("\n 2 \n");

	long userv_time;

	while(1)
	{

		tv4.tv_sec=serv_time;
		tv4.tv_usec=userv_time;

		pthread_mutex_lock(&mutex);

		//printf("1\n");
		while(My402ListEmpty(q2list) && shutdown==0)
		{
			//printf("\n\nserver 1 in wait..!!\n");
			pthread_cond_wait(&cv,&mutex);
			//if(shutdown==1)
			//	pthread_exit(NULL);
		}

		//if(shutdown==1)
		//	break;
		//printf("\nOutside if part server 1..!!\n");
		
		if(!My402ListEmpty(q2list) && shutdown==0)
		{
			My402ListElem *firstpacket=NULL;
			firstpacket=My402ListFirst(q2list);

			data *firstq2packet=firstpacket->obj;

			gettime();
			strcpy(firstq2packet->q2_exit_time,curtime);

			double t3,t4;
			sscanf(firstq2packet->q2_arr_time,"%lf",&t3);
			sscanf(firstq2packet->q2_exit_time,"%lf",&t4);

			t4=t4-t3;
			avg_packets_in_q2+=t4;

			sprintf(time_difference,"%0.3lf",t4);

			My402ListUnlink(q2list,firstpacket);

			printf("%sms: p%d leaves Q2, time in Q2 = %sms\n",curtime,firstq2packet->id,time_difference);

			gettime();
			strcpy(firstq2packet->s_arr_time,curtime);
			pthread_mutex_unlock(&mutex);

			//double tme;
			//sscanf(,"%lf",&tme);
			//sprintf(arrival_time,"%.3lf",tme);	
			if(mode==1)
			{
				tempmu=firstq2packet->mu;
				tempmu/=1000;
			}	
			else
				tempmu=1/(firstq2packet->mu);

			if(tempmu>10)
				tempmu=10;

			serv_time=tempmu;
			userv_time=(tempmu-serv_time)*1000000;
		
			long int tme=tempmu*1000;

			printf("%sms: p%d begins service at S1, requesting = %ldms of service\n",curtime,firstq2packet->id,tme);

			server1_processing=1;

			tv4.tv_sec=serv_time;
			tv4.tv_usec=userv_time;

			//sleep to service the packet
			select(FD_SETSIZE,NULL,NULL,NULL,&tv4);

			gettime();
			strcpy(firstq2packet->s_exit_time,curtime);

			double t5,t6;
			sscanf(firstq2packet->s_arr_time,"%lf",&t5);
			sscanf(firstq2packet->s_exit_time,"%lf",&t6);

			t6=t6-t5;
			avg_service_time+=t6;
			avg_packets_in_s1+=t6;

			sprintf(time_difference,"%0.3lf",t6);

			double t7,t8;
			sscanf(firstq2packet->arr_time,"%lf",&t7);
			sscanf(firstq2packet->s_exit_time,"%lf",&t8);

			t8=t8-t7;
			//printf("\n\n value - %lf\n\n",t8);
			avg_sq_time_in_sys+=t8*t8;
			avg_time_in_system+=t8;

			sprintf(time_in_system,"%0.3lf",t8);

			completed_packets++;

			printf("%sms: p%d departs from S1, service time = %sms, time in system = %sms\n",curtime,firstq2packet->id,time_difference,time_in_system);
			server1_processing=0;
		}

		if(shutdown==1)
			pthread_mutex_unlock(&mutex);
		
		//printf("\n\nIn the middle of server 1 thread..!!\n\n");
		
		if((My402ListEmpty(q2list) && tokenthreadexit==1) || (shutdown==1))
		{
			//printf("\n\nInside exit part..!!\n\n");
			//printf("Inside the exit part of s1\n");
			gettime();

			if(server2dead==1)
			{
				sscanf(curtime,"%lf",&end_time);
				//printf("\n\nEnd time %lf\n",end_time);
				printf("%sms: emulation ends\n",curtime);
			}
			
			server1dead=1;
			if(shutdown==0)
 			{	
 				shutdown=1;
 				pthread_cond_broadcast(&cv);
 			}
 			//printf("server1_thread dead\n");
			pthread_exit(NULL);
		}
	}

	//pthread_cleanup_pop(0);
	//printf("server1_thread exited\n");
	pthread_exit(NULL);
	//return 0;
}

//server2 thread
void *server2_thread()
{
	//pthread_cleanup_push(cleanup, NULL);

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	
	char time_difference[20];
	char time_in_system[20];

	double tempmu;
	long serv_time;
	//printf("\nl %d\n",l);

	long userv_time;

	while(1)
	{

		tv4.tv_sec=serv_time;
		tv4.tv_usec=userv_time;

		pthread_mutex_lock(&mutex);

		while(My402ListEmpty(q2list) && shutdown==0)
		{
			//printf("\n\nserver 2 thread cond wait ..!!\n\n");
			pthread_cond_wait(&cv,&mutex);
		}
			
	//	if(shutdown==1)
	//		break;

		//printf("\n\nBefore if of server 2 thread..!!\n\n");

		if(!My402ListEmpty(q2list) && shutdown==0)
		{
			My402ListElem *firstpacket=NULL;
			firstpacket=My402ListFirst(q2list);

			data *firstq2packet=firstpacket->obj;

			gettime();
			strcpy(firstq2packet->q2_exit_time,curtime);

			double t3,t4;
			sscanf(firstq2packet->q2_arr_time,"%lf",&t3);
			sscanf(firstq2packet->q2_exit_time,"%lf",&t4);

			t4=t4-t3;
			avg_packets_in_q2+=t4;
		
			sprintf(time_difference,"%0.3lf",t4);

			My402ListUnlink(q2list,firstpacket);

			printf("%sms: p%d leaves Q2, time in Q2 = %sms\n",curtime,firstq2packet->id,time_difference);

			gettime();
			strcpy(firstq2packet->s_arr_time,curtime);
			pthread_mutex_unlock(&mutex);

			if(mode==1)
			{
				tempmu=firstq2packet->mu;
				tempmu/=1000;
			}
			else
				tempmu=1/(firstq2packet->mu);
			
		
			if(tempmu>10)
				tempmu=10;

			serv_time=tempmu;
			userv_time=(tempmu-serv_time)*1000000;
		
			long int tme=tempmu*1000;
		

		
			//double tme;
			//sscanf(,"%lf",&tme);
			//sprintf(arrival_time,"%.3lf",tme);	
		
		
			printf("%sms: p%d begins service at S2, requesting = %ldms of service\n",curtime,firstq2packet->id,tme);
			server2_processing=1;

			tv4.tv_sec=serv_time;
			tv4.tv_usec=userv_time;

			//sleep to service the packet
			select(FD_SETSIZE,NULL,NULL,NULL,&tv4);

			gettime();
			strcpy(firstq2packet->s_exit_time,curtime);

			double t5,t6;
			sscanf(firstq2packet->s_arr_time,"%lf",&t5);
			sscanf(firstq2packet->s_exit_time,"%lf",&t6);

			t6=t6-t5;
			avg_service_time+=t6;
			avg_packets_in_s2+=t6;

			sprintf(time_difference,"%0.3lf",t6);

			double t7,t8;
			sscanf(firstq2packet->arr_time,"%lf",&t7);
			sscanf(firstq2packet->s_exit_time,"%lf",&t8);

			t8=t8-t7;
			//printf("\n\n value - %lf\n\n",t8);
			avg_sq_time_in_sys+=t8*t8;
			avg_time_in_system+=t8;

			sprintf(time_in_system,"%0.3lf",t8);

			completed_packets++;
			printf("%sms: p%d departs from S2, service time = %sms, time in system = %sms\n",curtime,firstq2packet->id,time_difference,time_in_system);
			server2_processing=0;
		
		}

		if(shutdown==1)
			pthread_mutex_unlock(&mutex);

		//printf("\n\nIn the middle of server 2 thread..!!\n\n");
		
		if((My402ListEmpty(q2list) && tokenthreadexit==1) || (shutdown==1))
		{
			//printf("Inside the exit part of s2\n");
			gettime();

			if(server1dead==1)
			{
				sscanf(curtime,"%lf",&end_time);
				//printf("\n\nEnd time %lf\n",end_time);
				printf("%sms: emulation ends\n",curtime);
			}
 			
			server2dead=1;
			if(shutdown==0)
 			{	
 				shutdown=1;
 				pthread_cond_broadcast(&cv);
 			}
 			
			//printf("server2_thread dead\n");		
			pthread_exit(NULL);
		}
	}
	
	//pthread_cleanup_pop(0);

	pthread_exit(NULL);
}

//signal handler thread
void *signal_thread()
{
	//int sig=1;

//printf("Signal HAndler thread started..!!");

	while(1)
	{
		//if(server2dead==1 && server1dead==1)
		//pthread_exit(NULL);
		sigwait(&set);
		signalactive=1;
		


		//while(server1_processing==0 || server2_processing==0)
		//usleep(10000);
		//printf("\n\nsignal received..!!\n\n");

		//pthread_mutex_unlock(&mutex);
		//pthread_mutex_unlock(&mutex);
	
		nopacket=1;
		tokenthreadexit=1;
	
		pthread_cancel(id_arr);
		pthread_cancel(id_token);

		pthread_mutex_lock(&mutex);
		//display contents of q1
		My402ListElem *elem=NULL;

        for (elem=My402ListFirst(q1list);
                elem != NULL;
                elem=My402ListNext(q1list,elem)) {
            data *pack=(data*)(elem->obj);

        	gettime();

        	printf("%sms: packet%d removed from q1\n",curtime,pack->id);
            /* access foo here */
        }
    
        for (elem=My402ListFirst(q2list);
                elem != NULL;
                elem=My402ListNext(q2list,elem)) {
            data *pack2=(data*)(elem->obj);

        	gettime();

        	printf("%sms: packet%d removed from q2\n",curtime,pack2->id);
            /* access foo here */
        }
    
   
		My402ListUnlinkAll(q1list);

		My402ListUnlinkAll(q2list);

		//printf("Q1 and Q2 emptied..!!\n");
		//while(1)
		//{


		shutdown=1;
		pthread_cond_broadcast(&cv);

		pthread_mutex_unlock(&mutex);


		//shutdown=1;
		//pthread_cond_broadcast(&cv);
/*
		if(server1_processing==0)
		{
			printf("Server 1 exit code\n");
			server1dead=1;
			//pthread_cond_broadcast(&cv);
			pthread_cancel(id_server1);
		}

		if(server2_processing==0)
		{
			printf("Server 2 exit code\n");
			server2dead=1;
			//pthread_cond_broadcast(&cv);
			pthread_cancel(id_server2);
		}	
	*/	
		pthread_exit(NULL);
	}
}




		//pthread_cond_broadcast(&cv);
/*
		
*/	
		/*	if(server1_processing==0 && server2_processing==0)
		{
			server1dead=1;
			server2dead=1;
			pthread_cancel(id_server1);
			pthread_cancel(id_server2);
		}	
		*/

		//if(server2dead==1 && server1dead==1)
		//{

			//pthread_cancel(id_server1);
			//pthread_cancel(id_server2);
			//tokenthreadexit=1;

			//printf("both waiting not dead\n\n");
		
			//pthread_cond_broadcast(&cv);
			//pthread_cancel(id_server1);
			//pthread_cancel(id_server2);


			//pthread_cond_broadcast(&cv);
			//pthread_cancel(id_server2);

			//pthread_cancel(id_arr);
			//pthread_cancel(id_token);	
			//pthread_cancel(id_server1);
			//pthread_cancel(id_server2);
			//pthread_cancel(id_sighandler);
			//signalactive=0;
			

		
		//}
		//pthread_cond_broadcast(&cv);
		//pthread_cond_broadcast(&cv);
				
		//pthread_exit(NULL);
	
	//pthread_cond_broadcast(&cv);

	//pthread_exit(NULL);
	//return 0;




int main(int argc,char *argv[])
{
//printf("Welcome to warmup2\n");
//printf("argc is %d\n",argc);
//printf("arv[0] is %s\n",argv[0]); 
//printf("arv[1] is %s\n",argv[1]);
struct stat statbuf;
int dataexists=0;

int i=1;

double templambda=(double)1;
double tempmu=(double)0.35;
int tempP=3;

char tempstr[10]="";
char filename[20]="";

int tfile=0;
int errcode=0;

FILE *fp;
char buff[1024];


int temp;

while(i<argc)
{
	if(argc%2==0)
	{
		errcode=6;
		break;
	}

	if(strcmp(argv[i],"-lambda")==0)
	{
		if(argv[i+1][0]!='-')
		{
			if(sscanf(argv[++i],"%lf",&templambda)!=1)
			{
				errcode=2;
				break;
			}
		}
		else
		{
			errcode=1;
			break;
		}
	}
	
	else if(strcmp(argv[i],"-mu")==0)
	{
		if(argv[i+1][0]!='-')
		{
			if(sscanf(argv[++i],"%lf",&tempmu)!=1)
			{
				errcode=2;
				break;
			}
		}
		else
		{
			errcode=1;
			break;
		}
		
	}
	
	else if(strcmp(argv[i],"-r")==0)
	{
		if(argv[i+1][0]!='-')
		{
			if(sscanf(argv[++i],"%lf",&r)!=1)
			{
				errcode=2;
				break;
			}
		}
		else
		{
			errcode=1;
			break;
		}

		//strcpy(tempstr,argv[++i]);
		//r=atoi(tempstr);
		
	}
	
	else if(strcmp(argv[i],"-B")==0)
	{
		if(argv[i+1][0]!='-')
		{
			if(sscanf(argv[++i],"%d",&B)!=1)
			{
				errcode=3;
				break;
			}
		}
		else
		{
			errcode=1;
			break;
		}

		//strcpy(tempstr,argv[++i]);
		//B=atoi(tempstr);
		
	}
	
	else if(strcmp(argv[i],"-P")==0)
	{
		if(argv[i+1][0]!='-')
		{
			if(sscanf(argv[++i],"%d",&tempP)!=1)
			{
				errcode=3;
				break;
			}
		}
		else
		{
			errcode=1;
			break;
		}

		//strcpy(tempstr,argv[++i]);
		//P=atoi(tempstr);
		
	}
	
	else if(strcmp(argv[i],"-n")==0)
	{
		if(argv[i+1][0]!='-')
		{
			if(sscanf(argv[++i],"%d",&num)!=1)
			{
				errcode=3;
				break;
			}
		}
		else
		{
			errcode=1;
			break;
		}

		//strcpy(tempstr,argv[++i]);
		//num=atoi(tempstr);
		
	}

	else if(strcmp(argv[i],"-t")==0)
	{
		if(argv[i+1][0]!='-')
		{
			strcpy(tempstr,argv[++i]);
			strcpy(filename,tempstr);
			tfile=1;
			//printf("\nFilename is - %s\n",filename);
		}
		else
		{
			errcode=5;
			break;
		}

	}

	else
	{
		errcode=4;
		break;
	}

		i++;	
}

if(errcode!=0)
{
	if(errcode==1)
		fprintf(stderr,"Incorrect commandline arguments..!!\n");
	
	if(errcode==2)
		fprintf(stderr,"Cannot Parse one or more commandline argument(s) to get a Double value..!!\n");
	
	if(errcode==3)
		fprintf(stderr,"Cannot Parse one or more commandline argument(s) to obtain a value..!!\n");

	if(errcode==4)
		fprintf(stderr,"Malformed Command..!!\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");

	if(errcode==5)
		fprintf(stderr, "Tracefile Filename not found..!!\n");

	if(errcode==6)
		fprintf(stderr, "Malformed Command..!!\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");

	if(errcode<7)
		return 0;
}


if(tfile==1)
{

	fp=fopen(filename,"r");

	if(fp==NULL)
	{
		fprintf(stderr,"Cannot open %s for reading.\n", argv[2]);
		return 0;
	}

	stat(filename, &statbuf);

	if(S_ISDIR(statbuf.st_mode))
	{
    	printf("Input %s is a directory\n",filename);
    	return 0;
	}

	if(fgets(buff, sizeof(buff), fp) != NULL)
	{
		dataexists=1;

		//printf("%s",buff);

		int length = strlen(buff);
    	for (i=0;i<(length-1); i++)
        if (!isdigit(buff[i]))
        {
            
            fprintf(stderr, "Input file is not in the right format\n");
            return 0;
        }
        else
        {

		//Assign number of packets
		sscanf(buff,"%d",&num);
		}

		
		//printf("The number of packets is %d\n",num);
	}	
	else
	{

		fprintf(stderr, "Error : Not a file or File does not exist..!!\n");
		return 0;
	}

	if(dataexists==0)
	{
		fprintf(stderr,"Not a file or File does not exist..!!\n");
		return 0;
	}
	
	mode=1;
}
else
{
	fp=NULL;
	mode=0;
	lambda=templambda;
	mu=tempmu;
	P=tempP;
}

printf("Emulation Parameters:\n");

printf("\tnumber to arrive = %d\n",num);

if(tfile==0)
	printf("\tlambda = %.6g\n",templambda);

if(tfile==0)
	printf("\tmu = %.6g\n",tempmu);

printf("\tr = %.6g\n",r);
printf("\tB = %d\n",B);

if(tfile==0)
	printf("\tP = %d\n",tempP);

if(tfile==1)
	printf("\ttfile = %s\n",filename);

//return 0;

gettimeofday(&tv,NULL);
gettime();
strcpy(prev_arr_time,curtime);

//printf("\nTime start: tv_sec %ld\n",tv.tv_sec);
//printf("\nTime start: tv_sec %ld\n",tv.tv_usec);
//Initialize q1
	
q1list = (struct tagMy402List *)malloc(sizeof(struct tagMy402List));
My402ListInit(q1list);
	

q2list = (struct tagMy402List *)malloc(sizeof(struct tagMy402List));
My402ListInit(q2list);


sigemptyset(&set);

sigaddset(&set,SIGINT);

sigprocmask(SIG_BLOCK,&set,0);

//sscanf(curtime,"%lf",&start_time);	
printf("\n%sms: emulation begins\n",curtime);


//starting the arrival thread
pthread_create(&id_arr,
		0,
		arrival_thread,
		(void *)fp);


//starting the token depositing thread
pthread_create(&id_token,
		0,
		token_deposit_thread,
		NULL);
//starting the server2 thread
pthread_create(&id_server2,
		0,
		server2_thread,
		NULL);

//starting the server1 thread
pthread_create(&id_server1,
		0,
		server1_thread,
		NULL);

//starting the signal handler thread
pthread_create(&id_sighandler,
				0,
				signal_thread,
				NULL);

pthread_join(id_arr,0);

//printf("\n\nEnd of arr 0x%08x\n", (int)id_arr);

pthread_join(id_token,0);

//printf("\n\nEnd of token 0x%08x\n",(int)id_token);

pthread_join(id_server1,0);

//printf("\n\nEnd of server 1 0x%08x\n", (int)id_server1);

//printf("\n\nbefore of server 2 0x%08x\n", (int)id_server2);

pthread_join(id_server2,0);

//printf("\n\nEnd of server 2 0x%08x\n", (int)id_server2);

//cancel the signal handler thread
//printf("\n\n\nCancelling signal handler ..!!\n\n\n\n");

if(signalactive==0)
	pthread_cancel(id_sighandler);

//printf("\n\numberAfter cancel signal handler\n");
//if(signalactive==0)
//	pthread_join(id_sighandler,0);

avg_interval_time/=num_of_packets;
avg_service_time/=completed_packets;

avg_interval_time/=1000;
avg_service_time/=1000;

//printf("\taverage number of packets in Q2 = %0.6lf\n",avg_packets_in_q2);
//printf("\tend time %lf\n",end_time);

avg_packets_in_q1/=end_time;
avg_packets_in_q2/=end_time;
avg_packets_in_s1/=end_time;
avg_packets_in_s2/=end_time;

if(signalactive==1)
{
	temp=num_of_packets;
	num_of_packets=completed_packets;
}

avg_time_in_system/=num_of_packets;
avg_time_in_system/=1000;

printf("\nStatistics:\n\n");

printf("\taverage packet inter-arrival time = %0.6lf\n",avg_interval_time);
printf("\taverage packet service-time = %0.6lf\n\n",avg_service_time);

printf("\taverage number of packets in Q1 = %0.6lf\n",avg_packets_in_q1);
printf("\taverage number of packets in Q2 = %0.6lf\n",avg_packets_in_q2);
printf("\taverage number of packets in S1 = %0.6lf\n",avg_packets_in_s1);
printf("\taverage number of packets in S2 = %0.6lf\n\n",avg_packets_in_s2);

printf("\taverage time a packet spent in the system = %0.6lf\n",avg_time_in_system);

avg_sq_time_in_sys/=num_of_packets;

avg_time_in_system*=1000;
avg_time_in_system*=avg_time_in_system;

std_deviation=avg_sq_time_in_sys - avg_time_in_system;
std_deviation = sqrt(std_deviation);
std_deviation /= 1000;

if(signalactive==1)
	num_of_packets=temp;

printf("\tstandard deviation for time spent in system = %0.6g\n\n",std_deviation);

//printf("\n\nTokens - %d\n",tokens);
//			printf("\n Dropped-tokens - %d\n",dropped_tokens);
token_drop_probability=(double)dropped_tokens/(double)tokens;

printf("token drop probability = %0.6g\n",token_drop_probability);

packet_drop_probability=(double)dropped_packets/(double)num_of_packets;

printf("packet drop probability = %0.6g\n",packet_drop_probability);

//tv1.tv_sec=2;
//tv1.tv_usec=0;

//printf("\nThe total emulation time is : %lfms:\n",end_time);

//select(FD_SETSIZE,NULL,NULL,NULL,&tv1);

//gettime();
//printf("\nThe elapsed time is : %sms:\n",curtime);

if(tfile==1)
	fclose(fp);

return 0;

}


