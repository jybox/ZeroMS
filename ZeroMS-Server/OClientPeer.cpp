#include <QStringList>
#include <QDateTime>
#include "OClientPeer.h"
#include "OClient.h"
#include "global.h"
#include "OServerCore.h"

OClientPeer::OClientPeer(QTcpSocket *connect):OAbstractPeer(connect)
{

}

OPeerType OClientPeer::getPeerType()
{
    return ClientPeer;
}

void OClientPeer::onLogin(QString uname,QString pwdHash,QVector<int> p2pPort,bool isMain,bool isForce,bool isShowIp)
{
    if(client->isLoged)//如果已经登录了
    {
        LoginResult(ALREADY);
        return;
    }

    //测试用代码
    //if(true)
    if(!publicKey.isEmpty() && core->db.checkPWD(uname,pwdHash,publicKey))
    {//如果已经申请过公钥，且密码正确
        if(isMain)
        {//如果是主连接
            if(core->cl.contains(uname))
            {//如果连接列表里已经有这个用户名了
                if(isForce)
                {//如果是强制登录
                    delete core->cl[uname];
                    core->cl.remove(uname);
                }
                else
                {//如果不是强制登录
                    LoginResult(ISONLINE,core->cl[uname]->main->conn->peerAddress().toString());
                    return;
                }
            }
            core->cl.remove(client->getSignature());
            core->cl.insert(uname,client);
            client->uname=uname;
            client->isLoged=true;
            client->isShowIp=isShowIp;
            (client->p2pPorts)<<p2pPort;
            LoginResult(OK);
            core->userListChange(uname);
        }
        else
        {//如果是次要连接
            if(core->cl.contains(uname))
            {//如果有同名的主连接
                QString signature=client->getSignature();
                core->cl[uname]->addSubConn(this);
                client->main=0;
                delete client;
                core->cl.remove(signature);
                (core->cl[uname]->p2pPorts)<<p2pPort;
                LoginResult(OK);
            }
            else
            {//如果没有同名的主连接
                LoginResult(NOMAIN);
            }
        }
    }
    else
    {//如果没有申请过公钥，或者密码错误
        LoginResult(PWDERR);
    }

    publicKey="";
}

void OClientPeer::AskPublicKey()
{
    QByteArray key;

    //这里的公钥采用15个ascii从32到126的随机字符
    for(int i=0;i<15;i++)
    {
        char c = ( qrand()%(126-32) )+32;
        key.append(QString(c));
    }
    publicKey=QString(key);

    PublicKey(key);
}

void OClientPeer::State(QString status)
{
    if(client->status!=status)
    {
        client->status=status;
        core->userListChange(client->uname);
    }
}

void OClientPeer::AskInfo(QStringList keys)
{
    QMap<QString,QString> result;
    QStringListIterator i(keys);
    while(i.hasNext())
    {
        QString key=i.next();
        if(core->info.contains(key))
        {
            result.insert(key,core->info[key]);
        }
        else
        {
            if(key==TIME)
                result.insert(key,QString::number(QDateTime::currentDateTime().toTime_t()));
        }
    }
    Info(result);
}

void OClientPeer::AskUserList(QString listname,QString operation,bool isHasAvatar)
{
    using namespace OSDB;

    QVector<OUserlistItem> allList;
    if(listname!=client->uname)
    {//如果是在请求一个群的成员列表
        listname=listname.remove(0,1);//移除星号
        if(core->db.checkGroup(listname) && core->db.checkGroupMember(listname,client->uname))
        {//如果存在这个群,且是这个群的成员
            QVector<GroupMember> memberList=core->db.selectTable<GroupMember>(OT(GroupMember::_groupname,listname));

            QVectorIterator<GroupMember> i(memberList);
            while(i.hasNext())
            {
                QString user=i.next().uname;
                User userInfo=core->db.selectFrist<User>(OT(User::_uname,user));

                GroupMember groupStatus=core->db.selectFrist<GroupMember>( OT(GroupMember::_groupname,listname) && OT(GroupMember::_uname,user) );

                OUserlistItem item;
                if(!core->cl.contains(user))
                {//如果这个用户不在线
                    if(operation==ONLINE)
                        continue;
                }
                else
                {//如果这个用户在线
                    if(core->cl[user]->isShowIp)
                    {
                        item.ip=core->cl[user]->main->conn->peerAddress().toString();

                        QVectorIterator<int> i(core->cl[user]->p2pPorts);
                        while(i.hasNext())
                            item.p2pPorts.append(i.next());
                    }
                }
                item.uname=user;
                item.status=core->getUserStatus(user);
                QStringList status;
                if(groupStatus.isAdmin)
                    status.append(ADMIN);
                if(groupStatus.isDeny)
                    status.append(DENY);
                item.groupStatus=status.join(",");
                if(isHasAvatar)
                    item.avatar=userInfo.avatar;

                allList.append(item);
            }
        }
        else
        {//如果不存在这个群，或不是这个群的成员
            Unknown();
            return;
        }
    }
    else
    {//如果是在请求自己的好友列表
        QVector<GroupMember> groups=core->db.selectTable<GroupMember>(OT(GroupMember::_uname,client->uname));
        QVectorIterator<GroupMember> iGroup(groups);
        while(iGroup.hasNext())
        {
            Group info=core->db.selectFrist<Group>(OT(Group::_groupname,iGroup.next().groupname));
            OUserlistItem item;
            item.uname=QString("*%1,%2").arg(info.groupname).arg(info.caption);
            item.status=ONLINE;
            if(isHasAvatar)
                item.avatar=info.avatar;
            allList.append(item);
        }

        QVector<OSDB::UserList> userlist=core->db.selectTable<OSDB::UserList>(OT(OSDB::UserList::_uname,client->uname));
        QVectorIterator<OSDB::UserList> iUserlist(userlist);
        while(iUserlist.hasNext())
        {
            OSDB::UserList listItem=iUserlist.next();
            OUserlistItem item;

            User userInfo=core->db.selectFrist<User>(OT(User::_uname,listItem.user));
            if(!core->cl.contains(listItem.user))
            {//如果这个用户不在线
                if(operation==ONLINE)
                    continue;
            }
            else
            {//如果这个用户在线
                if(core->cl[listItem.user]->isShowIp)
                {
                    item.ip=core->cl[listItem.user]->main->conn->peerAddress().toString();

                    QVectorIterator<int> i(core->cl[listItem.user]->p2pPorts);
                    while(i.hasNext())
                        item.p2pPorts.append(i.next());
                }
            }
            item.uname=listItem.user;
            item.status=core->getUserStatus(listItem.user);
            if(isHasAvatar)
                item.avatar=userInfo.avatar;

            allList.append(item);
        }
    }

    QVector<OUserlistItem> *cache=&(client->userlistCache[listname]);

    if(operation==DIFFONLY)
    {
        QVector<OUserlistItem> result;

        QVectorIterator<OUserlistItem> iAll(allList),iCache(*cache);
        while(iAll.hasNext())
        {
            OUserlistItem item=iAll.next();

            if(!iCache.findNext(item))
                result.append(item);
            iCache.toFront();
        }

        while(iCache.hasNext())
        {
            OUserlistItem item=iCache.next();

            iAll.toFront();
            if(!iAll.findNext(item))
            {
                if(item.uname.left(1)=="*")
                {//如果是一个群
                    item.status=REMOVED;
                }
                else if(!core->db.selectFrist<OSDB::UserList>( OT(OSDB::UserList::_uname,client->uname) && OT(OSDB::UserList::_user,item.uname))._isEmpty)
                {//如果是一个用户，且这个用户还在用户列表中
                   item.status=OFFLINE;
                }
                else
                {//如果这个用户已经不在用户列表中了
                    item.status=REMOVED;
                }

                result.append(item);
            }
        }
        UserList(listname,operation,result);
    }

    *cache=allList;

    UserList(listname,operation,allList);
}

void OClientPeer::ModifyUserList(QString listname,QString uname,QString operation,QString message)
{
    if(client->isLoged)
    {//如果已经登录
        if(listname==client->uname)
        {//如果是在操作自己的好友列表
            bool isGroup=(uname.left(1)=="*")?true:false;
            if(!isGroup)
            {//如果要操作的目标用户名是用户
                if(core->db.checkUser(uname))
                {//如果存在这个用户
                    if(operation==ADD)
                    {//如果是添加好友

                    }
                }
                else
                {//如果不存在这个用户
                    Unknown();
                }
            }
            else
            {//如果要操作的目标用户名是小组

            }
        }
        else
        {
            if(core->db.checkGroup(listname))
            {

            }
            else
            {
                Unknown();
            }
        }
    }
    else
    {//如果没有登录
        Unknown();
    }
}
