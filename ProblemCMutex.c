/*  main.c  - main */

#include <xinu.h>

pid32 producer_id;
pid32 consumer_id;
pid32 timer_id;

int32 consumed_count = 0;
const int32 CONSUMED_MAX = 100;


/* Define your circular buffer structure and semaphore variables here */
struct cir_buffer {
    int head;
    int tail;
    int buffentry[10];
    } buffer;
int Buffer_Size  = 10;
int i;
int j;
int loop_Size = 2000;

sid32 prod;
sid32 cons;

/* Place your code for entering a critical section here */
void mutex_acquire(sid32 mutex)
{
	wait(mutex);
}

/* Place your code for leaving a critical section here */
void mutex_release(sid32 mutex)
{
	signal(mutex);
}

/* Place the code for the buffer producer here */
process producer(void)
{
	for(i = 1; i <= loop_Size; i++)
	{
		mutex_acquire(prod);	
		buffer.buffentry[buffer.tail]=i;
		printf("Producer produced %d\n", i);
		buffer.tail=(buffer.tail+1)%Buffer_Size;
		mutex_release(cons);
	}
	return OK;
}

/* Place the code for the buffer consumer here */
process consumer(void)
{
	/* Every time your consumer consumes another buffer element,
	 * make sure to include the statement:
	 *   consumed_count += 1;
	 * this will allow the timing function to record performance */
	/* */
	for(j = 1; j <= loop_Size; j++)
	{
		mutex_acquire(cons);
   	 	consumed_count += 1;
    		printf("Consumer consumed %d\n", buffer.buffentry[buffer.head]);
    		buffer.head = (buffer.head+1)%Buffer_Size;
		mutex_release(prod);
    	}
	return OK;
}


/* Timing utility function - please ignore */
process time_and_end(void)
{
	int32 times[5];
	int32 i;

	for (i = 0; i < 5; ++i)
	{
		times[i] = clktime_ms;
		yield();

		consumed_count = 0;
		while (consumed_count < CONSUMED_MAX * (i+1))
		{
			yield();
		}

		times[i] = clktime_ms - times[i];
		printf("Running %d time", i);
	}

	kill(producer_id);
	kill(consumer_id);

	for (i = 0; i < 5; ++i)
	{
		printf("TIME ELAPSED (%d): %d\n", (i+1) * CONSUMED_MAX, times[i]);
	}
}

process	main(void)
{
	recvclr();

	/* Create the shared circular buffer and semaphores here */
	buffer.head = 0;
    	buffer.tail = 0;

	prod = semcreate(1);
	cons = semcreate(0);

	producer_id = create(producer, 4096, 50, "producer", 0);
	consumer_id = create(consumer, 4096, 50, "consumer", 0);
	timer_id = create(time_and_end, 4096, 50, "timer", 0);
	resched_cntl(DEFER_START);
	resume(producer_id);
	resume(consumer_id);
	/* Uncomment the following line for part 3 to see timing results */
	//resume(timer_id);
	resched_cntl(DEFER_STOP);

	return OK;
}
