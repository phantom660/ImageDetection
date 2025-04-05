#include "../include/server.h"

// /********************* [ Helpful Global Variables ] **********************/
int num_dispatcher = 0; //Global integer to indicate the number of dispatcher threads   
int num_worker = 0;  //Global integer to indicate the number of worker threads
FILE *logfile;  //Global file pointer to the log file
int queue_len = 0; //Global integer to indicate the length of the queue

//  Dispatcher locking mech

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER; // Condition variable for queue not full
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER; // Mutex lock for queue
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER; // Mutex lock for log file

//  Worker locking mech

pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER; 
// pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;


request_t *req_entries; // Request queue
// for (int i = 0; i < MAX_QUEUE_LEN; i ++) {
//   req_entries[i].buffer = malloc(2048);
// }

database_entry_t database[100]; // Database of images
pthread_t dispatcher_thread[MAX_THREADS]; // Dispatcher threads
pthread_t worker_thread[MAX_THREADS]; // Worker threads
int thread_idsD[MAX_THREADS]; // Dispatcher thread IDs
int thread_idsW[MAX_THREADS]; // Worker thread IDs
int connection_fd; // Connection file descriptor

int qHead = 0; // Queue head
int qTail = 0; // Queue tail
int qEnd; // Queue end
int numEle = 0;   // Queue is full if numEle == MAX_QUEUE_LEN
int databaseSize = 0; // Number of images in the database

void enque(char *buffer, int fd, int fs) { // function to add request to the queue
  // FILE* fh = fopen("test.png", "w");
  // int bytesWritten = fwrite(&buffer, 1, fs,fh);
  // printf("BR= %d\n", bytesWritten);
  // printf("FS= %d\n" ,fs);
  printf("EQ\n");
  req_entries[qTail].buffer = malloc(2048); // Allocate memory for the buffer
  strcpy(req_entries[qTail].buffer, buffer); // Copy the buffer
  req_entries[qTail].file_size = fs; // Set the file size
  req_entries[qTail].file_descriptor = fd; // Set the file descriptor
  // printf("Enq: %d %d\n", req_entries[qTail].file_descriptor, req_entries[qTail].file_size);
  qTail ++; // Increment the tail
  numEle ++; // Increment the number of elements in the queue
  if (qTail == qEnd) {
    qTail = 0;
  }
}

request_t* deque() { // function to remove request from the queue
  printf("DQ\n"); 
  request_t *req = malloc(sizeof(request_t)); // Allocate memory for the request
  memcpy(req, &req_entries[qHead], sizeof(request_t)); // Copy the request
  // free(req_entries[qHead].buffer);
  qHead++; // Increment the head
  numEle --; // Decrement the number of elements in the queue
  if (qHead == qEnd) {
    qHead = 0;
  }
  // printf("DQ %ld\n", req_entries[]);
  // free(req_entries[qHead].buffer);
  // printf("Deq: %d %d\n", req.file_size, req.file_descriptor);
  return req;
}



/**********************************************
 * image_match
   - parameters:
      - input_image is the image data to compare
      - size is the size of the image data
   - returns:
       - database_entry_t that is the closest match to the input_image
************************************************/
database_entry_t image_match(char *input_image, int size)  // input_image is the image data to compare, size is the size of the image data
{
  // const char *closest_file     = NULL;
  // int         closest_distance = INT_MAX;
  // int closest_index = 0;
  // // printf("%d\n", databaseSize);
  // for(int i = 0; i < databaseSize /* replace with your database size*/; i++)
  // {
  //  const char *current_file;
  //   // current_file = malloc(2048);
  //   // current_file = database[i].buffer;
  //   current_file = database[i].buffer;
  //  int result = memcmp(input_image, current_file, size);
  //  if(result == 0)
  //  {
  //    return database[i];
  //  }

  //  else if(result < closest_distance)
  //  {
  //    closest_distance = result;
  //    closest_file     = current_file;
  //     closest_index = i;
  //  }
  // }

  const char *closest_file = NULL; // variable to store the closest file
  int closest_distance = INT_MAX; // variable to store the closest distance
  int closest_index = 0; // variable to store the closest index
  int closest_file_size = INT_MAX; // ADD THIS VARIABLE
  for(int i = 0; i < databaseSize /* replace with your database size*/; i++)
  {
    const char *current_file;
    current_file = database[i].buffer;
    int result = memcmp(input_image, current_file, size); // compare the input image with the current file
    if(result == 0)
    {
      return database[i];
    }

    else if(result < closest_distance || (result == closest_distance && database[i].file_size < closest_file_size)) // if the result is less than the closest distance or if the result is equal to the closest distance and the file size is less than the closest file size
    {
      closest_distance = result; // update the closest distance
      closest_file = current_file; // update the closest file
      closest_index = i; // update the closest index
      closest_file_size = database[i].file_size;
    }

  }

  if(closest_file != NULL)
  {
    return database[closest_index];
  }
  else
  {
    return database[closest_index];
  }
  
  
}

/**********************************************
 * LogPrettyPrint
   - parameters:
      - to_write is expected to be an open file pointer, or it 
        can be NULL which means that the output is printed to the terminal
      - All other inputs are self explanatory or specified in the writeup
   - returns:
       - no return value
************************************************/
void LogPrettyPrint(FILE* to_write, int threadId, int requestNumber, char * file_name, int file_size, int fd){ // to_write is expected to be an open file pointer, or it can be NULL which means that the output is printed to the terminal
  char output[1024] = ""; // variable to store the output
  sprintf(output, "[%d][%d][%d][%s][%d]", threadId, requestNumber, fd, file_name, file_size); // format the output

  printf("Output: %s\n", output);
  if (to_write != NULL) {
    fprintf(to_write, "%s\n", output);
    fflush(to_write);
  }
}
/*
  * loadDatabase
    - parameters:
        - path is the path to the directory containing the images
    - returns:
        - no return value
    - Description:
        - Traverse the directory and load all the images into the database
          - Load the images from the directory into the database
          - You will need to read the images into memory
          - You will need to store the image data in the database_entry_t struct
          - You will need to store the file name in the database_entry_t struct
          - You will need to store the file size in the database_entry_t struct
          - You will need to store the image data in the database_entry_t struct
          - You will need to increment the number of images in the database
*/
/***********/

void loadDatabase(char *path) // function to load the database
{
  DIR *dir;
  struct dirent *entry;   // Directory entry
  struct stat st;        // File status

  int num_images = 0;

  dir = opendir(path);    // Open the directory
  if (!dir) {
    perror("Failed to open directory");
    exit(1);
  }

  while ((entry = readdir(dir)) != NULL) {   // Read the directory
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {  // Skip the . and .. directories
      continue;
    }

    if (stat(path, &st) == -1) {
      perror("Failed to get file status");
      exit(1);
    }


    char fileName[1024] = ""; // File name
    sprintf(fileName, "%s/%s", path, entry->d_name); // Format the file name
    // printf("%s\n", fileName);

    FILE* fh = fopen(fileName, "rb");
    if(fh == NULL){
      perror("fopen");
    }

    char * buffer = malloc(2048);

    //fread(buffer, 1, sizeof(buffer), fh);

    int bytes_read = fread(buffer,1 , 1028, fh);
    // printf("Number of bytes %d\n", bytes_read);

    database[num_images].file_name = malloc(1024);
    //database[num_images].buffer = malloc(2048);

    // printf("%s\n", database[num_images].file_name);

    strcpy(database[num_images].file_name, entry->d_name); 
    database[num_images].file_size = bytes_read;
    database[num_images].buffer =  buffer;
    databaseSize++;

    // printf("%d\n", database[num_images].file_size);

    fclose(fh);

    num_images++;
  }
}


void * dispatch(void *arg) // function  to handle incomming requests from the client by accepting connections, writting to the file
// and adding the request to the queue for the worker threads to process 
// the function ensures thread safety by using locks and condition variables
{  


  int ID = *(int *)arg;
  printf("Dispatch ID: %d\n", ID);
  // fprintf(logfile, "Dispatcher ID: %d\n", ID);
  // sleep(5);
  while (1) 
  {
    size_t file_size = 0;
    // request_detials_t request_details;

    /*
    *    Description:      Accept client connection
    *    Utility Function: int accept_connection(void)
    */
    // printf("Dispatch\n");
    int fd = accept_connection();    // Global variable (int)
    if (fd == -1) {
      perror("Connection failed");
      exit(1);
    }
    
    /*
    *    Description:      Get request from client
    *    Utility Function: char * get_request_server(int fd, size_t *filelength)
    */
    char *request = get_request_server(fd, &file_size);    // (1)
    char bu[1028]; 
    sprintf(bu, "test%d.png", ID);
    ID++;
    FILE* fh = fopen(bu, "wb");
    int bytesWritten = fwrite(request, 1, file_size, fh);
    fclose(fh);
    // sleep(1);

    printf("Request file size Dispatch: %ld\n", file_size);

    // if (logfile != NULL) {
    //   fprintf(logfile, "Request file size: %ld\n", file_size);
    // }
    // pthread_exit(NULL);

    if (request == NULL) {
      perror("Failed to get request");
      exit(1);
    }

    // printf("%d %d\n", connection_fd, file_size);


   /* 
    *    Description:      Add the request into the queue
        //(1) Copy the filename from get_request_server into allocated memory to put on request queue
      

        //(2) Request thread safe access to the request queue

        //(3) Check for a full queue... wait for an empty one which is signaled from req_queue_notfull

        //(4) Insert the request into the queue
        
        //(5) Update the queue index in a circular fashion

        //(6) Release the lock on the request queue and signal that the queue is not empty anymore
   */
    
    pthread_mutex_lock(&lock1);    // (2)

    while (numEle == queue_len) {
      pthread_cond_wait(&cond1, &lock1);    // (3)
    }

    // FILE* fh = fopen("test.png", "wb");
    // int bytesWritten = fwrite(request, 1, file_size,fh);

    enque(request, fd, file_size);    // (4 & 5)
    // printf("%d %d\n", connection_fd, file_size);

    pthread_cond_signal(&cond2);  // (6)
    pthread_mutex_unlock(&lock1);   // (6)

    free(request);
  }
  return NULL;
}

void * worker(void *arg) {    // arg is the thread_id
// the function continously dequeues requests from the queue and processes them
// then sends the result to the client
// the function ensures thread safety by using locks and condition variables

  int num_request = 0;                                    //Integer for tracking each request for printing into the log file
  int fileSize    = 0;                                    //Integer to hold the size of the file being requested
  void *memory    = NULL;                                 //memory pointer where contents being requested are read and stored
  int fd          = INVALID;                              //Integer to hold the file descriptor of incoming request
  char *mybuf;                                  //String to hold the contents of the file being requested
  mybuf = malloc(2048);


  /* 
  *    Description:      Get the id as an input argument from arg, set it to ID
  */

  int ID = *(int *)arg;
  printf("Worker ID: %d\n", ID);
  // fprintf(logfile, "Worker ID: %d\n", ID);
  // pthread_exit(NULL);
  
    
  while (1) {
    /* 
    *    Description:      Get the request from the queue and do as follows
      //(1) Request thread safe access to the request queue by getting the req_queue_mutex lock
      //(2) While the request queue is empty conditionally wait for the request queue lock once the not empty signal is raised

      //(3) Now that you have the lock AND the queue is not empty, read from the request queue

      //(4) Update the request queue remove index in a circular fashion

      //(5) Fire the request queue not full signal to indicate the queue has a slot opened up and release the request queue lock  
      */

    // printf("WORKER\n");

    pthread_mutex_lock(&lock1);    // (1)

    while(numEle == 0) {
      pthread_cond_wait(&cond2, &lock1);
    }

    request_t* dequed_Ele;
    // dequed_Ele = malloc(sizeof(request_t));
    dequed_Ele = deque();
    // strcpy(mybuf, dequed_Ele->buffer);
    // fileSize = dequed_Ele->file_size;
    // fd = dequed_Ele->file_descriptor;

    pthread_cond_signal(&cond1);
    pthread_mutex_unlock(&lock1);   // (5)
        
      
    /* 
    *    Description:       Call image_match with the request buffer and file size
    *    store the result into a typeof database_entry_t
    *    send the file to the client using send_file_to_client(int socket, char * buffer, int size)              
    */

    strcpy(mybuf, dequed_Ele->buffer);
    fileSize = dequed_Ele->file_size;
    fd = dequed_Ele->file_descriptor;

    // char bu[1024];
    // sprintf(bu, "testW%d.png", ID);
    // ID++;
    // FILE* fh = fopen(bu, "wb");
    // int bytesWritten = fwrite(dequed_Ele->buffer, 1, dequed_Ele->file_size, fh);
    // fclose(fh);

    // FILE* fh = fopen("test.png", "w");
    // int bytesWritten = fwrite(dequed_Ele.buffer, 1, dequed_Ele.file_size ,fh);

    database_entry_t dbE;
    // dbE = malloc(sizeof(database_entry_t));
    // printf("Request file size dQE: %d\n", dequed_Ele.file_size);
    dbE = image_match(dequed_Ele->buffer, dequed_Ele->file_size);
    printf("Matched file size Worker: %d\n", dbE.file_size);

    printf("here\n");

    pthread_mutex_lock(&lock2);

    LogPrettyPrint(logfile, ID, num_request, dbE.file_name, dbE.file_size, fd);

    pthread_mutex_unlock(&lock2);
    // char bu[1024];
    // sprintf(bu, "testW%d.png", ID);
    ID++;
    num_request++;
    // FILE* fh = fopen(bu, "wb");
    // int bytesWritten = fwrite(dbE.buffer, 1, dbE.file_size, fh);
    // fclose(fh);

    // char bu2[1024];
    // sprintf(bu2, "testRet%d.png", ID);
    // FILE* fh2 = fopen(bu2, "wb");
    // int bytesWritten2 = fwrite(dbE.buffer, 1, dbE.file_size, fh2);
    // fclose(fh2);

    if (send_file_to_client(fd, dbE.buffer, dbE.file_size) == -1) {
      perror("Send file_to_client");
    };
    printf("Worker after send_file_to_client %d, %d\n", fd, dbE.file_size);
  }
}

int main(int argc , char *argv[])
{
  // loadDatabase("../database");
  // for (int i = 0; i < 20; i ++) {
  //   char fn[12] = "";
  //   sprintf(fn, "Img%d.png", i);
  //   FILE* fp = fopen(fn, "wb");
  //   printf("Size of buffe %ld\n", sizeof(database[i].buffer));
  //   int bytes_written = fwrite(database[i].buffer, 1, database[i].file_size ,fp);
  //   // printf("%s\n", database[i].);
  //   printf("Written %d --- > %d\n", bytes_written, database[i].file_size);
  // }

  if(argc != 6){
    printf("usage: %s port path num_dispatcher num_workers queue_length \n", argv[0]);
    return -1;
  }


  int port            = -1;
  char path[BUFF_SIZE] = "no path set\0";
  num_dispatcher      = -1;                               //global variable
  num_worker          = -1;                               //global variable
  queue_len           = -1;                               //global variable
 

  /* 
  *    Description:      Get the input args --> (1) port (2) path (3) num_dispatcher (4) num_workers  (5) queue_length
  */
  port = atoi(argv[1]);   // Port number
  strcpy(path, argv[2]); // Path to the database
  num_dispatcher = atoi(argv[3]); // Number of dispatcher threads
  num_worker = atoi(argv[4]); // Number of worker threads
  queue_len = atoi(argv[5]); // Length of the queue

  req_entries = malloc(queue_len*sizeof(request_t));
  qEnd = queue_len;
  

  /* 
  *    Description:      Open log file
  */
  // logfile = open("server_log", O_WRONLY | O_TRUNC);    // Flags and mode
  logfile = fopen("server_log", "w");
  
 

  /*
  *    Description:      Start the server
  *    Utility Function: void init(int port); //look in utils.h 
  */
  init(port);


  /*
  *    Description:      Load the database
  */
  loadDatabase(path);
 

  /*
  *    Description:      Create dispatcher and worker threads 
  */

  for (int j = 0; j < num_dispatcher; j++) {   // Create dispatchers : Implement dispatch first
    thread_idsD[j] = j;
  }
  for (int j = 0; j < num_worker; j++) {   // Create dispatchers : Implement dispatch first
    thread_idsW[j] = j;
  }

  for (int j = 0; j < num_dispatcher; j++) {   // Create dispatchers : Implement dispatch first
    pthread_create(&dispatcher_thread[j], NULL, dispatch, &thread_idsD[j]);
  }

  for (int i = 0; i < num_worker; i++) {   // Create workers : Implement worker first
    pthread_create(&worker_thread[i], NULL, worker, &thread_idsW[i]);
  }

  // printf("HELLO\n");

  // Wait for each of the threads to complete their work
  // Threads (if created) will not exit (see while loop), but this keeps main from exiting
  // fclose(logfile);//closing the log files
  int i;
  for(i = 0; i < num_dispatcher; i++){ // Joining the dispatcher threads
    fprintf(stderr, "JOINING DISPATCHER %d \n",i); 
    if((pthread_join(dispatcher_thread[i], NULL)) != 0){
      printf("ERROR : Fail to join dispatcher thread %d.\n", i);
    }
  }

  // printf("HELLO\n");

  for(i = 0; i < num_worker; i++){ // Joining the worker threads
   // fprintf(stderr, "JOINING WORKER %d \n",i);
    if((pthread_join(worker_thread[i], NULL)) != 0){
      printf("ERROR : Fail to join worker thread %d.\n", i);
    }
  }
  fprintf(stderr, "SERVER DONE \n");  // will never be reached in SOLUTION
  fclose(logfile);//closing the log files

}