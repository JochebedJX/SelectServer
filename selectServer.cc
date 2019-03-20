#include<iostream>
#include<unistd.h>
#include<sys/select.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string>
#include<stdio.h>

#define SIZE (sizeof(fd_set)*8)
#define INIT -1

using namespace std;

int StartUp(int port_)
{
  int sock=socket(AF_INET,SOCK_STREAM,0);
  if(sock<0)
  {
    cerr<<"socket error!"<<endl;
    exit(1);
  }

  int opt = 1;
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));//设置调用close(socket)后,仍可继续重用该socket。调用close(socket)一般不会立即关闭socket，而经历TIME_WAIT的过程。

  struct sockaddr_in local;
  local.sin_family=AF_INET;
  local.sin_port=htons(port_);
  local.sin_addr.s_addr=htonl(INADDR_ANY);

  if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
  {
    cerr<<"bind error!"<<endl;
    exit(2);
  }

  if(listen(sock,5)<0)
  {
    cerr<<"listen error!"<<endl;
    exit(3);
  }

  cout<<"listen sock create success......"<<endl;
  return sock;
}

void InitFdArray(int fd_array[],int size)
{
  for(auto i=0;i<size;i++)
  {
    fd_array[i]=INIT;
  }
}

int ArrayFdToSet(int fd_array[],int& size,fd_set& rfds)
{
  int max=fd_array[0];
  for(auto i=0;i<size;i++)
  {
    if(fd_array[i]==INIT)
    {
      continue;
    }
    FD_SET(fd_array[i],&rfds);
    if(max<fd_array[i])
    {
      max=fd_array[i];
    }
  }
  return max;
}

void AddFdToArray(int fd_array[],int& size,const int &sock)
{
  if(size==SIZE)
  {
    cout<<"socket is full......"<<endl;
    close(sock);
  }
  fd_array[size++]=sock;
}

void DelFdToArray(int fd_array[],int& size,int index)
{
  fd_array[index]=fd_array[size-1];
  fd_array[size]=INIT;
}

int main()
{
  int fd_array[SIZE];
  int size=0;
  int listen_sock=StartUp(8888);//这里的listen_sock是文件描述符
  InitFdArray(fd_array,SIZE);
  fd_array[size++]=listen_sock;

  for(;;)
  {
    fd_set rfds;
    FD_ZERO(&rfds);

    int max_fds=ArrayFdToSet(fd_array,size,rfds);
    //FD_SET(listen_sock,&rfds);//将该文件描述符进行设置
    //int max_fds=listen_sock+1;

    struct timeval timeout={5,0};//秒，微秒
    int ret = select(max_fds+1,&rfds,NULL,NULL,&timeout);
    if(ret<0)
    {
      cerr<<"select error!"<<endl;
      continue;
    }
    else if(ret==0)
    {
      cerr<<"time out!"<<endl;
      continue;
    }
    else 
    {//读事件就绪！
      for(auto i=0;i<size;i++)
      {
        int sock_=fd_array[i];
        if(sock_<0)
        {
          continue;
        }
        if(FD_ISSET(sock_,&rfds))
        {
          if(sock_==listen_sock)
          {
            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);
            int sock=accept(listen_sock,(struct sockaddr* )&peer,&len);
            if(sock<0)
            {
              cerr<<"accept error!"<<endl;
              continue;
            }
            //连接获得成功，需要放入数组中等待下次处理
            //将新连接添加至select中的rfds中，让select帮你去等待
            //会将listen_sock和sock都放入select中
            AddFdToArray(fd_array,size,sock);
          }
          else{
            //connect_fd
            //读！！！
            char buffer[10240];
            ssize_t s=recv(sock_,buffer,sizeof(buffer),0);
            if(s>0)
            {
              buffer[s]=0;
              cout<<buffer<<endl;
              string response="HTTP/1.1 200 OK\r\n\r\n<html><h3>select server!</h3></html>";

              send(sock_,response.c_str(),response.size(),0);
              close(sock_);
              DelFdToArray(fd_array,size,i);
              i--;
            }
            else
            {
              close(sock_);
              DelFdToArray(fd_array,size,i);
              i--;
            }
          }
        }
      }
    }
  }
  return 0;
}



