#include <unordered_map>
#include <string>
#include "mapper.h"
class Event;
class Category;
EventMapper::EventMapper(){}
void EventMapper::addMapping(const std::string& id, Event* event) {
    eventMap[id] = event;
}
void EventMapper::removeMapping(const std::string& id) {
    eventMap.erase(id);
}
Event* EventMapper::getEvent(const std::string& id) {
    return eventMap[id];
}
bool EventMapper::EventidExists(const std::string& id) {
    return eventMap.find(id) != eventMap.end();
}
CategoryMapper::CategoryMapper() {}
void CategoryMapper::addMapping(const std::string& id, Category* Category) {
    CategoryMap[id] = Category;
}
void CategoryMapper::removeMapping(const std::string& id) {
    CategoryMap.erase(id);
}
Category* CategoryMapper::getCategory(const std::string& id) {
    return CategoryMap[id];
}
bool CategoryMapper::CategoryidExists(const std::string& id) {
    return CategoryMap.find(id) != CategoryMap.end();
}
