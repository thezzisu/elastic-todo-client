//includes...
#include<queue>
#include<vector>
#include<string>
#include<algorithm>
#include<iostream>
#include<ctime>
#include<unordered_map>
#include<map>
#include<set>
#include<unordered_map>
#include"databaseManager.h"

//���ֺ궨��
#define MAX_URGENCY 128
#define MAX_TIME 50
#define MAX_INTERVAL 128

//����ÿһ���¼���event��
#define EVENT 1
#define RECURRING 2
#define DDL 3
#define PROJECT 4

class Event {
public:
    // �⼸��event���캯����û��tag��һ�����Ҫ���ڼ���ȥ
    // ���µ�set...�������������ݿ�ͬ����
    Event(bool init,DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
          const std::string& description, const int& urgency, const int& kind_of_event=EVENT);
    Event(bool init,DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
          const std::string& description, const int& urgency, const int& kind_of_event,
          const std::tm& create_time, const std::tm& finish_time, const std::tm& update_time);

    virtual ~Event();
    virtual int getInterval() const;
    virtual bool setInterval(int interval);
    virtual std::tm getDeadline() const;
    virtual bool setDeadline(const std::tm& deadline);
    virtual std::set<Event*> getSubEvents() const;
    virtual bool addSubEvent(Event* event);
    virtual bool deleteSubEvent(Event* event);
    virtual std::set<Event*> getFatherEvent() const;
    virtual bool addFatherEvent(Event* event);
    virtual bool deleteFatherEvent(Event* event);

    // getters and setters
    std::string getId() const;
    bool setId(const std::string& id);
    std::string getTitle() const;
    bool setTitle(const std::string& title);
    //for due time
    std::tm getTime() const;
    bool setTime(const std::tm& time);
    //�¼�������3��ʱ�䣺
    std::tm getCreateTime() const;
    bool setCreateTime(const std::tm& time);
    std::tm getFinishTime() const;
    bool setFinishTime(const std::tm& time);
    std::tm getUpdateTime() const;
    bool setUpdateTime(const std::tm& time);

    std::string getDescription() const;
    bool setDescription(const std::string& description);
    int getUrgency() const;
    bool setUrgency(const int& urgency);
    // �����¼����ͣ�
    int getKindOfEvent() const;
    bool setKindOfEvent(const int& kind_of_event);

    bool operator<(const Event& event)const;

    // ����tag����catagoryʹ�ã�
    // ������е�tag�� ���ڸ���/�鿴�Ȳ���
    std::set<std::string> getAllTags()const;
    // �ж��Ƿ������tag
    bool hasTag(const std::string& tag)const;
    // ɾ��tag��ɾ���ɹ�����1������0
    bool deleteTag(const std::string& tag);
    bool addTag(const std::string& tag);
    bool addTag_init(const std::string& tag);


    DatabaseManager* dbm;
private:
    std::string id;
    std::string title;
    //due time
    std::tm time;
    std::string description;
    int urgency;
    int kind_of_event;//type
    std::tm create_time, finish_time, update_time;
    std::set<std::string> tags;
};

class RecurringEvent : public Event {
public:
    RecurringEvent(bool init,DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
                   const std::string& description, const int& urgency, const int& interval);
    RecurringEvent(bool init,DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
                   const std::string& description, const int& urgency, const int& interval, const std::tm& create_time,
                   const std::tm& finish_time, const std::tm& update_time);
    virtual ~RecurringEvent();
    int getInterval() const;
    // �����ݿ�ͬ��
    bool setInterval(int interval);
private:
    int interval;
};
class DdlEvent : public Event {
public:
    DdlEvent(bool init,DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
             const std::string& description, const int& urgency, const std::tm& deadline);
    DdlEvent(bool init,DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
             const std::string& description, const int& urgency, const std::tm& deadline, const std::tm& create_time,
             const std::tm& finish_time, const std::tm& update_time);
    virtual ~DdlEvent();
    std::tm getDeadline()const;
    // �����ݿ�ͬ��
    bool setDeadline(const std::tm& deadline);
private:
    std::tm deadline;
};
class ProjectEvent : public Event {
public:
    ProjectEvent(bool init,DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
                 const std::string& description, const int& urgency);
    ProjectEvent(bool init,DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
                 const std::string& description, const int& urgency, const std::tm& create_time,
                 const std::tm& finish_time, const std::tm& update_time);
    virtual ~ProjectEvent();
    std::set<Event*> getSubEvents() const;
    bool addSubEvent(Event* event);
    bool deleteSubEvent(Event* event);

    std::set<Event*> getFatherEvent() const;
    bool addFatherEvent(Event* event);
    bool deleteFatherEvent(Event* event);

private:
    std::set<Event*> subEvents;
    std::set<Event*> fatherEvents;
};

long long int timeComparer(std::tm _tm1, std::tm _tm2);
//����ʽ���Զ��޸ĸ�ʽ
//����ֵ--0��false��--1(û���κ����⣩ --2����ʽ����ȷ�����������ˣ�
//����0�������
//�ޱ�������Ϊ���з���
//date��time��ȫ�޷�����
bool titleChecker(const std::string& title);
//�������ڹ�ȥ��ʱ������üƻ���
bool timeChecker(std::tm _time);
//���ڱȽ�����tm�������ݽṹ��ʱ���Ⱥ�
//
long long int timeComparer(std::tm _tm1, std::tm _tm2);
bool urgencyChecker(const int& _urgency);
//����calendar��catalog����event֮ǰ�����Ƚ��еĹ���event�Ƿ���ֵ�����޸�ʽ����ļ��
//��ʱ��ļ���ǿ�ѡ�ģ���Ϊ֮ǰд���event��������һ�δ����ݿ�������ʱ�Ѿ����˵�ǰʱ��
//��ЩeventsӦ����ʾ������ҲӦ�üӵ�calendar�У���ʱ�򣬾�ѡ�񲻼��ʱ�䡣
int eventFormatChecker(Event* event, bool checkTime=0);

class Calendar {
public:
    //Events*
    Calendar();
    Calendar(const std::string& id);
    ~Calendar();
    bool addEvent(Event* event);
    bool deleteEvent(Event* event);
    void displayEvents() const;
    void displayEvent(Event* event) const;
    std::set<Event*>& getAllEvents();
    std::string getId() const;
    void setId(const std::string& id);
    QString getClosestId(QString name);
private:
    //����ʱ������
    std::string id;
    std::set<Event*> events;
};

class Category {
public:
    Category(bool init,DatabaseManager* dbm, const std::string& id, const std::string& tag);
    Category(bool init,DatabaseManager* dbm, const std::string& tag);
    ~Category();
    bool addEvent(Event* event);
    bool addEvent_init(Event* event);
    bool deleteEvent(Event* event);
    void displayEvents() const;
    void displayEvent(Event* event) const;
    std::set<Event*>& getAllEvents();
    std::string getId() const;
    bool setId(const std::string& id);
    std::string getTag() const;
    bool setTag(const std::string& id);

    DatabaseManager* dbm;
private:
    std::string id;
    std::string tag;
    std::set<Event*> events;
};


class CategoryOrganizer{
public:
    void addCategory(const std::string& id, Category* category);
    bool deleteCategory(const std::string& id);
    Category* getCategory(const std::string& id);
    std::map<std::string, Category*> getAllCategories();
private:
    std::map<std::string, Category*> categories;
};
