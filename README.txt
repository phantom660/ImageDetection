How to compile the program:


SERVER:
* To compile the server program run make in the command line.​​ This will generate an executable named ‘server’ which you can run with the desired command line arguments:
./server <Port> <Database path> <num_dispatcher> <num_workers> <queue_lenghth>
For example:
./server 8000 database 50 50 20


CLIENT:
* To compile the client program run make in the command line.​​ This will generate an executable named ‘client’ which you can run with the desired command line arguments:
./client <directory path> <server Port> <output directory path
For example:
./client img 8000 output/img

 
- output/img directory is currently populated with matched images from img directory (run with 50 dispatchers, 50 workers and queue size 20)



To enable each individual request to be parallelized, you can implement a thread pool architecture where a pool of worker threads is responsible for handling incoming requests concurrently. 

Here’s pseudo-code to enable each individual request to be parallelized.
# Define a thread pool with a fixed number of worker threads
worker_thread_pool = ThreadPool(num_worker_threads)


# Dispatcher thread function
def dispatch():
    while True:
        # Accept connection and receive request from client
        connection, request = accept_connection_and_receive_request()


        # Submit the request to the thread pool for processing
        worker_thread_pool.submit(process_request, request, connection)


# Worker thread function
def process_request(request, connection):
    # Process the request independently
    response = handle_request(request)


    # Send the response back to the client
    send_response(connection, response)


# Initialize the thread pool upon server startup
worker_thread_pool.initialize()


# Start dispatcher threads
for i in range(num_dispatcher_threads):
    start_thread(dispatch)


# Wait for each dispatcher thread to complete
for dispatcher_thread in dispatcher_threads:
    join(dispatcher_thread)


In this setup, each incoming request is handled by a separate worker thread from the thread pool. This allows multiple requests to be processed concurrently, maximizing the utilization of system resources and improving overall throughput.