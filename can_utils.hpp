/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 
 * File : can_utils.hpp
 *
 */
#pragma once

#include <unordered_map>
#include <atomic>

namespace brt {
namespace can {

/**
 * \class BiKeyMap
 *
 */
template<typename Key1, typename Key2, typename Value>
class BiKeyMap
{
public:
  typedef std::pair<Key2, Value> value_type1;
  typedef std::pair<Key1, Value> value_type2;

  typedef std::unordered_map<Key1, value_type1>   Map1;
  typedef std::unordered_map<Key2, value_type2>   Map2;

  BiKeyMap() {}

  /**
   * \fn  insert
   *
   * @param  key1 : Key1 
   * @param  key2 : Key2 
   * @param  value : Value 
   * @return  bool
   */
  bool insert(Key1 key1,Key2 key2,Value value)
  {
    _map1.erase(key1);
    _map2.erase(key2);

    auto result1 = _map1.insert(typename BiKeyMap::Map1::value_type(key1,typename BiKeyMap::value_type1(key2,value)));
    if (!result1.second)
      return false;

    auto result2 = _map2.insert(typename BiKeyMap::Map2::value_type(key2,typename BiKeyMap::value_type2(key1,value)));
    if (!result2.second)
    {
      _map1.erase(result1.first);
      return false;
    }

    return true;
  }

  /**
   * \fn  erase_left
   *
   * @param  key : Key1 
   */
  void  erase_left(Key1 key) 
  {
    auto left = _map1.find(key);
    if (left == _map1.end())
      return;

    _map2.erase(left->second.first);
    _map1.erase(left);
  }

  /**
   * \fn  erase_right
   *
   * @param  key : Key2 
   */
  void  erase_right(Key2 key) 
  {
    auto right = _map2.find(key);
    if (right == _map2.end())
      return;

    _map1.erase(right->second.first);
    _map2.erase(right);
  }

  /**
   * \fn  find_left
   *
   * @param  key : Key1 
   * @return  std::pair<bool, value_type1
   */
  std::pair<bool, value_type1> find_left(Key1 key) const
  {
    auto left = _map1.find(key);
    if (left == _map1.end())
      return std::pair<bool, value_type1>(false,value_type1());

    return std::pair<bool, value_type1>(true,left->second);
  }

  /**
   * \fn  find_right
   *
   * @param  key : Key2 
   * @return  std::pair<bool, value_type2
   */
  std::pair<bool, value_type2> find_right(Key2 key) const
  {
    auto right = _map2.find(key);
    if (right == _map2.end())
      return std::pair<bool, value_type2>(false,value_type2());

    return std::pair<bool, value_type2>(true,right->second);
  }


private:

  Map1                            _map1;
  Map2                            _map2;
};


class CanProcessor;
/**
 * \class Mutex
 *
 */
class Mutex
{
public:
  Mutex(CanProcessor* processor);
  virtual ~Mutex();

  virtual void                    lock();
  virtual void                    unlock();

private:
  uint32_t                        _mutex_id;
  CanProcessor*                   _processor;
};

/**
 * \class RecoursiveMutex
 *
 */
class RecoursiveMutex : public Mutex
{
public:
  RecoursiveMutex(CanProcessor* processor);
  virtual ~RecoursiveMutex() {}

  virtual void                    lock();
  virtual void                    unlock();
private:
  std::atomic_uint_fast32_t       _lock_counter;
};


} // can
} // brt

