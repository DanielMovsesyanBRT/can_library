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
      if (_ptr < _end)
        _ptr++;

      while (_ptr < _end && _ptr->_empty)
        _ptr++;

      return *this;
    }

    self_type operator++(int) 
    {
      self_type __r(_ptr, _end);

      if (_ptr < _end)
        _ptr++;

      while (_ptr < _end && _ptr->_empty)
        _ptr++;
      return __r;
    }

    reference operator*()  { return _ptr->_v; }
    pointer operator->() { return &_ptr->_v;  }
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
      if (_ptr < _end)
        _ptr++;

      while (_ptr < _end && _ptr->_empty)
        _ptr++;
      return *this;
    }

    self_type operator++(int) 
    {
      self_type __r(_ptr, _end);

      if (_ptr < _end)
        _ptr++;

      while (_ptr < _end && _ptr->_empty)
        _ptr++;
      return __r;
    }

    reference operator*() { return _ptr->_v; }
    pointer operator->() { return &_ptr->_v; }
    bool operator==(const self_type& rhs) { return _ptr == rhs._ptr; }
    bool operator!=(const self_type& rhs) { return _ptr != rhs._ptr; }

  private:
    filler*                         _ptr;
    filler*                         _end;
  };

  typedef _Type& reference;
  typedef const _Type& const_reference;

  fixed_list() : _num_elements(0) {}
  
  fixed_list(const std::initializer_list<_Type>& __l)
  : _num_elements(std::min(__l.size(), _Size))
  {
    size_t index = 0;
    for (auto __e : __l)
    {
      _buffer[index]._v = __e;
      _buffer[index]._empty = false;

      if (++index >= _num_elements)
        break;
    }
  }
  
  ~fixed_list() 
  { clear(); }

  size_t size() const { return _num_elements; }
  bool   empty() const { return (_num_elements == 0);}
  
  iterator begin() { return iterator(_buffer.data(),_buffer.data() + _buffer.size()); }
  iterator end() { return iterator(_buffer.data() + _buffer.size(),_buffer.data() + _buffer.size()); }

  const_iterator begin() const { return const_iterator(_buffer.data(),_buffer.data() + _buffer.size()); }
  const_iterator end() const { return const_iterator(_buffer.data() + _buffer.size(),_buffer.data() + _buffer.size()); }

  iterator push(const _Type& v)
  {
    for (auto& fl : _buffer)
    {
      if (fl._empty)
      {
        fl._v = v;
        fl._empty = false;
        _num_elements++;
        return iterator(&fl, _buffer.data() + _buffer.size());
      }
    }
    return end();
  }

  iterator erase(iterator i)
  {
    if (i == end())
      return end();

    if (!i._ptr->_empty)
      _num_elements--;

    i._ptr->_v = _Type();
    i._ptr->_empty = true;

    return ++i;
  }

  void clear()
  {
    for (auto& fl : _buffer)
    {
      if (!fl._empty)
      {
        fl._v = _Type();
        fl._empty = true;
        _num_elements--;
      }
    }
  }

  template<typename _Predicate>
  iterator find_if(_Predicate p) { return std::find_if(begin(), end(), p); }

  template<typename _Predicate>
  const_iterator find_if(_Predicate p) const { return std::find_if(begin(), end(), p);  }
};

/**
 * \class allocator
 *
 *  Fixe size allocator for real time operations
 */
template<typename _Type,size_t _Extrasize = 0,size_t _Poolsize = 1024>
class allocator
{
  struct filler
  {
    filler() : _empty(true) {}
    uint8_t  _v[sizeof(_Type) + _Extrasize];
    bool    _empty;
  };
  std::array<filler,_Poolsize>  _buffer;

public:
  allocator() {}
  ~allocator() {}

  void*  allocate()
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

  bool free(void* ptr)
  {
    filler* fl = reinterpret_cast<filler*>(ptr);
    if ((fl < _buffer.data()) || (fl > (_buffer.data() + _buffer.size() - 1)))
      return false;

    fl->_empty = true;
    return true;
  }
};


template<typename _Class>
class shared_pointer;
/**
 * \class shared_class
 *
 */
template<typename _Class>
class shared_class
{
public:
  shared_class() : _reference_counter(0) {}
  virtual ~shared_class() {}

          uint_fast32_t           addref()
          { return ++_reference_counter; }

          uint_fast32_t           release()
          { return --_reference_counter; }

          shared_pointer<_Class>  getptr()
          { return shared_pointer<_Class>(this); }

private:
  std::atomic_uint_fast32_t       _reference_counter;
};

/**
 * \class shared_pointer
 *
 */
template<typename _Class>
class shared_pointer
{
public:
  shared_pointer() : _ptr(nullptr) {}
  
  shared_pointer(_Class* ptr) : _ptr(ptr) 
  {
    if (_ptr != nullptr)
      _ptr->addref();
  }

  template<typename _Tp>
  shared_pointer(_Tp* ptr) : _ptr(dynamic_cast<_Class*>(ptr))
  {
    if (_ptr != nullptr)
      _ptr->addref();
  }

  shared_pointer(const shared_pointer<_Class>& s)
  {
    _ptr = s._ptr;
    if (_ptr != nullptr)
      _ptr->addref();
  }

  template <typename _Tp>
  shared_pointer(const shared_pointer<_Tp>& s)
  {
    _ptr = dynamic_cast<_Class*>(s.get());
    if (_ptr != nullptr)
      _ptr->addref();
  }

  virtual ~shared_pointer()
  {
    if (_ptr != nullptr)
    {
      if (_ptr->release() == 0)
        delete _ptr;
    }
  }

  void  reset(_Class* ptr = nullptr)
  {
    if (_ptr == ptr)
      return;

    if (_ptr != nullptr)
    {
      if (_ptr->release() == 0)
        delete _ptr;
    }

    _ptr = ptr;

    if (_ptr != nullptr)
      _ptr->addref();
  }

  shared_pointer<_Class>& operator=(const shared_pointer<_Class>& s)
  {
    reset(s._ptr);
    return *this;
  }

  template <typename _Tp>
  shared_pointer<_Class>& operator=(const shared_pointer<_Tp>& s)
  {
    reset(dynamic_cast<_Class*>(s.get()));
    return *this;
  }

  // _Class operator*() const
  // { return *_ptr; }

  _Class* operator->() const
  { return _ptr; }

  _Class* get() const
  { return _ptr; }

  operator bool() const
  { return (_ptr != nullptr); }

  bool operator==(const shared_pointer<_Class>& s) const
  {
    return _ptr == s._ptr;
  }

  bool operator!=(const shared_pointer<_Class>& s) const
  {
    return _ptr != s._ptr;
  }

private:
  _Class*                         _ptr;
};

template<typename _Tp, typename _Up>
inline shared_pointer<_Tp> dynamic_shared_cast(const shared_pointer<_Up>& __r) noexcept
{
  using _Sp = shared_pointer<_Tp>;
  if (auto* __p = dynamic_cast<_Tp*>(__r.get()))
	  return _Sp(__p);
  return _Sp();
}

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

