#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#define SA struct sockaddr 
#include <pthread.h>
#define PORT 25659  // the port users will be connecting to
#define NUM_OBS 5000 //number of objection in data
#define input_file "ransac_data"
#define MSS_SIZE 2 //MInimum Sample Set Size is 2
#define THRESHOLD 150 
#define DATA_SIZE 1024*10  //The DATA SIZE of input_file
static float m,c;
static int num_inliers;
//Declared as global variables as stack size cannot support this much memory requirement.
float observations[NUM_OBS][2]; //x and y co-ordinate of each observation
static float inliers[NUM_OBS][2] = {0};

static pthread_mutex_t my_mutex;

int *process(void *arg){
	int num = *(int*)arg;
        //Testing Phase
	int i;
	int local_num =0; 
        for (i = num; i < num+NUM_OBS/10; i++) {
                float epsilon = (observations[i][1] - m*observations[i][0] - c)*(observations[i][1] - m*observations[i][0] - c);
                if (epsilon < THRESHOLD) {
//			printf("num_inliers: %d \n",num_inliers);
			pthread_mutex_lock(&my_mutex);
                        inliers[num][0] = observations[i][0];
                        inliers[num][1] = observations[i][1];
                        local_num++;
			pthread_mutex_unlock(&my_mutex);
                }

        }
	pthread_mutex_lock(&my_mutex);
//	printf("local: %d \n",local_num);
	num_inliers+=local_num;
	pthread_mutex_unlock(&my_mutex);
}

int deal_shit() {
	
	num_inliers = 0;	

	FILE* fp;
	
	if (!(fp=fopen(input_file, "rb"))) {
		printf("can not opern file\n");
		return 1;
	}

	int i=0;
	for (i=0;i < NUM_OBS; i++)
		fscanf(fp,"%f %f\n", &observations[i][0], &observations[i][1]);

	fclose(fp);

	struct timespec start, stop; 
	double exe_time;
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

	int max_iteration = 100;
	int iteration = 0;
	//Randomly select points for MSS
	while(iteration < max_iteration){
	float mss_points[MSS_SIZE][2];
	int rand_index = rand() % NUM_OBS;
	mss_points[0][0] = observations[rand_index][0];
	mss_points[0][1] = observations[rand_index][1];

	rand_index = rand() % NUM_OBS;
	while (observations[rand_index][0] == mss_points[0][0]) {
		rand_index = rand() % NUM_OBS;
	}
	
	mss_points[1][0] = observations[rand_index][0];
	mss_points[1][1] = observations[rand_index][1];
	//Fit a model using the MSS
	m = (mss_points[1][1] - mss_points[0][1])/(mss_points[1][0] - mss_points[0][0]);
	c = (mss_points[0][1]*mss_points[1][0] - mss_points[1][1]*mss_points[0][0])/(mss_points[1][0] - mss_points[0][0]);

	int i, rc;
	pthread_t thread[10];
	int num_threads = 10;
	pthread_attr_t thread_attr;
	pthread_mutex_init(&my_mutex, NULL);
	pthread_attr_init(&thread_attr);
        pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
	int index[10]={0};
	for (i=0;i<num_threads;i++)
		index[i]= (NUM_OBS/10)*i;
	int* _index;
        /* Create threads to perform computation  */
        for (i = 0; i < num_threads; i++)
        {
	_index = &index[i];
        /* Now we just run the thread, on its subset of the data */
               rc = pthread_create(&thread[i], &thread_attr,process, (void *)_index);
               if (rc)
               {
               printf("ERROR: return code from pthread_create() is %d\n", rc);
               exit(-1);
               }
         }
         pthread_attr_destroy(&thread_attr);
	/* Watch until completion  */
                for (i = 0; i < num_threads; i++)
                {
			                    void *status;
                        rc = pthread_join(thread[i],&status);

                        if (rc)
                        {
                                printf("ERROR: return code from pthread_join() is %d\n", rc);
                                exit(-1);
                        }
                }
	//printf("num_inliers: %d \n",num_inliers);
        pthread_mutex_destroy(&my_mutex);
	if(num_inliers > NUM_OBS/10) break;
	else num_inliers = 0;
	iteration++; 
	}
	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		

	exe_time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	
	//printf("Number of Inliers: %d and iteration is: %d m: %f c: %f\n", num_inliers, iteration, m, c);
	//printf("\nExecution time = %f sec\n",  exe_time);

	return 0;
		
}
void reset(struct timeval* tv){
    tv->tv_sec = 0;
    tv->tv_usec = 30;
}

int main()
{
	srand(time(NULL));
        int server_fd, new_socket, valread, check, opt=1,result;
	FILE* fd;
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int mega_bytes = (1024);
        char *buffer = (char *) malloc(DATA_SIZE);
        char *buf_ptr, *ack = "AKC", *f_ack="NAK";
        int writesize = DATA_SIZE;
        int rec_left;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 30;
  	 // Creating socket file descriptor 

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket failed\n");
        exit(EXIT_FAILURE);
    }
    // Forcefully attaching socket to the port  
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        printf("setsockopt\n");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    // Forcefully attaching socket to the port  
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        printf("bind failed\n");
        exit(EXIT_FAILURE);
    }
    printf("launching...\n");
    if (listen(server_fd, 10) < 0)
    {
   //     printf("listen");
        exit(EXIT_FAILURE);
    }
    fd_set socketfds;  //create select set
    int select_ret;
while(1)
    {
    FD_ZERO (&socketfds);   //clean set
    FD_SET(server_fd,&socketfds); //add socket fd to the set

    reset(&tv); //once have timeout, the time should reinit. also as a function migrated point
    //listening server_fd socket file describtor, once have incomming socket rise the accept socket.
    select_ret = select (server_fd + 1, &socketfds, NULL, NULL, &tv);
    if (select_ret == -1 ) {printf("select error\n");continue;}
    if (FD_ISSET(server_fd , &socketfds)){
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
    {
         perror("accept\n");
         exit(EXIT_FAILURE);
    }

    bzero(buffer, sizeof(buffer));
    valread = read(new_socket, buffer, writesize);
    rec_left = writesize - valread;
    buf_ptr = buffer;
    buf_ptr += valread;
    while(rec_left > 0){
        valread = read(new_socket, buf_ptr, rec_left);
        if(valread <0) break;
        rec_left -= valread;
        buf_ptr += valread;
    }

    fd=fopen("kmeans_data","w");
    check = fwrite(buffer, writesize, 1, fd);
    if(check<0) { printf("file write error\n"); fclose(fd); continue; }
    bzero(buffer,sizeof(buffer));
    fclose(fd);
    result = deal_shit();
    if (result ==0) send(new_socket,f_ack,3,0);
    else  send(new_socket,ack,3,0);
    close (new_socket);
    }}
    free(buffer);
    return 0;

}
