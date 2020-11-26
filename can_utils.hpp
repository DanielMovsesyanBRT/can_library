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
#include <queue>
#include <array>
#include <stdexcept>
#include <algorithm>

namespace brt {
namespace can {

/**
 * \class fifo
 *
 */
template<typename _Type,size_t _Size = 1024>
class fifo
{
public:
  typedef _Type& reference;
  typedef const _Type& const_reference;

  fifo() : _buffer(), _read(0), _write(0) {}
  ~fifo() {}

  bool empty() const
  {
    return (_read == _write);
  }

  reference front()
  {
    if (empty())
      throw std::out_of_range("fifo is empty");
    
    return _buffer[_read];
  }

  const_reference front() const
  {
    if (empty())
      throw std::out_of_range("fifo is empty");
    
    return _buffer[_read];
  }

  size_t size() const
  {
    if (_write < _read)
      return _Size - _read + _write;
    
    return _write - _read;
  }

  bool push(reference v)
  {
    if (size() >= (_Size - 1))
      return false;
    
    _buffer[_write++] = v;
    if (_write == _Size)
      _write = 0;
    return true;
  }

  bool push(const_reference v)
  {
    if (size() >= (_Size - 1))
      return false;
    
    _buffer[_write++] = v;
    if (_write == _Size)
      _write = 0;
    return true;
  }

  bool pop()
  {
    if (empty())
      return false;
    
    _buffer[_read++] = _Type();
    if (_read == _Size)
      _read = 0;
    return false;
  }

  void clear()
  {
    while (_read != _write)
    {
      _buffer[_read++] = _Type();
      if (_read == _Size)
        _read = 0;
    }
  }

  fifo<_Type,_Size>& operator=(const fifo<_Type,_Size>& rval) = default;

private:
  std::array<_Type,_Size>         _buffer;
  size_t                          _read;
  size_t                          _write;
};

/**
 * \class fixed_list
 *
 */
template<typename _Type,size_t _Size = 1024>
class fixed_list
{
private:
  struct filler
  {
    filler() : _v(), _empty(true) {}
    _Type         _v;
    bool          _empty;
  };
  std::array<filler, _Size>       _buffer;
  size_t                          _num_elements;

public:
  
  /**
   * \class iterator
   *
   */
  class iterator
  {
  friend fixed_list<_Type,_Size>;
    typedef fixed_list<_Type,_Size>::filler _Raw;
    
    iterator(_Raw* ptr,_Raw* end) : _ptr(ptr), _end(end)
    {
      while (_ptr < _end && _ptr->_empty)
        _ptr++;
    }

  public:
    typedef iterator self_type;
    typedef _Type value_type;
    typedef _Type& reference;
    typedef _Type* pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;

    self_type operator++() 
    {
      while (_ptr < _end && _ptr->_empty)
        _ptr++;
      return *this;
    }

    reference operator*() 
    { 
      return _ptr->_v;
    }

    pointer operator->() 
    { 
      return &_ptr->_v; 
    }

    bool operator==(const self_type& rhs) { return _ptr == rhs._ptr; }
    bool operator!=(const self_type& rhs) { return _ptr != rhs._ptr; }

  private:
    filler*                         _ptr;
    filler*                         _end;
  };

  /**
   * \class const_iterator
   *
   */
  class const_iterator
  {
  friend fixed_list<_Type,_Size>;
    typedef fixed_list<_Type,_Size>::filler _Raw;
    
    const_iterator(_Raw* ptr,_Raw* end) : _ptr(ptr), _end(end)
    {
      while (_ptr < _end && _ptr->_empty)
        _ptr++;
    }

  public:
    typedef const_iterator self_type;
    typedef _Type value_type;
    typedef _Type& reference;
    typedef _Type* pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;


    self_type operator++() 
    {
      while (_ptr < _end && _ptr->_empty)
        _ptr++;
      return *this;
    }

    self_type operator++(int) 
    {
      while (_ptr < _end && _ptr->_empty)
        _ptr++;
      return *this;
    }

    reference operator*() 
    { 
      return _ptr->_v;
    }

    pointer operator->() 
    { 
      return &_ptr->_v; 
    }

    bool operator==(const self_type& rhs) { return _ptr == rhs._ptr; }
    bool operator!=(const self_type& rhs) { return _ptr != rhs._ptr; }

  private:
    filler*                         _ptr;
    filler*                         _end;
  };

  typedef _Type& reference;
  typedef const _Type& const_reference;

  fixed_list() : _num_elements(0) {}
  ~fixed_list() {}

  size_t size() const { return _num_elements; }
  
  iterator begin() { return iterator(_buffer.data(),_buffer.data() + _buffer.size()); }
  iterator end() { return iterator(_buffer.data() + _buffer.size(),_buffer.data() + _buffer.size()); }

  const_iterator begin() const { return const_iterator(_buffer.data(),_buffer.data() + _buffer.size()); }
  const_iterator end() const { return const_iterator(_buffer.data() + _buffer.size(),_buffer.data() + _buffer.size()); }

  bool push(const _Type& v)
  {
    for (auto& fl : _buffer)
    {
      if (fl._empty)
      {
        fl._v = v;
        fl._empty = false;
        return true;
      }
    }
    return false;
  }

  iterator erase(iterator i)
  {
    if (i == end())
      return end();

    i._ptr->_v = _Type();
    i._ptr->_empty = true;

    return ++i;
  }

  template<typename _Predicate>
  iterator find_if(_Predicate p)
  {
    return std::find_if(begin(), end(), p);
  }

  template<typename _Predicate>
  const_iterator find_if(_Predicate p) const
  {
    return std::find_if(begin(), end(), p);
  }
};

/**
 * \class allocator
 *
 *  Fixe size allocator for real time operations
 */
template<typename _Type,size_t _Buffersize = 1024>
class allocator
{
  struct filler
  {
    filler() : _empty(true) {}
    uint8_t  _v[sizeof(_Type)];
    bool    _empty;
  };
  std::array<filler,_Buffersize>  _buffer;

public:
  allocator() {}
  ~allocator() {}

  void*    allocate()
  {
    for (size_t index = 0; index < _buffer.size(); index++)
    {
      if (_buffer[index]._empty)
      {
        _buffer[index]._empty = false;
        return &_buffer[index]._v[0];
      }
    }
    return nullptr;
  }

  void     free(void* ptr)
  {
    filler* fl = reinterpret_cast<filler*>(ptr);
    if ((fl >= _buffer.data()) && (fl < (_buffer.data() + _buffer.size())))
    {
      if (!fl->_empty)
        reinterpret_cast<_Type*>(&fl->_v[0])->~_Type();

      fl->_empty = true;
    }
  }
};


/**
 * \class allocator_extra
 *
 */
template<typename _Type,size_t _Extrasize = 0,size_t _Buffersize = 1024>
class allocator_extra
{
  struct filler
  {
    filler() : _empty(true) {}
    uint8_t  _v[sizeof(_Type) + _Extrasize];
    bool    _empty;
  };
  std::array<filler,_Buffersize>  _buffer;

public:
  allocator_extra() {}
  ~allocator_extra() {}

  void*    allocate()
  {
    for (size_t index = 0; index < _buffer.size(); index++)
    {
      if (_buffer[index]._empty)
      {
        _buffer[index]._empty = false;
        return &_buffer[index]._v[0];
      }
    }
    return nullptr;
  }

  void     free(void* ptr)
  {
    filler* fl = reinterpret_cast<filler*>(ptr);
    if ((fl >= _buffer.data()) && (fl < (_buffer.data() + _buffer.size())))
    {
      if (!fl->_empty)
        reinterpret_cast<_Type*>(&fl->_v[0])->~_Type();

      fl->_empty = true;
    }
  }
};


// template<typename T>
// class shared_pointer


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
          CanProcessor*           processor() { return _processor; }

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
  std::atomic_uint32_t            _lock_counter;
  std::atomic_uint32_t            _thread_id;
};


} // can
} // brt

