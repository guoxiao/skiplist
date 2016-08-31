// SkipList: A STL-map like container using skip-list

// Copyright (c) 2015-2016 Guo Xiao <guoxiao08@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <assert.h>
#include <cstdlib>
#include <functional>
#include <memory>
#include <stdexcept>
#ifndef NDEBUG
#include <iostream>
#endif
namespace guoxiao {
namespace skiplist {

template <typename KeyType, typename ValueType>
struct SkipNode {
  KeyType key;
  ValueType value;
  size_t level;
  SkipNode **next;
  SkipNode() : key(), value(), level(0), next(nullptr) {}

  SkipNode(const SkipNode &) = delete;
  SkipNode &operator=(const SkipNode &) = delete;
};

template <typename T>
class Iterator {
public:
  Iterator() : ptr(nullptr){};
  explicit Iterator(T *p) : ptr(p){};
  Iterator &operator=(T *p) {
    ptr = p;
    return *this;
  }

  Iterator &operator++() {
    ptr = ptr->next[0];
    return *this;
  }

  Iterator operator++(int) {
    Iterator temp = *this;
    ptr = ptr->next[0];
    return temp;
  }

  bool operator!=(const Iterator &rhs) const { return ptr != rhs.ptr; }

  bool operator==(const Iterator &rhs) const { return ptr == rhs.ptr; }

  bool operator==(const T *p) const { return ptr == p; }

  bool operator!=(const T *p) const { return ptr != p; }

  operator bool() const { return ptr != nullptr; }

  operator T *() const { return ptr; }

  T *operator->() const { return ptr; }

  T &operator*() const { return *ptr; }

private:
  T *ptr;
};

template <typename Key,
  typename T,
  typename Compare = std::less<Key>,
  typename Allocator = std::allocator<std::pair<Key, T>>>
class SkipList {
public:
  typedef Key key_type;
  typedef T mapped_type;
  typedef std::pair<Key, T> value_type;
  typedef Compare key_compare;
  typedef Allocator allocator_type;
  typedef SkipNode<Key, T> node_type;
  typedef size_t size_type;

  typedef Iterator<node_type> iterator;
  typedef Iterator<node_type> const_iterator;

  typedef typename std::allocator_traits<Allocator>::template rebind_alloc<node_type> node_allocator;
  typedef typename std::allocator_traits<Allocator>::template rebind_alloc<node_type *> next_allocator;

  SkipList() : size_(0), head_(create_node()), compare() {}

  ~SkipList() noexcept {
    iterator node = head_, temp;
    while (node) {
      temp = node;
      node = node->next[0];
      destroy_node(temp);
    }
  }

  // Move Ctor
  SkipList(SkipList &&s) noexcept : size_(s.size_),
                                    head_(s.head_),
                                    compare() {
    s.head_ = create_node();
    s.size_ = 0;
  }

  // Copy Ctor
  SkipList(const SkipList &s)
    : size_(s.size_), head_(create_node()), compare() {
    iterator snode = s.head_, node = head_;
    next_allocator nl;
    node_type **last = nl.allocate(s.level() + 1);
    head_->level = s.level();
    nl.deallocate(head_->next, 1);
    head_->next = nl.allocate(level() + 1);
    for (int i = level(); i >= 0; i--) {
      last[i] = head_;
    }
    snode = snode->next[0];
    while (snode) {
      node = create_node();
      node->key = snode->key;
      node->value = snode->value;
      node->level = snode->level;
      nl.deallocate(node->next, 1);
      node->next = nl.allocate(snode->level + 1);
      for (int i = snode->level; i >= 0; i--) {
        node->next[i] = nullptr;
        last[i]->next[i] = node;
        last[i] = node;
      }
      snode = snode->next[0];
    }
    nl.deallocate(last, level() + 1);
  }

  // Copy Assignment
  SkipList &operator=(const SkipList &s) {
    if (this == &s)
      return *this;
    SkipList tmp(s);
    return *this = std::move(tmp);
  }

  // Move Assignment
  SkipList &operator=(SkipList &&s) noexcept {
    this->~SkipList();
    size_ = s.size_;
    head_ = s.head_;
    s.head_ = create_node();
    s.size_ = 0;
    return *this;
  }

  size_type size() const { return size_; }
  bool empty() const { return size_ == 0; }
  size_type level() const { return head_->level; }

  iterator begin() noexcept { return iterator(head_->next[0]); }
  const_iterator begin() const noexcept { return iterator(head_->next[0]); }

  const_iterator cbegin() noexcept { return iterator(head_->next[0]); }
  const_iterator cend() const noexcept { return iterator(); }

  iterator end() noexcept { return iterator(); }
  const_iterator end() const noexcept { return iterator(); }

  template <typename K, typename V>
  iterator emplace(K &&key, V &&value) {
    iterator node = head_;
    next_allocator nl;
    size_t update_size = level() + 1 + 1;
    node_type **update = nl.allocate(update_size);
    for (int i = level(); i >= 0; i--) {
      assert(static_cast<int>(node->level) >= i);
      while (node->next[i] && compare(node->next[i]->key, key)) {
        node = node->next[i];
      }
      update[i] = node;
      if (node->next[i] && !compare(node->next[i]->key, key) && !compare(key, node->next[i]->key)) {
        nl.deallocate(update, update_size);
        throw std::runtime_error("conflict");
      }
    }

    node_type *n = create_node();
    n->key = std::forward<K>(key);
    n->value = std::forward<V>(value);

    size_t newlevel = getRandomLevel();
    if (newlevel > level()) {
      newlevel = level() + 1;
      node_type **newnext = nl.allocate(newlevel + 1);
      for (size_t i = 0; i < newlevel; i++) {
        newnext[i] = head_->next[i];
      }
      newnext[newlevel] = nullptr;
      nl.deallocate(head_->next, head_->level + 1);
      head_->next = newnext;
      head_->level = newlevel;
      update[newlevel] = head_;
    }
    nl.deallocate(n->next, 1);
    n->next = nl.allocate(newlevel + 1);
    n->level = newlevel;

    for (int i = newlevel; i >= 0; i--) {
      if (update[i]) {
        n->next[i] = update[i]->next[i];
        update[i]->next[i] = n;
      }
    }
    ++size_;
    nl.deallocate(update, update_size);
    return iterator(n);
  }

  iterator insert(const value_type &value) {
    return emplace(value.first, value.second);
  }

  iterator insert(value_type &&value) {
    return emplace(std::move(value.first), std::move(value.second));
  }

  iterator find(const key_type &key) const {
    iterator node = head_;
    for (int i = level(); i >= 0; i--) {
      while (node->next[i] && compare(node->next[i]->key, key)) {
        node = node->next[i];
      }
      if (node->next[i] && !compare(node->next[i]->key, key) && !compare(key, node->next[i]->key)) {
        return iterator(node->next[i]);
      }
    }
    return end();
  }

  size_t count(const key_type &key) const {
    return find(key) ? 1 : 0;
  }

  void erase(const key_type &key) {
    iterator node = head_;
    next_allocator nl;
    size_t update_size = level() + 1;
    node_type **update = nl.allocate(update_size);

    for (int i = level(); i >= 0; i--) {
      while (node->next[i] && compare(node->next[i]->key, key)) {
        node = node->next[i];
      }
      if (node->next[i] && !compare(node->next[i]->key, key) && !compare(key, node->next[i]->key)) {
        update[i] = node;
      }
    }
    node = node->next[0];
    if (node == end()) {
      nl.deallocate(update, update_size);
      throw std::out_of_range("skiplist::erase");
    }
    assert(!compare(node->key, key) && !compare(key, node->key));

    for (int i = level(); i >= 0; i--) {
      if (update[i]) {
        assert(node == iterator(update[i]->next[i]));
        update[i]->next[i] = node->next[i];
      }
    }
    nl.deallocate(update, update_size);
    destroy_node(node);
    --size_;

    if (level() > 0 && head_->next[level()] == end()) {
      head_->level--;
      next_allocator nl;
      node_type **newnext = nl.allocate(level() + 1);
      for (size_t i = 0; i <= level(); i++) {
        newnext[i] = head_->next[i];
      }
      newnext[level()] = nullptr;
      nl.deallocate(head_->next, level() + 2);
      head_->next = newnext;
    }
  }

  void erase(iterator it) {
    erase(it->key);
  }

  mapped_type &operator[](const key_type &key) {
    iterator node = find(key);
    if (node == end()) {
      mapped_type value;
      node = emplace(key, value);
    }
    return node->value;
  }

  mapped_type &at(const key_type &key) {
    iterator node = find(key);
    if (node == end()) {
      throw std::out_of_range("skiplist::at");
    }
    return node->value;
  }

  const mapped_type &at(const key_type &key) const {
    iterator node = find(key);
    if (node == end()) {
      throw std::out_of_range("skiplist::at");
    }
    return node->value;
  }

#ifndef NDEBUG
  void dump() const {
    std::cout << "====== level: " << level() << " size: " << size_ << std::endl;
    for (int i = level(); i >= 0; i--) {
      std::cout << "====== level " << i << std::endl;
      auto node = head_;
      while (node->next[i]) {
        std::cout << node->next[i]->key << " : " << node->next[i]->value
                  << std::endl;
        node = node->next[i];
      }
    }
  }
#endif

private:
  size_t size_;
  iterator head_;
  Compare compare;
  static size_t getRandomLevel() {
    size_t level = 0;
    while (rand() % 2) {
      ++level;
    }
    return level;
  }
  static iterator create_node() {
    node_allocator al;
    next_allocator nl;
    node_type *ret = al.allocate(1);
    al.construct(ret);
    ret->next = nl.allocate(1);
    ret->next[0] = nullptr;
    return static_cast<iterator>(ret);
  }
  static void destroy_node(const iterator &node) {
    node_allocator al;
    next_allocator nl;
    nl.deallocate(node->next, node->level + 1);
    al.destroy(static_cast<node_type *>(node));
    al.deallocate(static_cast<node_type *>(node), 1);
  }
};

} // namespace skiplist
} // namespace guoxiao
