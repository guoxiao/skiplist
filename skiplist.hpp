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

#include <vector>
#include <assert.h>
#include <cstdlib>
#include <stdexcept>
#include <functional>
#include <memory>
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
  std::vector<SkipNode *> next;
  SkipNode() : key(), value(), level(0), next(1) {}
  SkipNode(SkipNode &&rhs) : key(), value(), level(rhs.level), next(std::move(rhs.next)) {}

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
  typedef SkipNode<Key, T> node_type;
  typedef Iterator<node_type> iterator;
  typedef Iterator<node_type> const_iterator;
  typedef typename std::allocator_traits<Allocator>::template rebind_alloc<node_type> node_allocator;

  SkipList() : size_(0), level_(0), head_(create_node()) {}

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
                                    level_(s.level_),
                                    head_(s.head_) {
    s.head_ = create_node();
    s.level_ = 0;
    s.size_ = 0;
  }

  // Copy Ctor
  SkipList(const SkipList &s)
    : size_(s.size_), level_(s.level_), head_(create_node()) {
    iterator snode = s.head_, node = head_;
    std::vector<iterator> last(level_ + 1);
    head_->level = level_;
    head_->next.resize(level_ + 1);
    for (int i = level_; i >= 0; i--) {
      last[i] = head_;
    }
    snode = snode->next[0];
    while (snode) {
      node = create_node();
      node->key = snode->key;
      node->value = snode->value;
      node->level = snode->level;
      node->next.resize(snode->next.size());
      for (int i = snode->level; i >= 0; i--) {
        last[i]->next[i] = node;
        last[i] = node;
      }
      snode = snode->next[0];
    }
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
    level_ = s.level_;
    head_ = s.head_;
    s.head_ = create_node();
    s.level_ = 0;
    s.size_ = 0;
    return *this;
  }

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }
  size_t level() const { return level_; }

  iterator begin() noexcept { return iterator(head_->next[0]); }
  const_iterator begin() const noexcept { return iterator(head_->next[0]); }

  const_iterator cbegin() noexcept { return iterator(head_->next[0]); }
  const_iterator cend() const noexcept { return iterator(); }

  iterator end() noexcept { return iterator(); }
  const_iterator end() const noexcept { return iterator(); }

  template <typename K, typename V>
  iterator emplace(K &&key, V &&value) {
    iterator node = head_;
    std::vector<iterator> update(level_ + 1 + 1);
    for (int i = level_; i >= 0; i--) {
      assert(static_cast<int>(node->level) >= i);
      while (node->next[i] && compare(node->next[i]->key, key)) {
        node = node->next[i];
      }
      update[i] = node;
      if (node->next[i] && node->next[i]->key == key) {
        throw std::runtime_error("conflict");
      }
    }

    node_type *n = create_node();
    n->key = std::forward<K>(key);
    n->value = std::forward<V>(value);

    size_t level = getRandomLevel();
    if (level > level_) {
      level = level_ + 1;
      head_->next.resize(level + 1);
      head_->level = level;
      update[level] = head_;
      level_ = level;
    }
    n->next.resize(level + 1);
    n->level = level;

    for (int i = level; i >= 0; i--) {
      if (update[i]) {
        n->next[i] = update[i]->next[i];
        update[i]->next[i] = n;
      }
    }
    ++size_;
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
    for (int i = level_; i >= 0; i--) {
      while (node->next[i] && compare(node->next[i]->key, key)) {
        node = node->next[i];
      }
      if (node->next[i] && node->next[i]->key == key) {
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
    std::vector<iterator> update(level_ + 1);

    for (int i = level_; i >= 0; i--) {
      while (node->next[i] && compare(node->next[i]->key, key)) {
        node = node->next[i];
      }
      if (node->next[i] && node->next[i]->key == key) {
        update[i] = node;
      }
    }
    node = node->next[0];
    if (node == end()) {
      throw std::out_of_range("skiplist::erase");
    }
    assert(node->key == key);

    for (int i = level_; i >= 0; i--) {
      if (update[i]) {
        assert(node == iterator(update[i]->next[i]));
        update[i]->next[i] = node->next[i];
      }
    }

    destroy_node(node);
    --size_;

    if (level_ > 0 && head_->next[level_] == end()) {
      level_--;
      head_->level = level_;
      head_->next.resize(level_ + 1);
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
    std::cout << "====== level: " << level_ << " size: " << size_ << std::endl;
    for (int i = level_; i >= 0; i--) {
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
  size_t level_;
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
    node_type *ret = al.allocate(1);
    al.construct(ret);
    return static_cast<iterator>(ret);
  }
  static void destroy_node(const iterator &node) {
    node_allocator al;
    al.destroy(static_cast<node_type *>(node));
    al.deallocate(static_cast<node_type *>(node), 1);
  }
};

} // namespace skiplist
} // namespace guoxiao
