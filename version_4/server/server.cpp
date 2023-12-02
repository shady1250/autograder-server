#include <iostream>
#include <fstream>
#include <cstring>
#include <thread>
#include <filesystem>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <netinet/in.h>
#include<queue>
#include<pthread.h>
#include "FileQueue.h"
#include "send_recv_utility.h"
#include <chrono>
#include<random>
#include<cassert>
#include<string>

using namespace std;
namespace fs = filesystem;

struct ThreadArgs {
    string request_id;
    int client_sockfd;
};

int Size;
pthread_mutex_t Lock=PTHREAD_MUTEX_INITIALIZER, fileLock=PTHREAD_MUTEX_INITIALIZER,StatusFileLock=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t space_available=PTHREAD_COND_INITIALIZER, items_available=PTHREAD_COND_INITIALIZER, data_available=PTHREAD_COND_INITIALIZER;

FileQueue fileQueue("RequestQueue.txt"); 
FileQueue statusQueue("StatusQueue.txt");
FileQueue Queue("queue.txt");

const char SUBMISSIONS_DIR[] = "./submissions/";
const char EXECUTABLES_DIR[] = "./executables/";
const char OUTPUTS_DIR[] = "./outputs/";
const char COMPILER_ERROR_DIR[] = "./compiler_error/";
const char RUNTIME_ERROR_DIR[] = "./runtime_error/";
const char DIFF_ERROR_DIR[]="./diff_error/";
const char EXPECTED_OUTPUT[] = "./expected/output.txt";
const char PASS_MSG[] = "PROGRAM RAN";
const char COMPILER_ERROR_MSG[] = "COMPILER ERROR";
const char RUNTIME_ERROR_MSG[] = "RUNTIME ERROR";
const char OUTPUT_ERROR_MSG[] = "OUTPUT ERROR";


int total_requests = 0;
int served_requests = 0;


//the function to generate unique Request ID : timestamp appended with random number
string generateReqID() {
    auto currentTime = chrono::system_clock::now();
    auto durationSinceEpoch = currentTime.time_since_epoch();
    auto secondsSinceEpoch = chrono::duration_cast<std::chrono::nanoseconds>(durationSinceEpoch);
    time_t timeInNano = secondsSinceEpoch.count();
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1000, 9999);
    int randomNum = dis(gen);
    string uuid = to_string(timeInNano) + to_string(randomNum);
    return uuid;
}



struct Result {
    string result_msg;
    string result_details;
};


//function to find the filename in the status queue
Result findResult(const string& filename) {
    ifstream file("StatusQueue.txt");
    string line;
    Result result;

    while (getline(file, line)) {
        istringstream iss(line);
        string file, result_msg, result_details;
        if (getline(iss, file, ',') &&
            getline(iss, result_msg, ',') &&
            getline(iss, result_details)) {
            if (file == filename) {
                result.result_msg = result_msg;
                result.result_details = result_details;
                return result; 
            }
        }
    }

   
    result.result_msg = "File not found";
    return result;
}

//The pool of threads which are continuously grading the submissions ,and storing in the statusQueue
//they are taking the request from the queue.
void* start_worker(void *)
{
    while(1){
        string filename;
        pthread_mutex_lock(&Lock);
        while(Queue.isEmpty()){
            pthread_cond_wait(&items_available,&Lock);
        }
        pthread_mutex_lock(&StatusFileLock);
        filename=Queue.pop();
        string to_input=filename+","+"It is being processed, no file";
        statusQueue.push(to_input.c_str());
        pthread_mutex_unlock(&StatusFileLock);
        
        pthread_cond_signal(&space_available);
        pthread_mutex_unlock(&Lock);
            
        string source_file =  filename;

        //getting request id from filename
        size_t lastSlashPos = source_file.find_last_of('/');
        size_t lastDotPos = source_file.find_last_of('.');
        string request_id = source_file.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);

        string executable = EXECUTABLES_DIR + request_id + ".o";
        string output_file = OUTPUTS_DIR + request_id + ".txt";
        string compiler_error_file = COMPILER_ERROR_DIR + request_id + ".err";
        string runtime_error_file = RUNTIME_ERROR_DIR + request_id+ ".err";
        string diff_error_file=DIFF_ERROR_DIR + request_id+ ".err";
        string compile_command = "g++ " + source_file + " -o " + executable + " > /dev/null 2> " + compiler_error_file;
        string run_command = executable + " > " + output_file + " 2> " + runtime_error_file;
        string diff_command="diff ./expected/output.txt " + output_file + " > " + diff_error_file;

        string result_msg = "";
        string result_details = "";

        if (system(compile_command.c_str()) != 0)
        {
            result_msg = COMPILER_ERROR_MSG;
            result_details = compiler_error_file;
        }
        else if (system(run_command.c_str()) != 0)
        {
            result_msg = RUNTIME_ERROR_MSG;
            result_details = runtime_error_file;
        }
        else if (system(diff_command.c_str())!=0){
            result_msg=OUTPUT_ERROR_MSG;
            result_details = diff_error_file;
        }
        else 
        {
            result_msg = PASS_MSG;
            result_details = output_file;
        }
        
        string status=filename+","+result_msg+","+result_details;
        string process=filename+","+"It is being processed, no file";
        
        pthread_mutex_lock(&StatusFileLock);
        statusQueue.removeElement(process.c_str());
        statusQueue.push(status.c_str());
        pthread_mutex_unlock(&StatusFileLock);
    
    }

}


//The grader function which takes request from the fileQueue and puts it into the Queue
//So that the workers can process it.
void* grader(void *){
	while(1){
	pthread_mutex_lock(&fileLock);
	if(fileQueue.isEmpty()){
		pthread_cond_wait(&data_available,&fileLock);
	}
	string filename=fileQueue.pop();
	
	pthread_mutex_unlock(&fileLock);
	
	pthread_mutex_lock(&Lock);
    while(Queue.size()==Size){
        pthread_cond_wait(&space_available,&Lock);
    }
    Queue.push(filename.c_str());
    pthread_cond_signal(&items_available);
    pthread_mutex_unlock(&Lock);
 	}
 	pthread_exit(nullptr);

}


//The function which looks for the request ID in the queues ,and sends the status back to the client.
void* serve_request(void* args) {
    ThreadArgs* threadArgs = reinterpret_cast<ThreadArgs*>(args);

    // Access the parameters in the thread function

    string request_id = threadArgs->request_id;
    int client_sockfd = threadArgs->client_sockfd;
    string submit(SUBMISSIONS_DIR);
    string filename=submit+request_id+".cpp";
   
    int pos=Queue.findPosition(filename.c_str());
    if(pos!=-1){
    	string position=to_string(pos);
    	send(client_sockfd,"0",sizeof(int),0);
    	send(client_sockfd,position.c_str(),position.length()+1,0);
        close(client_sockfd);
    	pthread_exit(nullptr);
    }
    pos=fileQueue.findPosition(filename.c_str());
    if(pos!=-1){
    	pos+=Queue.size();
    	string position=to_string(pos);
    	send(client_sockfd,"0",sizeof(int),0);
    	send(client_sockfd,position.c_str(),position.length(),0);
        close(client_sockfd);
    	pthread_exit(nullptr);
    }
    
    string result_Msg,result_Details;
    pthread_mutex_lock(&StatusFileLock);
    Result foundResult = findResult(filename.c_str());
    pthread_mutex_unlock(&StatusFileLock);
    if (!foundResult.result_msg.empty() && foundResult.result_msg != "File not found") {
    	if(foundResult.result_msg=="It is being processed"){
    		send(client_sockfd,"3",sizeof(int),0);
    		pthread_exit(nullptr);
    	}
        result_Msg=foundResult.result_msg;
        result_Details=foundResult.result_details;
    } else {
    	send(client_sockfd,"1",sizeof(int),0);
        close(client_sockfd);
        pthread_exit(nullptr);
    }
    
    ifstream inputFile(result_Details); 
    string file_to_send="./results/result_"+request_id+".txt";


    ofstream outputFile(file_to_send.c_str(), ios::app); 

    if (outputFile.is_open() && inputFile.is_open()) {
        outputFile << "DONE\n" << result_Msg << endl; 

      
        outputFile << inputFile.rdbuf();

        inputFile.close(); 
        outputFile.close(); 

    }
    
    string removing_element = filename+","+result_Msg+","+result_Details;
    pthread_mutex_lock(&StatusFileLock);
    statusQueue.removeElement(removing_element);
    pthread_mutex_unlock(&StatusFileLock);
    send(client_sockfd,"2",sizeof(int),0);
    if(send_file(client_sockfd,file_to_send.c_str())!=0){
    	perror("Error in sending result to client");
        close(client_sockfd);
    	pthread_exit(nullptr);
    }
    close(client_sockfd);
    pthread_exit(nullptr);     
}
	

//The function which handles the new submissions
void* handle_new(void *args){
    int sockfd= *(int*)args;
    string request_id=generateReqID();
    string submit(SUBMISSIONS_DIR);
    string filename=submit+request_id+".cpp";
 
    if (recv_file(sockfd, filename) != 0)
    {
        close(sockfd);
        pthread_exit(nullptr);
    }
    pthread_mutex_lock(&fileLock);
    fileQueue.push(filename.c_str());
    pthread_cond_signal(&data_available);
    pthread_mutex_unlock(&fileLock);
    cout<<"File Received"<<endl;
    string message="Submission accepted for grading , your request id is:"+request_id;
    send(sockfd,message.c_str(),100,0);
  

    pthread_exit(NULL);

}

//the function which handles the request for the status of the submission.
void* handle_status(void *args){
    int sockfd=*(int*)args;
    char request[256];
    bzero(request,256);
    recv(sockfd,request,255,0);
    string request_id=request;
    ThreadArgs* Args = new ThreadArgs();
    Args->request_id = request_id;
    Args->client_sockfd = sockfd;
    pthread_t find_request;
    int rc=pthread_create(&find_request,nullptr, serve_request,  reinterpret_cast<void*>(Args));
    assert(rc==0);
    pthread_detach(rc);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: ./server  <port> <pool-size>\n";
        return -1;
    }
     
    Size=atoi(argv[2]);
    int port = stoi(argv[1]);
    
    system("touch RequestQueue.txt");
    system("sed -i '/It is being processed/d' StatusQueue.txt > /dev/null");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Socket creation failed");
        return 1;
    }
    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    int iSetOption = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&iSetOption, sizeof(iSetOption));
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
    {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }
    if (listen(sockfd, MAX_QUEUE) != 0)
    {
        perror("Listen failed");
        close(sockfd);
        return -1;
    }

    cout << "Server listening on port: " << port << "\n";

	// creating the directories
    try
    {
        if (!fs::exists(SUBMISSIONS_DIR))
            fs::create_directory(SUBMISSIONS_DIR);
        if (!fs::exists(EXECUTABLES_DIR))
            fs::create_directory(EXECUTABLES_DIR);
        if (!fs::exists(OUTPUTS_DIR))
            fs::create_directory(OUTPUTS_DIR);
        if (!fs::exists(COMPILER_ERROR_DIR))
            fs::create_directory(COMPILER_ERROR_DIR);
        if (!fs::exists(RUNTIME_ERROR_DIR))
            fs::create_directories(RUNTIME_ERROR_DIR);
        if (!fs::exists(DIFF_ERROR_DIR))
            fs::create_directories(DIFF_ERROR_DIR);
        if (!fs::exists(EXPECTED_OUTPUT))
            fs::create_directories(EXPECTED_OUTPUT);
        
    }
    catch (fs::filesystem_error &e)
    {
        cerr << "Error creating directories: " << e.what() << "\n";
        close(sockfd);
        return -1;
    }
    
    // creating the pool of threads
    pthread_t receive_thread[Size];
	for (int i = 0; i < Size; i++) 
	{
        int rc=pthread_create(&receive_thread[i], nullptr, start_worker, nullptr);
        assert(rc==0);
        pthread_detach(rc);
    }
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    
    pthread_t grader_var;
    //creating the grader threads
    int ac=pthread_create(&grader_var,nullptr, grader,nullptr);
    assert(ac==0);
    pthread_detach(ac);

    
    while (true)
    {
        int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        int size_int=sizeof(int);
        char request_type[size_int];
    	if (recv(client_sockfd, request_type, sizeof(request_type), 0) == -1)
    	{
        	perror("Error receiving request type");
        	return -1;
    	}
    	int type=atoi(request_type);
    	
    	
    	
    	//new request
    	if(type==0){ 

                pthread_t new_req;
                int *arg=(int*)malloc(sizeof(int));
     	    	*arg=client_sockfd;
                int rc= pthread_create(&new_req,nullptr,handle_new,(void*)arg);
                assert(rc==0);
                pthread_detach(rc);
      
    	}
    	
    	// status request
    	else{

            pthread_t status_req;
            int *arg=(int*)malloc(sizeof(int));
     	    *arg=client_sockfd;
            int rc= pthread_create(&status_req,nullptr, handle_status,(void*)arg);
            assert(rc==0);
            pthread_detach(rc);
    		
    	}        
    }

    close(sockfd);
    return 0;
}
