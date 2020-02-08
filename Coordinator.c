#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/wait.h>
#include <sys/types.h>
#include<sys/shm.h>
#include <time.h>
#include <math.h>
#define lo 15

#include <sys/sem.h>
#include <semaphore.h>


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
  exp = -log(x)/lo;
  return exp;

}



//_________________________________________MAIN__________________________________
int main(int argc, char *argv[])
{
  //srand(time(0));
  int readers_num = 10;

  if(argc != 5)
  {
    printf("There are not enough arguments\nTry again \n");
    return 0;
  }

  int peers_num = atoi(argv[1]);
  printf("Number of peers are %d\n",peers_num );

  int entries_num = atoi(argv[2]);
  printf("Number of entries are %d\n",entries_num );

  int repetitions_num = atoi(argv[3]);
  printf("Number of repetitions are %d\n",repetitions_num );

  int wrt_per_100 = atoi(argv[4]);
  printf("Writters generated with percentance %d/100\n",wrt_per_100 );



  // shared memory--------------------------------------------------------------
  int shmid;
  int key = 123467574;
  int shmsize = entries_num*sizeof(int)*2;
  int *data;

  printf("size of shared memmory = %d\n",shmsize);
  if ((shmid = shmget(key,shmsize, 0644 | IPC_CREAT)) == -1)
  {
    perror("shmget");
    //shmdt(data);
    //shmctl(shmid, IPC_RMID, 0);
    exit(1);
  }
  printf("Entries Shared memory id: %d\n",shmid);
  data = shmat(shmid, (void *)0, 0);
  if (*data == (-1))
  {
   perror("shmat");
   exit(1);
  }

  shmctl (shmid, IPC_STAT, &shmbuffer);
  segment_size=shmbuffer.shm_segsz; //i have no idea but i need this

  int* readers_data = data + entries_num;
  //initialize shared data
  for(int x = 0;x < entries_num*2; x++)
  {
    data[x] = 0;
  }







  //code for semaphores---------------------------------------------------------
  for(int z =0;z<entries_num;z++) //one for each entry
  {
    key_t k = z+1000000;//create a key depending on the entry
    sem_id = semget((key_t)k, 1, 0666 | IPC_CREAT);
    if (!set_semvalue()) {fprintf(stderr, "Failed to initialize semaphore\n");exit(EXIT_FAILURE);}
    printf("Sem id is %d...........................\n",sem_id );

    //initialize semaphore depending on readers Number
    // union senum arg;
    // arg.val=readers_num;

    //semctl(sem_id, 0, SETVAL, readers_num);
  }



///////////////////////////////////////////////////////////////////////////////
//                        readers counter shared memmory
int shm_readers_id;
int*readers_counter;
key_t shm_readers_key = 43256541;

if ((shm_readers_id = shmget(shm_readers_key,shmsize, 0644 | IPC_CREAT)) == -1)
{//shmid
  perror("shmget");
  exit(1);
}

printf("read counter Shared memory id: %d\n",shm_readers_id);
readers_counter = shmat(shm_readers_id, (void *)0, 0);
if (*readers_counter == (-1))
{
 perror("shmat");
 exit(1);
}

shmctl (shm_readers_id, IPC_STAT, &shmbuffer);
segment_size=shmbuffer.shm_segsz; //i have no idea but i need this!!!!!????????????????

//initialize shared data
for(int x = 0;x < entries_num; x++)
{
  readers_counter[x] = 0;
}

//-----------------------create semaphores for reader counters------------------

for(int l =0;l<entries_num;l++) //one for each entry
{
  key_t lol = l+1;
  sem_id = semget((key_t)lol, 1, 0666 | IPC_CREAT);
  if (!set_semvalue()) {fprintf(stderr, "Failed to initialize semaphore\n");exit(EXIT_FAILURE);}
  printf("Sem id for readers is %d...........................\n",sem_id );

  //initialize semaphore depending on readers Number
  // union senum arg;
  // arg.val=readers_num;

  //semctl(sem_id, 0, SETVAL, readers_num);
}











//------------------------------CREATE PEERS---------------------------------------




  for(int i=0; i<peers_num; i++) // loop will run n times
  {

      //sleep(1);
      if(fork() == 0)
      {

        char exec_repetitions[100];
        char exec_entries[100];
        char exec_entries_shmID[100];
        char exec_read_count_shmID[100];
        char exec_percent[100];


        snprintf(exec_entries , sizeof(exec_entries)," %d",entries_num);
        snprintf(exec_repetitions , sizeof(exec_repetitions)," %d",repetitions_num);
        snprintf(exec_entries_shmID , sizeof(exec_entries_shmID)," %d",shmid);
        snprintf(exec_read_count_shmID , sizeof(exec_read_count_shmID)," %d",shm_readers_id);
        snprintf(exec_percent , sizeof(exec_percent)," %d",wrt_per_100);



        if(execlp("./Peers",exec_entries ,exec_repetitions ,exec_entries_shmID ,exec_read_count_shmID,exec_percent ,(int *)0  ) == -1)
        {
                printf("exec not working \n");
                exit(1);
        }


          
	}
  }

  //sleep(5);
  for(int i=0; i<peers_num; i++) // loop in order to wait for the children to finish
  wait(NULL);
  printf("This is the Coordinator\n");
  printf("\nWritters data :\n");
  print_shmem(data,entries_num);//print the shared data
  printf("\nReaders data :\n");
  print_shmem(readers_data,entries_num);

  //printf("\ncounters (must be zero)\n");//print the readers counters
 // print_shmem(readers_counter,entries_num);

  //delete memories
  shmdt(data);
  shmctl(shmid, IPC_RMID, 0);

  shmdt(readers_counter);
  shmctl(shm_readers_id, IPC_RMID, 0);
  //delete semaphores
  for(int f = 0;f < entries_num; f++)
  {
    //delete readers counter semaphores
    key_t del_sem_key = f +1;
    int readers_counter_sem_id = semget((key_t)del_sem_key, 1, 0666 );
    del_semvalue(readers_counter_sem_id);
    //delete wrt semaphores
    key_t del_sem_key2 = f +1000000;
    sem_id = semget((key_t)del_sem_key2, 1, 0666 );
    del_semvalue(sem_id);

  }
}
