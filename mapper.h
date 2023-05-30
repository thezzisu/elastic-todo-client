#ifndef MAPPER_H
#define MAPPER_H
#include <unordered_map>
#include <string>
#include "event.h"
class EventMapper {
public:
    EventMapper() {}

    void addMapping(const std::string& id, Event* event) {
        eventMap[id] = event;
    }

    void removeMapping(const std::string& id) {
        eventMap.erase(id);
    }

    Event* getEvent(const std::string& id) {
        return eventMap[id];
    }

private:
    std::unordered_map<std::string, Event*> eventMap;
};
class CategoryMapper {
public:
    CategoryMapper() {}

    void addMapping(const std::string& id, Category* Category) {
        CategoryMap[id] = Category;
    }

    void removeMapping(const std::string& id) {
        CategoryMap.erase(id);
    }

    Category* getEvent(const std::string& id) {
        return CategoryMap[id];
    }

private:
    std::unordered_map<std::string, Category*> CategoryMap;
};
#endif // MAPPER_H
