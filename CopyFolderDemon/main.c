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

long long AvgSize;

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


void DebugFile()
{
    //struct stat x;
    //struct stat y;
    //stat("/home/kali/CLionProjects/CopyFolderDemon/cmake-build-debug/TestObjects/TargetFolder/folder1/plik99xd.txt",&x);
    //stat("/home/kali/CLionProjects/CopyFolderDemon/cmake-build-debug/TestObjects/SourceFolder/folder1/plik99xd.txt",&y);
    //printf("%d\n",x.st_mtime);
    //printf("%d\n",y.st_mtime);
}

int Copy_y(char *file0, char *file1, int size)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    int input, output;
    char *addr;
    ssize_t t;

    struct stat InputFileStat;
    struct stat OutputFileStat;

    //struct utimbuf InputFileBuf;
    struct utimbuf OutputFileBuf;

    unsigned char *BufforSpace;
    BufforSpace = (unsigned char *) malloc(size * sizeof(unsigned char));

    input = open(file0, O_RDONLY);
    output = open(file1, O_WRONLY | O_CREAT | O_TRUNC, 0664);

    stat(file0,&InputFileStat);
    stat(file1,&OutputFileStat);

    if (input == -1)
    {
        return 0xDEAD;
    }
    if (output == -1)
    {
        close(input);
        return 0xBEAF;
    }

    printf("%d  pies %d",InputFileStat.st_size,AvgSize);

    if(InputFileStat.st_size<=AvgSize)
    {
        while ((t = read(input, BufforSpace, size)) > 0)
        {
            write(output, BufforSpace, t);
        }
        free(BufforSpace);
        BufforSpace = NULL;

        close(input);
        close(output);
    }
    else
    {
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
        off_t offset,pa_offset;
        size_t lenght;
        lenght = InputFileStat.st_size;
        offset = 4196;
        pa_offset =4196;

        addr = mmap(0, lenght, PROT_READ,MAP_SHARED, input, 0);

        printf("%s",addr);

        write(output,addr,lenght);

        close(input);
        close(output);
    }

    OutputFileBuf.modtime = InputFileStat.st_mtime;
    utime(file1,&OutputFileBuf);
}
char* MergeStrings(char * first,char * second)
{
    char* CreatedString = "";
    asprintf(&CreatedString,"%s%s",first,second);
    return CreatedString;
}
int FileIsFile(const char *file)
{
    printf("%s <- plik nazwa w fileisfile\n",file);
    struct stat FileStat;
    if(lstat(file,&FileStat)==-1)
    {
        syslog(LOG_ERR,"Error occurred during FileIsFile %s", strerror(errno));
    }
    lstat(file,&FileStat);
    if(S_ISREG (FileStat.st_mode))
    {
        printf("xd wraca1 \n");
        return 1;
    }
    else
    {
        printf("xd-wraca0\n");
        return 0;
    }
}
int FileIsDirectory(const char *file)
{
    struct stat FileStat;
    if(lstat(file,&FileStat)==-1)
    {
        syslog(LOG_ERR,"Error occurred during FileIsDirectory %s", strerror(errno));
    }
    lstat(file,&FileStat);
    if(S_ISDIR (FileStat.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
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
int GetOnlyDirectories(const struct dirent* Element)
{
    return FileIsDirectory(Element->d_name);
}

int SearchForChanges(char *path)
{
    char *PathOffset = (path);
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

    //source = AddSlashToPath(source);
    //target = AddSlashToPath(target);

    SourceDirectory = opendir(source);
    TargetDirectory = opendir(target);

    chdir(source);
    NumberOfElements_Source = scandir(source, &SourceFilesList,GetOnlyFiles, alphasort);
    printf("  %d <-pliki \n",NumberOfElements_Source);
    NumberOfDirectories_Source = scandir(source, &SourceDirectoriesList,GetOnlyDirectories, alphasort);
    printf(" %d <-katalogi \n",NumberOfDirectories_Source);
    chdir(target);
    NumberOfElements_Target = scandir(target, &TargetFilesList,GetOnlyFiles, alphasort);
    NumberOfDirectories_Target = scandir(target, &TargetDirectoriesList,GetOnlyDirectories, alphasort);

    if(NumberOfElements_Source<0 || NumberOfElements_Target<0)
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
static int RemoveFile(const char *path, const struct stat *data, int type)
{
    if(remove(path) == -1)
    {
        return -1;
    }
    return 0;
}
int RemoveDirectories(char *path)
{
    ftw(path, RemoveFile, 10);
}
int MergeDirectories(char *source, char *target)
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
            printf("found smt\n");

            char tmp_target[512];
            char tmp_source[512];

            //dodanie / do wywalenia w inne miejsce
            source = AddSlashToPath(source);
            target = AddSlashToPath(target);
            //-------
            strcpy(tmp_source, source);
            strcpy(tmp_target, target);
            strcat(tmp_source, Elements.SourceDirectories[j]->d_name);
            strcat(tmp_target, Elements.SourceDirectories[j]->d_name);


            printf("%s\n",tmp_source);
            printf("%s\n",tmp_target);
            printf("found smt1\n");

            struct stat mdata;
            stat(tmp_source, &mdata);
            mkdir(tmp_target, mdata.st_mode);
            printf("Stworzono folder //////////////////////////////////////////////////////////// %s\n",tmp_target);
        }
    }
}
int MergeFiles(char *source, char *target)
{
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
                    //dodanie / do wywalenia w inne miejsce

                    //----
                    char tmp_target[512];
                    char tmp_source[512];
                    strcpy(tmp_target, target);
                    strcat(tmp_target, Elements.SourceElements[i]->d_name);
                    strcpy(tmp_source, source);
                    strcat(tmp_source, Elements.SourceElements[i]->d_name);

                    struct stat source_data, target_data;
                    stat(tmp_target, &target_data);
                    stat(tmp_source, &source_data);
                    printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
                    printf("%s\n", tmp_source);
                    printf("%s\n", tmp_target);

                    if (source_data.st_mtime != target_data.st_mtime) {

                        printf("=================================================================================================================\n");
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
}
void InitDeamon(char *source,char *target)
{
    DebugFile();
    MergeDirectories(source,target);
    DebugFile();
    MergeFiles(source,target);
    DebugFile();

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

int main(/*int argc,char *argv[]*/)
{
    AvgSize=6000;
    //char *source = argv[1];
    //char *target = argv[2];

    char *source ="/home/kali/CLionProjects/untitled1/cmake-build-debug/TestObjects/SourceFolder";
    char *target ="/home/kali/CLionProjects/untitled1/cmake-build-debug/TestObjects/TargetFolder";

    printf("\n%s \n%s\n",source,target);

    InitDeamon(source,target);

}