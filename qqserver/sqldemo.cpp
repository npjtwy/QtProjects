
#include <string.h>
#include <iostream>
#include <string>
using namespace std;

#include "mysqlhelper.h"
using namespace mysqlhelper;


int main(int argc,char* argv[]){

    //初始化mysql对象并建立连接
    MysqlHelper mysqlHelper;
    mysqlHelper.init("localhost","wang","wang","StudentCourse");
    try{
        mysqlHelper.connect();
    }catch(MysqlHelper_Exception& excep){
        cout<<excep.errorInfo;
        return -1;
    }

    cout << "数据库连接成功" << endl;

    //增加一条学生记录
    //示例插入语句
    //string sql="insert into student values("201421031060","吕吕","华南理工大学","2014","软件工程",1)";

    /*MysqlHelper::RECORD_DATA record;
    record.insert(make_pair("studentNo",make_pair(MysqlHelper::DB_STR,"201421031060")));
    record.insert(make_pair("name",make_pair(MysqlHelper::DB_STR,"吕吕")));
    record.insert(make_pair("school",make_pair(MysqlHelper::DB_STR,"广州中医药大学")));
    record.insert(make_pair("grade",make_pair(MysqlHelper::DB_STR,"2014")));
    record.insert(make_pair("major",make_pair(MysqlHelper::DB_STR,"计算机科学与技术")));
    record.insert(make_pair("gender",make_pair(MysqlHelper::DB_INT,"1")));
       int res=0;
    try{
        res=mysqlHelper.insertRecord("student",record);
    }catch(MysqlHelper_Exception& excep){
        cout<<excep.errorInfo;
        return -1;
    }
    cout<<"res:"<<res<<" insert successfully "<<endl;
    */
    mysqlHelper.setDefalutCharacterSet();
    cout<< "插入1个学生: " << endl;

    try{
        mysqlHelper.execute("insert into student values('201421031065', '积极','华南理工','2014','软件工程',0)");
    }
    catch(MysqlHelper_Exception& excep){
        cout<<excep.errorInfo;
        return -1;
    }
    cout << "插入成功" << endl;

    cout << "删除一条学生记录，学号为201421031059:" << endl;
    try{
        mysqlHelper.execute("delete from student where studentNo = '201421031059'");
    }catch(MysqlHelper_Exception& excep){
        cout<<excep.errorInfo;
        return -1;
    }
    cout<<" delete successfully "<<endl;

    cout <<"查找学号为201421031060的学生选择的所有课程名称: " << endl;
    mysqlhelper::MysqlData dataSet;
    string querySQL="select courseName from course co where co.courseNo in (select courseNo from courseSelection where studentNo='201421031060')";
    try{
        dataSet=mysqlHelper.queryRecord(querySQL);
    }catch(MysqlHelper_Exception& excep){
        cout<<excep.errorInfo;
        return -1;
    }
    cout<<"query successfully"<<endl;
    for(size_t i=0;i<dataSet.size();++i){
        cout<<dataSet[i]["courseName"]<<endl;
    }

    return 0;
}
