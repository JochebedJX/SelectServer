#include<iostream>
#include<unistd.h>
#include<sys/select.h>

using namespace std;

int main()
{
  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(0,&read_fds);

  for(;;)
  {
    printf("> ");
    fflush(stdout);
    int ret=select(1,&read_fds,NULL,NULL,NULL);//fds==1,最大文件描述符+1；
    if(ret<0)
    {
      cerr<<"select error!"<<endl;
      continue;
    }
    if(ret==0)
    {
      cout<<"invaild fd!"<<endl;
      continue;
    }
    else if(FD_ISSET(0,&read_fds))
    {
      char buf[1024]={0};
      read(0,buf,sizeof(buf)-1);
      cout<<"input: "<<buf<<endl;
    }
    FD_ZERO(&read_fds);
    FD_SET(0,&read_fds);
  }
  return 0;
}

