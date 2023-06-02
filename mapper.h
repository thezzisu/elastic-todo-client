#ifndef MAPPER_H
#define MAPPER_H
#include <unordered_map>
#include <string>
class Event;
class Category;
class EventMapper {
public:
    EventMapper();
    void addMapping(const std::string& id, Event* event);
    void removeMapping(const std::string& id);
    Event* getEvent(const std::string& id);
    bool EventidExists(const std::string& id);
private:
    std::unordered_map<std::string, Event*> eventMap;
};
class CategoryMapper {
public:
    CategoryMapper();
    void addMapping(const std::string& id, Category* Category);
    void removeMapping(const std::string& id);
    Category* getCategory(const std::string& id);
    bool CategoryidExists(const std::string& id);
private:
    std::unordered_map<std::string, Category*> CategoryMap;
};
#endif // MAPPER_H
