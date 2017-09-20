/*********************************************************************************************
 * Author  : Hemant Rumde
 * Date    : Tue Apr 28 11:27:16 EDT 2015
 * Version : 1.0
 * (C) Voya Financial Services 2015 
 * 
 * Description: This shell uses separate process id kernel space 
 *
 ********************************************************************************************/

#define		_GNU_SOURCE

#include	<stdio.h>
#include	<string.h>
#include	<signal.h>
#include	<unistd.h>
#include	<sched.h>
#include	<sys/wait.h>
#include	<sys/mount.h>
#include	<sys/types.h>
#include	<sys/stat.h>

static int 
MyShell(void *arg)
{
        struct stat DirInfo;

	// Take new procfs mount directory for this shell
    	char *ProcFSMountDir = arg;
        
	// Create this directory if it is missing
        if( ProcFSMountDir == NULL ){
	    fprintf(stderr,"Error: proc file system mount directory is NULL!\n");
            exit(1);
        } 
        if( stat(ProcFSMountDir, &DirInfo) != 0 )
        if( !S_ISDIR( DirInfo.st_mode ) ){
            if( mkdir(ProcFSMountDir, 0555) < 0){
                perror(ProcFSMountDir);
                exit(2);
            } else {
                fprintf(stderr, "%s: directory is created!\n", ProcFSMountDir);
            }
        }
        
        //Use directory to mount procfs
        if( mount("proc", ProcFSMountDir, "proc", 0, NULL) < 0 ){
                perror(ProcFSMountDir);
                exit(3);
        } else {
                fprintf(stderr, "ProcFs is mounted on %s\n", ProcFSMountDir);
        }
        execl("/bin/bash","bash",(char*)NULL);
        perror("execlp(bash)");
        exit(4);
}

#define STACK_SIZE (1024*1024)
static char stack[STACK_SIZE];

int 
main(int argc, char *argv[])
{
       pid_t ChildPid;
       ChildPid = clone(MyShell, 
                        stack+STACK_SIZE, 
                        CLONE_NEWPID|CLONE_NEWNS|CLONE_NEWNET|CLONE_NEWIPC|CLONE_NEWUTS|SIGCHLD, 
                        argv[1]);
       if( ChildPid < 0 ){
           perror("clone()");
           exit(5);
       }
       if( waitpid(ChildPid, NULL, 0) < 0 ){
           perror("waitpid()");
           exit(5);
       }
       return 0;
}
