#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>

using namespace std;


struct CShareData
{
	sem_t lock_;
	char data_[1000];
};

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		cout << "usage: " << endl;
		cout << argv[0] << " r" << endl;    //读共享内存
		cout << argv[0] << " w data" << endl; //写共享内存
		cout << argv[0] << " d" << endl;  //删除共享内存
		return 0;
	}

	const key_t key = 123456;
	int create = 0;
	int id = shmget(key, sizeof(CShareData), 0666);		
	if(id < 0)
	{
		if(errno != ENOENT)
		{
			perror("shmget1 ");
			return 0;
		}

		//create
		id = shmget(key, sizeof(CShareData), 0666 | IPC_CREAT | IPC_EXCL);		
		if(id < 0 && errno != EEXIST)
		{
			perror("shmget2 ");
			return 0;
		}
		
		if(id >= 0)
		{
			cout << "create" << endl;
			create = 1;
		}
	}

	CShareData* addr = (CShareData*)shmat(id, NULL, 0);	
	if(addr == (void*)-1)
	{
		perror("shmat ");
		return 0;
	}

	if(create == 1)
	{
		memset(addr, sizeof(CShareData), 0);
		if(sem_init((sem_t*)addr, 1, 1) < 0 )
		{
			perror("sem_init ");
			return 0;
		}
	}

	if(sem_wait(&(addr->lock_)) < 0)	
	{
		perror("sem_wait ");
		return 0;
	}

	if(strcmp(argv[1], "r") == 0)
	{
		cout << "data: " << addr->data_ << endl;
	}	
	else if(strcmp(argv[1], "w") == 0)
	{
		strcpy(addr->data_, argv[2]);
		//sleep(5);
	}

	if(sem_post(&(addr->lock_)) < 0)	
	{
		perror("sem_post ");
		return 0;
	}

	if(strcmp(argv[1], "d") == 0)
	{
		sem_destroy(&(addr->lock_));
		if(shmctl(id, IPC_RMID, NULL) < 0)
		{
			perror("shmctl ");
		}
	}

	shmdt(addr);
	return 0;
}



