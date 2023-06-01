#ifndef MAPPER_H
#define MAPPER_H
#include <unordered_map>
#include <string>
class Event;
class Category;
class EventMapper {
public:
    EventMapper();
    inline void addMapping(const std::string& id, Event* event);
    inline void removeMapping(const std::string& id);
    inline Event* getEvent(const std::string& id);
    inline bool EventidExists(const std::string& id);
private:
    std::unordered_map<std::string, Event*> eventMap;
};
class CategoryMapper {
public:
    CategoryMapper();
    inline void addMapping(const std::string& id, Category* Category);
    inline void removeMapping(const std::string& id);
    inline Category* getCategory(const std::string& id);
    inline bool CategoryidExists(const std::string& id);
private:
    std::unordered_map<std::string, Category*> CategoryMap;
};
#endif // MAPPER_H
