#include "dialog.h"
#include "ui_dialog.h"
#include<iterator>
#include <QTableWidget>
#include<QString>
#include <QProcess>
#include<vector>
#include<stdlib.h>
#include<time.h>
#include<iostream>
#include <conio.h>
#include<algorithm>
#include <iomanip>
#include<fstream>
#include <windows.h> //win头文件
#define ProNumber 3
#define runnable 2
#define runningg 1
#define block 3
#define dead     3//进程运行结束
#define error    -1
using namespace std;

//中断处理程序运行时不能发生进程切换,
//因为中断处理程序使用当前进程的内核栈......??代表进程运行

//QT自带的代码，声明一个类，将设计出来的ui界面作为该类的一个子对象，在其构造函数中，先完成对子对象的构造，再使用子对象ui调用其setupUi(this)函数实现ui的现实。
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

//假设一条指令运行时间为一个时间单位
//假设从磁盘调页装入内存总时间为一个时间单位
//在cin或cout中指明数制后，该数制将一直有效，直到重新指明使用其他数制。hex oct dec
/*****************************************************全局区*****************************************************/

//进程页表项
class pagetableentry//进程页表页表项(请求式的页表项)
{
public:
    int pagenumber;//页号 5位
    int framenumber;//页框号 4位
    int flag;//驻留标志位 1位
    int diskaddress;//外存地址 4位 //不弄一个内页表外页表了个
    int changeflag;//修改位 1位 等于1为修改了
    int r;//引用位 SCR算法中用到
    pagetableentry()
    {
        r=1;
    }
};//一个页表项共16位，一个页表最多32个页表项，页表最多占64B

//快表表项
class tlbentry//快表表项
{
public:
    int pagenumber;//页号
    int framenumber;//页框号
};

//物理页框
class pageframe
{
public:
    pageframe()//初始化
    {
        counter=0;
        flag0=0;
    }
    int flag0;//该页框是否有页装入的标志
    int page;//装入的页面
    int counter;//LRU算法中用到的计数器

};

//磁盘块
class diskentry
{
public:
    diskentry()
    {
    dflag=0;
    }

    int diskaddress;
    int dpage;//装入的页面即装入的内容
    int dflag;//外存是否页面装入标志位=1为占用
};


vector<int> externstorage(int instructcount, int id);//外存分配函数
vector<vector<pagetableentry> > pgtlist(10);//将各个进程页表放在一起，把他们看做在一个容器里，他们的地址其实是没有联系的 假设页表基址就是在这个容器里的下标
vector<tlbentry> tlb(0);//快表  假设快表中有3个表项
vector<diskentry> disk(500);// 磁盘 假设外存大小500块
vector<pageframe> memorytable(16);//内存分块表 假设主64KB 一页4KB 则内存可分为16块 假设把内存的最后一块分出来专门存放各个进程的页表
ofstream ofile;//文件
vector<pageframe>::iterator it;//定义遍历循环队列的指针
//pcb 模拟
class pcb
{
public:
    int i,j;
    int state;//进程状态
    int pid;//进程标识符
    int priority;//进程优先级 数值越小优先级越高
    int pagetableaddress;//页表起始地址
    int pagetablelength;//页表长度 (页表项个数)
    int pagetableentrylen;//页表项长度 16位
    int framecount;//已占用的物理块数
    int intime;//进程到达时间
    int pf[3];//记录占用了哪些物理页框的数组
    int instructcount;//指令的数目
    //假设一条指令占1页
    int instructaddress;//第一条指令的首地址(逻辑地址)
    int instructnum;//当前执行到哪条指令
    int inss;//当前执行到哪条指令（这里的位置指的是在指令序列容器里的下标)
    vector<int> instructorder;//如果这里写vector<int> instructorder(0);报错syntax error:'constant' 因为这样等于初始化了大小 得在构造函数里初始化 或者不初始化
    void pcb0()//初始化  如果把初始化写在构造函数里 则在main函数之前会先执行全局变量对象的构造函数 产生错误。。。
    {

        ofile.open("d:\\lane.txt",ios::app);//打开记录文件，以追加的方式写入 入口调用了Writedown()函数，该文件一定存在了
        instructorder.resize(0);
        state=runnable;
        priority=(rand()%10)+1;
        pid=(rand()%100)+1;//rand()%(b-a+1)+a 产生[a,b]之间的随机数 为了让pid最好不相同。。设置的范围稍微大一点
        cout<<endl<<"进程id:"<<dec<<pid<<"  ";
        cout<<endl<<"进程优先级(优先级数值越小优先级越高):"<<dec<<priority<<"  ";
        ofile<<endl<<"进程id:"<<dec<<pid<<"  ";
        ofile<<endl<<"进程优先级(优先级数值越小优先级越高):"<<dec<<priority<<"  ";
        //如果页表放在内存，页表放置的位置是随机的吗 此地址是虚拟地址？页表存放的地址是怎么确定的 而且页表大小现在应该不是确定的 如何挑选出一个合适的位置呢
        //先对内存空闲的地方进行检测？--固定把内存最后一块放所有进程的页表了。。进程不能超过2^7
        pagetableaddress=(rand()%10);//这里的页表基址是在装有页表基址中的相对位置 0-9
        //因为进程的指令数是随机生成的，即页表项大小也是不确定的，页表长度不确定，这里按最大的算，每个进程最大逻辑地址空间为32页，最多32个页表项，每个页表项长度16位，最长64B
        cout<<"页表基址为"<<dec<<15000+pagetableaddress*64<<"h"<<"  ";//这个值已经是16进制的，不用hex格式输出
        ofile<<"页表基址为"<<dec<<15000+pagetableaddress*64<<"h"<<"  ";
        pagetablelength=32;//其实应该是页表项个数，这样判断越界时候可以通过页号判断。。但是初始化为instructcount会出错。。。
        pagetableentrylen=16;//单位是位
        framecount=0;//占用的页框刚开始为0
        intime=(rand()%10)+1;
        cout<<"到达时间："<<intime<<"  ";
        ofile<<"到达时间:"<<intime<<"  ";
        this->pf[0]=-1;//初始化为-1，值为-1就是没有占用物理页框，大于等于0的话，值就代表占用的物理块号
        this->pf[1]=-1;
        this->pf[2]=-1;
        instructcount=(rand()%10)+1;//产生[1,10]条指令
        cout<<"指令数:"<<instructcount<<"  ";
        ofile<<"指令数:"<<instructcount<<"  ";
        instructaddress=0;//假设一条指令占0800h，第一条指令首地址是虚拟地址  逻辑空间地址连续从0地址开始
        /*指令的存放地址是顺序的,如果指令顺序执行，而且按设置的一条指令占1页，就会一直发生缺页中断。。。转换后的页面序列是依次增大的,逻辑地址空间是连续的*/ //假设指令都是零地址指令
        instructorder.push_back(0);
        for(i=0;i<instructcount-1;i++)//产生指令的随机执行序列 但是序列第一个一定是0号指令
        {
            j=rand()%instructcount;
            instructorder.push_back(j);
        }
        pgtlist[pagetableaddress].resize(instructcount);
        cout<<"页表长度:"<<dec<<pgtlist[pagetableaddress].size()*2<<"B"<<"  ";
        cout<<endl<<"磁盘分配情况:"<<endl;
        ofile<<"页表长度:"<<dec<<pgtlist[pagetableaddress].size()*2<<"B"<<"  ";
        ofile<<endl<<"磁盘分配情况:"<<endl;
        ofile.close();
        vector<int> exs=externstorage(instructcount,pid);
        instructnum=0;
        ofile.open("d:\\lane.txt",ios::app);
        for(i=0;i<instructcount;i++)//页表项的页号
        {
            pgtlist[pagetableaddress][i].pagenumber=i;
            pgtlist[pagetableaddress][i].flag=0;//刚开始都不在内存中
            pgtlist[pagetableaddress][i].changeflag=0;
        }
        for( j=0;j<instructcount;j++)
        {

            pgtlist[pagetableaddress][j].diskaddress=exs[j];
        }
        for( i=0;i<instructcount;i++)
        {
            cout<<dec<<"进程页表第"<<i<<"项的外存地址(块号)："<<pgtlist[pagetableaddress][i].diskaddress<<endl;
            ofile<<dec<<"进程页表第"<<i<<"项的外存地址(块号)："<<pgtlist[pagetableaddress][i].diskaddress<<endl;
        }
        cout<<endl<<endl<<endl;
        ofile<<endl<<endl<<endl;
        ofile.close();
    }

};




//全局区
int n;//进程个数
int ptr;//页表基址寄存器(值代表当前活动页表的基址)
int system_time=0;//系统时间，初始化为0
int DISKFLAG;
pcb runningprocess;//在CPU中正在运行的进程
vector<pcb> ReadyQueue(0);//就绪队列,,注意要设为0，因为插入都是从队尾插
vector<pcb> EndQueue(0);//结束队列
vector<pcb> WaitQueue(0);//等待队列
vector<pcb> pcblist(0);//进程 队列

void Writedown()
{
        ofile.open("d:\\lane.txt",ios::app);//打开记录文件，以追加的方式写入
        if(!ofile)//如果文件不存在，则创建该文件
        {
            ofstream nfile( "d:\\lane.txt" );
        }
        cout<<"当前一共有"<<n<<"个进程"<<endl;
        ofile<<"当前一共有"<<n<<"个进程"<<endl;
        cout<<"生成的进程信息如下:"<<endl;
        ofile<<"生成的进程信息如下:"<<endl;
        ofile.close();
}

void Writedown2()//写下内存信息
{

        ofile.open("d:\\stlane.txt",ios::app);//打开记录文件，以追加的方式写入
        if(!ofile)//如果文件不存在，则创建该文件
        {
            ofstream nfile( "d:\\stlane.txt" );
        }
        ofile<<dec<<"当前系统时间:"<<system_time<<endl;
        ofile<<"当前内存使用情况:"<<endl;
        for(int i=0;i<15;i++)
        {
            ofile<<dec<<"内存第"<<i<<"块："<<"  ";
            if(memorytable[i].flag0==1)
            {
                ofile<<dec<<"被进程"<<runningprocess.pid<<"的第"<<memorytable[i].page<<"条指令占用"<<endl;
            }
            else
            {
                ofile<<"空闲";
            }
            ofile<<endl;
        }
        ofile<<"内存第15块被各个进程页表的占用"<<endl;
        ofile<<endl<<endl;
        ofile.close();
}

//判断产生的进程有没有与当前系统时间相等的，有没有进程到达
void JudgeIn(int n)//n为进程个数
    //遍历所有产生的进程的进入时间
{
    int i=0;
    while(i<n)
    {
        if(pcblist[i].intime==system_time)
        {
            ReadyQueue.push_back(pcblist[i]);}//插入就绪队列
        else
        {}
        i++;
    }

}

//打印队列信息
void PrintQueue()
{
    ofile.open("d:\\lane.txt",ios::app);//打开总信息文件
    int k;
      cout<<endl<<"*******************各队列情况*******************";
      cout<<endl<<"当前就绪队列(只输出id):"<<endl;
      ofile<<endl<<"*******************各队列情况*******************";
      ofile<<endl<<"当前就绪队列(只输出id):"<<endl;
      if(ReadyQueue.size()==0)
      {
          cout<<"当前就绪队列无进程"<<endl;
          ofile<<"当前就绪队列无进程"<<endl;
      }
      else
      {
        for(k=0;k<ReadyQueue.size();k++)
      {
          cout<<dec<<"进程"<<ReadyQueue[k].pid<<".";
          ofile<<dec<<"进程"<<ReadyQueue[k].pid<<".";
      }
      }
      cout<<endl<<"当前等待队列(只输出id):"<<endl;
      ofile<<endl<<"当前等待队列(只输出id):"<<endl;
      if(WaitQueue.size()==0)
      {
          cout<<"当前等待队列无进程"<<endl;
          ofile<<"当前等待队列无进程"<<endl;
      }
      else
      {
          for(k=0;k<WaitQueue.size();k++)
          {
              cout<<dec<<"进程"<<WaitQueue[k].pid<<".";
              ofile<<dec<<"进程"<<WaitQueue[k].pid<<".";
          }
      }
      cout<<endl<<"当前完成队列(只输出id):"<<endl;
      ofile<<endl<<"当前完成队列(只输出id):"<<endl;
      if(EndQueue.size()==0)
      {
          cout<<"当前完成队列无进程"<<endl;
          ofile<<"当前完成队列无进程"<<endl;
      }
      else
      {
          for(k=0;k<EndQueue.size();k++)
          {
              cout<<dec<<"进程"<<EndQueue[k].pid<<".";
              ofile<<dec<<"进程"<<EndQueue[k].pid<<".";
          }
      }
      ofile.close();
}

//创建随机进程
void createprocess()
{
    //进程备份在磁盘上
    //创建进程逻辑地址空间，即创建那种映射机制所需要的数据结构，页目，页表。
    //假设每个进程的逻辑空间有128KB 每页4KB 即 32页
    srand(unsigned(time(0)));//由于计算机运行很快，所以每次用time得到的时间都是一样的（time的时间精度较低，只有55ms）。这样相当于使用同一个种子产生随机序列，所以产生的随机数总是相同的。所以把srand放在循环外
    for(int i=0;i<n;i++)
    {
        pcb a;
        a.pcb0();//初始化
        pcblist.push_back(a);//将产生的随机进程加入到进程队列中
    }

}



//磁盘分配函数 备份进程的信息
vector<int> externstorage(int instructcount,int id)
{
    int j=0;
    ofile.open("d:\\exslane.txt",ios::app);//打开记录文件，以追加的方式写入
    if(!ofile)//如果文件不存在，则创建该文件
    {
        ofstream nnfile( "d:\\exslane.txt" );
    }
    vector<int> exstorage(0);//从0号指令开始 对应的外存的位置 也是连续的
    while(j<instructcount)
    {
        for(int i=0;i<500;i++)
        {
         if(disk[i].dflag==0)
         {
            disk[i].dpage=j;//磁盘第i块分配给第j条指令(第j号页面)
            ofile<<"磁盘第"<<i<<"块分给进程"<<id<<"的第"<<j<<"条指令"<<endl;
            disk[i].dflag=1;//修改占用位为1
            exstorage.push_back(i);
            break;
         }
        }
         j++;
    }
    cout<<"占用外存大小"<<exstorage.size();
    ofile<<"进程"<<id<<"占用外存大小:"<<exstorage.size()<<"块"<<endl;
    ofile<<endl;
    ofile.close();
    return exstorage;
}


//自定义排序方式，对进程进入时间的比较
bool CmpInTime( const pcb &p1, const pcb &p2)
{
    return p1.intime <=p2.intime;//升序排列
}

//自定义排序方式，对进程优先级进行比较
bool CmpPriority(const pcb& p1, const pcb& p2)
{
return p1.priority < p2.priority;
}

//自定义排序方式，对物理页框对应的计数器进行比较
bool CmpCounter( const pageframe &page1, const pageframe &page2)
{
    return page1.counter>page2.counter;//降序排列，不会有相等情况
}

//页框分配函数
int fenpei(int n,int pagenumber)//n:为进程分配的物理块个数
{
    int count=0;
    int i;
    for( i=0;i<16;i++)//检查空闲块数是否大于为进程分配的物理块数
    {
        if(memorytable[i].flag0==0)
        {
            count++;
            break;
        }
    }
    if(count>=1)//可以分配 从低地址分配
    {
        memorytable[i].flag0=1;//修改该位状态
        memorytable[i].counter=0;//使用LRU算法的话，该页框对应的计数器要清0
        memorytable[i].page=pagenumber;//装入的是哪一页



        //修改页表
        for(int j=0;j<pgtlist[ptr].size();j++)
        {
            if(pagenumber==pgtlist[ptr][j].pagenumber)
            {
                 pgtlist[ptr][j].framenumber=i;//修改对应的块号
                 break;
            }
        }
        return i;
    }
    else//空闲块不足1个分配失败
    {
        cout<<"内存不足分配失败"<<endl;
        return error;
    }
}

//进程执行完毕后释放内存函数
void free()
{
    for(int i=0;i<memorytable.size();i++)
    {
        if(i==runningprocess.pf[0]||i==runningprocess.pf[1]||i==runningprocess.pf[2])
        {
            memorytable[i].flag0=0;//将占用的物理页框的占用标志位都修改位0
        }
    }
}


//LRU
int Lru(int pagenumber)
{
    int b=runningprocess.pf[0];//获取该进程占用的物理页框
    int c=runningprocess.pf[1];
    int d=runningprocess.pf[2];
    vector<pageframe> pageframelist(3);
    pageframelist[0]=memorytable[b];//讲这些页框单独拿出来放在容器中
    pageframelist[1]=memorytable[c];
    pageframelist[2]=memorytable[d];
    sort(pageframelist.begin(),pageframelist.end(),CmpCounter);//根据计数器数值从大到小排序
    int e=pageframelist[0].page;//要淘汰的页号
    if(pgtlist[ptr][e].changeflag==1)//如果该页框装的页面被修改过
    {//写回磁盘
    /*
        int f=pagetable[e].diskdress;
        disk[f].dpage=pagetable[e].                                                                                                                                                                                                                                                                                                                                                                         agenumber;
   */
    }
    else{}
    int s=pgtlist[ptr][e].framenumber;//找到对应的物理块号
    memorytable[s].page=pagenumber;
    memorytable[s].counter=0;//新替换装入恶计数器值为0
    for(int p=0;p<3;p++)
            {
                if(runningprocess.pf[p]!=-1&&runningprocess.pf[p]!=s)
                {
                    memorytable[p].counter++;//其他装有页面的页框对应的计数器加1
                }
            }
     ofile.open("d:\\lane.txt",ios::app);
     cout<<dec<<"页面"<<pagenumber<<"替换页面"<<e<<"装入块号为"<<s<<"的物理页框"<<endl;
     ofile<<dec<<"页面"<<pagenumber<<"替换页面"<<e<<"装入块号为"<<s<<"的物理页框"<<endl;
     ofile.close();
     //如果在这里修改页表 此时活动页表应该是新进程的页表 所以在装入过后 立马返回运行？？这是活动页表也变成了这个进程的页表？
               //修改页表
                pgtlist[ptr][e].flag=0;
                pgtlist[ptr][pagenumber].flag=1;
                pgtlist[ptr][pagenumber].framenumber=s;
                //修改快表
                for(int i=0;i<3;i++)
                {
                    if(tlb[i].framenumber==s)
                    {
                        tlb[i].pagenumber=pagenumber;
                    }
                }
     return s;
}

//SCR

int Scr(int pagenumber)
{
    int i;
    int b=runningprocess.pf[0];//获得该进程占用的各个物理页框
    int c=runningprocess.pf[1];
    int d=runningprocess.pf[2];//应该是每个页面设置一个引用位，为了方便直接加页表项了。。本来给页框设置的但是错了。。。
    pagetableentry temp;//用来后面保存队头的缓存页面
    vector<pagetableentry> pagelist(0);
    for(i=0;i<pgtlist[ptr].size();i++)//将已经装入的页面单独拿出来放在pagelist这个容器中
    {
        if(pgtlist[ptr][i].framenumber==b||pgtlist[ptr][i].framenumber==c||pgtlist[ptr][i].framenumber==d)//顺序肯定是正确的，一定是FIFO页面队列，因为从低地址分配
        {
            pagelist.push_back(pgtlist[ptr][i]);
        }

    }
    //先检查队头
    while(pagelist[0].r!=0)
    {
        temp=pagelist[0];//暂时保存队头
        pagelist.erase(pagelist.begin());//删除队头
        temp.r=0;//引用位不为0，将其置0
        pagelist.push_back(temp);//将队头重新插入队尾
    }
    //队头引用位不为1将其淘汰
    int e=pagelist[0].pagenumber;//要淘汰的页号
    if(pgtlist[ptr][e].changeflag==1)//如果该页框装的页面被修改过
    {//写回磁盘
    /*
        int f=pagetable[e].diskdress;
        disk[f].dpage=pagetable[e].                                                                                                                                                                                                                                                                                                                                                                         agenumber;
   */
    }
    else{}
    int s=pgtlist[ptr][e].framenumber;//找到对应的物理块号
    memorytable[s].page=pagenumber;
    //修改这个页面在页表项的引用位
    pgtlist[ptr][pagenumber].r=1;//新装入的页面引用位为1
    //将缓存容器里的页框的引用位与页表项中的统一
    b=pagelist[1].pagenumber;
    pgtlist[ptr][b].r=1;pagelist[1].r;
    c=pagelist[2].pagenumber;
    pgtlist[ptr][c].r=1;pagelist[2].r;
    ofile.open("d:\\lane.txt",ios::app);
    cout<<"页面"<<dec<<pagenumber<<"替换页面"<<e<<"装入块号为"<<s<<"的物理页框"<<endl;
    ofile<<"页面"<<dec<<pagenumber<<"替换页面"<<e<<"装入块号为"<<s<<"的物理页框"<<endl;
    ofile.close();
     //如果在这里修改页表 此时活动页表应该是新进程的页表 所以在装入过后 立马返回运行？？这是活动页表也变成了这个进程的页表？
               //修改页表和快表
                pgtlist[ptr][e].flag=0;
                pgtlist[ptr][pagenumber].flag=1;
                pgtlist[ptr][pagenumber].framenumber=s;
                //修改快表
                //如果我把快表的大小设置为8，分配的物理块数只有3个。。。如果快表没满的话，建立新的快表项，那么原快表项对应的物理块失效 可是没法写
                  for(int i=0;i<3;i++)
                {
                    if(tlb[i].framenumber==s)
                    {
                        tlb[i].pagenumber=pagenumber;
                    }
                }
     return s;
}

//Fifo
int Fifo(int pagenumber)
{
    int i,min,b2,c2,d2;
    int b=runningprocess.pf[0];//获得该进程占用的各个物理页框
    int c=runningprocess.pf[1];
    int d=runningprocess.pf[2];
    int b1=memorytable[b].page;
    int c1=memorytable[c].page;
    int d1=memorytable[d].page;
    for(i=0;i<runningprocess.inss+1;i++)//找到各个页面对应的指令在该进程的指令序列中第一次出现的位置
    {
         if(runningprocess.instructorder[i]==b1)
         {
              b2=i;
             break;//在此处break，可以获得该页面号（指令号）第一次出现的位置
         }

    }
    for(i=0;i<runningprocess.inss+1;i++)
    {
         if(runningprocess.instructorder[i]==c1)
         {
             c2=i;
             break;//在此处break，可以获得该页面号（指令号）第一次出现的位置
         }

    }
    for( i=0;i<runningprocess.inss+1;i++)
    {
         if(runningprocess.instructorder[i]==d1)
         {
              d2=i;
             break;//在此处break，可以获得该页面号（指令号）第一次出现的位置
         }

    }
    if(b2>=c2)//找这些位置的最小值，即最早执行的指令（最早装入内存的页面）
    {
        min=c2;
        if(c2>=d2)
        {
            min=d2;
        }
        else
        {
            min=c2;
        }
    }
    else
    {
        min=b2;
        if(b2>=d2)
        {
            min=d2;
        }
        else
        {
            min=b2;
        }
    }
    int e=runningprocess.instructorder[min];//要淘汰的页号(页号和指令号相同)
    if(pgtlist[ptr][e].changeflag==1)//如果该页框装的页面被修改过
    {//写回磁盘
    /*
        int f=pagetable[e].diskdress;
        disk[f].dpage=pagetable[e].                                                                                                                                                                                                                                                                                                                                                                         agenumber;
   */
    }
    else{}
    int s=pgtlist[ptr][e].framenumber;//找到对应的物理块号
    memorytable[s].page=pagenumber;
    ofile.open("d:\\lane.txt",ios::app);
    cout<<"页面"<<dec<<pagenumber<<"替换页面"<<e<<"装入块号为"<<s<<"的物理页框"<<endl;
    ofile<<"页面"<<dec<<pagenumber<<"替换页面"<<e<<"装入块号为"<<s<<"的物理页框"<<endl;
    ofile.close();
     //如果在这里修改页表 此时活动页表应该是新进程的页表 所以在装入过后 立马返回运行？？这是活动页表也变成了这个进程的页表？
               //修改页表
                pgtlist[ptr][e].flag=0;
                pgtlist[ptr][pagenumber].flag=1;
                pgtlist[ptr][pagenumber].framenumber=s;
                //修改快表
                for(int i=0;i<3;i++)
                {
                    if(tlb[i].framenumber==s)
                    {
                        tlb[i].pagenumber=pagenumber;
                    }
                }
     return s;
}

//Clock
int Clock(int pagenumber)
{

    while(pgtlist[ptr][(*it).page].r!=0)//遍历循环队列，直到找到引用位为0的
                    {
                        pgtlist[ptr][(*it).page].r=0;//引用位改为0
                         if((*it).page==memorytable[2].page)//如果指针指向队尾
                    {
                        it=it-3+1;//修改指针，指向队头，实现循环 3为分配的物理框数
                    }
                    else
                    {
                        it++;//指针向后移一位
                    }
                    }

    //队头引用位不为1将其淘汰
    int e=(*it).page;//要淘汰的页号
    if(pgtlist[ptr][e].changeflag==1)//如果该页框装的页面被修改过
    {//写回磁盘
    /*
        int f=pagetable[e].diskdress;
        disk[f].dpage=pagetable[e].                                                                                                                                                                                                                                                                                                                                                                         agenumber;
    */
    }
    else{}
    int s=pgtlist[ptr][e].framenumber;//找到对应的物理块号
    memorytable[s].page=pagenumber;
    //修改这个页面在页表项的引用位
    pgtlist[ptr][pagenumber].r=1;//新装入的页面引用位为1
    ofile.open("d:\\lane.txt",ios::app);
    cout<<"页面"<<dec<<pagenumber<<"替换页面"<<e<<"装入块号为"<<s<<"的物理页框"<<endl;
    ofile<<"页面"<<dec<<pagenumber<<"替换页面"<<e<<"装入块号为"<<s<<"的物理页框"<<endl;
    ofile.close();
     //如果在这里修改页表 此时活动页表应该是新进程的页表 所以在装入过后 立马返回运行？？这是活动页表也变成了这个进程的页表？
               //修改页表和快表
                pgtlist[ptr][e].flag=0;
                pgtlist[ptr][pagenumber].flag=1;
                pgtlist[ptr][pagenumber].framenumber=s;
                //修改快表
                //如果我把快表的大小设置为8，分配的物理块数只有3个。。。如果快表没满的话，建立新的快表项，那么原快表项对应的物理块失效 可是没法写
                  for(int i=0;i<3;i++)
                {
                    if(tlb[i].framenumber==s)
                    {
                        tlb[i].pagenumber=pagenumber;
                    }
                }
     return s;
                }






void running(int n);

//缺页异常处理
int pagehandling(int pagenumber,Ui::Dialog *dis)//
{
    //挂起请求调页的进程
    QString tempStr;
    runningprocess.state=block;
    WaitQueue.push_back(runningprocess);//进程等待 假设等待==挂起
    DISKFLAG=1;
    int framenumber;
    //根据调度策略调度进程
    //暂时不放cpu
    //根据页号搜索对应的外存地址  这里应该设个时间- -假设从磁盘调入某页内容到内存为t(ns)
    ofile.open("d:\\lane.txt",ios::app);
    for(int i=0;i<pgtlist[ptr].size();i++)
    {
        if(pagenumber==pgtlist[ptr][i].pagenumber)
        {
            int diskaddress=pgtlist[ptr][i].diskaddress;//获取辅存地址
            cout<<"获得该页在辅存的地址"<<diskaddress<<endl;
            ofile<<"获得该页在辅存的地址"<<diskaddress<<endl;
            //查看内存中有无空闲页框，有则分配一个，根据固定分配局部置换策略分配(这个页框指从分配的固定物理块数中取，如果固定物理块数满，不可能再拿一个空的出来)
            ofile.close();
            if(runningprocess.framecount<3)
            {
                int l=fenpei(3,pagenumber);//分配物理内存
                if(l!=error)//如果分配成功
                {
                    ofile.open("d:\\lane.txt",ios::app);
                    cout<<dec<<"分配的物理块号为"<<l<<endl;
                    ofile<<dec<<"分配的物理块号为"<<l<<endl;
                    it=memorytable.begin();//指针指向内存的开始，其实每个进程一开始又都是从0号内存块开始占用的
                    ofile.close();
                    framenumber=l;
                    runningprocess.framecount=runningprocess.framecount+1;//a占用的页框数增加
                   for(int k=0;k<3;k++)
                   {
                    if(runningprocess.pf[k]==-1)
                    {
                    runningprocess.pf[k]=framenumber;//修改进程中信息
                    break;
                    }

                   }

                }
                ofile.open("d:\\lane.txt",ios::app);
                cout<<dec<<"该进程已经占用的物理页框数"<<runningprocess.framecount;
                ofile<<dec<<"该进程已经占用的物理页框数"<<runningprocess.framecount;
                ofile.close();
                //写入页表
                pgtlist[ptr][i].framenumber=l;
                //写入快表
                if(tlb.size()==3)
                  {//按先入先出淘汰 其实 如果快表的size为3的话肯定有页面置换，不会执行到这里
                  tlb[0].pagenumber=pagenumber;
                  tlb[0].framenumber=framenumber;
                  }
                  else
                  {//创建新的快表项
                      tlbentry newentry;
                      newentry.pagenumber=pagenumber;
                      newentry.framenumber=framenumber;
                      tlb.push_back(newentry);
                  }
                return framenumber;
            }
            else
            {
                //根据置换算法选择页面替换
                tempStr=dis->comboBox_2->currentText();
                if(tempStr=="SCR")
                    {
                      framenumber=Scr(pagenumber);//SCR调换算法
                    }
                    else if(tempStr=="LRU")
                    {
                        framenumber=Lru(pagenumber);
                    }
                   else if(tempStr=="FIFO")
                    {
                    framenumber=Fifo(pagenumber);
                    }
                else if(tempStr=="CLOCK")
                 {
                 framenumber=Clock(pagenumber);
                 }
                   //调度完毕后返回到原进程发生中断的地方执行
                  ofile.open("d:\\lane.txt",ios::app);
                  cout<<"分配的物理块号为"<<framenumber<<endl;
                  ofile<<"分配的物理块号为"<<framenumber<<endl;
                  ofile.close();
                return framenumber;
                running(n);//运行下一条指令
                return framenumber;
            }
        }
    }
}

//清空快表 其实应该是mmu的功能
void cleartlb()
{
    tlb.resize(0);//重置快表大小
}

//mmu功能模拟
int  mmu(int logicaladdress,int instructnum,Ui::Dialog *dis)//mmu
{
    int i,f;
    QString tempStr;
    //根据页表基址获得页表 逻辑地址17位 页号5位 偏移地址12位
    ofile.open("d:\\lane.txt",ios::app);
    cout<<endl<<"逻辑地址:"<<hex<<logicaladdress<<"h"<<endl;//hex以16进制输出
    ofile<<endl<<"逻辑地址:"<<hex<<logicaladdress<<"h"<<endl;//hex以16进制输出
    int pagenumber=logicaladdress>>11;//获得页号
    cout<<dec<<"页号:"<<pagenumber<<endl;
    ofile<<dec<<"页号:"<<pagenumber<<endl;
    int offsetaddress=logicaladdress&0x07ff;//获得偏移地址
    cout<<"偏移地址:"<<hex<<offsetaddress<<"h"<<endl;
    ofile<<"偏移地址:"<<hex<<offsetaddress<<"h"<<endl;
    ofile.close();
    int physicaladdress;//转化后的物理地址
    int framenumber;//页框号
    if(pagenumber>runningprocess.pagetablelength-1)//如果页号大于页表长度-1
    {
        return error;//发生越界中断
    }

    else{
    //int position=pagetableaddress+pagenumber*pagetableentrylen;//将页表始址与页号和页表项长度的乘积相加，得到该页表项在页表中的位置
    for( i=0;i<tlb.size();i++)//搜索快表 如果快表里没有记录 那肯定不在快表里( tlb.size==0)
    {
        if(pagenumber==tlb[i].pagenumber)//命中
        {
            ofile.open("d:\\lane.txt",ios::app);
            framenumber=tlb[i].framenumber;
            physicaladdress=(framenumber<<12)+offsetaddress;//拼接物理地址
            cout<<"命中快表"<<endl;
            ofile<<"命中快表"<<endl;
            cout<<"该进程已经占用的物理页框数"<<runningprocess.framecount<<endl;
            ofile<<"该进程已经占用的物理页框数"<<runningprocess.framecount<<endl;
            cout<<hex<<"物理地址为"<<physicaladdress<<"h"<<endl;
            ofile<<hex<<"物理地址为"<<physicaladdress<<"h"<<endl;
            ofile.close();
            //访问权限检查  未设保护位 暂时不写
            //其实下面这段不应该写在这里。。。
            memorytable[framenumber].counter=0;//LRU 算法中用到的计数器
            for(int p=0;p<3;p++)
            {
                if(runningprocess.pf[p]!=-1&&runningprocess.pf[p]!=framenumber)
                {
                    memorytable[p].counter++;//其他装有页面的页框对应的计数器加1
                }
            }
            pgtlist[ptr][pagenumber].r=1;//命中，引用位修改为1
            memorytable[framenumber].page=pagenumber;//装入的页面的页号
            return physicaladdress;
        }
    }
    //快表未命中
        for(i=0;i<pgtlist[ptr].size();i++)//搜索进程页表 如果进程页表size为0 一定不命中
        {
            if(pagenumber==pgtlist[ptr][i].pagenumber)//命中  这里应该是都命中的 书上说如果命中说明进程存在于内存中没能理解。。。
            {

                if(pgtlist[ptr][i].flag==1)//如果该页面在主存
                {
                  ofile.open("d:\\lane.txt",ios::app);
                  framenumber=pgtlist[ptr][i].framenumber;
                  physicaladdress=(framenumber<<12)+offsetaddress;//拼接物理地址
                  cout<<"命中页表"<<endl;
                  ofile<<"命中页表"<<endl;
                  cout<<"该进程已经占用的物理页框数"<<runningprocess.framecount<<endl;
                  ofile<<"该进程已经占用的物理页框数"<<runningprocess.framecount<<endl;
                  cout<<hex<<"物理地址为"<<physicaladdress<<"h"<<endl;
                  ofile<<hex<<"物理地址为"<<physicaladdress<<"h"<<endl;
                  ofile.close();
                  memorytable[framenumber].counter=0;
                  for(int p=0;p<3;p++)
                  {
                     if(runningprocess.pf[p]!=-1&&runningprocess.pf[p]!=framenumber)
                     {
                       memorytable[p].counter++;//其他装有页面的页框对应的计数器加1
                     }
                  }
                  memorytable[framenumber].page=pagenumber;
                  pgtlist[ptr][pagenumber].r=1;//命中，引用位修改为1
                  //memorytable[framenumber].flag0=1;本来就是已占用
                  //写入快表
                  /*这里的问题是
                  1 如果快表满了，写新的表项是应该淘汰哪个旧的表项 书上说最简单的是先入先出
                  2 快表表项与页表表项能不能统一，这样就能直接添加了，但是这样快表表项要添加新的成员，快表就长了*/
                  if(tlb.size()==3)
                  {
                  tlb[0].pagenumber=pagenumber;
                  tlb[0].framenumber=framenumber;//如果快表已满，以先入先出的方式，淘汰最早写入快表的快表项内容
                  }
                  else
                  {//创建新的快表项
                      tlbentry newentry;
                      newentry.pagenumber=pagenumber;
                      newentry.framenumber=framenumber;
                      tlb.push_back(newentry);
                  }
                  return physicaladdress;
                }
            }
            else{}
        }


        //进程页表未命中
        //缺页
                   ofile.open("d:\\lane.txt",ios::app);
                   cout<<"缺页"<<endl;
                   ofile<<"缺页"<<endl;
                   ofile.close();
                   runningprocess.instructnum=instructnum;
                   framenumber=pagehandling(pagenumber,dis);//缺页异常处理
                   system_time=system_time+1;
                   Writedown2();
                   dis->textEdit->setText(tempStr.setNum(system_time));
                   for(f=0;f<15;f++)//表格操作
                   {

                       if(memorytable[f].flag0==1)
                       {
                          dis->tableWidget->setItem(f,1,new QTableWidgetItem(tempStr.fromLocal8Bit("占用")));
                          dis->tableWidget->setItem(f,2,new QTableWidgetItem(tempStr.setNum(memorytable[f].page)));
                       }
                       else
                       {
                           dis->tableWidget->setItem(f,1,new QTableWidgetItem(tempStr.fromLocal8Bit("空闲")));
                           dis->tableWidget->setItem(f,2,new QTableWidgetItem(tempStr.fromLocal8Bit("无")));
                       }
                  }
                    Sleep(1000);//挂起1s
                    qApp->processEvents();//调用qApp->processEvents()来强制刷新
                    for(int g=0;g<n;g++)//这里直接调用JudgeIn(n)会出错。。。
                    {
                        if(pcblist[g].intime==system_time)
                        ReadyQueue.push_back(pcblist[g]);
                    }
                   ofile.open("d:\\lane.txt",ios::app);
                   cout<<endl<<"**当前系统时间**:"<<system_time<<endl;
                   ofile<<endl<<"**当前系统时间**:"<<system_time<<endl;
                   cout<<"完成缺页调度"<<endl;
                   ofile<<"完成缺页调度"<<endl;
                   cout<<endl;
                   ofile<<endl;
                   physicaladdress=(framenumber<<12)+offsetaddress;//拼接物理地址
                   cout<<"物理地址为"<<hex<<physicaladdress<<"h"<<endl;
                   ofile<<"物理地址为"<<hex<<physicaladdress<<"h"<<endl;
                   cout<<endl<<"发生缺页中断时的队列信息如下:"<<endl;
                   ofile<<endl<<"发生缺页中断时的队列信息如下:"<<endl;
                   ofile.close();
                   PrintQueue();
                   WaitQueue.erase(WaitQueue.begin());//在进程离开等待队列之前打印队列信息
                   return physicaladdress;


        }//else{}
}

//先入先出调度算法
void sche_fifo(Ui::Dialog *dis)
{

       int i;
       QString tempStr;
        while(ReadyQueue.size()==0)//如果就绪队列里没有进程
        {
        system_time=system_time+1;//等待新进程到达
        dis->textEdit->setText(tempStr.setNum(system_time));
        for(i=0;i<15;i++)//显示内存分块表操作
        {

            if(memorytable[i].flag0==1)
            {
               dis->tableWidget->setItem(i,1,new QTableWidgetItem(tempStr.fromLocal8Bit("占用")));
               dis->tableWidget->setItem(i,2,new QTableWidgetItem(tempStr.setNum(memorytable[i].page)));
            }
            else
            {
                dis->tableWidget->setItem(i,1,new QTableWidgetItem(tempStr.fromLocal8Bit("空闲")));
                dis->tableWidget->setItem(i,2,new QTableWidgetItem(tempStr.fromLocal8Bit("无")));
            }
       }
        Sleep(1000);//挂起1s
        qApp->processEvents();//调用qApp->processEvents()来强制刷新
        cout<<"当前系统时间"<<system_time;

       for(int i=0;i<n;i++)//这里直接调用JudgeIn(n)会出错。。。
       {
           if(pcblist[i].intime==system_time)
           ReadyQueue.push_back(pcblist[i]);
       }

        }
    ReadyQueue[0].state=runningg;//选取最先进入就绪队列的进程运行
    runningprocess=ReadyQueue[0];//进入cpu运行
    ReadyQueue.erase(ReadyQueue.begin());//从就绪队列移除

}


//优先级调度算法
void sche_pr(Ui::Dialog *dis)//优先级数大的优先级高
{

        int i;
           QString tempStr;
            while(ReadyQueue.size()==0)
            {
            system_time=system_time+1;
            dis->textEdit->setText(tempStr.setNum(system_time));
            dis->textEdit_3->setText(tempStr.setNum(runningprocess.instructnum));
            for(i=0;i<15;i++)//显示内存分块表
            {

                if(memorytable[i].flag0==1)
                {
                    dis->tableWidget->setItem(i,1,new QTableWidgetItem(tempStr.fromLocal8Bit("占用")));
                   dis->tableWidget->setItem(i,2,new QTableWidgetItem(tempStr.setNum(memorytable[i].page)));
                }
                else
                {
                    dis->tableWidget->setItem(i,1,new QTableWidgetItem(tempStr.fromLocal8Bit("空闲")));
                    dis->tableWidget->setItem(i,2,new QTableWidgetItem(tempStr.fromLocal8Bit("无")));
                }
           }
            Sleep(1000);;
            qApp->processEvents();//调用qApp->processEvents()来强制刷新
            cout<<"当前系统时间"<<system_time;

           for(int i=0;i<n;i++)//这里直接调用JudgeIn(n)会出错。。。
           {
               if(pcblist[i].intime==system_time)//将到达的进程放入就绪队列中
               ReadyQueue.push_back(pcblist[i]);
           }

            }
        sort(ReadyQueue.begin(),ReadyQueue.end(),CmpPriority);//优先级排序
        ReadyQueue[0].state=runningg;//选取最先进入就绪队列的进程运行
        runningprocess=ReadyQueue[0];//进入cpu运行
        ReadyQueue.erase(ReadyQueue.begin());//从就绪队列移除

}



void Dialog::on_pushButton_clicked()
{
        pcblist.resize(0);//每次使用前清空进程列表
        EndQueue.resize(0);//清空完成队列
        ReadyQueue.resize(0);//清空就绪队列
        WaitQueue.resize(0);//清空等待队列
        bool ok;
        int i;
        QString tempStr;
        QString valueStr=ui->lineEdit->text();//获得lineEdit的text的值（即进程个数），但是是一个QString，需要转化为int类型才能赋值给n
        n=valueStr.toInt(&ok);//转化为int赋值给n
        Writedown();//调用writedown函数 既有本身的作用 也可以创建文件
        createprocess();//生成进程
        sort(pcblist.begin(),pcblist.end(),CmpInTime);//将产生的每个进程的进入时间按从小到大排列
        system_time=pcblist[0].intime;//将系统时间初始化为进入时间最早的那个进程的进入时间(相对于系统当前时间)
        ui->textEdit->setText(tempStr.setNum(system_time));
        Sleep(1000);//为了让时间的变化直观一点，挂起1s
        ReadyQueue.push_back(pcblist[0]);
        for( i=1;i<n;i++)//如果有到达时间相同的情况 也把其插入就绪队列 从进程队列第二个开始检查
           {
             if(pcblist[i].intime==system_time)
               {
                    ReadyQueue.push_back(pcblist[i]);
               }
           }
        //检查两个相同时间的进程的优先级，优先级高的先运行
        sort(ReadyQueue.begin(),ReadyQueue.end(),CmpPriority);//优先级排序
        runningprocess=ReadyQueue[0];//把最早到达的作业放入cpu运行
        ReadyQueue.erase(ReadyQueue.begin());//从就绪队列移除
        QTableWidget *tableWidget = new QTableWidget(n,4);//建立一个行为n（进程个数）列为4（一些要显示的属性）的表格
        QString str,str1,str2,str3;//QT直接显示中文会乱码，使用QString的fromLocal8Bit()函数转化
        str = str.fromLocal8Bit("进程信息表");
        tableWidget->setWindowTitle(str); //修改表格名，从左到右显示
        QStringList header;
        str1= str1.fromLocal8Bit("优先级");
        str2= str2.fromLocal8Bit("到达时间");
        str3= str3.fromLocal8Bit("指令数目");
        header<<"ID"<<str1<<str2<<str3;
        tableWidget->setHorizontalHeaderLabels(header);//显示表格操作
        for(int i=0;i<n;i++)
        {

            tableWidget->setItem(i,0,new QTableWidgetItem(tempStr.setNum(pcblist[i].pid)));
            tableWidget->setItem(i,1,new QTableWidgetItem(tempStr.setNum(pcblist[i].priority)));
            tableWidget->setItem(i,2,new QTableWidgetItem(tempStr.setNum(pcblist[i].intime)));
            tableWidget->setItem(i,3,new QTableWidgetItem(tempStr.setNum(pcblist[i].instructcount)));

        }
        tableWidget->show();//显示表格
}


void Dialog::on_pushButton_2_clicked()
{
    int f;
    QString tempStr;
    while(EndQueue.size()<n)
    {//进程没有全部执行完毕
        //假设该进程指令备份全在磁盘
        ui->textEdit_4->setText("  ");
       ofile.open("d:\\lane.txt",ios::app);
       cleartlb();//清空快表表项
       ptr=runningprocess.pagetableaddress;//将该运行进程的页表基址放入页表基址寄存器
       cout<<endl<<endl<<dec<<"当前系统时间:"<<system_time<<endl;
       ofile<<endl<<endl<<dec<<"当前系统时间:"<<system_time<<endl;
       ofile.close();
       ofile.open("d:\\lane.txt",ios::app);
       cout<<"当前运行的进程为:"<<dec<<runningprocess.pid<<endl;
       cout<<"当前进程运行的指令序列为:"<<endl;
       ofile<<"当前运行的进程为:"<<dec<<runningprocess.pid<<endl;
       ofile<<"当前进程运行的指令序列为:"<<endl;

       for(int i=0;i<runningprocess.instructorder.size();i++)
       {
           cout<<runningprocess.instructorder[i]<<" ";
           ofile<<runningprocess.instructorder[i]<<" ";
       }
       ofile.close();
       int j=runningprocess.instructnum;
       for(f=0;f<runningprocess.instructorder.size();f++)//在textEdit控件框中显示当前运行进程的指令序列
          {
              ui->textEdit_4->insertPlainText(tempStr.setNum(runningprocess.instructorder[f])+"   ");
              ui->textEdit_4->moveCursor(QTextCursor::End);//将光标移动到末尾

          }
       while(j<runningprocess.instructorder.size())//小于指令序列长度而不是指令个数
       {

         ofile.open("d:\\lane.txt",ios::app);
         cout<<endl<<dec<<"**当前系统时间**:"<<system_time<<endl;
         ofile<<endl<<dec<<"**当前系统时间**:"<<system_time<<endl;
         cout<<endl<<dec<<"**************当前执行第"<<runningprocess.instructorder[j]<<"条指令**************"<<endl;
         ofile<<endl<<dec<<"**************当前执行第"<<runningprocess.instructorder[j]<<"条指令**************"<<endl;
         runningprocess.instructnum=runningprocess.instructorder[j];
         runningprocess.inss=j;
         ofile.close();
         mmu(runningprocess.instructaddress,runningprocess.instructorder[j],ui);//启动mmu
         ofile.open("d:\\lane.txt",ios::app);//以追加的方式写入
         cout<<endl;
         ofile<<endl;
         ofile.close();
         j++;
         runningprocess.instructaddress=2048*(runningprocess.instructorder[j]);//+2^12
         system_time=system_time+1;
         ui->textEdit->setText(tempStr.setNum(system_time));
         for(f=0;f<15;f++)//显示内存分块表的操作
         {

             if(memorytable[f].flag0==1)
             {
                 ui->tableWidget->setItem(f,1,new QTableWidgetItem(tempStr.fromLocal8Bit("占用")));
                 ui->tableWidget->setItem(f,2,new QTableWidgetItem(tempStr.setNum(memorytable[f].page)));
             }
             else
             {
                 ui->tableWidget->setItem(f,1,new QTableWidgetItem(tempStr.fromLocal8Bit("空闲")));
                 ui->tableWidget->setItem(f,2,new QTableWidgetItem(tempStr.fromLocal8Bit("无")));
             }
        }
         ui->textEdit_2->setText(tempStr.setNum(runningprocess.pid));
         ui->textEdit_3->setText(tempStr.setNum(runningprocess.instructnum));

         Sleep(1000);
         JudgeIn(n);
         ofile.open("d:\\lane.txt",ios::app);
         qApp->processEvents();
         ofile.close();
         Writedown2();//写**********
         PrintQueue();//写**********

       }
       ofile.open("d:\\lane.txt",ios::app);
       cout<<endl<<"该进程运行完毕"<<endl<<endl;
       ofile<<endl<<"该进程运行完毕"<<endl<<endl;
       ofile.close();
       runningprocess.state=dead;
       EndQueue.push_back(runningprocess);
       free();//归还内存
       tempStr=ui->comboBox->currentText();
       if(EndQueue.size()<n)
       {
           if(tempStr=="Priority")
           {
             sche_pr(ui);//非剥夺优先级调度
           }
           else if(tempStr=="FIFO")
           {
               sche_fifo(ui);//先入先出调度
           }
       }
   }
       ofile.open("d:\\lane.txt",ios::app);
       cout<<endl<<"进程全部运行完毕";
       ofile<<endl<<"进程全部运行完毕";
       ofile.close();
       PrintQueue();//写**********打印队列信息
}





void Dialog::on_pushButton_3_clicked()//打开外存分配文件d:\\exslane
{
        //QProcess类:启动一个外部的程序并与之交互
        QProcess* process = new QProcess();
        QString notepadPath = "notepad.exe d:\\exslane.txt";//notepad即电脑上基本都有的记事本
        process->start(notepadPath);

}

void Dialog::on_pushButton_4_clicked()//打开内存分配文件
{
    //QProcess类:启动一个外部的程序并与之交互
    QProcess* process = new QProcess();
    QString notepadPath = "notepad.exe d:\\stlane.txt";//notepad即电脑上基本都有的记事本
    process->start(notepadPath);

}


void Dialog::on_pushButton_5_clicked()//打开总运行信息文件
{
    //QProcess类:启动一个外部的程序并与之交互
    QProcess* process = new QProcess();
    QString notepadPath = "notepad.exe d:\\lane.txt";//notepad即电脑上基本都有的记事本
    process->start(notepadPath);
}
