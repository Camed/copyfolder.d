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


int Copy_y(char *file0,char *file1,int size) {
    int input, output;
    ssize_t t;

    unsigned char *BufforSpace;
    BufforSpace = (unsigned char *) malloc(size * sizeof(unsigned char));

    input = open(file0, O_RDONLY);
    output = open(file1, O_WRONLY | O_CREAT | O_TRUNC, 0664);

    if (input == -1) {
        return 0xDEAD;
    }
    if (output == -1) {
        close(input);
        return 0xBEAF;
    }
    while ((t = read(input, BufforSpace, size)) > 0) {
        write(output, BufforSpace, t);
    }
    free(BufforSpace);
    BufforSpace = NULL;

    close(input);
    close(output);
}
char* MergeStrings(char * first,char * second)
{
    char* CreatedString = "";
    asprintf(&CreatedString,"%s%s",first,second);
    return CreatedString;
}
bool FileIsFile(char *file)
{
    struct stat FileStat;
    if(stat(file,&FileStat)== -1)
    {
        syslog(LOG_ERR,"Error occurred during FileIsFile %s", strerror(errno));
    }

    if(S_ISREG(FileStat.st_mode)){
        return true;
    }
    else{
        return false;
    }
}
bool FileIsDirectory(char *file)
{
    struct stat FileStat;
    if(stat(file,&FileStat)==-1){
        syslog(LOG_ERR,"Error occurred during FileIsDirectory %s", strerror(errno));
    }
    if(S_ISDIR(FileStat.st_mode))
    {
        return true;
    }
    else{
        return false;
    }
}
char* AddSlashToPath(char *source)
{
    if(source[strlen(source)] != '/'){
        return MergeStrings(source,"/");
    }
    else{
        return source;
    }
}
int MergeDirectories(char *source,char* target)
{
    //struct listed_entries entries;

}
int GetOnlyFiles(struct dirent* Element)
{
    return FileIsFile(Element->d_name);
}
int GetOnlyDirecotires(struct dirent* Element)
{
    return FileIsDirectory(Element->d_name);
}




int SearchForChanges(char *path)
{
    char *PathOffset = (path);
}


void InitDeamon(char *source,char *target) {
    AddSlashToPath(source);
    AddSlashToPath(target);


}
int GetListOfDirectories(char *source,char *target,struct ListOfElements* List)
{
    DIR* SourceDirectory;
    DIR* TargetDirectory;
    struct dirent Element;
    struct dirent** SourceFilesList;
    struct dirent** SourceDirectoriesList;
    struct dirent** TargetFilesList;
    struct dirent** TargetDirectoriesList;
    int NumberOfElements_Source =0;
    int NumberOfElements_Target=0;

    SourceDirectory = opendir(source);
    TargetDirectory = opendir(target);

    chdir(source);
    NumberOfElements_Source = scandir(source,&SourceFilesList,GetOnlyFiles,alphasort);

}

int main(/*int argc,char *argv[]*/)
{
	//char *source = argv[1];
	//char *target = argv[2];

	char *source = "TestObjects/SourceFolder/plik0.txt";
	char *target ="TestObjects/DestinationFolder";



}
