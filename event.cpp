#include"Event.h"
#include"databaseManager.h"
#include<ctime>

Event::Event(DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
             const std::string& description, const int& urgency, const int& kind_of_event):
    dbm(dbm),
    id(id),
    title(title),
    time(time),
    description(description),
    urgency(urgency),
    kind_of_event(kind_of_event)
{
    if(kind_of_event==EVENT)
        dbm->addEvent(this);
}
Event::Event(DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
             const std::string& description, const int& urgency, const int& kind_of_event,
             const std::tm& create_time, const std::tm& finish_time, const std::tm& update_time) :
    dbm(dbm),
    id(id),
    title(title),
    time(time),
    description(description),
    urgency(urgency),
    kind_of_event(kind_of_event),
    create_time(create_time),
    finish_time(finish_time),
    update_time(update_time)
{
    if (kind_of_event == EVENT)
        dbm->addEvent(this);
}

Event::~Event(){
    dbm->delEvent(this);
}

std::string Event::getId()const{return id;}
std::string Event::getTitle() const { return title; }
std::tm Event::getTime() const { return time; }
std::string Event:: getDescription() const { return description; }
int Event:: getUrgency() const { return urgency; }
int Event:: getKindOfEvent() const { return kind_of_event; }
std::tm Event::getCreateTime() const { return create_time; }
std::tm Event::getFinishTime() const { return finish_time; }
std::tm Event::getUpdateTime() const { return update_time; }


//virtual functions
int Event::getInterval() const {
    return 0;
}
bool Event::setInterval(int interval) {
    return 0;
}
std::tm Event::getDeadline() const
{
    tm error_tm;
    std::cout << "error...\n";
    return error_tm;
}
bool Event::setDeadline(const std::tm& deadline) {
    return 0;
}
std::set<Event*> Event::getSubEvents() const {
    std::set<Event*> s;
    std::cout << "error...\n";
    return s;
}
bool Event::addSubEvent(Event* event) {
    return 0;
}
bool Event::deleteSubEvent(Event* event) {
    return 0;
}
std::set<Event*>Event::getFatherEvent() const {
    std::set<Event*> s;
    std::cout << "error...\n";
    return s;
}
bool Event::addFatherEvent(Event* event) {
    return 0;
}
bool Event::deleteFatherEvent(Event* event) {
    return 0;
}



bool Event:: setCreateTime(const std::tm& create_time) {
    this->create_time = create_time;
    return dbm->modEvent(this);
}
bool Event:: setFinishTime(const std::tm& finish_time) {
    this->finish_time = finish_time;
    return dbm->modEvent(this);
}
bool Event:: setUpdateTime(const std::tm& update_time) {
    this->update_time = update_time;
    return dbm->modEvent(this);
}

std::set<std::string> Event:: getAllTags()const{
    return tags;
}
bool Event::hasTag(const std::string& tag)const{
    if(tags.find(tag)!=tags.end()){
        return 1;
    }
    return 0;
}
bool Event::deleteTag(const std::string& tag){
    tags.erase(tag);
    return dbm->modEvent(this);
}
bool Event:: addTag(const std::string& tag){
    tags.insert(tag);
    return dbm->modEvent(this);
}
bool Event:: setTitle(const std::string& title){
    this->title = title;
    return dbm->modEvent(this);
}
bool Event:: setId(const std::string& id){
    this->id=id;
    return dbm->modEvent(this);
}
bool Event:: setTime(const std::tm& time){
    this->time=time;
    return dbm->modEvent(this);
}
bool Event:: setDescription(const std::string& description){
    this->description=description;
    return dbm->modEvent(this);
}
bool Event:: setUrgency(const int& urgency){
    this->urgency=urgency;
    return dbm->modEvent(this);
}
bool Event:: setKindOfEvent(const int& kind_of_event){
    this->kind_of_event=kind_of_event;
    return dbm->modEvent(this);
}

bool Event::operator<(const Event& event) const {
    std::tm _tm = (event.getTime());
    std::tm thisTime = time;
    bool result = std::mktime(&thisTime) < std::mktime(&_tm);
    return result;
}






RecurringEvent::RecurringEvent(DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
                               const std::string& description, const int& urgency, const int& interval) :
    Event(dbm, id, title, time, description, urgency, RECURRING), interval(interval) {
    dbm->addEvent((Event*)this);
}
RecurringEvent::RecurringEvent(DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
                               const std::string& description, const int& urgency, const int& interval, const std::tm& create_time,
                               const std::tm& finish_time, const std::tm& update_time) :
    Event(dbm, id, title, time, description, urgency, RECURRING, create_time, finish_time, update_time), interval(interval) {
    dbm->addEvent((Event*)this);
}
RecurringEvent:: ~RecurringEvent(){
    dbm->delEvent((Event*)this);
}
int RecurringEvent:: getInterval() const{
    return interval;
}
bool RecurringEvent::setInterval(int interval){
    this->interval=interval;
    return dbm->modEvent((Event*)this);
}





DdlEvent::DdlEvent(DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
                   const std::string& description, const int& urgency, const std::tm& deadline) :
    Event(dbm, id, title, time, description, urgency, DDL), deadline(deadline) {
    dbm->addEvent((Event*)this);
}
DdlEvent::DdlEvent(DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
                   const std::string& description, const int& urgency, const std::tm& deadline, const std::tm& create_time,
                   const std::tm& finish_time, const std::tm& update_time) :
    Event(dbm, id, title, time, description, urgency, DDL, create_time, finish_time, update_time), deadline(deadline) {
    dbm->addEvent((Event*)this);
}
DdlEvent:: ~DdlEvent(){
    dbm->delEvent((Event*)this);
}
std::tm DdlEvent:: getDeadline() const{
    return deadline;
}
bool DdlEvent:: setDeadline(const std::tm& deadline){
    this->deadline=deadline;
    return dbm->modEvent((Event*)this);
}


ProjectEvent::ProjectEvent(DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
                           const std::string& description, const int& urgency) :
    Event(dbm, id, title, time, description, urgency, PROJECT) {
    dbm->addEvent((Event*)this);
}
ProjectEvent::ProjectEvent(DatabaseManager* dbm, const std::string& id, const std::string& title, const std::tm& time,
                           const std::string& description, const int& urgency, const std::tm& create_time,
                           const std::tm& finish_time, const std::tm& update_time) :
    Event(dbm, id, title, time, description, urgency, PROJECT, create_time, finish_time, update_time) {
    dbm->addEvent((Event*)this);
}
ProjectEvent:: ~ProjectEvent(){
    dbm->delEvent((Event*)this);
}
std::set<Event*> ProjectEvent:: getSubEvents() const{
    return subEvents;
}
bool ProjectEvent:: addSubEvent(Event* event){
    if(subEvents.find(event)!=subEvents.end()){
        return 0;
    }
    subEvents.insert(event);
    return dbm->modEvent((Event*)this);
}
bool ProjectEvent:: deleteSubEvent(Event* event){
    if(subEvents.find(event)==subEvents.end()){
        return 0;
    }
    subEvents.erase(event);
    return dbm->modEvent((Event*)this);
}

std::set<Event*> ProjectEvent:: getFatherEvent() const{
    return fatherEvents;
}
bool ProjectEvent:: addFatherEvent(Event* event){
    if(fatherEvents.find(event)!=fatherEvents.end()){
        return 0;
    }
    fatherEvents.insert(event);
    return dbm->modEvent((Event*)this);
}
bool ProjectEvent:: deleteFatherEvent(Event* event){
    if(fatherEvents.find(event)==subEvents.end()){
        return 0;
    }
    subEvents.erase(event);
    return dbm->modEvent((Event*)this);
}

long long int timeComparer(std::tm _tm1, std::tm _tm2) {
    std::tm* p1 = new std::tm(_tm1);
    std::tm* p2 = new std::tm(_tm2);
    return mktime(p1) - mktime(p2);
}

bool titleChecker(const std::string& title) {
    if (title == "" || title == "\n") {
        return 0;
    }
    for (auto i : title) {
        if (i != ' ') return 1;
    }
    std::cout << "titleû���κ�����...\n";
    return 0;
}

bool timeChecker(std::tm _time) {
    std::time_t tm_now;
    std::time(&tm_now);
    std::tm* pt = new std::tm(_time);
    if (mktime(pt) < tm_now) {
        std::cout << "�������ڹ�ȥ��ʱ������üƻ�...\n";
        return 0;
    }
    return 1;
}

bool urgencyChecker(const int& _urgency) {
    if (_urgency <= MAX_URGENCY && _urgency >= 0) {
        return 1;
    }
    else {
        std::cout << "urgency value out of bound...\n";
        return 0;
    }
}

int eventFormatChecker(Event* event, bool checkTime) {
    if (!titleChecker(event->getTitle()))
        return 0;
    if (checkTime && timeChecker(event->getTime()))
        return 0;
    if (urgencyChecker(event->getUrgency()))
        return 0;
    int kind = event->getKindOfEvent();
    if (kind == EVENT) {
        //û��������Ҫ���Ķ�����
        return 1;
    }
    else if (kind == RECURRING) {
        RecurringEvent* _event = (RecurringEvent*)event;
        if (_event->getInterval()>MAX_INTERVAL || _event->getInterval() < 0) {
            std::cout << "recurring interval out of range...\n";
            return 0;
        }
        return 1;
    }
    else if (kind == DDL) {
        //֮���������û�õ�����ʲô��Ҫ�ټ���
        //��������Ϊdueday�࣬��Ϊ���ǻ�ȱ�ٶ�dueday������
        //�������ó�һ���ֶ�Ҳ�С�
        return 1;
    }
    else if (kind == PROJECT) {
        ProjectEvent* _event = (ProjectEvent*)event;
        //�������fatherEvents����subEvents�ǿյģ���ô��������Ӧ�ļ��
        for (auto i : _event->getFatherEvent()) {//����Ƿ�father events�Ľ�ֹʱ���ں�
            if (timeComparer(i->getTime(), _event->getTime()) < 0) {
                std::cout << "fatherEvent��ʱ�����ڵ�ǰevent��ʱ��...\n";
                return 0;
            }
        }
        for (auto i : _event->getSubEvents()) {//����Ƿ�subEvent�Ľ�ֹʱ����ǰ
            if (timeComparer(i->getTime(), _event->getTime()) > 0) {
                std::cout << "subEvent��ʱ����ڵ�ǰevent��ʱ��...\n";
                return 0;
            }
        }
        return 1;
    }
    std::cout << "kind�ֶ����Ͳ���ȷ...\n";
    return 0;
}



Calendar::Calendar() {
    // �����ݿ������ӣ�
}
Calendar::Calendar(const std::string& id) {
    this->id = id;
    // �����ݿ������ӣ�
}
Calendar::~Calendar() {
    // �����ݿ���ģ�
}
std::string Calendar:: getId() const{
    return id;
}
void Calendar:: setId(const std::string& id){
    this->id=id;
}
bool Calendar:: addEvent(Event* event){
    if(events.find(event)!=events.end()){
        return 0;
    }
    events.insert(event);
    return 1;
}
bool Calendar:: deleteEvent(Event* event){
    if(events.find(event)==events.end()){
        return 0;
    }
    return events.erase(event);
}
void Calendar:: displayEvents() const{

}
void Calendar:: displayEvent(Event* event) const{
    std::cout<<"---event---"<<std::endl;
    std::cout<<event->getTitle()<<std::endl;
    std::cout<<event->getKindOfEvent()<<std::endl;
}
std::set<Event*>& Calendar:: getAllEvents(){
    return events;
}




// û���ҵ������ݿ�������/ɾ��/���ĵĽӿ�
Category::Category(DatabaseManager* dbm, const std::string& tag) {
    this->dbm = dbm;
    this->tag = tag;
    dbm->addCategory(this);
}
Category::Category(DatabaseManager* dbm, const std::string& id, const std::string& tag) {
    this->dbm = dbm;
    this->tag = tag;
    this->id = id;
    dbm->addCategory(this);
}
Category::~Category() {
    dbm->delCategory(this);
}
std::string Category:: getId() const{
    return id;
}
bool Category:: setId(const std::string& id){
    this->id=id;
    return dbm->modCategory(this);
}
std::string Category::getTag() const {
    return tag;
}
bool Category::setTag(const std::string& tag) {
    this->tag = tag;
    return dbm->modCategory(this);
}
bool Category:: addEvent(Event* event){
    if(events.find(event)!=events.end()){
        return 0;
    }
    events.insert(event);
    return dbm->modCategory(this);
}
bool Category:: deleteEvent(Event* event){
    if(events.find(event)==events.end()){
        return 0;
    }
    events.erase(event);
    return dbm->modCategory(this);
}
void Category:: displayEvents() const{

}
void Category:: displayEvent(Event* event) const{
    std::cout<<"---event---"<<std::endl;
    std::cout<<event->getTitle()<<std::endl;
    std::cout<<event->getKindOfEvent()<<std::endl;
}
std::set<Event*>& Category:: getAllEvents(){
    return events;
}






void CategoryOrganizer:: addCategory(const std::string& id, Category* category) {
    categories.insert(std::make_pair(id, category));
}
bool CategoryOrganizer:: deleteCategory(const std::string& id) {
    Category* pt=categories[id];
    delete pt;
    return categories.erase(id);
}
Category* CategoryOrganizer:: getCategory(const std::string& id) {
    return categories[id];
}
std::map<std::string, Category*> CategoryOrganizer::getAllCategories() {
    return categories;
}
