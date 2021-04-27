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
#define _XOPEN_SOURCE 500

typedef struct ListOfElements
{
    struct dirent** SourceElements;
    struct dirent** TargetElements;
    struct dirent** SourceDirectories;
    struct dirent** TargetDirectories;
    int NumberOfSourceElements;
    int NumberOfTargetElements;
    int NumberOfSourceDirectories;
    int NumberOfTargetDirectories;
} ListOfElements;
/*typedef enum NftwFlags{
    FTW_PHYS = 1,
    #define FTW_PHYS FTW_PHYS
    FTW_MOUNT = 2,
    #define FTW_MOUNT FTW_MOUNT
    FTW_CHIDR = 4,
    #define FTW_CHIDR FTW_CHIDR
    FTW_DEPTH = 8,
    #define FTW_DEPTH FTW_DEPTH
    #if_def __USE_GNU
    FTW_ACTIONRETVAL = 16
    #define FTW_ACTIONRETVAL FTW_ACTIONRETVAL
    #endif

}NftwFlags; chuj wie  co tu sie dzieje do zobaczenia to jest xD*/

int Copy_y(char *file0, char *file1, int size)
{
    int input, output;
    ssize_t t;

    unsigned char *BufforSpace;
    BufforSpace = (unsigned char *) malloc(size * sizeof(unsigned char));

    input = open(file0, O_RDONLY);
    output = open(file1, O_WRONLY | O_CREAT | O_TRUNC, 0664);

    if (input == -1)
    {
        return 0xDEAD;
    }
    if (output == -1)
    {
        close(input);
        return 0xBEAF;
    }
    while ((t = read(input, BufforSpace, size)) > 0)
    {
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

    if(S_ISREG(FileStat.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool FileIsDirectory(const char *file)
{
    struct stat FileStat;
    if(stat(file,&FileStat)==-1)
    {
        syslog(LOG_ERR,"Error occurred during FileIsDirectory %s", strerror(errno));
    }
    if(S_ISDIR(FileStat.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}
char* AddSlashToPath(char *source)
{
    if(source[strlen(source)-1] != '/')
    {
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


void InitDeamon(char *source,char *target)
{
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
    int NumberOfElements_Source=0;
    int NumberOfElements_Target=0;
    int NumberOfDirectories_Source=0;
    int NumberOfDirectories_Target=0;

    SourceDirectory = opendir(source);
    TargetDirectory = opendir(target);

    chdir(source);
    NumberOfElements_Source = scandir(source, &SourceFilesList,GetOnlyFiles, alphasort);
    NumberOfDirectories_Source = scandir(source, &SourceDirectoriesList,GetOnlyDirectoires, alphasort);
    chdir(target);
    NumberOfElements_Source = scandir(target, &TargetFilesList,GetOnlyFiles, alphasort);
    NumberOfDirectories_Target = scandir(target, &TargetDirectoriesList,GetOnlyDirectoires, alphasort);

    if(NumberOfElements_Source<0)
    {
        return 0;
    }
    else if(NumberOfElements_Target<0)
    {
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
static int RemoveFile(const char *path, const struct stat *data, int type, struct FTW *ftw)
{
    if(remove(path)<0)
    {
        return -1;
    }
    return 0;
}
int RemoveDirectories(char *path)
{
    nftw(path, RemoveFile, 10, FTW_MOUNT | FTW_DEPTH | FTW PHYS))
}
int MergeDirectories(char *source, char* target)
{
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
            RemoveDirectories(Elements.TargetDirectoriesList[i]->d_name);
            scandris(source, target, &Elements);
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
            char tmp_target[256];
            char tmp_source[256];
            strcpy(tmp_target, target);
            strcat(tmp_target, Elements.TargetDirectories[j]->d_name);
            strcpy(tmp_source, source);
            strcat(tmp_source, Elements.TargetDirectories[j]->d_name);

            struct stat mdata;
            stat(tmp_source, &mdata);
            mkdir(tmp_target, mdata.st_mode);
        }
    }
}
int MergeFiles(char* source, char* target)
{
    struct ListOfElements Elements;
    int output=-1;
    GetListOfDirectories(source, target, &Elements);
    chdir(target);

    for(int i=0; i<Elements.NumberOfTargetElements; i++)
    {
        output=-1;
        for(int ii=0; ii<Elements.NumberOfSourceElements; ii++)
        {

            if(strcmp(Elements.SourceElements[ii]->d_name,Elements.TargetElements[i]->d_name) ==0)
            {
                char tmp_target[256];
                char tmp_source[256];
                strcpy(tmp_target, target);
                strcat(tmp_target, Elements.TargetElements[i]->d_name);
                strcpy(tmp_source, source);
                strcat(tmp_source, Elements.TargetElements[i]->d_name);

                struct stat source_data, target_data;
                stat(tmp_target, &target_data);
                stat(tmp_source, &source_data);

                if (source_data.st_mtime != target_data.st_mtime)
                {
                    Copy_y(tmp_source, tmp_target, 8192);
                }
            }
        }
        if (output!=0)
        {
            unlink(Elements.TargetElements[i]->d_name);
            GetListOfDirectories(source, target, &Elements);
            i--;
        }
    }

    output=-1;
    for(int j=0; j<Elements.NumberOfSourceElements; j++)
    {
        output=-1;
        for(int jj=0; jj<Elements.NumberOfTargetElements; jj++)
        {
            if(strcmp(Elements.SourceElements[i]->d_name,Elements.TargetElements[jj]->d_name) ==0)
            {
                output=0;
            }
        }
        if (output==-1)
        {
                char tmp_target[256];
                char tmp_source[256];
                strcpy(tmp_target, target);
                strcat(tmp_target, Elements.TargetElements[i]->d_name);
                strcpy(tmp_source, source);
                strcat(tmp_source, Elements.TargetElements[i]->d_name);

                Copy_y(tmp_source, tmp_target, 8192);
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