#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <pwd.h>
#include <math.h>
#include <grp.h>

char *path2;

struct filevals{
  char *owner;
  char *group;
  int inode;
  char* perms;
  int size;
  char* name;
  char *acctime;
  char * modtime;
  int block;
  int fs;
};


char *getPermissions(mode_t st_mode) {
    char *permissions = malloc(11);
    char *rwx[] = {"---", "--w", "-w-", "-wx", "r--", "r-x", "r-x", "rw-", "rwx"};

    // Get file type letter
    if (S_ISREG(st_mode)) {
        permissions[0] = '-';
    } else if (S_ISDIR(st_mode)) {
        permissions[0] = 'd';
    } else if (S_ISBLK(st_mode)) {
        permissions[0] = 'b';
    } else if (S_ISCHR(st_mode)) {
        permissions[0] = 'c';
    }
    #ifdef S_ISFIFO
        else if (S_ISFIFO(st_mode)) {
            permissions[0] = 'p';
        }
    #endif
    #ifdef S_ISLNK
        else if (S_ISLNK(st_mode)) {
            permissions[0] = 'l';
        }
    #endif
    #ifdef S_ISSOCK 
        else if(S_ISSOCK(st_mode)) {
            permissions[0] = 's';
        }
    #endif
    #ifdef S_ISDOOR
        else if(S_ISDOOR(st_mode)) {
            permissions[0] = 'D';
        }
    #endif
    else {
        // unknown type
        permissions[0] = '?';
    }
    strcpy(&permissions[1], rwx[(st_mode >> 6) & 7]);
    strcpy(&permissions[4], rwx[(st_mode >> 3) & 7]);
    strcpy(&permissions[7], rwx[(st_mode & 7)]);
    if (st_mode & S_ISUID) {
        permissions[3] = (st_mode & S_IXUSR) ? 's' : 'S';
    }
    if (st_mode & S_ISGID) {
        permissions[6] = (st_mode & S_IXGRP) ? 's' : 'l';
    }
    if (st_mode & S_ISVTX) {
        permissions[9] = (st_mode & S_IXOTH) ? 't' : 'T';
    }
    permissions[10] = '\0';
    return permissions;
}



void buildtree(char *path,int level,char* namedir){
  struct dirent *pDirent,*dirempt,*dir2,*dirFiles;
  struct passwd *pwd;
  struct group *grp;
  struct filevals filearray[1000];
  struct filevals dirarray[1000];
  DIR *temp = opendir(path);
  char *files[100];
  char *folders[100];
  int filesnum=0;
  int foldnum=0;
  struct stat sb;
  while ((pDirent = readdir(temp)) != NULL) {
         int statuts = stat(pDirent->d_name, &sb);
       
        if ((pDirent->d_type == DT_REG) && (strcmp(pDirent->d_name, ".") != 0) && (strcmp(pDirent->d_name, "..") != 0)) {
            files[filesnum++]=pDirent->d_name;
        } else if ((pDirent->d_type == DT_DIR) && (strcmp(pDirent->d_name, ".") != 0) && (strcmp(pDirent->d_name, "..") != 0)) {
          folders[foldnum++]=pDirent->d_name;
        }
    }
 
    if(level==1){
        printf("\n\nLevel %d: %s\n", level, path);
    }else{
        printf("\n\nLevel %d: %s\n", level, namedir);
    }
     
     if(foldnum != 0){
         printf("Directories\n");
     }
     
      struct stat stdir;
     
    DIR *direc = opendir(path);
    int i = 0;

   char *newpathfold = path;
   int num=0;
    while ((dir2 = readdir(direc)) != NULL && i < foldnum) {
    //  printf("test555\n");
        if (((strcmp(dir2->d_name, ".")) != 0) && ((strcmp(dir2->d_name, "..")) != 0) && (strcmp(dir2->d_name, folders[i]) == 0)) {
            char newpathfold2[256];
           // printf("testingggg: %s\n",newpathfold2);
            sprintf(newpathfold2, "%s/%s", path, folders[i]);
            newpathfold = newpathfold2;
           int status = stat(newpathfold, &stdir);
            pwd = getpwuid(stdir.st_uid);
            grp = getgrgid(stdir.st_gid);

            // Get string of permissions for file
            char *permissions = malloc(11);
            permissions = getPermissions(stdir.st_mode);

          //  printf("%s(%s)\t%ld\t%s\t%ld\t%s\n", pwd->pw_name, grp->gr_name, dir2->d_ino, permissions, stdir.st_size, dir2->d_name);
        
            char * t1 =ctime(&stdir.st_ctime);
            char * t2=ctime(&stdir.st_mtime);
            strtok(t1,"\n");
           // printf("\t%s\t%s\n", t1, t2);
            dirarray[num].name = dir2->d_name;
            dirarray[num].owner = pwd->pw_name;
            dirarray[num].group = grp->gr_name;
            dirarray[num].inode = dir2->d_ino;
            dirarray[num].perms = permissions;
            dirarray[num].size = stdir.st_size;
            dirarray[num].acctime = t1;
            dirarray[num].modtime = t2;
            i++;   
            num++;
        }
    }
    struct filevals t;
     for(i=0;i<foldnum;i++)
     {
          for(int j=i+1;j<foldnum;j++)
          {
               if(strcmp(dirarray[i].name,dirarray[j].name)>0)
               {
                    t=dirarray[i];
                    dirarray[i]=dirarray[j];
                    dirarray[j]=t;
               }
          }
     }
    for(int g=0; g<foldnum;g++){
        //printf("name: %s\n",dirarray[g].name);
        printf("%s(%s)\t%ld\t%s\t%ld\t%s\n", dirarray[g].owner, dirarray[g].group, dirarray[g].inode, dirarray[g].perms, dirarray[g].size, dirarray[g].name);
        printf("\t%s\t%s\n", dirarray[g].acctime, dirarray[g].modtime);
    }
    closedir(direc);

    DIR *dfile2 = opendir(path);
    int j = 0;
    struct stat stfile;
    char *newpath = path;
    if(filesnum != 0){
     printf("Files\n");
    }
    int num2=0;
    while ((dirFiles = readdir(dfile2)) != NULL && (j<filesnum)) {
         // printf("heyyy\n");
        if (((strcmp(dirFiles->d_name, ".")) != 0) && ((strcmp(dirFiles->d_name, "..")) != 0) && (strcmp(dirFiles->d_name, files[j]) == 0)) {
             char newpath2[256];
          //  printf("testingggg: %s\n",newpath2);
           sprintf(newpath2, "%s/%s", path, files[j]);
          
            newpath = newpath2;
            int stat2 = stat(newpath, &stfile);
            // Get owner and group names
            pwd = getpwuid(stfile.st_uid);
            grp = getgrgid(stfile.st_gid);

            char *permissions2 = malloc(11);
            permissions2 = getPermissions(stfile.st_mode);
           // printf("%s(%s)\t%ld\t%s\t%ld\t%s\n", pwd->pw_name, grp->gr_name, dirFiles->d_ino, permissions2, stfile.st_size, dirFiles->d_name);
            
            char * tf1 =ctime(&stfile.st_ctime);
            char * tf2=ctime(&stfile.st_mtime);
            strtok(tf1,"\n");      
            // Print line 2:
         //   printf("\t%s\t%s\n", tf1, tf2);
            filearray[num2].name = dirFiles->d_name;
            filearray[num2].owner = pwd->pw_name;
            filearray[num2].group = grp->gr_name;
            filearray[num2].inode = dirFiles->d_ino;
            filearray[num2].perms = permissions2;
            filearray[num2].size = stfile.st_size;
            filearray[num2].acctime = tf1;
            filearray[num2].modtime = tf2;
            j++;
            num2++;
        }
    }
    struct filevals t2;
     for(int l=0;l<filesnum;l++)
     {
          for(int j=l+1;j<filesnum;j++)
          {
               if(strcmp(filearray[l].name,filearray[j].name)>0)
               {
                    t2=filearray[l];
                    filearray[l]=filearray[j];
                    filearray[j]=t2;
               }
          }
     }
     for(int g=0; g<filesnum;g++){
        //printf("name: %s\n",dirarray[g].name);
        printf("%s(%s)\t%ld\t%s\t%ld\t%s\n", filearray[g].owner, filearray[g].group, filearray[g].inode, filearray[g].perms, filearray[g].size, filearray[g].name);
        printf("\t%s\t%s\n", filearray[g].acctime, filearray[g].modtime);
    }
   // printf("heyyy\n");
    closedir(dfile2);
    level +=1;
    for(int k=0;k<foldnum;k++){
      char d_path[2000]; // here I am using sprintf which is safer than strcat
      sprintf(d_path, "%s/%s", path, folders[k]);
      buildtree(d_path,level,folders[k]);
    }
}
int age_sorter(const void* p1, const void* p2)
{
  struct filevals *person1 =(struct filevals *)p1;
  struct filevals *person2 = (struct filevals *)p2;
 
  return person1->inode - person2->inode;
}


void buildinode(char *path,int level,char* namedir){
  struct dirent *pDirent,*dirempt,*dir2,*dirFiles;
  struct passwd *pwd;
  struct group *grp;
  struct filevals nodearray[1000];
  DIR *temp = opendir(path);
  char *files[100];
  char *folders[100];
  int filesnum=0;
  int foldnum=0;
  struct stat sb;
  char* allin[100];
  int count1=0;
    int num=0;
  while ((pDirent = readdir(temp)) != NULL) {
         int statuts = stat(pDirent->d_name, &sb);
            printf(" ");
        if ((pDirent->d_type == DT_REG) && (strcmp(pDirent->d_name, ".") != 0) && (strcmp(pDirent->d_name, "..") != 0)) {
            files[filesnum++]=pDirent->d_name;
        } else if ((pDirent->d_type == DT_DIR) && (strcmp(pDirent->d_name, ".") != 0) && (strcmp(pDirent->d_name, "..") != 0)) {
          folders[foldnum++]=pDirent->d_name;
        }if ((pDirent->d_type == DT_DIR)||(pDirent->d_type == DT_REG) && (strcmp(pDirent->d_name, ".") != 0) && (strcmp(pDirent->d_name, "..") != 0)) {
          allin[count1++]=pDirent->d_name;
        }
    }
    if(level==1){
        printf("\n\nLevel %d Inodes: %s\n", level, path);
    }else{
        printf("\n\nLevel %d Inodes: %s\n", level, namedir);
    }
    struct stat stdir;
    DIR *direc = opendir(path);
    int i = 0;
   // printf("test2\n");
   char *newpathfold = path;
    while ((dir2 = readdir(direc)) != NULL) {
    //  printf("test555\n");
        if (((strcmp(dir2->d_name, ".")) != 0) && ((strcmp(dir2->d_name, "..")) != 0)) {
            char newpathfold2[256];
           // printf("testingggg: %s\n",newpathfold2);
            sprintf(newpathfold2, "%s/%s", path, dir2->d_name);
    
            newpathfold = newpathfold2;
            int status = stat(newpathfold, &stdir);
            int size = ceil(stdir.st_size/512);
            int fs = size;
            if(size >=0 && size<=1){
                fs=1;
            }
            nodearray[num].name =dir2->d_name;
            nodearray[num].block = stdir.st_blocks;
            nodearray[num].inode = dir2->d_ino;
            nodearray[num].size = stdir.st_size;
            nodearray[num].fs = fs;
           // printf("%d\n", (int) ceil((double) (stdir.st_size/512)));
         //   printf("\t%ld\t%d\t%ld\t%ld\t%s\n",  dir2->d_ino, stdir.st_size,stdir.st_blocks,fs, dir2->d_name);
            
            // Get dates to display on line 2
            i++;   
            num++;
        }
    }
    for(int h=0;h<count1;h++){
        qsort(nodearray, count1, sizeof(struct filevals), age_sorter);
    }
    for (int i = 0; i < count1; ++i){
     //   printf("%d\n", nodearray[i].inode);
     if(nodearray[i].inode != 0){
        printf("\t%ld\t%d\t%ld\t%ld\t%s\n", nodearray[i].inode, nodearray[i].size,nodearray[i].block,nodearray[i].fs, nodearray[i].name );

     }
    }
  
    closedir(direc);
    level +=1;
    for(int k=0;k<foldnum;k++){
      char d_path[2000]; // here I am using sprintf which is safer than strcat
      sprintf(d_path, "%s/%s", path, folders[k]);
      buildinode(d_path,level,folders[k]);
    }
}


int main(int argc, char * argv[]){
    if(argc != 3){
        printf("usage: ./FSreport format(-inode or -tree) pathname\n");
        exit(1);
    }
    char *format = argv[1];
    path2 = argv[2];
    int formflag =0;  
    if(strcmp(format,"-inode")==0){
        formflag = 1;
    }else{
        formflag = 2;
    }
    if(formflag == 2){
      printf("File System Report: Tree Directory Structure\n");
      buildtree(path2,1,"none");
    } else{
      printf("File System Report: Inodes\n");
      buildinode(path2,1,"none");
    } 
}

