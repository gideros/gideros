/* Session 7. multiPCca.c	 Lab 7-1			*/
/* Maintain a producer thread and several consumer threads	*/
/* The producer periodically creates checksummed data buffers, 	*/
/* or "message block" which the consumers display as soon	*/
/* as possible. The conusmers read the NEXT complete	 	*/
/* set of data, and each consumer validates the data before	*/
/* before display.						*/
/* Consumers are created and cancelled on demand based		*/
/* user input.							*/
/* Usage: multiPCca maxconsumer 				*/

#if defined (_MSC_VER)
#include <windows.h>
#define sleep(i) Sleep(i*1000)
#endif
#include <pthread.h>
//#include "errors.h"
//#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#define DATA_SIZE 256

typedef struct msg_block_tag { /* Message block */
	pthread_mutex_t mguard;	/* Guard the message block	*/
	pthread_cond_t  mready; /* Message ready		*/
	pthread_cond_t  mok;    /* Ok for the producer to produce */
	volatile int f_ready;
	volatile int f_stop;
		/* ready state flag; producer stopped flag	*/
	volatile int sequence; /* Message block sequence number	*/
	time_t timestamp;
	int checksum; /* Message contents checksum		*/
	int data[DATA_SIZE]; /* Message Contents		*/

} msg_block_t;

/* The invariant and condition variable predicates are:		*/
/*	Invariant - 					 	*/
/*	  f_ready && data is valid				*/
/*	   && checksum and timestamp are valid			*/ 
/*	Condition variable predicate				*/
/*	  	mready if and only if f_ready and a new message	*/
/*		has just been generated				*/


/* Single message block, ready to fill with a new message 	*/
struct msg_block_tag mblock = { PTHREAD_MUTEX_INITIALIZER, 
	PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0, 0 }; 

void * produce (void *), * consume (void *);
void message_fill (msg_block_t *);
void message_display (int, msg_block_t *);
static volatile int ShutDown = 0;
	
int main (int argc, char * argv[])
{
	int tstatus, nthread, ithread;
	int * f_consumer;    /* Array of flags to indicate corresponding thread exists */
	void *join_result;
	pthread_t *consume_t;
	char command [10];
	
	if (argc != 2) {
		printf ("Usage: multiPCca maxconsumer\n");
		return 1;
	}
	
	nthread = atoi(argv[1]);
	consume_t = calloc (nthread, sizeof(pthread_t));
	f_consumer = (int *)calloc (nthread, sizeof(int));
	
	while (!ShutDown) {	
		printf ("Enter command: nc (new consumer), cc (cancel), ");
		printf ("pr (produce msg), sh (shutdown):"); 
		fflush (stdout);
		scanf ("%s", command);
		printf ("Command received: %s.\n", command); fflush (stdout);
		if (strcmp (command, "nc") == 0) { /* New consumer thread */
			/* Look for empty thread slot */
			for (ithread = 0; ithread < nthread; ithread++) {
				if (!f_consumer[ithread]) break;
			}
			if (ithread >= nthread) {
				printf ("Maximum # consumers (%d) already exist\n", nthread);
				fflush (stdout);
				continue;
			}

			tstatus = pthread_create (&consume_t[ithread], NULL, 
										consume, (void *)ithread);
//			if (tstatus != 0) 
//				err_abort (tstatus, "Cannot create consumer thread");
			f_consumer[ithread] = 1;
			printf ("Consumer number %d created successfully.\n", ithread);			
			fflush (stdout);

		} else if (strcmp (command, "cc") == 0) { /* cancel consumer thread */
			printf ("Enter consumer number: 0 to %d:", nthread-1);
			fflush (stdout);
			scanf ("%d", &ithread);
			if (ithread < 0 || ithread >= nthread || !f_consumer[ithread]) {
				printf ("Thread %d does not exist.\n", ithread); fflush (stdout);
				fflush (stdout);
			} else {
				printf ("About to cancel thread # %d.\n", ithread); fflush (stdout);
				tstatus = pthread_cancel (consume_t[ithread]);
				printf ("Cancel status: %d. About to join thread # %d\n", tstatus, ithread); fflush (stdout);
				tstatus = pthread_join (consume_t[ithread], &join_result);
				printf ("Join status: %d after joining thread # %d. Result: %d\n", tstatus, ithread, (int) join_result); fflush (stdout);
				f_consumer[ithread] = 0;
			}
			continue;

		} else if (strcmp (command, "pr") == 0) { /* Produce a message */
			printf ("About to produce a new message.\n"); fflush (stdout);
			produce(NULL);
			/* Note the race to prompt before/after message display */
		} else if (strcmp (command, "sh") == 0) { /* shutdown system */
			printf ("Shutdown command received\n");
			for (ithread = 0; ithread < nthread; ithread++) 
				printf ("Thread #: %d. Flag: %d.\n", ithread, f_consumer[ithread]);

			fflush (stdout);
			ShutDown = 1; /* Cancel and join all running threads */
			for (ithread = 0; ithread < nthread; ithread++) {
				printf ("Thread #: %d. Flag: %d.\n", ithread, f_consumer[ithread]);
				fflush (stdout);
				if (f_consumer[ithread]) {
					printf ("About to cancel consumer thread #: %d.\n", ithread);
					fflush (stdout);
					tstatus = pthread_cancel (consume_t[ithread]);
					if (tstatus != 0) 
						printf ("Cannot cancel consumer thread %d", ithread);
					printf ("About to join consumer thread #: %d.\n", ithread);
					fflush (stdout);
					tstatus = pthread_join (consume_t[ithread], &join_result);
					if (tstatus != 0 || join_result != PTHREAD_CANCELED) 
						printf ("Error joining thread #: %d. tstatus: %d.\n",
								ithread, tstatus);
					printf ("Joined consumer thread #: %d.\n", ithread);
					fflush (stdout);

				}
			}
			fflush (stdout);
			printf ("All consumer threads cancelled and joined.\n");
			fflush (stdout);
			
		} else { /* Illegal command */
			printf ("Illegal command. %s. Try again\n", command);
			fflush (stdout);
		}
	}	

	free (consume_t);
	printf ("Producer and consumer threads have terminated\n");
	fflush (stdout);
	return 0;
}


void * produce (void *arg)
/* Producer function.  Create new message when called, and notify consumers */
/* The arg is there as this was derived from a thread function.             */
{
	int tstatus = 1;
	/* Get the buffer, fill it,*/
	/* and inform all consumers with a broadcast		*/
	printf ("Entering producer.\n"); fflush (stdout);
	while (tstatus != 0) {
		tstatus = pthread_mutex_trylock (&mblock.mguard);
		printf ("Trylock status: %d\n", tstatus); fflush (stdout);
		if (tstatus != 0) sleep (1);
	}
	message_fill (&mblock);
	mblock.sequence++;
	mblock.f_ready = 1;
	pthread_cond_broadcast (&mblock.mready);
	printf ("Producer produced one message.\n"); fflush (stdout);
	pthread_mutex_unlock (&mblock.mguard);
	return NULL;
}

/* Mutex cleanup handler used by the consumers */
void free_mutex (void * arg)
{
	int tstatus;
	printf ("Entering free_mutex cleanup handler.\n");
	tstatus = pthread_mutex_unlock ((pthread_mutex_t *)arg);
	printf ("Unlocked mutex. Status: %d\n", tstatus); fflush (stdout);
}

/* Cleanup handler for the consumer thread */
void cancel_consumer (void * arg)
{
	int ithread;
	
	ithread = (int) arg;
	printf ("Thread number %d cancellation handler. Curently, does nothing.\n", ithread);
	fflush (stdout);
}

void *consume (void *arg)
{
	int ithread, old_state, old_type;
	struct timespec timeout;
	timeout.tv_nsec = 0;

	ithread = (int)arg;

	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_state);
	pthread_cleanup_push (cancel_consumer, (void *)ithread);
	pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, &old_type);
	pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, &old_state);
	
	/* Consume the NEXT message  */
	do { 
		pthread_testcancel();
		pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_state);
		pthread_mutex_lock (&mblock.mguard);
		pthread_cleanup_push (free_mutex, &mblock.mguard);
		pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, &old_state);
		if (!mblock.f_stop) {
			do {	/* Wait for the NEXT message */
				pthread_testcancel();
				printf ("Thread #: %d about to wait for mready.\n", ithread);  fflush (stdout);
				pthread_cond_wait (&mblock.mready, &mblock.mguard);
				printf ("Thread #: %d after wait for mready.\n", ithread);  fflush (stdout);
			} while (!mblock.f_ready && !ShutDown);
			message_display (ithread, &mblock);
			pthread_testcancel();
		}
		/* Free the mutex through the free_mutex cleanup handler (a macro) */
		pthread_cleanup_pop (1);
	} while (!ShutDown);
	pthread_cleanup_pop(1);  /* NOTE: Try removing this!! Compiler error!
							    Why?? Because it's a macro                */
	printf ("Consumer number %d is shutting down\n", ithread);
	fflush (stdout);
	return NULL;		
}

void message_fill (msg_block_t *mblock)
{
	/* Fill the message buffer, and include checksum and timestamp	*/
	/* This function is called from the producer thread while it 	*/
	/* owns the message block mutex					*/
	
	int i;
	
	mblock->checksum = 0;	
	for (i = 0; i < DATA_SIZE; i++) {
		mblock->data[i] = rand();
		mblock->checksum ^= mblock->data[i];
	}
	mblock->timestamp = time(NULL);
	return;
}

void message_display (int ithread, msg_block_t *mblock)
{
	/* Display message buffer and timestamp, validate checksum	*/
	/* This function is called from the consumer thread while it 	*/
	/* owns the message block mutex					*/
	int i, tcheck = 0;
	
	for (i = 0; i < DATA_SIZE; i++) 
		tcheck ^= mblock->data[i];
	printf ("\nConsumer thread #: %d\n", ithread);
	printf ("Message number %d generated at: %s", 
		mblock->sequence, ctime (&(mblock->timestamp)));
	printf ("First and last entries: %x %x\n",
		mblock->data[0], mblock->data[DATA_SIZE-1]);
	fflush (stdout);
	if (tcheck == mblock->checksum)
		printf ("GOOD ->Checksum was validated.\n");
	else
		printf ("BAD  ->Checksum failed. message was corrupted\n");
	fflush (stdout);	
	return;

}
