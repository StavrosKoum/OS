#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/wait.h>
#include <sys/types.h>
#include<sys/shm.h>
#include <time.h>
#include <math.h>
#include <sys/sem.h>
#include <semaphore.h>
#define lo 15


////////////________________FUNCTIONS________________________________
union semun
{
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

//static
int sem_id;

struct shmid_ds shmbuffer;
//struct shmid_ds2 shmbuffer2;
int segment_size;


//______________________FUNCTIONS________________________________
static int set_semvalue(void)
{
	union semun sem_union;
	sem_union.val = 1;
	if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
	return(1);
}

static void del_semvalue(int sem_id)
{
	union semun sem_union;
	if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
	fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(int sem_id)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1; /* P() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_p failed\n");
		return(0);
	}
	return(1);
}

static int semaphore_v(int sem_id)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1; /* V() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_v failed\n");
		return(0);
	}
	return(1);
}



int Randoms(int lower, int upper)
{
  //srand(time(0));
  int num = (rand() % (upper - lower + 1)) + lower;
  return num;

}

int choose_task(int i)
{
  int temp = Randoms(1,100);
  if(temp <= i)
  return 1;
  return 0;
}

void print_shmem(int* data,int entries_num)
{
  for(int y = 0; y < entries_num; y++)
  {
      printf("data[%d]= %d\n",y,data[y]);
  }
}

double sleep_time()
{
  srand(getpid());
  double exp;
  double x = (rand() % 10000) /10000.0;
  exp = -log(x)/lo; //lo is defined you can change it for more or less time
  return exp;

}
//----------------------------------------------------------------------------------------
//--------------------------------------main--------------------------------------------------
//__________________________________________________________________________________________
int main(int argc, char *argv[])
{

  int wrt_per_100 = atoi(argv[4]);
  int repetitions_num = atoi(argv[1]);
  //printf("Number of repetitions are %d\n",repetitions_num );

  int entries_num = atoi(argv[0]);
  //printf("Number of entries are %d\n",entries_num);

  int shmid = atoi(argv[2]);
  //printf("-----------------------%d is shmid\n",shmid);

  int shm_readers_id = atoi(argv[3]);
  //printf("-----------------------%d is shm_readers_id\n",shm_readers_id);








  
 

  //_______________TAKE DATA PTRS_________________
  int *data; //writters data
  data = shmat(shmid, (void *)0, 0);
  if (*data == (-1))
  {
   perror("shmat");
   exit(1);
  }

  int* readers_data = data + entries_num; //reader data

  int*readers_counter; //readers counter data
  readers_counter = shmat(shm_readers_id, (void *)0, 0);
  if (*readers_counter == (-1))
  {
   perror("shmat");
   exit(1);
  }



  //PEER SECTION_______________
  double time_taken = 0;
  clock_t t;
  double total_time = 0;


  int reads = 0;
  int wrt = 0;
  srand(getpid());
  for(int b= 0;b<repetitions_num;b++)
  {



    int rand_num = Randoms(0,(entries_num-1)); //find a random entry

    int task = choose_task(wrt_per_100);
    //printf("%d\n",task );
    //TOTAL TIME

    if(/*writter*/task == 1)///////////////////////////WRITTER
    {
      //GET SEM_ID HERE DEPENDED ON KEY AND RAND
      key_t sem_key = rand_num + 1000000;
      sem_id = semget((key_t)sem_key, 1, 0666 );
      //printf("%d\n",sem_id );
      //printf("WRITTER_i want entry [%d]\n",rand_num );
      //--------------------------CRITICAL SECTION----------------------------
      //START TIME HERE
      t = clock();
      if (!semaphore_p(sem_id)) exit(EXIT_FAILURE);
      //STOP TIME HERE
      t = clock() - t;
      //sleep(1);
      sleep(sleep_time());
      time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
      data[rand_num]++;
      //sleep(1);
      //printf("[son] pid %d from [parent] pid %d\n",getpid(),getppid());

      if (!semaphore_v(sem_id)) exit(EXIT_FAILURE);
      wrt++;

    }



    else if(task==0)////////////////////////////////////READER
    {

      //printf("I am a reader\n" );
      int readers_counter_sem_id; //get the id depending on the random entry num
      key_t sem_key = rand_num +1;
      readers_counter_sem_id = semget((key_t)sem_key, 1, 0666 );
      //START TIME HERE
      t = clock();
      //printf("Opening %d\n",readers_counter_sem_id );
      if (!semaphore_p(readers_counter_sem_id)) exit(EXIT_FAILURE);//semaphore counter_p__________________
      //STOP TIME HERE
      t = clock() - t;
      time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
      readers_counter[rand_num]++;
      //printf("I am a reader1\n" );

      if(readers_counter[rand_num]==1) //it means he is the first reader
      {
        //GET SEM_ID HERE DEPENDED ON KEY AND RAND
        key_t sem_key = rand_num + 1000000;
        sem_id = semget((key_t)sem_key, 1, 0666 );
        if (!semaphore_p(sem_id)) exit(EXIT_FAILURE); //use writers semaphore++++++++++++++++++
      }



      readers_data[rand_num]++; //++ readers counter in my shm
      //---------------------------------------------
      //semaphore counter v in order to enter another
      if (!semaphore_v(readers_counter_sem_id)) exit(EXIT_FAILURE);
      //do stuff/read data etc
      sleep(sleep_time());
      //semaphore readers counter p in order to decreace counter
      if (!semaphore_p(readers_counter_sem_id)) exit(EXIT_FAILURE);
      //-----------------------------------------------------------
      readers_counter[rand_num]--;

      if(readers_counter[rand_num]==0) //if there are no other readers
      {
        //printf("freeing entry[%d]\n",rand_num);
        key_t sem_key = rand_num + 1000000;
        sem_id = semget((key_t)sem_key, 1, 0666 );
        if (!semaphore_v(sem_id)) exit(EXIT_FAILURE); //change wrt_semaphore so a writter can enter
      }
      if (!semaphore_v(readers_counter_sem_id)) exit(EXIT_FAILURE); //semaphore counter v___________________
      reads++;

    }

    total_time = total_time + time_taken;
  }
  double avg_time = total_time/repetitions_num;
  printf("child_pid %d finished with %d writes and %d reads--Average time was %f\n",getpid(),wrt,reads,avg_time);
  exit(0);

  return 0;
  //exit(0);

}
