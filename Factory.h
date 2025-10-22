#pragma once
#ifndef __FACTORY_H__
#define __FACTORY_H__

#include <map>
#include <functional>
#include <string>
#include <iostream>
#include <vector>

template<class B>
class Factory {
  std::map<std::string, std::function<std::shared_ptr<B>()>> s_creators;

public:
  static Factory<B>& getInstance() {
    static Factory<B> s_instance;
    return s_instance;
  }

  template<class T>
  void registerClass(const std::string& name) {
    s_creators.insert({name, []() -> std::shared_ptr<B> { return std::make_shared<T>(); }});
  }

  std::shared_ptr<B> create(const std::string& name) {
    const auto it = s_creators.find(name);
    if (it == s_creators.end()) return nullptr; // not a derived class
    return (it->second)();
  }

  void printRegisteredClasses() {
    for (const auto &creator : s_creators) {
      std::cout << creator.first << '\n';
    }
  }

  std::vector<std::string> GetRegisteredClasses()
  {
      std::vector<std::string> result;
      for (const auto& creator : s_creators) {
          result.push_back(creator.first);
      }

      return result;
  }
};
#define FACTORY(Class) Factory<Class>::getInstance()

template<class B, class T>
class Creator {
public:
  explicit Creator(const std::string& name) {
    FACTORY(B).registerClass<T>(name);
  }
};

#define REGISTER(base_class, derived_class) \
  Creator<base_class, derived_class> s_##derived_class##Creator(#derived_class);

#endif