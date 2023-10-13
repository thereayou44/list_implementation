#include <initializer_list>
#include <iostream>
#include <memory>

template <typename T, typename Allocator = std::allocator<T>>
class List {
 private:
  struct Node {
    T data = T();
    Node* prev = nullptr;
    Node* next = nullptr;

    Node(Node* left = nullptr, Node* right = nullptr)
        : prev(left), next(right) {}

    Node(const T& val, Node* left = nullptr, Node* right = nullptr)
        : data(val), prev(left), next(right) {}
  };

  using node_alloc =
      typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
  using node_alloc_traits = typename std::allocator_traits<node_alloc>;

  Node* base_node_ = nullptr;
  size_t size_ = 0;
  node_alloc nd_alloc_;
  // Allocator allocT;

  void gen_list(size_t size) {
    size_ = size;
    try {
      base_node_ = node_alloc_traits::allocate(nd_alloc_, 1);
      base_node_->next = base_node_->prev = base_node_;
    } catch (...) {
      node_alloc_traits::deallocate(nd_alloc_, base_node_, 1);
      throw;
    }

    if (size == 0) {
      return;
    }

    Node* first = base_node_;
    size_t idx = 0;
    try {
      for (; idx < size; idx++) {
        Node* second = node_alloc_traits::allocate(nd_alloc_, 1);

        first->next = second;
        second->prev = first;

        base_node_->prev = second;
        second->next = base_node_;

        first = second;
      }
    } catch (...) {
      for (size_t j = 0; j <= idx; j++) {
        Node* tmp = base_node_;
        base_node_ = base_node_->next;
        node_alloc_traits::deallocate(nd_alloc_, tmp, 1);
      }
      throw;
    }
  }

 public:
  using allocator_type = Allocator;
  using value_type = T;
  List() { gen_list(0); }
  explicit List(const Allocator& alloc) : nd_alloc_(alloc) { gen_list(0); }
  List(size_t count, const T& value, const Allocator& alloc = Allocator())
      : nd_alloc_(alloc) {
    try {
      gen_list(count);
    } catch (...) {
      throw;
    }
    Node* iter = base_node_->next;
    size_t cnt = 0;
    try {
      for (; iter != base_node_; iter = iter->next, cnt++) {
        node_alloc_traits::construct(nd_alloc_, iter, value, iter->prev,
                                     iter->next);
      }
    } catch (...) {
      base_node_ = base_node_->next;
      for (size_t i = 0; i < size_ + 1; i++) {
        Node* tmp = base_node_;
        base_node_ = base_node_->next;
        if (i < cnt) {
          node_alloc_traits::destroy(nd_alloc_, tmp);
        }
        node_alloc_traits::deallocate(nd_alloc_, tmp, 1);
      }

      base_node_ = nullptr, size_ = 0;
      throw;
    }
  }

  explicit List(size_t count, const Allocator& alloc = Allocator())
      : nd_alloc_(alloc) {
    try {
      gen_list(count);
    } catch (...) {
      throw;
    }
    Node* iter = base_node_->next;
    size_t cnt = 0;
    try {
      for (; iter != base_node_; iter = iter->next, cnt++) {
        node_alloc_traits::construct(nd_alloc_, iter, iter->prev, iter->next);
      }
    } catch (...) {
      base_node_ = base_node_->next;
      for (size_t i = 0; i < size_ + 1; i++) {
        Node* tmp = base_node_;
        base_node_ = base_node_->next;
        if (i < cnt) {
          node_alloc_traits::destroy(nd_alloc_, tmp);
        }
        node_alloc_traits::deallocate(nd_alloc_, tmp, 1);
      }

      base_node_ = nullptr, size_ = 0;
      throw;
    }
  }

  List(const List& other) {
    nd_alloc_ = node_alloc_traits::select_on_container_copy_construction(
        other.nd_alloc_);
    try {
      gen_list(other.size_);
    } catch (...) {
      throw;
    }
    Node* cur = base_node_->next;
    size_t cnt = 0;
    try {
      for (Node* other_cur = other.base_node_->next; cur != base_node_;
           cur = cur->next, other_cur = other_cur->next, cnt++) {
        node_alloc_traits::construct(nd_alloc_, cur, other_cur->data, cur->prev,
                                     cur->next);
      }
    } catch (...) {
      base_node_ = base_node_->next;
      for (size_t i = 0; i < other.size_ + 1; i++) {
        Node* tmp = base_node_;
        base_node_ = base_node_->next;
        if (i < cnt) {
          node_alloc_traits::destroy(nd_alloc_, tmp);
        }
        node_alloc_traits::deallocate(nd_alloc_, tmp, 1);
      }

      base_node_ = nullptr, size_ = 0;
      throw;
    }
  }

  List(std::initializer_list<T> init, const Allocator& alloc = Allocator())
      : nd_alloc_(alloc) {
    try {
      gen_list(init.size());
    } catch (...) {
      throw;
    }

    Node* cur = base_node_->next;
    size_t cnt = 0;
    try {
      for (const auto& value : init) {
        node_alloc_traits::construct(nd_alloc_, cur, value, cur->prev,
                                     cur->next);
        cur = cur->next;
        cnt++;
      }
    } catch (...) {
      base_node_ = base_node_->next;
      for (size_t i = 0; i < init.size() + 1; i++) {
        Node* tmp = base_node_;
        base_node_ = base_node_->next;
        if (i < cnt) {
          node_alloc_traits::destroy(nd_alloc_, tmp);
        }
        node_alloc_traits::deallocate(nd_alloc_, tmp, 1);
      }

      base_node_ = nullptr, size_ = 0;
      throw;
    }
  }
  ~List() {
    base_node_ = base_node_->next;
    for (size_t i = 0; i < size_; i++) {
      Node* tmp = base_node_;
      base_node_ = base_node_->next;
      node_alloc_traits::destroy(nd_alloc_, tmp);
      node_alloc_traits::deallocate(nd_alloc_, tmp, 1);
    }

    node_alloc_traits::deallocate(nd_alloc_, base_node_, 1);
    base_node_ = nullptr, size_ = 0;
  }

  List& operator=(const List& other) {
    if (this == &other) {
      return *this;
    }
    if (std::allocator_traits<
            Allocator>::propagate_on_container_copy_assignment::value) {
      nd_alloc_ = other.nd_alloc_;
    }

    List tmp(other);
    std::swap(tmp.size_, size_);
    std::swap(tmp.base_node_, base_node_);

    return *this;
  }

  allocator_type get_allocator() const { return nd_alloc_; }
  size_t size() const { return size_; }
  bool empty() const { return (size_ == 0); }
  void push_back(const T& value) {
    Node* tmp = node_alloc_traits::allocate(nd_alloc_, 1);
    try {
      node_alloc_traits::construct(nd_alloc_, tmp, value, base_node_->prev,
                                   base_node_);
    } catch (...) {
      node_alloc_traits::destroy(nd_alloc_, tmp);
      node_alloc_traits::deallocate(nd_alloc_, tmp, 1);
      throw;
    }
    if (size_ == 0) {
      base_node_->prev = tmp;
      base_node_->next = tmp;
    } else {
      base_node_->prev->next = tmp;
      base_node_->prev = tmp;
    }

    size_++;
  }
  void push_back(const T&& value) {
    Node* tmp = node_alloc_traits::allocate(nd_alloc_, 1);
    try {
      node_alloc_traits::construct(nd_alloc_, tmp, std::move(value),
                                   base_node_->prev, base_node_);
    } catch (...) {
      node_alloc_traits::destroy(nd_alloc_, tmp);
      node_alloc_traits::deallocate(nd_alloc_, tmp, 1);
      throw;
    }
    if (size_ == 0) {
      base_node_->prev = tmp;
      base_node_->next = tmp;
    } else {
      base_node_->prev->next = tmp;
      base_node_->prev = tmp;
    }

    size_++;
  }

  void push_front(const T& value) {
    Node* tmp = node_alloc_traits::allocate(nd_alloc_, 1);
    try {
      node_alloc_traits::construct(nd_alloc_, tmp, value, base_node_,
                                   base_node_->next);
    } catch (...) {
      node_alloc_traits::destroy(nd_alloc_, tmp);
      node_alloc_traits::deallocate(nd_alloc_, tmp, 1);
    }
    base_node_->next->prev = tmp;
    base_node_->next = tmp;

    size_++;
  }
  void push_front(const T&& value) {
    Node* tmp = node_alloc_traits::allocate(nd_alloc_, 1);
    try {
      node_alloc_traits::construct(nd_alloc_, tmp, std::move(value), base_node_,
                                   base_node_->next);
    } catch (...) {
      node_alloc_traits::destroy(nd_alloc_, tmp);
      node_alloc_traits::deallocate(nd_alloc_, tmp, 1);
      throw;
    }

    base_node_->next->prev = tmp;
    base_node_->next = tmp;
    size_++;
  }
  void pop_back() {
    if (size_ == 0) {
      return;
    }
    Node* to_delete = base_node_->prev;
    Node* new_last = base_node_->prev->prev;
    node_alloc_traits::destroy(nd_alloc_, to_delete);
    node_alloc_traits::deallocate(nd_alloc_, to_delete, 1);
    base_node_->prev = new_last;
    new_last->next = base_node_;
    size_--;
  }
  void pop_front() {
    if (size_ == 0) {
      return;
    }
    Node* to_delete = base_node_->next;
    Node* new_first = base_node_->next->next;
    node_alloc_traits::destroy(nd_alloc_, to_delete);
    node_alloc_traits::deallocate(nd_alloc_, to_delete, 1);
    new_first->prev = base_node_;
    base_node_->next = new_first;
    size_--;
  }
  T& front() { return *base_node_->next.data; }
  const T& front() const {
    return const_cast<const T&>(*base_node_->next.data);
  }
  T& back() { return *base_node_->prev.data; }
  const T& back() const { return const_cast<const T&>(*base_node_->prev.data); }

  template <bool IsConst>
  class CommonIterator {
   private:
    Node* node_;

   public:
    using value_type = typename std::conditional_t<IsConst, const T, T>;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;

    //        Node* getter() {
    //            return node_;
    //        }

    CommonIterator() : node_(nullptr) {}

    CommonIterator(Node* node) : node_(node) {}

    CommonIterator(const CommonIterator& other) : node_(other.node_) {}

    CommonIterator& operator=(const CommonIterator& other) {
      node_ = other.node_;
      return *this;
    }

    reference operator*() const { return node_->data; }

    pointer operator->() const { return &node_->data; }

    CommonIterator& operator++() {
      node_ = node_->next;
      return *this;
    }

    CommonIterator operator++(int) {
      CommonIterator old = *this;
      ++(*this);
      return old;
    }

    CommonIterator& operator--() {
      node_ = node_->prev;
      return *this;
    }

    CommonIterator operator--(int) {
      CommonIterator old = *this;
      --(*this);
      return old;
    }

    bool operator==(const CommonIterator& other) const {
      return node_ == other.node_;
    }

    bool operator!=(const CommonIterator& other) const {
      return !(*this == other);
    }
  };

  using iterator = CommonIterator<false>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_iterator = CommonIterator<true>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() { return iterator(base_node_->next); }
  iterator end() { return iterator(base_node_); }
  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }
  const_iterator cbegin() const { return const_iterator(base_node_->next); }
  const_iterator cend() const { return const_iterator(base_node_); }
  reverse_iterator rbegin() { return std::reverse_iterator<iterator>(end()); }
  reverse_iterator rend() { return std::reverse_iterator<iterator>(begin()); }
  const_reverse_iterator rbegin() const {
    return std::reverse_iterator<iterator>(cend());
  }

  const_reverse_iterator rend() const {
    return std::reverse_iterator<iterator>(cbegin());
  }

  const_reverse_iterator crbegin() const {
    return std::reverse_iterator<iterator>(cend());
  }

  const_reverse_iterator crend() const {
    return std::reverse_iterator<iterator>(cbegin());
  }
};

