// Yoav Eliav 312498207

#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <signal.h>
/****************************************************************************************************
 * helping functions
 **************************************************************************************************/
int writeToResult(int fd,char* name,char* grade,char* descripation);
int checkif_c_file(char* s);
void initializeArray(char array[3][150]);
void delay(int number_of_seconds);
void changeToPATH(char * path,char* add);
void initialize1dArray(char array[150]);
/*************************************************************************************************
 * main function
 ***********************************************************************************************/
int main(int argc, const char * argv[]) {
    // read the argument
    argv[1] = "conf.txt";
    char buffer[1];
    ssize_t reading;
    int row = 0;
    int collum =0;
    int argument = open(argv[1],O_RDONLY);
    char pathes[3][150];
    initializeArray(pathes);
    // divide the path of the argument file.
    do{
        buffer[0] = '\0';
        reading = read(argument,buffer,1);
        if(reading == -1){
            perror("Error in: read");
            return -1;
        }
        // divide the paths
        if(buffer[0] == '\n'){
            ++row;
            collum = 0;
            continue;
        }
        pathes[row][collum] = buffer[0];
        ++collum;
    }while(reading != 0);
    close(argument);
    //open stream of  the main directory
    DIR* dip;
    struct dirent *dit;
    if((dip = opendir(pathes[0])) == NULL){
        perror("Not a valid directory");
        return -1;
    }
    // save current runing file folder
    char mainProgramPath[150];
    initialize1dArray(mainProgramPath);
    // open input file.
    int inputFile = open(pathes[1],O_CREAT|O_RDWR,0666);
    // create csv file
    int outputfile = open("result.csv",O_CREAT|O_RDWR,0666);
    int errorfile = open("error.txt",O_CREAT|O_WRONLY);
    // check for failures.
    if( (inputFile == -1) || (errorfile == -1) || (outputfile == -1) ){
        perror("Error in: open");
        return -1;
    }
    // start moving over all the directories
    if(chdir(pathes[0]) == -1) return -1; // change our direction to the path direction.
    int i = 0;
    while((dit = readdir(dip)) != NULL){
        
        if(i > 2){ // the first 2 are irellevent
            if(dit->d_type != DT_DIR) continue; // if its not a dir we move on.
            pid_t pd;
            char* currentFolder = dit->d_name;
            pd = fork(); // create child process
            if(pd == -1){
                    perror("Error in: fork");
                    return -1;
            }
            
            /*******************
            * children process
            **********************/
            if(pd == 0 ){
                int fileExist = 0;
                char* filename;
                char* folder = dit->d_name; // save the directory name;
                dip = opendir(dit->d_name); // open directory process
                int newfd = open("filecheck",O_CREAT|O_TRUNC|O_WRONLY,0666); // create the input file for check
                if(newfd < -1){
                    perror("Error in: open");
                    return -1;
                }
                while((dit = readdir(dip)) != NULL){ // check if c file exist
                if(checkif_c_file(dit->d_name)){ // we find the c file
                    fileExist = 1; // found the file.
                    filename = dit->d_name;
                    break;
                }
                }
                // if the file exist we check there isn compilation error
                if(fileExist){
                    /*
                     * grand children.
                     */
                    filename = dit->d_name;
                    pid_t p = fork();
                    if(p == -1){
                        perror("Error in: fork");
                        return -1;
                    }
                    if(p == 0){
                        chdir(folder);
                        dup2(errorfile,2);
                        char* arginput[] = {"gcc",filename,NULL};
                        execvp("gcc", arginput);
                        return -1;
                    }
                    // return to children process
                    else{
                        int status;
                        waitpid(p, &status,0);
                        int child_answer = WEXITSTATUS(status);
                        if(child_answer != 0) return 2; // return 2 in case there compiler error.
                        // if we get here that mean that we cover the first two cases.
                        pid_t second_grandchild = fork();
                        if(second_grandchild == -1){
                            perror("Error in: fork");
                            return -1;
                        }
                        // second grandchild run the file
                        if(second_grandchild==0){
                            chdir(folder);
                            int outputFile = open("output.txt",O_CREAT|O_TRUNC|O_WRONLY,0666);
                            lseek(inputFile, 0, SEEK_SET);
                            dup2(outputFile,1);
                            dup2(inputFile,0);
                           // close(inputFile);
                            close(outputFile);
                            execl("./a.out","./a.out",NULL);
                            return -1;
                        }
                        else{
                            int status;
                            time_t start, end;
                            // wait 5 seconds
                            time(&start);
                            do time(&end); while(difftime(end, start) <= 5);
                            int x = waitpid(second_grandchild, &status, WNOHANG);
                            if(x == 0) return 3; // in this case we waited more then 5 seconds
                            // we cover the 3 first cases now the 4-5-6 cases at once.
                            // chdir(mainProgramPath);
                            pid_t third_grandchild = fork();
                            if(third_grandchild == -1){
                                perror("Error in: fork");
                                return -1;
                            }
                            // third grand children.
                            if(third_grandchild == 0){
                                chdir(mainProgramPath);
                                long size = strlen(dit->d_name);
                                size += strlen(pathes[0]);
                                size += 2;
                                char fullPath[150];
                                initialize1dArray(fullPath);
                                strcat(fullPath, pathes[0]);
                                strcat(fullPath, "/");
                                strcat(fullPath,currentFolder);
                                strcat(fullPath,"/");
                                strcat(fullPath,"output.txt");
                                char* arguments[] = {"./comp.out",pathes[2],fullPath,NULL};
                                execv("./comp.out",arguments);
                                return -1;
                            }
                            // return to the children
                            else{
                                int status;
                                waitpid(third_grandchild, &status, 0);
                                 chdir(folder);
                                // delete the relvant files.
                                 remove("output.txt");
                                 remove("a.out");
                                int child_answer = WEXITSTATUS(status);
                                if(child_answer == 1) return 6;
                                if(child_answer == 2) return 4;
                                if(child_answer == 3) return 5;
                            }
                            
                        }
                    }
                }
                else{
                    return 0; // in case c file dont exist
                }
            }
            else{
            /************************
             *father process
             ***********************/
                int status;
                waitpid(pd, &status, 0);
                int child_answer = WEXITSTATUS(status);
                // no c file
                if(child_answer == 0){
                    writeToResult(outputfile, dit->d_name, ",0",",NO_C_FILE");
                }
                // compiler error
                if(child_answer == 2){
                    writeToResult(outputfile, dit->d_name , ",10", ",COMPILATION_ERROR");
                }
                if(child_answer == 3){
                    writeToResult(outputfile, dit->d_name, ",20", ",TIMEOUT");
                }
                if(child_answer == 4){
                    writeToResult(outputfile, dit->d_name, ",50", ",WRONG");
                }
                if(child_answer == 5){
                    writeToResult(outputfile, dit->d_name, ",75", ",SIMILAR");
                }
                if(child_answer == 6){
                    writeToResult(outputfile, dit->d_name, ",100", ",EXCELLENT");
                }
            }
      
    }
        ++i;
    }
    // close all the opened files.
    close(errorfile);
    close(inputFile);
    close(outputfile);
    return 0;
}


/************************************************************
*helping functions.
*************************************************************/
void initialize1dArray(char array[150]){
    int i = 0;
    for(;i < 150; i++){
        array[i] = '\0';
    }
}
void initializeArray(char array[3][150]){
    int i = 0;
    for(; i < 3; i++){
        int x = 0;
        for(; x<150;x++)
        array[i][x] = '\0';
    }
}

int checkif_c_file(char* s){
    unsigned long length = strlen(s);
    if (length > 2 && !strcmp(s + length - 2,".c")) return 1;
    return 0;
}

int checkEqual(char* s){
    return 5;
}

int writeToResult(int fd,char* name,char* grade,char* descripation){
    long xsize = strlen(name);
    xsize += strlen(grade);
    xsize += strlen(descripation);
    char n[xsize+2];
    strcpy(n, name);
    strcat(n, grade);
    strcat(n, descripation);
    strcat(n, "\n");
    write(fd,n, xsize+2);
    return 1;
}
void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
  
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds);
}

void changeToPATH(char * path,char* add){
    long size = strlen(add);
    size += strlen(path);
    size += 2;
    char fullPath[size];
    strcat(fullPath, path);
    strcat(fullPath, "\\");
    strcat(fullPath, add);
    
}
