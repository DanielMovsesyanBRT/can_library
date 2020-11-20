/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 
 * File : can_utils.cpp
 *
 */

#include "can_utils.hpp"
#include "can_processor.hpp"

namespace brt {
namespace can {

/**
 * \fn  constructor Mutex::Mutex
 *
 * @param  processor : CanProcessor* 
 */
Mutex::Mutex(CanProcessor* processor)
: _processor(processor)
{
  _mutex_id = _processor->create_mutex();
}

/**
 * \fn  destructor Mutex::~Mutex
 *
 */
Mutex::~Mutex()
{
  _processor->delete_mutex(_mutex_id);
}

/**
 * \fn  Mutex::lock
 *
 */
void Mutex::lock()
{
  _processor->lock_mutex(_mutex_id);
}

/**
 * \fn  Mutex::unlock
 *
 */
void Mutex::unlock()
{
  _processor->unlock_mutex(_mutex_id);
}

/**
 * \fn  constructor RecoursiveMutex::RecoursiveMutex
 *
 * @param  processor : CanProcessor* 
 */
RecoursiveMutex::RecoursiveMutex(CanProcessor* processor)
: Mutex(processor)
, _lock_counter(0)
{

}

/**
 * \fn  RecoursiveMutex::lock
 *
 */
void RecoursiveMutex::lock()
{
}

/**
 * \fn  RecoursiveMutex::unlock
 *
 */
void RecoursiveMutex::unlock()
{

}



} // can
} // brt



