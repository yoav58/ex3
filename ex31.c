// Yoav Eliav 312498207

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
int CompareChars(char first,char second);

int main(int argc, const char * argv[]) {
    // arguements for read/write
    ssize_t size = 2;
    char buf[3];
    ssize_t count_read = 0;
    ssize_t count_write = 0;
    
    
    if(argc != 3){ // in case the argument number dont match our expetantion
        return -1;
    }
    int copyFile = open("copy.txt",O_CREAT|O_RDWR,0666); // we create a copy file for a case it the same file)
    int firstFile = open(argv[1],O_RDONLY);
    int secontFile = open(argv[2],O_RDONLY);
    if(copyFile < 0  || firstFile < 0 || secontFile <  0 ){ // check if all the open successed!
        perror("Error in: open");
        close(copyFile);
        remove("copy.txt");
        return -1;
    }
    
    // write to the copy file
    do{
        buf[0] = '\0';
        buf[1] = '\0';
        buf[2] = '\0';
        count_read  = read(secontFile,buf,size);
        if(count_read == -1){
            perror("Error in: read");
            close(copyFile);
            remove("copy.txt");
            return -1;
        }
        count_write = write(copyFile,buf,size);
        if(count_write == -1){
            perror("Error in: write");
            close(copyFile);
            remove("copy.txt");
            return -1;
        }
        if(count_write < count_read) perror("not all the bytes readed");
    }
    while(count_write == size && count_read == size);
    int closed = close(secontFile); // close the second file as we dont need him anymore
    if(closed == -1){
        perror("Error in: read");
        close(copyFile);
        remove("copy.txt");
        return -1;
    }
    // return to the begining.
    if(lseek(copyFile, 0, SEEK_SET) < 0){
        perror("Error in: lseek");
        close(copyFile);
        remove("copy.txt");
        return -1;
    }
    // start compare
    ssize_t count_read_copy;
    char first_char[1];
    char second_char[1];
    int charsNmb1 = 0;
    int charsNmb2 = 0;
    int files_similarity;
    int result = 1;
    do{
        // read the chars
        first_char[0] = '\0';
        second_char[0] = '\0';
        count_read = read(firstFile,first_char,1);
        count_read_copy = read(copyFile, second_char, 1);
        if(count_read == -1 || count_read_copy == -1){
            perror("Error in: read");
            return -1;
        }
        // check if its real character.
        if(first_char[0] != '\0' && first_char[0] != '\n' && first_char[0] != ' ') ++charsNmb1;
        if(second_char[0] != '\0' && second_char[0] != '\n' && second_char[0] != ' ') ++charsNmb2;
        // pass over all the spaces or new line
        if(first_char[0] == second_char[0]) continue;
        result = 3;
        while(first_char[0] == ' ' || first_char[0] == '\n'){
            first_char[0] = '\0';
            count_read = read(firstFile,first_char,1);
        }
        while(second_char[0] == ' ' || second_char[0] == '\n'){
            second_char[0] = '\0';
            count_read_copy = read(copyFile, second_char, 1);
        }
        // check the comparsion
        files_similarity = CompareChars(first_char[0], second_char[0]);
        if(files_similarity == 2){
            close(firstFile);
            close(copyFile);
            remove("copy.txt");
            return 2;
        }
        if(files_similarity == 3) result = 3;
    }while((count_read !=0) && (count_read_copy !=0));
    close(firstFile);
    close(copyFile);
    remove("copy.txt");
    return result;
    return 0;
}

// 1 means identical,3 similar, 2 diffrent
int CompareChars(char first,char second){
    if(first == second) return 1;
    if(first < 91 && second < 91) return 2;
    if(first > 96 && second > 96) return 2;
    if( (first + 32) == second) return 3;
    if( (first - 32) == second) return 3;
    if( first == (second + 32)) return 3;
    if(first == (second - 32)) return 3;
    return 2;
}

