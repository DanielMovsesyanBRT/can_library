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
#include <typeinfo>
#include <vector>

#include <string.h>

namespace brt {
namespace can {

/**
 * \struct LibraryConfig
 *
 */
struct LibraryConfig
{
  LibraryConfig() 
  // Default Values
  : _local_ecu_pool_size(1024)
  , _remote_ecu_pool_size(1024)
  , _big_messages_pool_size(1024)
  , _small_messages_pool_size(1024)
  , _tx_tpsessions_pool_size(32)
  , _rx_tpsessions_pool_size(32)
  , _transcoder_pool_size(32)
  {  }

  size_t                          _local_ecu_pool_size;
  size_t                          _remote_ecu_pool_size;
  size_t                          _big_messages_pool_size;
  size_t                          _small_messages_pool_size;
  size_t                          _tx_tpsessions_pool_size;
  size_t                          _rx_tpsessions_pool_size;

  size_t                          _transcoder_pool_size;
};

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
  
  typedef _Type value_type;

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
    
    const_iterator(const _Raw* ptr,const _Raw* end) : _ptr(ptr), _end(end)
    {
      while (_ptr < _end && _ptr->_empty)
        _ptr++;
    }

  public:
    typedef const_iterator self_type;
    typedef _Type value_type;
    typedef const _Type& reference;
    typedef const _Type* pointer;
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

    reference operator*() const { return _ptr->_v; }
    pointer operator->() const { return &_ptr->_v; }
    bool operator==(const self_type& rhs) const { return _ptr == rhs._ptr; }
    bool operator!=(const self_type& rhs) const { return _ptr != rhs._ptr; }

  private:
    const filler*                   _ptr;
    const filler*                   _end;
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
template<typename _Type,size_t _Extrasize = 0>
class allocator
{
  struct filler
  {
    filler() : _empty(true), _allocator(nullptr), _keyword(typeid(allocator<_Type,_Extrasize>).hash_code()) 
    { }

    uint8_t   _v[sizeof(_Type) + _Extrasize]   __attribute__ ((aligned (__BIGGEST_ALIGNMENT__)));
    std::atomic_bool              _empty;
    
    allocator<_Type, _Extrasize>* _allocator;
    size_t                        _keyword;
  };

  filler*                         _buffer;
  size_t                          _pool_size;

public:
  
  allocator(size_t pool_size = 1024) 
  : _pool_size(pool_size)
  { 
    _buffer = new filler[pool_size];

    for (size_t index = 0; index < pool_size; index++)
      _buffer[index]._allocator = this;
  }

  ~allocator() 
  {
    delete[] _buffer;
  }

  void* allocate()
  {
    for (size_t index = 0; index < _pool_size; index++)
    {
      bool expected = true;
      if (_buffer[index]._empty.compare_exchange_strong(expected, false))
        return &_buffer[index]._v[0];
    }
    return nullptr;
  }

  static bool free(void* ptr)
  {
    filler* fl = reinterpret_cast<filler*>(ptr);
    if (fl->_keyword != typeid(allocator<_Type,_Extrasize>).hash_code())
      return false;

    fl->_empty.store(true);
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
  typedef _Class  SelfType;

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


/**
 * \class CanString
 *
 */
class CanString
{
#define MAX_CAN_STRING                       (256)
public:
  CanString()
  { _string[0] = '\0'; }

  CanString(const char *name)
  { strncpy(_string, name, MAX_CAN_STRING-1);  }

  CanString(const CanString& canstring)
  { strncpy(_string, canstring._string, MAX_CAN_STRING-1);  }

          const char*             c_str() const { return _string; }
          size_t                  size() const { return strlen(_string); }
          operator const char*() const { return c_str(); }

          int compare(const char *str) const
          {
            return strncmp(_string, str, MAX_CAN_STRING-1);
          }

          bool operator==(const char *str) const { return compare(str) == 0; }
          bool operator!=(const char *str) const { return compare(str) != 0; }
          CanString& operator=(const char *str)
          {
            strncpy(_string, str, MAX_CAN_STRING-1);
            return *this;
          }

private:
  char                            _string[MAX_CAN_STRING];
};

/**
 * \class ConstantString
 *
 */
class ConstantString
{
public:
  ConstantString(const char* str = nullptr)
  : _string((str == nullptr) ? _empty_string : str)
  , _size(0)
  {
    const char* ptr = _string;
    while (*ptr++ != '\0')
      _size++;
  }

  ConstantString(const CanString& canstr)
  : _string(canstr.c_str())
  , _size(canstr.size())
  {  }

  ~ConstantString() {}

  ConstantString(const ConstantString&) = default;
  ConstantString& operator=(const ConstantString&) = default;
  bool operator==(const char *str) const = delete;
  bool operator!=(const char *str) const = delete;

          const char*             c_str() const { return _string; } 
          const char*             data() const { return _string; }

          size_t                  size() const { return _size; }
          size_t                  length() const { return _size; }

          bool                    empty() const { return _size == 0; }
          operator const char*() const { return c_str(); }
          
private:
  static const char               _empty_string[3];
  const char*                     _string;
  size_t                          _size;
};



inline std::array<uint8_t,2> can_pack16(uint16_t value)
{
  return {
    static_cast<uint8_t>(value & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF)
    };
}

inline std::array<uint8_t,3> can_pack24(uint32_t value)
{
  return {
    static_cast<uint8_t>(value & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
    static_cast<uint8_t>((value >> 16) & 0xFF),
    };
}

inline std::array<uint8_t,4> can_pack32(uint32_t value)
{
  return {
    static_cast<uint8_t>(value & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
    static_cast<uint8_t>((value >> 16) & 0xFF),
    static_cast<uint8_t>((value >> 24) & 0xFF),
    };
}

inline uint16_t can_unpack16(const uint8_t* data)
{
  return static_cast<uint16_t>(data[0] | (data[1] << 8));
}

inline uint32_t can_unpack24(const uint8_t* data)
{
  return static_cast<uint32_t>(data[0] | (data[1] << 8) | (data[2] << 16));
}

inline int32_t can_unpack24_signed(const uint8_t* data)
{
  int32_t val = static_cast<int32_t>(data[0] | (data[1] << 8) | (data[2] << 16));
  if ((val & 0x800000) != 0)
    val |= 0xFF000000;
  return val;
}

inline uint32_t can_unpack32(const uint8_t* data)
{
  return static_cast<uint32_t>(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}


} // can
} // brt

