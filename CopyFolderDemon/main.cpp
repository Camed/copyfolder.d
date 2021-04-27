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

typedef struct ListOfElements{
    struct dirent** SourceElements;
    struct dirent** TargetElements;
    struct dirent** SourceDirectories;
    struct dirent** TargetDirectories;
    int NumberOfSourceElements;
    int NumberOfTargetElements;
    int NumberOfSourceDirectories;
    int NumberOfTargetDirectories;
} ListOfElements;


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
bool FileIsFile(const char *file)
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
bool FileIsDirectory(const char *file)
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
    if(source[strlen(source)-1] != '/'){
        source= MergeStrings(source,"/");
        source[strlen(source)]='\0';
    }
    return source;
}
int GetOnlyFiles(const struct dirent* Element)
{
    return FileIsFile(Element->d_name);
}
int GetOnlyDirectoires(const struct dirent* Element)
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
int GetListOfDirectories(char *source,char *target,struct ListOfElements* Elements)
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
    int NumberOfDirectories_Source=0;
    int NumberOfDirectories_Target=0;

    SourceDirectory = opendir(source);
    TargetDirectory = opendir(target);

    chdir(source);
    NumberOfElements_Source = scandir(source,&SourceFilesList,GetOnlyFiles, alphasort);
    NumberOfDirectories_Source = scandir(source,&SourceDirectoriesList,GetOnlyDirectoires, alphasort);
    chdir(target);
    NumberOfElements_Source = scandir(target,&TargetFilesList,GetOnlyFiles, alphasort);
    NumberOfDirectories_Target = scandir(target,&TargetDirectoriesList,GetOnlyDirectoires, alphasort);

    if(NumberOfElements_Source<0){
        return 0;
    }
    else if(NumberOfElements_Target<0){
        return 0;
    }

    //Pliki
    Elements->NumberOfSourceElements=NumberOfElements_Source;
    Elements->NumberOfTargetElements=NumberOfElements_Target;
    Elements->SourceElements=SourceFilesList;
    Elements->TargetElements=TargetFilesList;
    //Katalogi
    Elements->NumberOfSourceDirectories = NumberOfDirectories_Source;
    Elements->NumberOfTargetDirectories = NumberOfDirectories_Target;
    Elements->SourceDirectories = SourceDirectoriesList;
    Elements->TargetDirectories = TargetDirectoriesList;

    return 1;
}
int MergeDirectories(char *source,char* target)
{
    struct ListOfElements Elements;
    int found=0;
    GetListOfDirectories(source,target,&Elements);
    chdir(target);

    for(int i=0;i<Elements.NumberOfSourceDirectories;i++){
        for(int ii=0;ii<Elements.NumberOfTargetDirectories;ii++) {

            if(strcmp(Elements.SourceDirectories[i]->d_name,Elements.TargetDirectories[ii]->d_name) ==0)
            {
                found=0;
            }

        }
    }



}

int main(/*int argc,char *argv[]*/)
{
	//char *source = argv[1];
	//char *target = argv[2];

	char *source = "TestObjects/SourceFolder";
	char *target ="TestObjects/DestinationFolder";



}
