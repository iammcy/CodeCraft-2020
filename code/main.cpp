#include <bits/stdc++.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

int path3[3*500000];
int path4[4*500000];
int path5[5*1000000];
int path6[6*2000000];
int path7[7*3000000];

int *path[]={path3,path4,path5,path6,path7};                   //存储不同长度路径的数组地址
int ps[5];                                                     //存储不同长度路径的数量

char a[25];
size_t idl,inputSize;
int pathCount,maxID=-1;
string idsComma[210000];
string idsLF[210000];
int idList[210000];
int input[560000];                                          //存储输入信息
int mapNode[210001];                                        //记录id映射信息
int graph[210000][30];                                      //有向图的邻接矩阵
int regraph[210000][30];                                    //有向图的反向邻接矩阵
int vis[210000];                                            //记录id访问信息
int indegree[210000];                                       //记录节点入度
int outdegree[210000];                                      //记录节点出度
unordered_map<int,vector<int> > depthTwo[210000];           //记录深度为2的路径信息
int startIndex[210000];                                     //倒数第二层节点下一次遍历开始的节点下标



class Circular{
public:
    void dfs(int start,int temp,int p[],int len){
        int index = upper_bound(graph[temp],graph[temp]+outdegree[temp],start)-graph[temp];
        size_t gl = outdegree[temp];
        for(size_t i=index;i<gl;i++){
            int t = graph[temp][i];
            if(vis[t]==0){
                if(startIndex[t]>=0){
                    p[len+1]=t;
                    auto &mv = depthTwo[start][t];
                    //int num = mv.size();
                    for(int j=0;j<mv.size();j++){
                        int last = mv[j];
                        if(vis[last]==0)
                        {
                            p[len+2]=last;
                            memcpy(path[len]+ps[len]*(len+3),p,(len+3)*sizeof(int));
                            pathCount++;
                            ps[len]++;
                        }
                    }
                }
                if(len<4){
                    p[len+1]=t;
                    vis[t]=1;
                    dfs(start,t,p,len+1);
                    vis[t]=0;                      //回滚
                }
            }
        }
    }

    void storeDepthTwo(){
        for(size_t i=0;i<idl;i++){
            int j=0;
            for(auto &mnode:graph[i]){
                if(j==outdegree[i])
                    break;
                auto &ev = graph[mnode];
                int k=0;
                for(auto &enode:ev){
                    if(k==outdegree[mnode]||enode>=i||enode>=mnode)break;
                    depthTwo[enode][i].push_back(mnode);
                    k++;
                }
                j++;
            }
        }
    }

    void topologicalSort(){
        int q[260000],q2[260000],hh=-1,tt=-1,tt2=-1;
        int in[260000],out[260000];
        memcpy(in,indegree,idl*sizeof(int));
        memcpy(out,outdegree,idl*sizeof(int));
        for(int i=0;i<idl;i++){
            if(in[i]==0)
                q[++tt]=i;
            if(out[i]==0)
                q2[++tt2]=i;
        }
        while(hh<tt){
            int t = q[++hh];
            //int out_size=outdegree[t];
            for(int i=0;i<outdegree[t];i++)
            {
                int node = graph[t][i];
                --in[node];
                if(in[node]==0)
                    q[++tt]=node;
            }
        }
        hh=-1;
        while(hh<tt2){
            int t = q2[++hh];
            int in_size=indegree[t];
            for(int i=0;i<in_size;i++)
            {
                int node = regraph[t][i];
                --out[node];
                if(out[node]==0)
                    q2[++tt2]=node;
            }
        }
        for(int i=0;i<idl;i++)
            if(in[i]==0||out[i]==0){
                outdegree[i]=0;
            }
            else{
                sort(graph[i],graph[i]+outdegree[i]);
            }
        return;
    }

    void Input(string& inputFile){
        int fd = open(inputFile.c_str(), O_RDONLY);
        int len = lseek(fd, 0, SEEK_END);
        char *buf = (char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
        char *buf_end = buf+len;
        while(buf<buf_end){
            int from=0,to=0;
            while(*buf!=','){
                from = from*10+(*buf-'0');
                ++buf;
            }
            input[inputSize++]=from;
            maxID=from>maxID?from:maxID;
            mapNode[from]++;
            ++buf;
            while(*buf!=','){
                to = to*10+(*buf-'0');
                ++buf;
            }
            input[inputSize++]=to;
            maxID=to>maxID?to:maxID;
            mapNode[to]++;
            ++buf;
            while(*buf!='\n'&&buf!=buf_end)
                ++buf;
            ++buf;
        }
        return;
    }
    void createGraph(){
        int from,to;
        //生成id重映射
        for(int i=0;i<=maxID;i++){
            if(mapNode[i]){
                mapNode[i]=idl;
                idList[idl]=i;
                idsComma[idl]=to_string(i)+',';
                idsLF[idl++]=to_string(i)+'\n';
            }
        }


        //构造有向图
        for(size_t i=0;i<inputSize;i+=2){
            from = mapNode[input[i]];
            to = mapNode[input[i+1]];
            graph[from][outdegree[from]++]=to;
            regraph[to][indegree[to]++]=from;
        }
    }

    void work(){
        memset(startIndex,-1,sizeof(startIndex));
        int p[7],temp[260000],top=-1;
        for(size_t i=0;i<idl;i++){
            if(idList[i]>50000)break;
            if(outdegree[i]>0){
                p[0]=i;
                vis[i]=1;
                for(auto &m:depthTwo[i]){
                    int node = m.first;
                    startIndex[node]=0;
                    temp[++top]=node;
                }
                dfs(i,i,p,0);
                while(top!=-1)
                    startIndex[temp[top--]]=-1;
                vis[i]=0;       //回滚
            }
        }
    }

    void mmap_save(string& answerFile){
        size_t realsize = 0;
        size_t filesize = 140000000;
        int fd = open(answerFile.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
        lseek(fd, filesize - 1, SEEK_SET);
        write(fd, " ", 1);
        char* ans = (char*)mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        sprintf(a,"%d",pathCount);
        int len = strlen(a);
        memcpy(ans,a,len*sizeof(char));
        ans+=len;
        *(ans++)='\n';
        realsize+=len+1;
        for(int i=0;i<5;i++) {
            auto &pi = path[i];
            for (int k=0;k<ps[i];k++) {
                int sz = i+3;
                for(int j=0;j<sz-1;j++){
                    auto res=idsComma[pi[k*sz+j]];
                    memcpy(ans,res.c_str(),res.size()*sizeof(char));
                    ans+=res.size();
                    realsize+=res.size();
                }
                auto res=idsLF[pi[(k+1)*sz-1]];
                memcpy(ans,res.c_str(),res.size()*sizeof(char));
                ans+=res.size();
                realsize+=res.size();
            }
        }
        ftruncate(fd,realsize);
        munmap(ans, filesize);
        close(fd);
    }
};

string check_ans(const string& answer, const string& result) {
  ifstream file1(answer.c_str());
  ifstream file2(result.c_str());
  int flag = 1;
  string str1,str2;
  while (getline(file1,str1)) {
    getline(file2,str2);
    if (str1 != str2) {
      cout<<str1<<endl<<str2;
      flag = 0;
      break;
    }
  }
  file1.close();
  file2.close();
  if(flag){
    return "accept";
  }
  else{
    return "wrong answer";
  }
}


int main()
{
    //文件路径
    string inputFile  = "./test_data.txt";
    // string inputFile  = "/data/test_data.txt";
    string answerFile = "./result.txt";
    // string answerFile = "/projects/student/result.txt";

    //clock_t start, finish;//记录起始时间
    //start = clock();

    Circular circular;
    circular.Input(inputFile);
    //创建图
    circular.createGraph();
    //拓扑排序，去掉部分节点
    circular.topologicalSort();
    //存储深度为2的路径信息
    circular.storeDepthTwo();
    //暴力dfs
    circular.work();
    //保存
    circular.mmap_save(answerFile);
    //finish = clock();
    //cout<<pathCount<<endl;
    //cout<<(double)(finish - start) / CLOCKS_PER_SEC<<"s"<<endl;
    //cout<<check_ans(answerFile,resultFile)<<endl;
    exit(0);
}
