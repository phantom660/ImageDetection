#include "../include/client.h"



int port = 0;

pthread_t worker_thread[100]; // Array of worker threads
int worker_thread_id = 0; // Index of the worker thread
char output_path[1028]; // Output path
char input_path[1028]; // Input path

processing_args_t req_entries[100]; // Array of processing arguments

void * request_handle(void * args) // function to send the image to the server and receive the processed image
{
    char file_path[4096] = ""; // File path
    char file_path1[2048] = ""; // File path
    // Cast the argument to the appropriate type
    processing_args_t *processing_args = (processing_args_t *)args;

    sprintf(file_path, "%s/%s", input_path, processing_args->file_name); // combining the input path and the file name
    // Open the file in read-binary mode
    printf("Trying to open: %s\n", file_path);
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Get the file length using fseek and ftell
    fseek(file, 0, SEEK_END); // Seek to the end of the file
    size_t size = ftell(file); // Get the current position of the file pointer
    fseek(file, 0, SEEK_SET); // Seek back to the beginning of the file

    // Set up the connection with the server
    printf("Trying to set up connection with the server from client\n");
    int socket = setup_connection(port); // Set up the connection with the server
    // printf("bla bla");
    if (socket == -1) {
        perror("Error setting up connection");
        fclose(file);
        return NULL;
    }

    // FILE* fh = fopen("test1.png", "wb");
    // int bytesWritten = fwrite(request, 1, file_size, fh);
    // fclose(fh);

    // Send the file to the server
    if (send_file_to_server(socket, file, size) == -1) { // Send the file to the server
        perror("Error sending file to server");
        fclose(file);
        close(socket);
        return NULL;
    }

    // Receive the processed image from the server
    // char file_path[1028];
    // strcpy(file_path, processing_args->file_name);  // Assuming output_path is correctly set elsewhere
    printf("Client: %s %s\n", processing_args->file_name, output_path); // receiving the processed image from the server
    char file_name_test[4096]; // File name
    sprintf(file_name_test, "%s/%s", output_path, processing_args->file_name); // Combining the output path and the file name
    printf("%s\n", file_name_test); // Print the file name
    if (receive_file_from_server(socket, file_name_test) == -1) {
        perror("Error receiving file from server");
        fclose(file);
        close(socket);
        return NULL;
    }
    sprintf(file_name_test, "%s/%s", output_path, processing_args->file_name); // Combining the output path and the file name

    // Close the file and the socket
    fclose(file);
    close(socket);

    return NULL;
}

/*
* The directory_trav function to traverse the directory and send the images to the server 
* 1. Open the directory
* 2. Read the directory entries
* 3. If the entry is a file, create a new thread to invoke the request_handle function which takes the file path as an argument
* 4. Join all the threads
* Note: Make sure to avoid any race conditions when creating the threads and passing the file path to the request_handle function. 
* use the req_entries array to store the file path and pass the index of the array to the thread. 
*/
void directory_trav(char * args) // function to traverse the directory and send the images to the server
{
    DIR *dir; // Directory
    struct dirent *entry; // Directory entry
    int idx = 0; // Index

    // Open the directory
    if ((dir = opendir(args)) == NULL) {
        perror("Error opening directory");
        return;
    }

    // Read the directory entries
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // if (entry->d_type == DT_REG) { // If the entry is a regular file
        char file_path1[1028] = "";

        // sprintf(file_path1, "%s", entry->d_name);
        sprintf(file_path1, "%s", entry->d_name); // Combining the input path and the file name
        printf("%s\n", file_path1);
        
        // Store the file path in the req_entries array
        req_entries[idx].file_name = strdup(file_path1); 
        req_entries[idx].number_worker = worker_thread_id;
        

        // Create a new thread to invoke the request_handle function
        if (pthread_create(&worker_thread[worker_thread_id], NULL, request_handle, &req_entries[idx]) != 0) {
            perror("Error creating thread");
            return;
        }

        worker_thread_id++;
        idx++;
        // }
    }

    closedir(dir); // Close the directory

    // Join all the threads
    for (int i = 0; i < worker_thread_id; i++) { // joins the threads to wait for them to finish processing
        if (pthread_join(worker_thread[i], NULL) != 0) {
            perror("Error joining thread");
            return;
        }
    }
}
int main(int argc, char *argv[]) // Main function takes in the directory path, server port, and output path it checks if the enough arguments are passed esle print error
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: ./client <directory path> <Server Port> <output path>\n"); //
        exit(-1);
    }
    /*
    * 1. Get the input args --> (1) directory path (2) Server Port (3) output path
    */
    char *directory_path = argv[1]; // Directory path
    port = atoi(argv[2]); // Server port
    strcpy(output_path, argv[3]); // Output path
    strcpy(input_path, argv[1]); // Input path


    /*
    * Call the directory_trav function to traverse the directory and send the images to the server
    */
    directory_trav(input_path); // Traverse the directory and send the images to the server
    return 0;  
}