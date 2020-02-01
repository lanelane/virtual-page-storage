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
#include <windows.h> //winͷ�ļ�
#define ProNumber 3
#define runnable 2
#define runningg 1
#define block 3
#define dead     3//�������н���
#define error    -1
using namespace std;

//�жϴ����������ʱ���ܷ��������л�,
//��Ϊ�жϴ������ʹ�õ�ǰ���̵��ں�ջ......??�����������

//QT�Դ��Ĵ��룬����һ���࣬����Ƴ�����ui������Ϊ�����һ���Ӷ������乹�캯���У�����ɶ��Ӷ���Ĺ��죬��ʹ���Ӷ���ui������setupUi(this)����ʵ��ui����ʵ��
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

//����һ��ָ������ʱ��Ϊһ��ʱ�䵥λ
//����Ӵ��̵�ҳװ���ڴ���ʱ��Ϊһ��ʱ�䵥λ
//��cin��cout��ָ�����ƺ󣬸����ƽ�һֱ��Ч��ֱ������ָ��ʹ���������ơ�hex oct dec
/*****************************************************ȫ����*****************************************************/

//����ҳ����
class pagetableentry//����ҳ��ҳ����(����ʽ��ҳ����)
{
public:
    int pagenumber;//ҳ�� 5λ
    int framenumber;//ҳ��� 4λ
    int flag;//פ����־λ 1λ
    int diskaddress;//����ַ 4λ //��Ūһ����ҳ����ҳ���˸�
    int changeflag;//�޸�λ 1λ ����1Ϊ�޸���
    int r;//����λ SCR�㷨���õ�
    pagetableentry()
    {
        r=1;
    }
};//һ��ҳ���16λ��һ��ҳ�����32��ҳ���ҳ�����ռ64B

//������
class tlbentry//������
{
public:
    int pagenumber;//ҳ��
    int framenumber;//ҳ���
};

//����ҳ��
class pageframe
{
public:
    pageframe()//��ʼ��
    {
        counter=0;
        flag0=0;
    }
    int flag0;//��ҳ���Ƿ���ҳװ��ı�־
    int page;//װ���ҳ��
    int counter;//LRU�㷨���õ��ļ�����

};

//���̿�
class diskentry
{
public:
    diskentry()
    {
    dflag=0;
    }

    int diskaddress;
    int dpage;//װ���ҳ�漴װ�������
    int dflag;//����Ƿ�ҳ��װ���־λ=1Ϊռ��
};


vector<int> externstorage(int instructcount, int id);//�����亯��
vector<vector<pagetableentry> > pgtlist(10);//����������ҳ�����һ�𣬰����ǿ�����һ����������ǵĵ�ַ��ʵ��û����ϵ�� ����ҳ���ַ�����������������±�
vector<tlbentry> tlb(0);//���  ����������3������
vector<diskentry> disk(500);// ���� ��������С500��
vector<pageframe> memorytable(16);//�ڴ�ֿ�� ������64KB һҳ4KB ���ڴ�ɷ�Ϊ16�� ������ڴ�����һ��ֳ���ר�Ŵ�Ÿ������̵�ҳ��
ofstream ofile;//�ļ�
vector<pageframe>::iterator it;//�������ѭ�����е�ָ��
//pcb ģ��
class pcb
{
public:
    int i,j;
    int state;//����״̬
    int pid;//���̱�ʶ��
    int priority;//�������ȼ� ��ֵԽС���ȼ�Խ��
    int pagetableaddress;//ҳ����ʼ��ַ
    int pagetablelength;//ҳ���� (ҳ�������)
    int pagetableentrylen;//ҳ����� 16λ
    int framecount;//��ռ�õ��������
    int intime;//���̵���ʱ��
    int pf[3];//��¼ռ������Щ����ҳ�������
    int instructcount;//ָ�����Ŀ
    //����һ��ָ��ռ1ҳ
    int instructaddress;//��һ��ָ����׵�ַ(�߼���ַ)
    int instructnum;//��ǰִ�е�����ָ��
    int inss;//��ǰִ�е�����ָ������λ��ָ������ָ��������������±�)
    vector<int> instructorder;//�������дvector<int> instructorder(0);����syntax error:'constant' ��Ϊ�������ڳ�ʼ���˴�С ���ڹ��캯�����ʼ�� ���߲���ʼ��
    void pcb0()//��ʼ��  ����ѳ�ʼ��д�ڹ��캯���� ����main����֮ǰ����ִ��ȫ�ֱ�������Ĺ��캯�� �������󡣡���
    {

        ofile.open("d:\\lane.txt",ios::app);//�򿪼�¼�ļ�����׷�ӵķ�ʽд�� ��ڵ�����Writedown()���������ļ�һ��������
        instructorder.resize(0);
        state=runnable;
        priority=(rand()%10)+1;
        pid=(rand()%100)+1;//rand()%(b-a+1)+a ����[a,b]֮�������� Ϊ����pid��ò���ͬ�������õķ�Χ��΢��һ��
        cout<<endl<<"����id:"<<dec<<pid<<"  ";
        cout<<endl<<"�������ȼ�(���ȼ���ֵԽС���ȼ�Խ��):"<<dec<<priority<<"  ";
        ofile<<endl<<"����id:"<<dec<<pid<<"  ";
        ofile<<endl<<"�������ȼ�(���ȼ���ֵԽС���ȼ�Խ��):"<<dec<<priority<<"  ";
        //���ҳ������ڴ棬ҳ����õ�λ����������� �˵�ַ�������ַ��ҳ���ŵĵ�ַ����ôȷ���� ����ҳ���С����Ӧ�ò���ȷ���� �����ѡ��һ�����ʵ�λ����
        //�ȶ��ڴ���еĵط����м�⣿--�̶����ڴ����һ������н��̵�ҳ���ˡ������̲��ܳ���2^7
        pagetableaddress=(rand()%10);//�����ҳ���ַ����װ��ҳ���ַ�е����λ�� 0-9
        //��Ϊ���̵�ָ������������ɵģ���ҳ�����СҲ�ǲ�ȷ���ģ�ҳ���Ȳ�ȷ�������ﰴ�����㣬ÿ����������߼���ַ�ռ�Ϊ32ҳ�����32��ҳ���ÿ��ҳ�����16λ���64B
        cout<<"ҳ���ַΪ"<<dec<<15000+pagetableaddress*64<<"h"<<"  ";//���ֵ�Ѿ���16���Ƶģ�����hex��ʽ���
        ofile<<"ҳ���ַΪ"<<dec<<15000+pagetableaddress*64<<"h"<<"  ";
        pagetablelength=32;//��ʵӦ����ҳ��������������ж�Խ��ʱ�����ͨ��ҳ���жϡ������ǳ�ʼ��Ϊinstructcount���������
        pagetableentrylen=16;//��λ��λ
        framecount=0;//ռ�õ�ҳ��տ�ʼΪ0
        intime=(rand()%10)+1;
        cout<<"����ʱ�䣺"<<intime<<"  ";
        ofile<<"����ʱ��:"<<intime<<"  ";
        this->pf[0]=-1;//��ʼ��Ϊ-1��ֵΪ-1����û��ռ������ҳ�򣬴��ڵ���0�Ļ���ֵ�ʹ���ռ�õ�������
        this->pf[1]=-1;
        this->pf[2]=-1;
        instructcount=(rand()%10)+1;//����[1,10]��ָ��
        cout<<"ָ����:"<<instructcount<<"  ";
        ofile<<"ָ����:"<<instructcount<<"  ";
        instructaddress=0;//����һ��ָ��ռ0800h����һ��ָ���׵�ַ�������ַ  �߼��ռ��ַ������0��ַ��ʼ
        /*ָ��Ĵ�ŵ�ַ��˳���,���ָ��˳��ִ�У����Ұ����õ�һ��ָ��ռ1ҳ���ͻ�һֱ����ȱҳ�жϡ�����ת�����ҳ�����������������,�߼���ַ�ռ���������*/ //����ָ������ַָ��
        instructorder.push_back(0);
        for(i=0;i<instructcount-1;i++)//����ָ������ִ������ �������е�һ��һ����0��ָ��
        {
            j=rand()%instructcount;
            instructorder.push_back(j);
        }
        pgtlist[pagetableaddress].resize(instructcount);
        cout<<"ҳ����:"<<dec<<pgtlist[pagetableaddress].size()*2<<"B"<<"  ";
        cout<<endl<<"���̷������:"<<endl;
        ofile<<"ҳ����:"<<dec<<pgtlist[pagetableaddress].size()*2<<"B"<<"  ";
        ofile<<endl<<"���̷������:"<<endl;
        ofile.close();
        vector<int> exs=externstorage(instructcount,pid);
        instructnum=0;
        ofile.open("d:\\lane.txt",ios::app);
        for(i=0;i<instructcount;i++)//ҳ�����ҳ��
        {
            pgtlist[pagetableaddress][i].pagenumber=i;
            pgtlist[pagetableaddress][i].flag=0;//�տ�ʼ�������ڴ���
            pgtlist[pagetableaddress][i].changeflag=0;
        }
        for( j=0;j<instructcount;j++)
        {

            pgtlist[pagetableaddress][j].diskaddress=exs[j];
        }
        for( i=0;i<instructcount;i++)
        {
            cout<<dec<<"����ҳ���"<<i<<"�������ַ(���)��"<<pgtlist[pagetableaddress][i].diskaddress<<endl;
            ofile<<dec<<"����ҳ���"<<i<<"�������ַ(���)��"<<pgtlist[pagetableaddress][i].diskaddress<<endl;
        }
        cout<<endl<<endl<<endl;
        ofile<<endl<<endl<<endl;
        ofile.close();
    }

};




//ȫ����
int n;//���̸���
int ptr;//ҳ���ַ�Ĵ���(ֵ����ǰ�ҳ��Ļ�ַ)
int system_time=0;//ϵͳʱ�䣬��ʼ��Ϊ0
int DISKFLAG;
pcb runningprocess;//��CPU���������еĽ���
vector<pcb> ReadyQueue(0);//��������,,ע��Ҫ��Ϊ0����Ϊ���붼�ǴӶ�β��
vector<pcb> EndQueue(0);//��������
vector<pcb> WaitQueue(0);//�ȴ�����
vector<pcb> pcblist(0);//���� ����

void Writedown()
{
        ofile.open("d:\\lane.txt",ios::app);//�򿪼�¼�ļ�����׷�ӵķ�ʽд��
        if(!ofile)//����ļ������ڣ��򴴽����ļ�
        {
            ofstream nfile( "d:\\lane.txt" );
        }
        cout<<"��ǰһ����"<<n<<"������"<<endl;
        ofile<<"��ǰһ����"<<n<<"������"<<endl;
        cout<<"���ɵĽ�����Ϣ����:"<<endl;
        ofile<<"���ɵĽ�����Ϣ����:"<<endl;
        ofile.close();
}

void Writedown2()//д���ڴ���Ϣ
{

        ofile.open("d:\\stlane.txt",ios::app);//�򿪼�¼�ļ�����׷�ӵķ�ʽд��
        if(!ofile)//����ļ������ڣ��򴴽����ļ�
        {
            ofstream nfile( "d:\\stlane.txt" );
        }
        ofile<<dec<<"��ǰϵͳʱ��:"<<system_time<<endl;
        ofile<<"��ǰ�ڴ�ʹ�����:"<<endl;
        for(int i=0;i<15;i++)
        {
            ofile<<dec<<"�ڴ��"<<i<<"�飺"<<"  ";
            if(memorytable[i].flag0==1)
            {
                ofile<<dec<<"������"<<runningprocess.pid<<"�ĵ�"<<memorytable[i].page<<"��ָ��ռ��"<<endl;
            }
            else
            {
                ofile<<"����";
            }
            ofile<<endl;
        }
        ofile<<"�ڴ��15�鱻��������ҳ���ռ��"<<endl;
        ofile<<endl<<endl;
        ofile.close();
}

//�жϲ����Ľ�����û���뵱ǰϵͳʱ����ȵģ���û�н��̵���
void JudgeIn(int n)//nΪ���̸���
    //�������в����Ľ��̵Ľ���ʱ��
{
    int i=0;
    while(i<n)
    {
        if(pcblist[i].intime==system_time)
        {
            ReadyQueue.push_back(pcblist[i]);}//�����������
        else
        {}
        i++;
    }

}

//��ӡ������Ϣ
void PrintQueue()
{
    ofile.open("d:\\lane.txt",ios::app);//������Ϣ�ļ�
    int k;
      cout<<endl<<"*******************���������*******************";
      cout<<endl<<"��ǰ��������(ֻ���id):"<<endl;
      ofile<<endl<<"*******************���������*******************";
      ofile<<endl<<"��ǰ��������(ֻ���id):"<<endl;
      if(ReadyQueue.size()==0)
      {
          cout<<"��ǰ���������޽���"<<endl;
          ofile<<"��ǰ���������޽���"<<endl;
      }
      else
      {
        for(k=0;k<ReadyQueue.size();k++)
      {
          cout<<dec<<"����"<<ReadyQueue[k].pid<<".";
          ofile<<dec<<"����"<<ReadyQueue[k].pid<<".";
      }
      }
      cout<<endl<<"��ǰ�ȴ�����(ֻ���id):"<<endl;
      ofile<<endl<<"��ǰ�ȴ�����(ֻ���id):"<<endl;
      if(WaitQueue.size()==0)
      {
          cout<<"��ǰ�ȴ������޽���"<<endl;
          ofile<<"��ǰ�ȴ������޽���"<<endl;
      }
      else
      {
          for(k=0;k<WaitQueue.size();k++)
          {
              cout<<dec<<"����"<<WaitQueue[k].pid<<".";
              ofile<<dec<<"����"<<WaitQueue[k].pid<<".";
          }
      }
      cout<<endl<<"��ǰ��ɶ���(ֻ���id):"<<endl;
      ofile<<endl<<"��ǰ��ɶ���(ֻ���id):"<<endl;
      if(EndQueue.size()==0)
      {
          cout<<"��ǰ��ɶ����޽���"<<endl;
          ofile<<"��ǰ��ɶ����޽���"<<endl;
      }
      else
      {
          for(k=0;k<EndQueue.size();k++)
          {
              cout<<dec<<"����"<<EndQueue[k].pid<<".";
              ofile<<dec<<"����"<<EndQueue[k].pid<<".";
          }
      }
      ofile.close();
}

//�����������
void createprocess()
{
    //���̱����ڴ�����
    //���������߼���ַ�ռ䣬����������ӳ���������Ҫ�����ݽṹ��ҳĿ��ҳ��
    //����ÿ�����̵��߼��ռ���128KB ÿҳ4KB �� 32ҳ
    srand(unsigned(time(0)));//���ڼ�������кܿ죬����ÿ����time�õ���ʱ�䶼��һ���ģ�time��ʱ�侫�Ƚϵͣ�ֻ��55ms���������൱��ʹ��ͬһ�����Ӳ���������У����Բ����������������ͬ�ġ����԰�srand����ѭ����
    for(int i=0;i<n;i++)
    {
        pcb a;
        a.pcb0();//��ʼ��
        pcblist.push_back(a);//��������������̼��뵽���̶�����
    }

}



//���̷��亯�� ���ݽ��̵���Ϣ
vector<int> externstorage(int instructcount,int id)
{
    int j=0;
    ofile.open("d:\\exslane.txt",ios::app);//�򿪼�¼�ļ�����׷�ӵķ�ʽд��
    if(!ofile)//����ļ������ڣ��򴴽����ļ�
    {
        ofstream nnfile( "d:\\exslane.txt" );
    }
    vector<int> exstorage(0);//��0��ָ�ʼ ��Ӧ������λ�� Ҳ��������
    while(j<instructcount)
    {
        for(int i=0;i<500;i++)
        {
         if(disk[i].dflag==0)
         {
            disk[i].dpage=j;//���̵�i��������j��ָ��(��j��ҳ��)
            ofile<<"���̵�"<<i<<"��ָ�����"<<id<<"�ĵ�"<<j<<"��ָ��"<<endl;
            disk[i].dflag=1;//�޸�ռ��λΪ1
            exstorage.push_back(i);
            break;
         }
        }
         j++;
    }
    cout<<"ռ������С"<<exstorage.size();
    ofile<<"����"<<id<<"ռ������С:"<<exstorage.size()<<"��"<<endl;
    ofile<<endl;
    ofile.close();
    return exstorage;
}


//�Զ�������ʽ���Խ��̽���ʱ��ıȽ�
bool CmpInTime( const pcb &p1, const pcb &p2)
{
    return p1.intime <=p2.intime;//��������
}

//�Զ�������ʽ���Խ������ȼ����бȽ�
bool CmpPriority(const pcb& p1, const pcb& p2)
{
return p1.priority < p2.priority;
}

//�Զ�������ʽ��������ҳ���Ӧ�ļ��������бȽ�
bool CmpCounter( const pageframe &page1, const pageframe &page2)
{
    return page1.counter>page2.counter;//�������У�������������
}

//ҳ����亯��
int fenpei(int n,int pagenumber)//n:Ϊ���̷������������
{
    int count=0;
    int i;
    for( i=0;i<16;i++)//�����п����Ƿ����Ϊ���̷�����������
    {
        if(memorytable[i].flag0==0)
        {
            count++;
            break;
        }
    }
    if(count>=1)//���Է��� �ӵ͵�ַ����
    {
        memorytable[i].flag0=1;//�޸ĸ�λ״̬
        memorytable[i].counter=0;//ʹ��LRU�㷨�Ļ�����ҳ���Ӧ�ļ�����Ҫ��0
        memorytable[i].page=pagenumber;//װ�������һҳ



        //�޸�ҳ��
        for(int j=0;j<pgtlist[ptr].size();j++)
        {
            if(pagenumber==pgtlist[ptr][j].pagenumber)
            {
                 pgtlist[ptr][j].framenumber=i;//�޸Ķ�Ӧ�Ŀ��
                 break;
            }
        }
        return i;
    }
    else//���п鲻��1������ʧ��
    {
        cout<<"�ڴ治�����ʧ��"<<endl;
        return error;
    }
}

//����ִ����Ϻ��ͷ��ڴ溯��
void free()
{
    for(int i=0;i<memorytable.size();i++)
    {
        if(i==runningprocess.pf[0]||i==runningprocess.pf[1]||i==runningprocess.pf[2])
        {
            memorytable[i].flag0=0;//��ռ�õ�����ҳ���ռ�ñ�־λ���޸�λ0
        }
    }
}


//LRU
int Lru(int pagenumber)
{
    int b=runningprocess.pf[0];//��ȡ�ý���ռ�õ�����ҳ��
    int c=runningprocess.pf[1];
    int d=runningprocess.pf[2];
    vector<pageframe> pageframelist(3);
    pageframelist[0]=memorytable[b];//����Щҳ�򵥶��ó�������������
    pageframelist[1]=memorytable[c];
    pageframelist[2]=memorytable[d];
    sort(pageframelist.begin(),pageframelist.end(),CmpCounter);//���ݼ�������ֵ�Ӵ�С����
    int e=pageframelist[0].page;//Ҫ��̭��ҳ��
    if(pgtlist[ptr][e].changeflag==1)//�����ҳ��װ��ҳ�汻�޸Ĺ�
    {//д�ش���
    /*
        int f=pagetable[e].diskdress;
        disk[f].dpage=pagetable[e].                                                                                                                                                                                                                                                                                                                                                                         agenumber;
   */
    }
    else{}
    int s=pgtlist[ptr][e].framenumber;//�ҵ���Ӧ��������
    memorytable[s].page=pagenumber;
    memorytable[s].counter=0;//���滻װ��������ֵΪ0
    for(int p=0;p<3;p++)
            {
                if(runningprocess.pf[p]!=-1&&runningprocess.pf[p]!=s)
                {
                    memorytable[p].counter++;//����װ��ҳ���ҳ���Ӧ�ļ�������1
                }
            }
     ofile.open("d:\\lane.txt",ios::app);
     cout<<dec<<"ҳ��"<<pagenumber<<"�滻ҳ��"<<e<<"װ����Ϊ"<<s<<"������ҳ��"<<endl;
     ofile<<dec<<"ҳ��"<<pagenumber<<"�滻ҳ��"<<e<<"װ����Ϊ"<<s<<"������ҳ��"<<endl;
     ofile.close();
     //����������޸�ҳ�� ��ʱ�ҳ��Ӧ�����½��̵�ҳ�� ������װ����� ���������У������ǻҳ��Ҳ�����������̵�ҳ��
               //�޸�ҳ��
                pgtlist[ptr][e].flag=0;
                pgtlist[ptr][pagenumber].flag=1;
                pgtlist[ptr][pagenumber].framenumber=s;
                //�޸Ŀ��
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
    int b=runningprocess.pf[0];//��øý���ռ�õĸ�������ҳ��
    int c=runningprocess.pf[1];
    int d=runningprocess.pf[2];//Ӧ����ÿ��ҳ������һ������λ��Ϊ�˷���ֱ�Ӽ�ҳ�����ˡ���������ҳ�����õĵ��Ǵ��ˡ�����
    pagetableentry temp;//�������汣���ͷ�Ļ���ҳ��
    vector<pagetableentry> pagelist(0);
    for(i=0;i<pgtlist[ptr].size();i++)//���Ѿ�װ���ҳ�浥���ó�������pagelist���������
    {
        if(pgtlist[ptr][i].framenumber==b||pgtlist[ptr][i].framenumber==c||pgtlist[ptr][i].framenumber==d)//˳��϶�����ȷ�ģ�һ����FIFOҳ����У���Ϊ�ӵ͵�ַ����
        {
            pagelist.push_back(pgtlist[ptr][i]);
        }

    }
    //�ȼ���ͷ
    while(pagelist[0].r!=0)
    {
        temp=pagelist[0];//��ʱ�����ͷ
        pagelist.erase(pagelist.begin());//ɾ����ͷ
        temp.r=0;//����λ��Ϊ0��������0
        pagelist.push_back(temp);//����ͷ���²����β
    }
    //��ͷ����λ��Ϊ1������̭
    int e=pagelist[0].pagenumber;//Ҫ��̭��ҳ��
    if(pgtlist[ptr][e].changeflag==1)//�����ҳ��װ��ҳ�汻�޸Ĺ�
    {//д�ش���
    /*
        int f=pagetable[e].diskdress;
        disk[f].dpage=pagetable[e].                                                                                                                                                                                                                                                                                                                                                                         agenumber;
   */
    }
    else{}
    int s=pgtlist[ptr][e].framenumber;//�ҵ���Ӧ��������
    memorytable[s].page=pagenumber;
    //�޸����ҳ����ҳ���������λ
    pgtlist[ptr][pagenumber].r=1;//��װ���ҳ������λΪ1
    //�������������ҳ�������λ��ҳ�����е�ͳһ
    b=pagelist[1].pagenumber;
    pgtlist[ptr][b].r=1;pagelist[1].r;
    c=pagelist[2].pagenumber;
    pgtlist[ptr][c].r=1;pagelist[2].r;
    ofile.open("d:\\lane.txt",ios::app);
    cout<<"ҳ��"<<dec<<pagenumber<<"�滻ҳ��"<<e<<"װ����Ϊ"<<s<<"������ҳ��"<<endl;
    ofile<<"ҳ��"<<dec<<pagenumber<<"�滻ҳ��"<<e<<"װ����Ϊ"<<s<<"������ҳ��"<<endl;
    ofile.close();
     //����������޸�ҳ�� ��ʱ�ҳ��Ӧ�����½��̵�ҳ�� ������װ����� ���������У������ǻҳ��Ҳ�����������̵�ҳ��
               //�޸�ҳ��Ϳ��
                pgtlist[ptr][e].flag=0;
                pgtlist[ptr][pagenumber].flag=1;
                pgtlist[ptr][pagenumber].framenumber=s;
                //�޸Ŀ��
                //����Ұѿ��Ĵ�С����Ϊ8��������������ֻ��3��������������û���Ļ��������µĿ�����ôԭ������Ӧ�������ʧЧ ����û��д
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
    int b=runningprocess.pf[0];//��øý���ռ�õĸ�������ҳ��
    int c=runningprocess.pf[1];
    int d=runningprocess.pf[2];
    int b1=memorytable[b].page;
    int c1=memorytable[c].page;
    int d1=memorytable[d].page;
    for(i=0;i<runningprocess.inss+1;i++)//�ҵ�����ҳ���Ӧ��ָ���ڸý��̵�ָ�������е�һ�γ��ֵ�λ��
    {
         if(runningprocess.instructorder[i]==b1)
         {
              b2=i;
             break;//�ڴ˴�break�����Ի�ø�ҳ��ţ�ָ��ţ���һ�γ��ֵ�λ��
         }

    }
    for(i=0;i<runningprocess.inss+1;i++)
    {
         if(runningprocess.instructorder[i]==c1)
         {
             c2=i;
             break;//�ڴ˴�break�����Ի�ø�ҳ��ţ�ָ��ţ���һ�γ��ֵ�λ��
         }

    }
    for( i=0;i<runningprocess.inss+1;i++)
    {
         if(runningprocess.instructorder[i]==d1)
         {
              d2=i;
             break;//�ڴ˴�break�����Ի�ø�ҳ��ţ�ָ��ţ���һ�γ��ֵ�λ��
         }

    }
    if(b2>=c2)//����Щλ�õ���Сֵ��������ִ�е�ָ�����װ���ڴ��ҳ�棩
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
    int e=runningprocess.instructorder[min];//Ҫ��̭��ҳ��(ҳ�ź�ָ�����ͬ)
    if(pgtlist[ptr][e].changeflag==1)//�����ҳ��װ��ҳ�汻�޸Ĺ�
    {//д�ش���
    /*
        int f=pagetable[e].diskdress;
        disk[f].dpage=pagetable[e].                                                                                                                                                                                                                                                                                                                                                                         agenumber;
   */
    }
    else{}
    int s=pgtlist[ptr][e].framenumber;//�ҵ���Ӧ��������
    memorytable[s].page=pagenumber;
    ofile.open("d:\\lane.txt",ios::app);
    cout<<"ҳ��"<<dec<<pagenumber<<"�滻ҳ��"<<e<<"װ����Ϊ"<<s<<"������ҳ��"<<endl;
    ofile<<"ҳ��"<<dec<<pagenumber<<"�滻ҳ��"<<e<<"װ����Ϊ"<<s<<"������ҳ��"<<endl;
    ofile.close();
     //����������޸�ҳ�� ��ʱ�ҳ��Ӧ�����½��̵�ҳ�� ������װ����� ���������У������ǻҳ��Ҳ�����������̵�ҳ��
               //�޸�ҳ��
                pgtlist[ptr][e].flag=0;
                pgtlist[ptr][pagenumber].flag=1;
                pgtlist[ptr][pagenumber].framenumber=s;
                //�޸Ŀ��
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

    while(pgtlist[ptr][(*it).page].r!=0)//����ѭ�����У�ֱ���ҵ�����λΪ0��
                    {
                        pgtlist[ptr][(*it).page].r=0;//����λ��Ϊ0
                         if((*it).page==memorytable[2].page)//���ָ��ָ���β
                    {
                        it=it-3+1;//�޸�ָ�룬ָ���ͷ��ʵ��ѭ�� 3Ϊ������������
                    }
                    else
                    {
                        it++;//ָ�������һλ
                    }
                    }

    //��ͷ����λ��Ϊ1������̭
    int e=(*it).page;//Ҫ��̭��ҳ��
    if(pgtlist[ptr][e].changeflag==1)//�����ҳ��װ��ҳ�汻�޸Ĺ�
    {//д�ش���
    /*
        int f=pagetable[e].diskdress;
        disk[f].dpage=pagetable[e].                                                                                                                                                                                                                                                                                                                                                                         agenumber;
    */
    }
    else{}
    int s=pgtlist[ptr][e].framenumber;//�ҵ���Ӧ��������
    memorytable[s].page=pagenumber;
    //�޸����ҳ����ҳ���������λ
    pgtlist[ptr][pagenumber].r=1;//��װ���ҳ������λΪ1
    ofile.open("d:\\lane.txt",ios::app);
    cout<<"ҳ��"<<dec<<pagenumber<<"�滻ҳ��"<<e<<"װ����Ϊ"<<s<<"������ҳ��"<<endl;
    ofile<<"ҳ��"<<dec<<pagenumber<<"�滻ҳ��"<<e<<"װ����Ϊ"<<s<<"������ҳ��"<<endl;
    ofile.close();
     //����������޸�ҳ�� ��ʱ�ҳ��Ӧ�����½��̵�ҳ�� ������װ����� ���������У������ǻҳ��Ҳ�����������̵�ҳ��
               //�޸�ҳ��Ϳ��
                pgtlist[ptr][e].flag=0;
                pgtlist[ptr][pagenumber].flag=1;
                pgtlist[ptr][pagenumber].framenumber=s;
                //�޸Ŀ��
                //����Ұѿ��Ĵ�С����Ϊ8��������������ֻ��3��������������û���Ļ��������µĿ�����ôԭ������Ӧ�������ʧЧ ����û��д
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

//ȱҳ�쳣����
int pagehandling(int pagenumber,Ui::Dialog *dis)//
{
    //���������ҳ�Ľ���
    QString tempStr;
    runningprocess.state=block;
    WaitQueue.push_back(runningprocess);//���̵ȴ� ����ȴ�==����
    DISKFLAG=1;
    int framenumber;
    //���ݵ��Ȳ��Ե��Ƚ���
    //��ʱ����cpu
    //����ҳ��������Ӧ������ַ  ����Ӧ�����ʱ��- -����Ӵ��̵���ĳҳ���ݵ��ڴ�Ϊt(ns)
    ofile.open("d:\\lane.txt",ios::app);
    for(int i=0;i<pgtlist[ptr].size();i++)
    {
        if(pagenumber==pgtlist[ptr][i].pagenumber)
        {
            int diskaddress=pgtlist[ptr][i].diskaddress;//��ȡ�����ַ
            cout<<"��ø�ҳ�ڸ���ĵ�ַ"<<diskaddress<<endl;
            ofile<<"��ø�ҳ�ڸ���ĵ�ַ"<<diskaddress<<endl;
            //�鿴�ڴ������޿���ҳ���������һ�������ݹ̶�����ֲ��û����Է���(���ҳ��ָ�ӷ���Ĺ̶����������ȡ������̶����������������������һ���յĳ���)
            ofile.close();
            if(runningprocess.framecount<3)
            {
                int l=fenpei(3,pagenumber);//���������ڴ�
                if(l!=error)//�������ɹ�
                {
                    ofile.open("d:\\lane.txt",ios::app);
                    cout<<dec<<"�����������Ϊ"<<l<<endl;
                    ofile<<dec<<"�����������Ϊ"<<l<<endl;
                    it=memorytable.begin();//ָ��ָ���ڴ�Ŀ�ʼ����ʵÿ������һ��ʼ�ֶ��Ǵ�0���ڴ�鿪ʼռ�õ�
                    ofile.close();
                    framenumber=l;
                    runningprocess.framecount=runningprocess.framecount+1;//aռ�õ�ҳ��������
                   for(int k=0;k<3;k++)
                   {
                    if(runningprocess.pf[k]==-1)
                    {
                    runningprocess.pf[k]=framenumber;//�޸Ľ�������Ϣ
                    break;
                    }

                   }

                }
                ofile.open("d:\\lane.txt",ios::app);
                cout<<dec<<"�ý����Ѿ�ռ�õ�����ҳ����"<<runningprocess.framecount;
                ofile<<dec<<"�ý����Ѿ�ռ�õ�����ҳ����"<<runningprocess.framecount;
                ofile.close();
                //д��ҳ��
                pgtlist[ptr][i].framenumber=l;
                //д����
                if(tlb.size()==3)
                  {//�������ȳ���̭ ��ʵ �������sizeΪ3�Ļ��϶���ҳ���û�������ִ�е�����
                  tlb[0].pagenumber=pagenumber;
                  tlb[0].framenumber=framenumber;
                  }
                  else
                  {//�����µĿ����
                      tlbentry newentry;
                      newentry.pagenumber=pagenumber;
                      newentry.framenumber=framenumber;
                      tlb.push_back(newentry);
                  }
                return framenumber;
            }
            else
            {
                //�����û��㷨ѡ��ҳ���滻
                tempStr=dis->comboBox_2->currentText();
                if(tempStr=="SCR")
                    {
                      framenumber=Scr(pagenumber);//SCR�����㷨
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
                   //������Ϻ󷵻ص�ԭ���̷����жϵĵط�ִ��
                  ofile.open("d:\\lane.txt",ios::app);
                  cout<<"�����������Ϊ"<<framenumber<<endl;
                  ofile<<"�����������Ϊ"<<framenumber<<endl;
                  ofile.close();
                return framenumber;
                running(n);//������һ��ָ��
                return framenumber;
            }
        }
    }
}

//��տ�� ��ʵӦ����mmu�Ĺ���
void cleartlb()
{
    tlb.resize(0);//���ÿ���С
}

//mmu����ģ��
int  mmu(int logicaladdress,int instructnum,Ui::Dialog *dis)//mmu
{
    int i,f;
    QString tempStr;
    //����ҳ���ַ���ҳ�� �߼���ַ17λ ҳ��5λ ƫ�Ƶ�ַ12λ
    ofile.open("d:\\lane.txt",ios::app);
    cout<<endl<<"�߼���ַ:"<<hex<<logicaladdress<<"h"<<endl;//hex��16�������
    ofile<<endl<<"�߼���ַ:"<<hex<<logicaladdress<<"h"<<endl;//hex��16�������
    int pagenumber=logicaladdress>>11;//���ҳ��
    cout<<dec<<"ҳ��:"<<pagenumber<<endl;
    ofile<<dec<<"ҳ��:"<<pagenumber<<endl;
    int offsetaddress=logicaladdress&0x07ff;//���ƫ�Ƶ�ַ
    cout<<"ƫ�Ƶ�ַ:"<<hex<<offsetaddress<<"h"<<endl;
    ofile<<"ƫ�Ƶ�ַ:"<<hex<<offsetaddress<<"h"<<endl;
    ofile.close();
    int physicaladdress;//ת����������ַ
    int framenumber;//ҳ���
    if(pagenumber>runningprocess.pagetablelength-1)//���ҳ�Ŵ���ҳ����-1
    {
        return error;//����Խ���ж�
    }

    else{
    //int position=pagetableaddress+pagenumber*pagetableentrylen;//��ҳ��ʼַ��ҳ�ź�ҳ����ȵĳ˻���ӣ��õ���ҳ������ҳ���е�λ��
    for( i=0;i<tlb.size();i++)//������� ��������û�м�¼ �ǿ϶����ڿ����( tlb.size==0)
    {
        if(pagenumber==tlb[i].pagenumber)//����
        {
            ofile.open("d:\\lane.txt",ios::app);
            framenumber=tlb[i].framenumber;
            physicaladdress=(framenumber<<12)+offsetaddress;//ƴ�������ַ
            cout<<"���п��"<<endl;
            ofile<<"���п��"<<endl;
            cout<<"�ý����Ѿ�ռ�õ�����ҳ����"<<runningprocess.framecount<<endl;
            ofile<<"�ý����Ѿ�ռ�õ�����ҳ����"<<runningprocess.framecount<<endl;
            cout<<hex<<"�����ַΪ"<<physicaladdress<<"h"<<endl;
            ofile<<hex<<"�����ַΪ"<<physicaladdress<<"h"<<endl;
            ofile.close();
            //����Ȩ�޼��  δ�豣��λ ��ʱ��д
            //��ʵ������β�Ӧ��д���������
            memorytable[framenumber].counter=0;//LRU �㷨���õ��ļ�����
            for(int p=0;p<3;p++)
            {
                if(runningprocess.pf[p]!=-1&&runningprocess.pf[p]!=framenumber)
                {
                    memorytable[p].counter++;//����װ��ҳ���ҳ���Ӧ�ļ�������1
                }
            }
            pgtlist[ptr][pagenumber].r=1;//���У�����λ�޸�Ϊ1
            memorytable[framenumber].page=pagenumber;//װ���ҳ���ҳ��
            return physicaladdress;
        }
    }
    //���δ����
        for(i=0;i<pgtlist[ptr].size();i++)//��������ҳ�� �������ҳ��sizeΪ0 һ��������
        {
            if(pagenumber==pgtlist[ptr][i].pagenumber)//����  ����Ӧ���Ƕ����е� ����˵�������˵�����̴������ڴ���û����⡣����
            {

                if(pgtlist[ptr][i].flag==1)//�����ҳ��������
                {
                  ofile.open("d:\\lane.txt",ios::app);
                  framenumber=pgtlist[ptr][i].framenumber;
                  physicaladdress=(framenumber<<12)+offsetaddress;//ƴ�������ַ
                  cout<<"����ҳ��"<<endl;
                  ofile<<"����ҳ��"<<endl;
                  cout<<"�ý����Ѿ�ռ�õ�����ҳ����"<<runningprocess.framecount<<endl;
                  ofile<<"�ý����Ѿ�ռ�õ�����ҳ����"<<runningprocess.framecount<<endl;
                  cout<<hex<<"�����ַΪ"<<physicaladdress<<"h"<<endl;
                  ofile<<hex<<"�����ַΪ"<<physicaladdress<<"h"<<endl;
                  ofile.close();
                  memorytable[framenumber].counter=0;
                  for(int p=0;p<3;p++)
                  {
                     if(runningprocess.pf[p]!=-1&&runningprocess.pf[p]!=framenumber)
                     {
                       memorytable[p].counter++;//����װ��ҳ���ҳ���Ӧ�ļ�������1
                     }
                  }
                  memorytable[framenumber].page=pagenumber;
                  pgtlist[ptr][pagenumber].r=1;//���У�����λ�޸�Ϊ1
                  //memorytable[framenumber].flag0=1;����������ռ��
                  //д����
                  /*�����������
                  1 ���������ˣ�д�µı�����Ӧ����̭�ĸ��ɵı��� ����˵��򵥵��������ȳ�
                  2 ��������ҳ������ܲ���ͳһ����������ֱ������ˣ���������������Ҫ����µĳ�Ա�����ͳ���*/
                  if(tlb.size()==3)
                  {
                  tlb[0].pagenumber=pagenumber;
                  tlb[0].framenumber=framenumber;//�������������������ȳ��ķ�ʽ����̭����д����Ŀ��������
                  }
                  else
                  {//�����µĿ����
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


        //����ҳ��δ����
        //ȱҳ
                   ofile.open("d:\\lane.txt",ios::app);
                   cout<<"ȱҳ"<<endl;
                   ofile<<"ȱҳ"<<endl;
                   ofile.close();
                   runningprocess.instructnum=instructnum;
                   framenumber=pagehandling(pagenumber,dis);//ȱҳ�쳣����
                   system_time=system_time+1;
                   Writedown2();
                   dis->textEdit->setText(tempStr.setNum(system_time));
                   for(f=0;f<15;f++)//������
                   {

                       if(memorytable[f].flag0==1)
                       {
                          dis->tableWidget->setItem(f,1,new QTableWidgetItem(tempStr.fromLocal8Bit("ռ��")));
                          dis->tableWidget->setItem(f,2,new QTableWidgetItem(tempStr.setNum(memorytable[f].page)));
                       }
                       else
                       {
                           dis->tableWidget->setItem(f,1,new QTableWidgetItem(tempStr.fromLocal8Bit("����")));
                           dis->tableWidget->setItem(f,2,new QTableWidgetItem(tempStr.fromLocal8Bit("��")));
                       }
                  }
                    Sleep(1000);//����1s
                    qApp->processEvents();//����qApp->processEvents()��ǿ��ˢ��
                    for(int g=0;g<n;g++)//����ֱ�ӵ���JudgeIn(n)���������
                    {
                        if(pcblist[g].intime==system_time)
                        ReadyQueue.push_back(pcblist[g]);
                    }
                   ofile.open("d:\\lane.txt",ios::app);
                   cout<<endl<<"**��ǰϵͳʱ��**:"<<system_time<<endl;
                   ofile<<endl<<"**��ǰϵͳʱ��**:"<<system_time<<endl;
                   cout<<"���ȱҳ����"<<endl;
                   ofile<<"���ȱҳ����"<<endl;
                   cout<<endl;
                   ofile<<endl;
                   physicaladdress=(framenumber<<12)+offsetaddress;//ƴ�������ַ
                   cout<<"�����ַΪ"<<hex<<physicaladdress<<"h"<<endl;
                   ofile<<"�����ַΪ"<<hex<<physicaladdress<<"h"<<endl;
                   cout<<endl<<"����ȱҳ�ж�ʱ�Ķ�����Ϣ����:"<<endl;
                   ofile<<endl<<"����ȱҳ�ж�ʱ�Ķ�����Ϣ����:"<<endl;
                   ofile.close();
                   PrintQueue();
                   WaitQueue.erase(WaitQueue.begin());//�ڽ����뿪�ȴ�����֮ǰ��ӡ������Ϣ
                   return physicaladdress;


        }//else{}
}

//�����ȳ������㷨
void sche_fifo(Ui::Dialog *dis)
{

       int i;
       QString tempStr;
        while(ReadyQueue.size()==0)//�������������û�н���
        {
        system_time=system_time+1;//�ȴ��½��̵���
        dis->textEdit->setText(tempStr.setNum(system_time));
        for(i=0;i<15;i++)//��ʾ�ڴ�ֿ�����
        {

            if(memorytable[i].flag0==1)
            {
               dis->tableWidget->setItem(i,1,new QTableWidgetItem(tempStr.fromLocal8Bit("ռ��")));
               dis->tableWidget->setItem(i,2,new QTableWidgetItem(tempStr.setNum(memorytable[i].page)));
            }
            else
            {
                dis->tableWidget->setItem(i,1,new QTableWidgetItem(tempStr.fromLocal8Bit("����")));
                dis->tableWidget->setItem(i,2,new QTableWidgetItem(tempStr.fromLocal8Bit("��")));
            }
       }
        Sleep(1000);//����1s
        qApp->processEvents();//����qApp->processEvents()��ǿ��ˢ��
        cout<<"��ǰϵͳʱ��"<<system_time;

       for(int i=0;i<n;i++)//����ֱ�ӵ���JudgeIn(n)���������
       {
           if(pcblist[i].intime==system_time)
           ReadyQueue.push_back(pcblist[i]);
       }

        }
    ReadyQueue[0].state=runningg;//ѡȡ���Ƚ���������еĽ�������
    runningprocess=ReadyQueue[0];//����cpu����
    ReadyQueue.erase(ReadyQueue.begin());//�Ӿ��������Ƴ�

}


//���ȼ������㷨
void sche_pr(Ui::Dialog *dis)//���ȼ���������ȼ���
{

        int i;
           QString tempStr;
            while(ReadyQueue.size()==0)
            {
            system_time=system_time+1;
            dis->textEdit->setText(tempStr.setNum(system_time));
            dis->textEdit_3->setText(tempStr.setNum(runningprocess.instructnum));
            for(i=0;i<15;i++)//��ʾ�ڴ�ֿ��
            {

                if(memorytable[i].flag0==1)
                {
                    dis->tableWidget->setItem(i,1,new QTableWidgetItem(tempStr.fromLocal8Bit("ռ��")));
                   dis->tableWidget->setItem(i,2,new QTableWidgetItem(tempStr.setNum(memorytable[i].page)));
                }
                else
                {
                    dis->tableWidget->setItem(i,1,new QTableWidgetItem(tempStr.fromLocal8Bit("����")));
                    dis->tableWidget->setItem(i,2,new QTableWidgetItem(tempStr.fromLocal8Bit("��")));
                }
           }
            Sleep(1000);;
            qApp->processEvents();//����qApp->processEvents()��ǿ��ˢ��
            cout<<"��ǰϵͳʱ��"<<system_time;

           for(int i=0;i<n;i++)//����ֱ�ӵ���JudgeIn(n)���������
           {
               if(pcblist[i].intime==system_time)//������Ľ��̷������������
               ReadyQueue.push_back(pcblist[i]);
           }

            }
        sort(ReadyQueue.begin(),ReadyQueue.end(),CmpPriority);//���ȼ�����
        ReadyQueue[0].state=runningg;//ѡȡ���Ƚ���������еĽ�������
        runningprocess=ReadyQueue[0];//����cpu����
        ReadyQueue.erase(ReadyQueue.begin());//�Ӿ��������Ƴ�

}



void Dialog::on_pushButton_clicked()
{
        pcblist.resize(0);//ÿ��ʹ��ǰ��ս����б�
        EndQueue.resize(0);//�����ɶ���
        ReadyQueue.resize(0);//��վ�������
        WaitQueue.resize(0);//��յȴ�����
        bool ok;
        int i;
        QString tempStr;
        QString valueStr=ui->lineEdit->text();//���lineEdit��text��ֵ�������̸�������������һ��QString����Ҫת��Ϊint���Ͳ��ܸ�ֵ��n
        n=valueStr.toInt(&ok);//ת��Ϊint��ֵ��n
        Writedown();//����writedown���� ���б�������� Ҳ���Դ����ļ�
        createprocess();//���ɽ���
        sort(pcblist.begin(),pcblist.end(),CmpInTime);//��������ÿ�����̵Ľ���ʱ�䰴��С��������
        system_time=pcblist[0].intime;//��ϵͳʱ���ʼ��Ϊ����ʱ��������Ǹ����̵Ľ���ʱ��(�����ϵͳ��ǰʱ��)
        ui->textEdit->setText(tempStr.setNum(system_time));
        Sleep(1000);//Ϊ����ʱ��ı仯ֱ��һ�㣬����1s
        ReadyQueue.push_back(pcblist[0]);
        for( i=1;i<n;i++)//����е���ʱ����ͬ����� Ҳ�������������� �ӽ��̶��еڶ�����ʼ���
           {
             if(pcblist[i].intime==system_time)
               {
                    ReadyQueue.push_back(pcblist[i]);
               }
           }
        //���������ͬʱ��Ľ��̵����ȼ������ȼ��ߵ�������
        sort(ReadyQueue.begin(),ReadyQueue.end(),CmpPriority);//���ȼ�����
        runningprocess=ReadyQueue[0];//�����絽�����ҵ����cpu����
        ReadyQueue.erase(ReadyQueue.begin());//�Ӿ��������Ƴ�
        QTableWidget *tableWidget = new QTableWidget(n,4);//����һ����Ϊn�����̸�������Ϊ4��һЩҪ��ʾ�����ԣ��ı��
        QString str,str1,str2,str3;//QTֱ����ʾ���Ļ����룬ʹ��QString��fromLocal8Bit()����ת��
        str = str.fromLocal8Bit("������Ϣ��");
        tableWidget->setWindowTitle(str); //�޸ı��������������ʾ
        QStringList header;
        str1= str1.fromLocal8Bit("���ȼ�");
        str2= str2.fromLocal8Bit("����ʱ��");
        str3= str3.fromLocal8Bit("ָ����Ŀ");
        header<<"ID"<<str1<<str2<<str3;
        tableWidget->setHorizontalHeaderLabels(header);//��ʾ������
        for(int i=0;i<n;i++)
        {

            tableWidget->setItem(i,0,new QTableWidgetItem(tempStr.setNum(pcblist[i].pid)));
            tableWidget->setItem(i,1,new QTableWidgetItem(tempStr.setNum(pcblist[i].priority)));
            tableWidget->setItem(i,2,new QTableWidgetItem(tempStr.setNum(pcblist[i].intime)));
            tableWidget->setItem(i,3,new QTableWidgetItem(tempStr.setNum(pcblist[i].instructcount)));

        }
        tableWidget->show();//��ʾ���
}


void Dialog::on_pushButton_2_clicked()
{
    int f;
    QString tempStr;
    while(EndQueue.size()<n)
    {//����û��ȫ��ִ�����
        //����ý���ָ���ȫ�ڴ���
        ui->textEdit_4->setText("  ");
       ofile.open("d:\\lane.txt",ios::app);
       cleartlb();//��տ�����
       ptr=runningprocess.pagetableaddress;//�������н��̵�ҳ���ַ����ҳ���ַ�Ĵ���
       cout<<endl<<endl<<dec<<"��ǰϵͳʱ��:"<<system_time<<endl;
       ofile<<endl<<endl<<dec<<"��ǰϵͳʱ��:"<<system_time<<endl;
       ofile.close();
       ofile.open("d:\\lane.txt",ios::app);
       cout<<"��ǰ���еĽ���Ϊ:"<<dec<<runningprocess.pid<<endl;
       cout<<"��ǰ�������е�ָ������Ϊ:"<<endl;
       ofile<<"��ǰ���еĽ���Ϊ:"<<dec<<runningprocess.pid<<endl;
       ofile<<"��ǰ�������е�ָ������Ϊ:"<<endl;

       for(int i=0;i<runningprocess.instructorder.size();i++)
       {
           cout<<runningprocess.instructorder[i]<<" ";
           ofile<<runningprocess.instructorder[i]<<" ";
       }
       ofile.close();
       int j=runningprocess.instructnum;
       for(f=0;f<runningprocess.instructorder.size();f++)//��textEdit�ؼ�������ʾ��ǰ���н��̵�ָ������
          {
              ui->textEdit_4->insertPlainText(tempStr.setNum(runningprocess.instructorder[f])+"   ");
              ui->textEdit_4->moveCursor(QTextCursor::End);//������ƶ���ĩβ

          }
       while(j<runningprocess.instructorder.size())//С��ָ�����г��ȶ�����ָ�����
       {

         ofile.open("d:\\lane.txt",ios::app);
         cout<<endl<<dec<<"**��ǰϵͳʱ��**:"<<system_time<<endl;
         ofile<<endl<<dec<<"**��ǰϵͳʱ��**:"<<system_time<<endl;
         cout<<endl<<dec<<"**************��ǰִ�е�"<<runningprocess.instructorder[j]<<"��ָ��**************"<<endl;
         ofile<<endl<<dec<<"**************��ǰִ�е�"<<runningprocess.instructorder[j]<<"��ָ��**************"<<endl;
         runningprocess.instructnum=runningprocess.instructorder[j];
         runningprocess.inss=j;
         ofile.close();
         mmu(runningprocess.instructaddress,runningprocess.instructorder[j],ui);//����mmu
         ofile.open("d:\\lane.txt",ios::app);//��׷�ӵķ�ʽд��
         cout<<endl;
         ofile<<endl;
         ofile.close();
         j++;
         runningprocess.instructaddress=2048*(runningprocess.instructorder[j]);//+2^12
         system_time=system_time+1;
         ui->textEdit->setText(tempStr.setNum(system_time));
         for(f=0;f<15;f++)//��ʾ�ڴ�ֿ��Ĳ���
         {

             if(memorytable[f].flag0==1)
             {
                 ui->tableWidget->setItem(f,1,new QTableWidgetItem(tempStr.fromLocal8Bit("ռ��")));
                 ui->tableWidget->setItem(f,2,new QTableWidgetItem(tempStr.setNum(memorytable[f].page)));
             }
             else
             {
                 ui->tableWidget->setItem(f,1,new QTableWidgetItem(tempStr.fromLocal8Bit("����")));
                 ui->tableWidget->setItem(f,2,new QTableWidgetItem(tempStr.fromLocal8Bit("��")));
             }
        }
         ui->textEdit_2->setText(tempStr.setNum(runningprocess.pid));
         ui->textEdit_3->setText(tempStr.setNum(runningprocess.instructnum));

         Sleep(1000);
         JudgeIn(n);
         ofile.open("d:\\lane.txt",ios::app);
         qApp->processEvents();
         ofile.close();
         Writedown2();//д**********
         PrintQueue();//д**********

       }
       ofile.open("d:\\lane.txt",ios::app);
       cout<<endl<<"�ý����������"<<endl<<endl;
       ofile<<endl<<"�ý����������"<<endl<<endl;
       ofile.close();
       runningprocess.state=dead;
       EndQueue.push_back(runningprocess);
       free();//�黹�ڴ�
       tempStr=ui->comboBox->currentText();
       if(EndQueue.size()<n)
       {
           if(tempStr=="Priority")
           {
             sche_pr(ui);//�ǰ������ȼ�����
           }
           else if(tempStr=="FIFO")
           {
               sche_fifo(ui);//�����ȳ�����
           }
       }
   }
       ofile.open("d:\\lane.txt",ios::app);
       cout<<endl<<"����ȫ���������";
       ofile<<endl<<"����ȫ���������";
       ofile.close();
       PrintQueue();//д**********��ӡ������Ϣ
}





void Dialog::on_pushButton_3_clicked()//���������ļ�d:\\exslane
{
        //QProcess��:����һ���ⲿ�ĳ�����֮����
        QProcess* process = new QProcess();
        QString notepadPath = "notepad.exe d:\\exslane.txt";//notepad�������ϻ������еļ��±�
        process->start(notepadPath);

}

void Dialog::on_pushButton_4_clicked()//���ڴ�����ļ�
{
    //QProcess��:����һ���ⲿ�ĳ�����֮����
    QProcess* process = new QProcess();
    QString notepadPath = "notepad.exe d:\\stlane.txt";//notepad�������ϻ������еļ��±�
    process->start(notepadPath);

}


void Dialog::on_pushButton_5_clicked()//����������Ϣ�ļ�
{
    //QProcess��:����һ���ⲿ�ĳ�����֮����
    QProcess* process = new QProcess();
    QString notepadPath = "notepad.exe d:\\lane.txt";//notepad�������ϻ������еļ��±�
    process->start(notepadPath);
}
