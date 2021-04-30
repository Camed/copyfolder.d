#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <ftw.h>
#include <features.h>
#include <utime.h>
#include <sys/mman.h>

#define _GNU_SOURCE
#define O_BINARY _O_BINARY

// bytes 31-16 - large file size
// bytes 15-1  - delay time
// byte 0 - recursive flag
// default - not recursive, 5 minutes delay (300 sec), 16384 bytes and more is regarded as a large file
// 0x|2000|025/8 - '8' contains info about recursiveness, if recursive will be changed to 9
int flags = 0x20000258;

// contains list of files, directories in both source and target directories
typedef struct ListOfElements {
    struct dirent** SourceElements;
    struct dirent** TargetElements;
    struct dirent** SourceDirectories;
    struct dirent** TargetDirectories;
    int NumberOfSourceElements;
    int NumberOfTargetElements;
    int NumberOfSourceDirectories;
    int NumberOfTargetDirectories;
} ListOfElements;


// gathers delay time
inline int getDelayTime() { return (flags >> 1) & 0x00007FFF; }

// gathers large file cap
inline int largeFileSize() { return (flags >> 16) & 0x0000FFFF; }

// checks if recursive
inline int isRecursive() { return flags & 1; }

// copy file copy0 (as path) to file1 (as path)
// size -> buffer size
// returns 0 if completedd succesfully
int Copy_y(char *file0, char *file1, int size) {
    int input, output;
    char *addr;
    ssize_t t;

    struct stat InputFileStat;
    struct stat OutputFileStat;

    struct utimbuf OutputFileBuf;

    unsigned char *BufforSpace;
    BufforSpace = (unsigned char *) malloc(size * sizeof(unsigned char));

    input = open(file0, O_RDONLY);
    output = open(file1, O_WRONLY | O_CREAT | O_TRUNC, 0664);

    stat(file0,&InputFileStat);
    stat(file1,&OutputFileStat);

    if (input == -1)
    {
        syslog(LOG_ERR, "Error, could not open file: %s", strerror(errno));
        return 0xDEAD;
    }
    if (output == -1)
    {
        syslog(LOG_ERR, "Error, could not open file: %s", strerror(errno));
        close(input);
        return 0xBEAF;
    }

    if(InputFileStat.st_size<=largeFileSize())
    {
        while ((t = read(input, BufforSpace, size)) > 0)
        {
            write(output, BufforSpace, t);
        }
        free(BufforSpace);
        BufforSpace = NULL;

        close(input);
        close(output);
        syslog(LOG_INFO, "Small file copied successfully!");
    }
    else
    {
        off_t offset,pa_offset;
        size_t length;
        length = InputFileStat.st_size;
        offset = 4196;
        pa_offset = 4196;

        addr = mmap(0, length, PROT_READ,MAP_SHARED, input, 0);

        printf("%s",addr);

        write(output,addr,length);

        close(input);
        close(output);

        syslog(LOG_INFO, "Large file copied successfully!");
    }

    OutputFileBuf.modtime = InputFileStat.st_mtime;
    utime(file1,&OutputFileBuf);

    return 0;
}

// concatenating strings
// RETURNS: concatenated strings
char* MergeStrings(char * first,char * second)  {
    char* CreatedString = "";
    asprintf(&CreatedString,"%s%s",first,second);
    return CreatedString;
}

// determine whether object is a file
// RETURNS: 1 if object is a file
//          0 if object is not a file
int FileIsFile(const char *file) {
    struct stat FileStat;
    if(lstat(file,&FileStat)==-1)
    {
        syslog(LOG_ERR,"Error occurred during determining object type: %s", strerror(errno));
    }
    lstat(file,&FileStat);
    if(S_ISREG (FileStat.st_mode))
    {
        syslog(LOG_INFO, "Object type determining completed successfully! Type: file");
        return 1;
    }
    else
    {
        syslog(LOG_INFO, "Object type determining completed successfully! Type: other");
        return 0;
    }
}

// determine whether object is a folder
// RETURNS: 1 if object is a folder
//          0 if object is not a folder
int FileIsDirectory(const char *file){
    struct stat FileStat;
    if(lstat(file,&FileStat)==-1)
    {
        syslog(LOG_ERR,"Error occurred during determining object type: %s", strerror(errno));
    }
    lstat(file,&FileStat);
    if(S_ISDIR (FileStat.st_mode))
    {
        syslog(LOG_INFO, "Object type determining completed successfully! Type: folder");
        return 1;
    }
    else
    {
        syslog(LOG_INFO, "Object type determining completed successfully! Type: other");
        return 0;
    }
}

// adds missing '/' to a path string
char* AddSlashToPath(char *source) {
    if(source[strlen(source)-1] != '/')
    {
        source= MergeStrings(source,"/");
        source[strlen(source)]='\0';
    }
    return source;
}

// gather objects which are only specified type
inline int GetOnlyFiles(const struct dirent* Element) { return FileIsFile(Element->d_name); }
inline int GetOnlyDirectories(const struct dirent* Element) { return FileIsDirectory(Element->d_name); }

// get list of directories, saves it into structure 'ListOfElements'.
// RETURNS: 0 if nothing to be done
//          1 if everything went smoothly
int GetListOfDirectories(char *source,char *target,struct ListOfElements* Elements) {
    DIR* SourceDirectory;
    DIR* TargetDirectory;

    struct dirent Element;
    struct dirent** SourceFilesList;
    struct dirent** SourceDirectoriesList;
    struct dirent** TargetFilesList;
    struct dirent** TargetDirectoriesList;

    int NumberOfElements_Source=0;
    int NumberOfElements_Target=0;

    int NumberOfDirectories_Source=0;
    int NumberOfDirectories_Target=0;

    //source = AddSlashToPath(source);
    //target = AddSlashToPath(target);

    SourceDirectory = opendir(source);
    TargetDirectory = opendir(target);

    // gathering data from source directory
    chdir(source);
    NumberOfElements_Source = scandir(source, &SourceFilesList,GetOnlyFiles, alphasort);
    NumberOfDirectories_Source = scandir(source, &SourceDirectoriesList,GetOnlyDirectories, alphasort);

    // gathering data from target directory
    chdir(target);
    NumberOfElements_Target = scandir(target, &TargetFilesList,GetOnlyFiles, alphasort);
    NumberOfDirectories_Target = scandir(target, &TargetDirectoriesList,GetOnlyDirectories, alphasort);

    // if nothing to be done
    if(NumberOfElements_Source<0 || NumberOfElements_Target<0)
    {
        return 0;
    }

    // setting up files data
    Elements->NumberOfSourceElements=NumberOfElements_Source;
    Elements->NumberOfTargetElements=NumberOfElements_Target;
    Elements->SourceElements=SourceFilesList;
    Elements->TargetElements=TargetFilesList;

    // setting up directory data
    Elements->NumberOfSourceDirectories = NumberOfDirectories_Source;
    Elements->NumberOfTargetDirectories = NumberOfDirectories_Target;
    Elements->SourceDirectories = SourceDirectoriesList;
    Elements->TargetDirectories = TargetDirectoriesList;

    return 1;
}

// deleting file
// RETURNS: 0 if file deleted successfully
//          -1 if error while deleting file
int RemoveFile(const char *path, const struct stat *data, int type) {
    if(remove(path) == -1)
    {
        syslog(LOG_ERR, "File deletion error, could not delete file: %s", strerror(errno));
        return -1;
    }
    syslog(LOG_INFO, "File deletion procedure completed successfully!");
    return 0;
}

// deleting folder
// RETURNS: 0 if processed successfully
//          -1 if errored while deleting folder
int RemoveDirectories(char *path) { return ftw(path, RemoveFile, 10); }

// merges directories into one
// RETURNS: 0 if processed successfully
int MergeDirectories(char *source, char *target) {
    struct ListOfElements Elements;
    int output=-1;
    GetListOfDirectories(source,target,&Elements);
    chdir(target);

    for(int i=0; i<Elements.NumberOfTargetDirectories; i++)
    {
        output=-1;
        for(int ii=0; ii<Elements.NumberOfSourceDirectories; ii++)
        {

            if(strcmp(Elements.SourceDirectories[ii]->d_name,Elements.TargetDirectories[i]->d_name) ==0)
            {
                output=0;
            }
        }
        if (output!=0)
        {
            RemoveDirectories(Elements.TargetDirectories[i]->d_name);
            GetListOfDirectories(source, target, &Elements);
            i--;
        }
    }

    output=-1;

    for(int j=0; j<Elements.NumberOfSourceDirectories; j++)
    {
        output=-1;

        for(int jj=0; jj<Elements.NumberOfTargetDirectories; jj++)
        {
            if(strcmp(Elements.SourceDirectories[j]->d_name,Elements.TargetDirectories[jj]->d_name) ==0)
            {
                output=0;
            }
        }
        if (output==-1)
        {
            char tmp_target[512];
            char tmp_source[512];

            source = AddSlashToPath(source);
            target = AddSlashToPath(target);

            strcpy(tmp_source, source);
            strcpy(tmp_target, target);
            strcat(tmp_source, Elements.SourceDirectories[j]->d_name);
            strcat(tmp_target, Elements.SourceDirectories[j]->d_name);


            printf("%s\n",tmp_source);
            printf("%s\n",tmp_target);

            struct stat mdata;
            stat(tmp_source, &mdata);
            mkdir(tmp_target, mdata.st_mode);
        }
    }
    return 0;
}

// merges files in both folders, source and target
// RETURNS: 0 if processed successfully
int MergeFiles(char *source, char *target) {
    struct ListOfElements Elements;
    int output=-1;

    GetListOfDirectories(source, target, &Elements);
    chdir(target);

    source = AddSlashToPath(source);
    target = AddSlashToPath(target);

    if(Elements.NumberOfTargetElements!=0) {
        for (int i = 0; i < Elements.NumberOfTargetElements; i++) {
            output = -1;
            for (int ii = 0; ii < Elements.NumberOfSourceElements; ii++) {
                if (strcmp(Elements.SourceElements[ii]->d_name, Elements.TargetElements[i]->d_name) == 0) {
                    output = 0;

                    char tmp_target[512];
                    char tmp_source[512];
                    strcpy(tmp_target, target);
                    strcat(tmp_target, Elements.SourceElements[i]->d_name);
                    strcpy(tmp_source, source);
                    strcat(tmp_source, Elements.SourceElements[i]->d_name);

                    struct stat source_data, target_data;
                    stat(tmp_target, &target_data);
                    stat(tmp_source, &source_data);
                    printf("%s\n", tmp_source);
                    printf("%s\n", tmp_target);

                    if (source_data.st_mtime != target_data.st_mtime) {

                        printf("%s\n", tmp_source);
                        printf("%s\n", tmp_target);
                        Copy_y(tmp_source, tmp_target, 4096);
                    }
                }
            }
            if (output != 0) {
                unlink(Elements.TargetElements[i]->d_name);
                GetListOfDirectories(source, target, &Elements);
                i--;
            }
        }
    }
    for(int j=0; j<Elements.NumberOfSourceElements; j++)
    {
        output=-1;
        for(int jj=0; jj<Elements.NumberOfTargetElements; jj++)
        {
            if(strcmp(Elements.SourceElements[j]->d_name,Elements.TargetElements[jj]->d_name) ==0)
            {
                output=0;
            }
        }
        if (output==-1)
        {
            char tmp_target[512];
            char tmp_source[512];
            strcpy(tmp_target, target);
            strcat(tmp_target, Elements.SourceElements[j]->d_name);
            strcpy(tmp_source, source);
            strcat(tmp_source, Elements.SourceElements[j]->d_name);

            Copy_y(tmp_source, tmp_target, 8192);
        }
    }
    return 0;
}

// recursive file and folder merging
void InitDeamon(char *source,char *target) {
    MergeDirectories(source,target);
    MergeFiles(source,target);

    char *TmpSourcePath;
    char *TmpTargetPath;

    struct ListOfElements Elements;
    GetListOfDirectories(source, target, &Elements);
    for(int i=0;i<Elements.NumberOfSourceDirectories;i++)
    {
        if(strcmp(Elements.SourceDirectories[i]->d_name,".")!=0)
        {
            if(strcmp(Elements.SourceDirectories[i]->d_name,"..")!=0)
            {
                printf("%s\n",Elements.SourceDirectories[i]->d_name);
                TmpSourcePath = source;
                TmpTargetPath = target;

                TmpSourcePath = AddSlashToPath(TmpSourcePath);
                TmpTargetPath = AddSlashToPath(TmpTargetPath);

                TmpSourcePath = MergeStrings(TmpSourcePath,Elements.SourceDirectories[i]->d_name);
                TmpTargetPath = MergeStrings(TmpTargetPath,Elements.SourceDirectories[i]->d_name);
                InitDeamon(TmpSourcePath,TmpTargetPath);
            }
        }

    }
}

int main(int argc, char** argv) {
    char *source = NULL;
    char *target = NULL;

    int state = 0;
    while((state = getopt(argc, argv, "s:d:R:t:b")) != -1) {
        switch(state){
            case 's':
                source = optarg;
                break;
            case 'd':
                target = optarg;
                break;
            case 'R':
                flags |= 0x00000001;
                break;
            case 't':
                if(optarg <= 0 || optarg > 16383) {
                    fprintf(stderr, "Option -t argument needs to be in range <1, 16383>.");
                }
                else {
                    flags = (flags & ~(0x7FFF << 1) | (optarg << 1));
                }
                break;
            case 'b':
                if(optarg <= 0 || optarg > 32767){
                    fprintf(stderr, "Option -t argument needs to be in range <1, 32767>.");
                }
                else {
                    flags = (flags & ~(0xFFFF << 16) | (optarg << 16));
                }
                break;
            case '?':
                if(optopt == 's' || optopt == 'd' || optopt == 't' || optopt == 'b'){
                    fprintf(stderr, "-%c requires an argument.\n", optopt);
                }
                else if(isprint(optopt)){
                    fprintf(stderr, "-%c - unrecognized.\n", optopt);
                }
                else {
                    fprintf(stderr, "Unknown: -%c\n", optopt);
                    return 1;
                }
            default:
                abort();
        }
    }

    if(source == NULL || target == NULL) {
        fprintf(stderr, "No args specified!\nUsage:\n\tcopyfolder.d -s <source_directory> -d <target_directory> {-R} {-t <delay_seconds>} {-b <size_in_bytes>}\n");
        exit(EXIT_FAILURE);
    }

    source = AddSlashToPath(source);
    target = AddSlashToPath(target);


    pid_t pid, sid;
    pid = fork();

    if(pid <= 0) exit(EXIT_FAILURE);
    else exit(EXIT_SUCCESS);

    unmask(0);

    sid = setsid();
    if(sid < 0) {
        syslog(LOG_ERR, "Error creating new SID: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if((chdir("/")) < 0) {
        syslog(LOG_ERR, "Error while changing current working directory: %s", strerror(errno))
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    syslog(LOG_INFO, "Daemon running, initialized successfully!");
    if(signal(SIGUSR1, wake_handler) == -1) {
        syslog(LOG_ERR, "FAILURE! Signal error %s", strerror(errno));
    }

    while(1){
        if(isRecursive()) {
            InitDeamon(source, target);
        }
        else {
            MergeFiles();
        }
        sleep(getDelayTime())
    }

    exit(EXIT_SUCCESS);
}
