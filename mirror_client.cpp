#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>
#include <iostream>      
#include <stdlib.h>
#include <cstring>
#include <fstream>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "list.h"


using namespace std;

bool flag=true;

void WriteLogFile(char* logfile,const char* szString,int size)      //  https://www.codeproject.com/Questions/97485/how-to-write-log-file-in-C
{

  FILE* pFile = fopen(logfile, "a");
  fprintf(pFile, "I received file with name %s",szString);
  fprintf(pFile, " and bytes =  %d\n",size); 
  fclose(pFile);

}


void signalhandler(int signal){
	flag = false;
    if(signal == SIGINT){
        cout << "  SIGNAL --> SIGINT " << endl;
    }
    else{
        cout << "  SIGNAL --> SIGQUIT " << endl; 
    }
}

void cptree(const char path[],int fd)      // https://stackoverflow.com/questions/5467725/how-to-delete-a-directory-and-its-contents-in-posix-c
{
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_path, stat_entry;
    struct dirent *entry;

    // stat for the path
    stat(path, &stat_path);

    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0) {
        fprintf(stderr, "%s: %s\n", "Is not directory", path);
        return;
    }

    // if not possible to read the directory for this user
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
        return;
    }

    // the length of the path
    path_len = strlen(path);

    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {

        // skip entries "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;


        // determinate a full path of an entry
        full_path = new char[path_len + strlen(entry->d_name) + 2];
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);


        // stat for the entry
        stat(full_path, &stat_entry);

        // recursively remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0) {
            cptree(full_path,fd);
            continue;
        }

        // copy a file object

        char* relative=strchr(full_path,'/')+1;

        short length= 1 + strlen(relative);

        write(fd,&length,sizeof(length));       // write 2 bytes --> size of name 

        write(fd,relative,length);              // write the name of file

        int size = stat_entry.st_size;      

        write(fd,&size,sizeof(size));           // write 4 bytes --> size of file

        //cout << "relative = " <<relative <<endl;

        int file =open(full_path,O_RDONLY);

        char buf[256];
        int len;

        while((len=read(file,buf,sizeof(buf)))>0){
            cout << "I just read the file with name " << relative << " . File with name  "  << relative << " has "<< len << " number of bytes ." << endl;
            cout << "                                                                                                                          " << endl;
            write(fd,buf,len);          // write the file
        }
        close(file);



        delete[] full_path;
    }

    closedir(dir);

}

void createsender(char* dir,char* fifo){
    pid_t pid;
    switch(fork()){                     // process generator 
        case -1:
            perror("Forking Error");
            return;
        case 0:
            break;
        default:
            return;
    }
    int fd = open(fifo,O_WRONLY);

    cptree(dir,fd);
    short length=0;
    write(fd,&length,sizeof(length));     // finish
    close(fd);
    exit(0);
}

void createreceiver(char* dir,char* fifo,char* logfile){

    pid_t pid;
    switch(fork()){                   // process generator
        case -1:
            perror("Forking Error");
            return;
        case 0:
            break;
        default:
            return;
    }

    int fd = open(fifo,O_RDONLY);

    while(true){

        char name[PATH_MAX+1];
        short length;
        read(fd,&length,sizeof(length));          // size of filename that I received

        if(length==0){
            break;
        }

        read(fd,name,length);                   // name of file that I received
        int size;
        read(fd,&size,sizeof(size));

        WriteLogFile(logfile,name,size);

        cout << "The size of filename that I received is " << length << endl;
        cout << "The name of file that I received is " << name << endl;
        cout << "The size of file that I received is " << size << endl;
        cout << "                                    " << endl;

        char tmp[PATH_MAX+1];
        
        strcpy(tmp , dir );
        strcat(tmp , "/" );
        strcat(tmp , name );
        //cout<< "tmp = " << tmp << endl;

        int file = open(tmp,O_CREAT|O_WRONLY,0644);        // create the file into mirror_dir
        char buf[256];
    
        while(size>0){
            int len;
            if(size < sizeof(buf)){
                len=read(fd,buf,size);
            }
            else{
                len=read(fd,buf,sizeof(buf));
            }

            //cout<< "len = " << len << endl;

            if(len>0){
                write(file,buf,len);
                size-=len;
            }
        }
        close(file);
    }
    close(fd);
    unlink(fifo);
    exit(0);
    
}

void rmtree(const char path[])		// https://stackoverflow.com/questions/5467725/how-to-delete-a-directory-and-its-contents-in-posix-c
{
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_path, stat_entry;
    struct dirent *entry;

    // stat for the path
    stat(path, &stat_path);

    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0) {
        fprintf(stderr, "%s: %s\n", "Is not directory", path);
        return;
    }

    // if not possible to read the directory for this user
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
        return;
    }

    // the length of the path
    path_len = strlen(path);

    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {

        // skip entries "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        // determinate a full path of an entry
        full_path = new char[path_len + strlen(entry->d_name) + 1];
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);

        // stat for the entry
        stat(full_path, &stat_entry);

        // recursively remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0) {
            rmtree(full_path);
            continue;
        }

        // remove a file object
        unlink(full_path);

        delete[] full_path;
    }
    closedir(dir);

    // remove the devastated directory and close the object of it
 	rmdir(path);

}


int main(int argc, char *argv[]){

	int clientId;
    int buffer_size;
    int mypid = getpid();
 


	if(argc > 7){                                // check the number of arguments,must be 7 .
        cout <<"Too many arguments \n"<< endl;
        return -1;
    }

    if(argc < 7){
        cout <<"Too few arguments\n" << endl;
        return -1;
    }


    clientId = atoi(argv[1]);
    buffer_size = atoi(argv[5]);
    char buffer[buffer_size];

    cout << "Hello , I am the client with id: " << clientId << " , and I just came to this dropbox app ... " << endl;
    cout << "                                                                                              " << endl;


    char* common_dir = argv[2];                       // common_dir directory creation
    struct stat st ;
    if (stat(common_dir, &st) == -1) {

        if(mkdir(common_dir, 0755) == -1){
            
            perror("mkdir: ");
            return -1;
        }
    }
    else{

        cout << "Directory with name : " << common_dir << " already exists , so we don't need to create it again ." << endl;
        cout << "                                                                                                 " << endl;

    }
   
	

    char idfile[PATH_MAX+1];      
    strcpy(idfile , common_dir );
    strcat(idfile , "/" );
    strcat(idfile , argv[1] );

	if(stat(idfile,&st)==0){
		perror(".id file exists");
		return -1;
	}

    FILE *fp = fopen(idfile , "w");         // create '.id ' file and write mypid inside
    if (fp == NULL)
    	exit(EXIT_FAILURE);

    fprintf(fp, "%d\n",mypid);
    fclose(fp);

       
    


    char* input_dir = argv[3];                        // check if input_dir exist , if not terminate
	                      
    if (stat(input_dir, &st) == -1) {
    	cout << "There is no input file , I need to say Goodbye ... " << endl;
    	return -1;
	}


    char* mirror_dir = argv[4];                       // mirror_dir directory creation 
    
    if (stat(mirror_dir, &st) != -1) {

    	cout << "Directory with name : " << mirror_dir << " already exists , so I need to say Goodbye ..." << endl;
    	return -1;
    }                     
	if(mkdir(mirror_dir, 0755) == -1){
		perror("mkdir: ");
		return -1;
	}



	signal(SIGINT,signalhandler);
	signal(SIGQUIT,signalhandler);

	list* previous = new list();
	list* current = new list();


	while(flag){                               // Checking for new clients 


		DIR* dir = opendir(argv[2]);

		struct dirent *entry;

		while ((entry = readdir(dir)) != NULL){                   

            if ( !strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")  || !strcmp(entry->d_name, argv[1]) || strstr(entry->d_name,"_to_") ){
                continue;
            }

            current->insert(entry->d_name);
            
		    if(!previous->find(entry->d_name)){           //query 4


			    cout << "New mirror client with id : " << entry->d_name << " has joined to our app ... "<<  endl;
                cout << "                                                                               " << endl;

                char mirror[PATH_MAX+1];
                strcpy(mirror , mirror_dir );
                strcat(mirror , "/" );
                strcat(mirror , entry->d_name );
                mkdir(mirror,0755);


                char metoother[PATH_MAX+1];
                strcpy(metoother , common_dir );
                strcat(metoother , "/" );
                strcat(metoother , argv[1] );
                strcat(metoother , "_to_" );
                strcat(metoother , entry->d_name );


                char othertome[PATH_MAX+1];
                strcpy(othertome , common_dir );
                strcat(othertome , "/" );
                strcat(othertome , entry->d_name );
                strcat(othertome , "_to_" );
                strcat(othertome , argv[1] );


                if( clientId < atoi(entry->d_name) ){             // client with the smallest id first

                    cout << "---------------------------------- Sender ------------------------------------------------" << endl;
                    

                    if (mkfifo(metoother, 0666) < 0)
                        perror("mkfifo");

                    if (mkfifo(othertome, 0666) < 0)
                        perror("mkfifo");

                    createsender(input_dir,metoother);
                    createreceiver(mirror,othertome,argv[6]);
                }
                else{
                    cout << "---------------------------------- Receiver ----------------------------------------------" << endl;

                    createreceiver(mirror,othertome,argv[6]);
                    createsender(input_dir,metoother);  
                }
		    }
    
        }


		while (!previous->isempty()){                 // query 5
            char* name = previous->pop();
			if(!current->find(name)){
                char path[PATH_MAX+1];
        
                strcpy(path , mirror_dir );
                strcat(path , "/" );
                strcat(path , name );
                rmtree(path);
				cout<< "Client deleted from : " << path << " . "<< endl;

			}
            delete[] name;
		}
		list* temp = previous;
		previous = current;
		current = temp;
		closedir(dir);
	}

	delete previous;
	delete current;
            
    unlink (idfile); 
    rmtree(argv[4]);   

    return 0;
}








