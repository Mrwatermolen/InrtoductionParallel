// #include <iostream>
// #include <memory>
// #include <thread>
// #include <utility>

// template <typename T>
// class MyListNode {
//  public:
//   explicit MyListNode(T data, std::shared_ptr<MyListNode<T>> next = nullptr,
//                       std::shared_ptr<MyListNode<T>> prev = nullptr)
//       : _data(data), _next(std::move(next)), _prev(std::move(prev)) {
//     std::cout << "Node Constructor\n";
//   }

//   ~MyListNode() {
//     if (_next) {
//       _next->_prev = _prev;
//     }
//     if (_prev) {
//       _prev->_next = _next;
//     }

//     std::cout << "Node Destructor\n";
//   }

//   const std::shared_ptr<const MyListNode<T>>& next() const { return _next; }

//   const std::shared_ptr<const MyListNode<T>>& prev() const { return _prev; }

//   const T& data() const { return _data; }

//   std::shared_ptr<MyListNode<T>>& next() { return _next; }

//   std::shared_ptr<MyListNode<T>>& prev() { return _prev; }

//   T& data() { return _data; }

//  private:
//   T _data;
//   std::shared_ptr<MyListNode<T>> _next{nullptr};
//   std::shared_ptr<MyListNode<T>> _prev{nullptr};
// };

// template <typename T>
// class MyList {
//  public:
//   ~MyList<T>() {
//     std::cout << "List Destructor\n";
//     _head = nullptr;
//     _tail = nullptr;
//   }

//   void insertHead(T data) {
//     auto n = (_head) ? (_head) : nullptr;
//     auto p = (_head) ? (_head->prev()) : nullptr;
//     auto node =
//         std::make_shared<MyListNode<T>>(data, std::move(n), std::move(p));
//     if (_head) {
//       _head->prev() = node;
//     }
//     _head = node;
//     if (!_tail) {
//       _tail = node;
//     }
//   }

//   std::string toString() {
//     std::string s;
//     auto node = _head;
//     while (node) {
//       s += std::to_string(node->data()) + " ";
//       node = node->next();
//     }
//     return s;
//   }

//  private:
//   std::shared_ptr<MyListNode<T>> _head{nullptr}, _tail{nullptr};
// };

// int main() {
//   auto l = MyList<int>();
//   for (auto i = 0; i < 8; ++i) {
//     l.insertHead(i);
//   }
//   std::cout << l.toString() << "\n";
//   return 0;
// }
