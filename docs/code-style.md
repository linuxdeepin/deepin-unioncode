## 基本原则

- C++ 类设计的代码风格主要依据  [Qt Coding Style](https://wiki.qt.io/Qt_Coding_Style)

- 以 [google C++ style](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/contents/)  作为补充说明

## 线程安全

准则如下：

  -  默认自定义任何函数、类、容器、变量等都不属于没有线程安全保障，需要在类注释中标识当前函数、类容器、变量等【线程安全】才能确认其线程安全。否则一律按照没有线程安全处理。
  -  线程外部变量在线程内使用时需要加锁。
  -  Qt默认规则，界面类不可在主线程外执行构造与析构。
  -  容器批量操作可采用QConcurrent::map进行操作（此函数线程安全）。
  -  禁用UT线程，与程序内部线程冲突是未定义行为，如果测试类函数等为线程安全，则可使用多线程使能UT。
  -  不建议线程直接对QObject派生类直接操作，因为在Close或者其他Qt释放派生类时，面向线程的QObject为已释放，此时如果使用该实例指针，实则为操作野指针（必定崩溃）。如果需要操作QObject类时。可通过信号槽转发的方式确保线程安全
  -  线程操作数据建议继承QSharedData数据类，在Qt上游的代码中，但凡使用线程的类数据。均非QObject的基础类型数据。
  -  QCoreApplication在析构时，默认等待全局容器30ms线程执行，此处30ms作为临界值是完全够用的（肉眼卡顿24ms）。在全局线程池使用中，应当遵循该准则（QConcurrent的任意模块与方法）。在构造相关全局线程处可调用框架时间执行检查，以方便生成报告与界面卡顿的追述文件。


## 头文件

### #define 保护

- 所有头文件都应该使用 `#define` 来防止头文件被多重包含，命名格式为 `PREFIX_H`

  ```c++
  // opticaldisc.h
  #ifndef OPTICALDISCINFO_H
  #define OPTICALDISCINFO_H
  
  // ...
  
  #endif // OPTICALDISCINFO_H
  ```

### 内联函数

- 只有当函数只有 10 行甚至更少时才将其定义为内联函数.

### `#include` 的路径及顺序

- 项目内头文件应按照项目源代码目录树结构排列, 避免使用 UNIX 特殊的快捷目录 `.` (当前目录) 或 `..` (上级目录)

  ```C++
  /*
  * 目录树
  *   --- project
  *   	--- base
  *   		--- logging.h
  *   	--- util
  *   		--- util.h
  */
  
  /// util.h 头文件
  // 坏 -> 包含了快捷目录
  #include "../base/logging.h"
  
  // 好 -> 使用源码目录树
  #include "base/logging.h"
  ```

- 一个 cpp 文件中，引入头文件的顺序如下

  ​         区分划分示例：
  ​         // 区域1：项目自身文件
  ​         #include "xxx.h"
  ​         // 区域2：DTK库文件
  ​         #include <DApplication>
  ​         // 区域3：Qt库文件
  ​         #include <QObject>
  ​         // 区域4：其它库文件
  ​         #include <xxx.h>
  ​         // 区域5：CPP标准库文件
  ​         #include <iostream>
  ​         // 区域6：Linux系统库文件
  ​         #include <unistd.h>

  

  总结就是 **include 中越上层的代码越靠上，越偏底层的头文件越往下放**

- 按字母顺序分别对每种类型的头文件进行二次排序

- 例外：有时，平台特定（system-specific）代码需要条件编译（conditional includes），这些代码可以放到其它 includes 之后

  ```c++
  #include "foo/public/fooserver.h"
  
  #include "base/port.h"  // For LANG_CXX11.
  
  #ifdef LANG_CXX11
  #include <initializer_list>
  #endif  // LANG_CXX11
  ```

## 作用域

### 命名空间

- 每个类必须有命名空间，在统一的头文件中使用宏封装命名空间，在代码中使用宏来使用命名空间

  ```c++
  // dfm-burn.h
  // namespace
  #define BEGIN_BURN_NAMESPACE  namespace dfm_burn {
  #define END_BURN_NAMESPACE    }
  #define USING_BURN_NAMESPACE  using namespace dfm_burn;
  ```

- 代码中可以根据业务需求自定义命名空间，但要注意

  - 小写 
  - 没有缩进
  - 最后大括号使用命名空间名称注释

  ```c++
  // 在 .h 中使用别名缩短常用的命名空间
  namespace librarian {
  namespace impl {  // 仅限内部使用
  namespace sidetable = ::pipeline_diagnostics::sidetable;
  }  // namespace impl
  
  inline void my_inline_function() {
    // 限制在一个函数中的命名空间别名
    namespace baz = ::foo::bar::baz;
    ...
  }
  }  // namespace librarian
  ```

### 匿名命名空间和静态变量

- 在 `.cpp` 文件中定义一个不需要被外部引用的变量时，可以将它们放在匿名命名空间或声明为 `static` , 但是不要在 `.h` 文件中这么做

  ```c++
  // *.cpp
  // 定义一个不需要被外部引用的变量, 有两种方式：
  
  // [1] static 声明
  static const char *const Str = "string";
  
  // [2] 匿名命名空间
  namespace {
      const char *const Str = "string";
  } // namespace
  
  ```

### 非成员函数、静态成员函数和全局函数

- 使用静态成员函数或命名空间内的非成员函数, 尽量不要用裸的全局函数

- 一系列函数直接置于命名空间中，不要用类的静态方法模拟出命名空间的效果，**类的静态方法应当和类的实例或静态数据紧密相关**

  ```c++
  // 好 -> 函数的定义同类的实例脱钩
  // 函数可以被定义成静态成员, 或是非成员函数. 非成员函数不应依赖于外部变量
  namespace myproject {
  namespace foo_bar {
  void Function1();
  void Function2();
  }  // namespace foo_bar
  }  // namespace myproject
  
  // 坏 -> 单纯为了封装若干不共享任何静态数据的静态成员函数而创建类
  namespace myproject {
  class FooBar {
   public:
    static void Function1();
    static void Function2();
  };
  }  // namespace myproject
  ```

### 局部变量

- 将函数变量尽可能置于最小作用域内, 并在变量声明时进行初始化.

  ```c++
  int i;
  i = f(); // 坏 -> 初始化和声明分离
  
  int j = g(); // 好 -> 初始化时声明
  
  vector<int> v;
  v.push_back(1); // 坏 -> 用花括号初始化更好
  v.push_back(2);
  
  vector<int> v = {1, 2}; // 好 -> v 一开始就初始化
  ```

- 属于 `if`, `while` 和 `for` 语句的变量应当在这些语句中正常地声明，这样子这些变量的作用域就被限制在这些语句中了

  ```c++
  while (const char* p = strchr(str, '/')) str = p + 1;
  ```

- 例外：如果变量是一个对象, 每次进入作用域都要调用其构造函数, 每次退出作用域都要调用其析构函数. 这会导致效率降低

  ```c++
  // 坏 -> 低效的实现
  for (int i = 0; i < 1000000; ++i) {
      Foo f;                  // 构造函数和析构函数分别调用 1000000 次!
      f.DoSomething(i);
  }
  
  // 好 -> 构造函数和析构函数只调用 1 次
  Foo f;                      
  for (int i = 0; i < 1000000; ++i) {
      f.DoSomething(i);
  }
  ```

### 静态和全局变量

- 禁止使用类的**静态储存周期变量**：由于构造和析构函数调用顺序的不确定性，它们会导致难以发现的 bug 。不过 `constexpr` 变量除外，毕竟它们又不涉及动态初始化或析构

  > 静态生存周期的对象，即包括了全局变量，静态变量，静态类成员变量和函数静态变量，都必须是原生数据类型 (POD : Plain Old Data): 即 int, char 和 float, 以及 POD 类型的指针、数组和结构体

## 类

### 构造函数的职责

- 不要在构造函数中调用虚函数
- 不要在无法报出错误时进行可能失败的初始化, 否则, 考虑用 `init()` 方法或工厂函数

### 结构体 & 类

- 仅当只有数据成员时使用 `struct`, 其它一概使用 `class`

### 使用私有类

- 私有类能良好的保证 ABI，请参考 Qt 私有类的实现方式

### 模板类

- 避免使用模板类的多次继承套用，因为其会是代码可阅读性变低。
- 顶层模板类应当存在功能性标识，如Factory
- 类传递模板参数可用T，返回值RT，变参模板 ...Args 如非必要，避免使用
- 函数传递模板参数应当与传递结构关联，如Functor
- 正确做法如下：
```cpp
template<class T> // 禁用typename区别于标准库
class SchemeFacotry{};

class DFMFileInfo{};

// 最终构造结果应与非模板类看齐
// 此处特化为类避免模板代码纠缠，同时保持类的普通性
class DFMFileInfoSchemeFacotry : public SchemeFacotry<DFMFileInfo>{};
```

## 函数

### 参数顺序

- 数的参数顺序为: 输入参数在先, 后跟输出参数

- 输入参数通常是值参或 `const` 引用, 输出参数或输入/输出参数则一般为非 `const` 指针

  ```c++
  void func(int a, const Type &b. Type *out);
  ```

### 引用参数

- 所有按左值引用传递的参数必须加上 `const`

  ```c++
  void foo(const string &in, string *out);
  ```

## 命名

### 通用规则

- 函数命名, 变量命名, 文件命名要有描述性; 少用缩写

  ```c++
  // 尽可能使用描述性的命名, 别心疼空间, 毕竟相比之下让代码易于新读者理解更重要. 
  // 不要用只有项目开发者能理解的缩写, 也不要通过砍掉几个字母来缩写单词
  
  // 好
  int priceCountReader;    // 无缩写
  int numErrors;            // "num" 是一个常见的写法
  int numDnsConnections;   // 人人都知道 "DNS" 是什么
  
  // 坏
  int n;                     // 毫无意义.
  int nerr;                  // 含糊不清的缩写.
  int nCompConns;            // 含糊不清的缩写.
  int wgcConnections;        // 只有贵团队知道是什么意思.
  int pcReader;              // "pc" 有太多可能的解释了.
  int cstmID;                // 删减了若干字母.
  ```

- 一些特定的广为人知的缩写是允许的, 例如用 `i` 表示迭代变量和用 `T` 表示模板参数

### 文件命名

- 文件名要全部小写, 可以包含下划线 (`_`) 
  - `myusefulclass.cpp`
  - `my-useful_class.cpp`
- 普通类头文件后缀为 `.h`
- 所有功能内联实现的头文件后缀为 `.hpp`
- c++ 文件后缀为 `.cpp`
- 单元测试文件前缀为 `ut_`

### 类型命名

- 大驼峰命名：类型名称的每个单词首字母均大写, 不包含下划线

  ```c++
  // 类和结构体
  class UrlTable { ...
  class UrlTableTester { ...
  struct UrlTableProperties { ...
  
  // 类型定义
  typedef hash_map<UrlTableProperties *, string> PropertiesMap;
  
  // using 别名
  using PropertiesMap = hash_map<UrlTableProperties *, string>;
  
  // 枚举
  enum UrlTableErrors { ...
  ```

- 抽象类使用 `Absract` 为前缀命名

### 变量命名

- 小驼峰命名：变量 (包括函数参数) 和数据成员名一律小写, 不包含下划线（d_ptr除外）

  ```c++
  string tableName;
  
  class TableInfo {
    ...
   private:
    string tableName; // 不要使用 m_ 前缀 
  };
  ```

### 常量命名

- 声明为 `constexpr` 或 `const` 的变量, 或在程序运行期间其值始终保持不变的,  大小写混合

  ```C++
  const int kDaysInAWeek = 7;
  ```

### 函数命名

- 小驼峰命名

  ```c++
  addTableEntry();
  deleteUrl();
  openFileOrDie();
  ```

### 枚举命名

- 枚举的命名应当和 [常量](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/naming/#constant-names) 或 [宏](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/naming/#macro-names) 一致: `EnumName` 或是 `ENUM_NAME`

- 单独的枚举值应该优先采用 [常量](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/naming/#constant-names) 的命名方式命名

  ```c++
  enum UrlTableErrors {
      kOK = 0,
      kErrorOutOfMemory,
      kErrorMalformedInput,
  };
  ```

### 宏命名

- 尽量不使用宏，如果你一定要用，像这样命名: `MY_MACRO_THAT_SCARES_SMALL_CHILDREN`

## 注释

### 文件头

- 每个代码文件的开头请加上开源协议头（将 xx 改为自己的昵称，时间根据当前的时间灵活修改）

  ```c++
  /*
   * Copyright (C) 2021 Deepin Technology Co., Ltd.
   *
   * Author:     xx <xx@uniontech.com>
   *
   * Maintainer: xx <xx@uniontech.com>
   *
   * This program is free software: you can redistribute it and/or modify
   * it under the terms of the GNU General Public License as published by
   * the Free Software Foundation, either version 3 of the License, or
   * any later version.
   *
   * This program is distributed in the hope that it will be useful,
   * but WITHOUT ANY WARRANTY; without even the implied warranty of
   * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   * GNU General Public License for more details.
   *
   * You should have received a copy of the GNU General Public License
   * along with this program.  If not, see <http://www.gnu.org/licenses/>.
   */
  ```

- 如果你使用的是 QtCreator，将 License 保存为文件，在<u>“工具 -> 选项 ->C++“</u>中的 <u>License_template</u> 进行添加
### 类注释

- 类注释在类定义文件（cpp）中，位于它的第一个成员函数之前

-  每个类的定义都要附带一份注释, 描述类的功能和用法, 除非它的功能相当明显(如私有类)

  ```C++
  #include <ddiskmanager.h>
  #include <dblockdevice.h>
  #include <ddiskdevice.h>
  #include <QStorageInfo>
  #include <QtConcurrentRun>
  
  DFM_USE_NAMESPACE
  
  ErrorHandleInfc::~ErrorHandleInfc()
  {
  }
  
  
  /*!
   * \class DAttachedUdisks2Device
   *
   * \brief An attached (mounted) block device (partition)
   */
  
  DAttachedUdisks2Device::DAttachedUdisks2Device(const DBlockDevice *blockDevicePointer)
  {
      QByteArrayList mountPoints = blockDevicePointer->mountPoints();
      mountPoint = mountPoints.isEmpty() ? "" : mountPoints.first();
      deviceDBusId = blockDevicePointer->path();
      c_blockDevice.reset(DDiskManager::createBlockDevice(deviceDBusId)); // not take the ownership of the passed pointer.
  }
  
  ```

  

### 命名空间、枚举注释

- 命名空间要看名知意，如果需要注释说明，形式和类注释一样

- 命名空间要在最后的大括号收注释命名空间名称（匿名明明空间只需写 namespace）

- 枚举注释的说明与类注释类似，如果枚举向不能见名知意，成员需要在行后加说明文字

  ```C++
  /*!
  *  \brief ...
  */
  namespace qt {
  ...
  } // namespace qt
  
  namespace {
  ...   
  } // namespace 
  
  /*!
  *  \brief ...
  */ 
  enum AppItemRole AppItemRole {
      成员1, // 说明文字
      成员2, // 说明文字
      成员3, // 说明文字
  }
  ```

### 函数注释

- 函数声明处的注**释描述函数功能**; 定义处的注释描述函数实现.

- 基本上每个函数**定义处**前都应当加上注释, 描述函数的功能和用途. 只有在函数的功能简单而明显时才能省略这些注释(例如, 简单的取值和设值函数)

  ```c++
  /*!
   * \brief Gets the identifier of the given kind for drive.
   *
   * Wrapper of Gio::Drive::get_identifier()
   *
   * The only identifier currently available is DGIODRIVE_IDENTIFIER_KIND_UNIX_DEVICE.
   *
   * \param kind the kind of identifier to return
   *
   * \return A string containing the requested identfier, or empty string if the drive doesn't have this kind of identifier.
   */
  QString DGioDrive::identifier(const QString &kind) const
  {
      Q_D(const DGioDrive);
  
      return QString::fromStdString(d->getGmmDriveInstence()->get_identifier(kind.toStdString()));
  }
  ```

### 变量注释

- 通常变量名本身足以很好说明变量用途. 某些情况下, 也需要额外的注释说明.

  ```c++
  // 简要说明
  int failNum;
  
  // 需要详细说明的
  /*!
  * /brief 成员变量variable要说明
  *
  * 成员变量variable的详细说明，这里可以对变量进行
  * 详细的说明和描述，具体方法和函数的标注是一样的
  */
  bool variable;
  ```

### 实现注释

- 代码前：对于代码中巧妙的, 晦涩的, 有趣的, 重要的地方加以注释

  ```c++
  // Divide result by two, taking into account that x
  // contains the carry from the add.
  for (int i = 0; i < result->size(); i++) {
    x = (x << 8) + (*result)[i];
    (*result)[i] = x >> 1;
    x &= 1;
  }
  ```

- 行注释：比较隐晦的地方要在行尾加入注释. 在行尾空两格进行注释

  ```c++
  // If we have enough memory, mmap the data portion too.
  mmap_budget = max<int64>(0, mmap_budget - index_->length());
  if (mmap_budget >= data_size_ && !MmapData(mmap_chunk_bytes, mlock))
    return;  // Error already logged.
  ```

- 如果你需要连续进行多行注释, 可以使之对齐获得更好的可读

  ```c++
  DoSomething();                  // Comment here so the comments line up.
  DoSomethingElseThatIsLonger();  // Two spaces between the code and the comment.
  { // One space before comment when opening a new scope is allowed,
    // thus the comment lines up with the following comments and code.
    DoSomethingElse();  // Two spaces before line comments normally.
  }
  std::vector<string> list{
                      // Comments in braced lists describe the next element...
                      "First item",
                      // .. and should be aligned appropriately.
  "Second item"};
  DoSomething(); /* For trailing block comments, one space is fine. */
  ```

### TODO 注释

- 对那些临时的, 短期的解决方案, 或已经够好但仍不完美的代码使用 `TODO` 注释.

- `TODO` 注释要使用全大写的字符串 `TODO`, 在随后的圆括号里写上你的名字, 邮件地址, bug ID, 或其它身份标识和与这一 `TODO` 相关的 issue.

  ```C++
  // TODO(zhangsheng@uniontech.com): Use a "*" here for concatenation operator.
  // TODO(zhangsheng) change this to use relations.
  // TODO(bug-12345): remove the "Last visitors" feature
  ```

  

## 格式

### 行长度

- 每一行代码字符数不超过 80

### 缩进

- 4个空格用于缩进
- 空格，而不是制表符！

### 声明变量

- 在单独的行中声明每个变量

- 避免使用简短或无意义的名称（例如“ a”，“ rbarr”，“ nughdeget”）

- 单字符变量名称仅适用于计数器和临时变量，其中变量的用途显而易见

- 在声明变量之前等待，直到需要它为止

  ```c++
  // 坏
  int a, b;
  char *c, *d;
  
  // 好
  int height;
  int width;
  char *nameOfThis;
  char *nameOfThat;
  ```

### 空格

- 使用空行将语句组合在一起

- 始终只使用一个空白行

- 始终在关键字之后和花括号前使用一个空格

  ```c++
   // 坏
   if(foo){
   }
  
   // 好
   if (foo) {
   }
  ```

- 对于指针或引用，请始终在类型和’*‘或’＆‘之间使用单个空格，但在’*‘或’＆'与变量名称之间请勿使用空格

  ```c++
   char *x;
   const QString &myString;
   const char * const y = "hello";
  ```

- 在每个逗号后留一个空格

- 尽可能避免使用C型转换

  ```c++
  // 坏
  char* blockOfMemory = (char* ) malloc(data.size());
  
  // 好
  char *blockOfMemory = reinterpret_cast<char *>(malloc(data.size()));
  ```

- 不要在一行上放置多个语句

- 通过扩展，对控制流语句的主体使用新行

  ```c++
  // 坏
  if (foo) bar();
  
  // 好
  if (foo)
      bar();
  ```

### 大括号

- 使用附带的括号：开头的括号与语句的开头在同一行。如果右括号后面紧跟着另一个关键字，则它也会进入同一行

  ```c++
  // 坏
  if (codec)
  {
  }
  else
  {
  }
  
  // 好
  if (codec) {
  } else {
  }
  ```

- 例外：函数实现（但不包括lambdas）和类声明始终在行首处使用左括号

  ```c++
  static void foo(int g)
  {
      qDebug("foo: %i", g);
  }
  
  class Moo
  {
  };
  ```

- 仅当条件语句的主体包含多行时才使用花括号

  ```c++
  // 坏
  if (address.isEmpty()) {
      return false;
  }
  
  for (int i = 0; i < 10; ++i) {
      qDebug("%i", i);
  }
  
  // 好
  if (address.isEmpty())
      return false;
  
  for (int i = 0; i < 10; ++i)
      qDebug("%i", i);
  ```

  - 例外1：如果父语句包含多行/自动换行，也请使用花括号

    ```c++
     // 好
     if (address.isEmpty() || !isValid()
         || !codec) {
         return false;
     }
    ```

  - 例外2：花括号对称：在if-then-else块中，如果if代码或else代码覆盖几行，也要使用花括号

    ```c++
     // 坏
     if (address.isEmpty())
         qDebug("empty!");
     else {
         qDebug("%s", qPrintable(address));
         it;
     }
    
     // 好
     if (address.isEmpty()) {
         qDebug("empty!");
     } else {
         qDebug("%s", qPrintable(address));
         it;
     }
    
     // 坏
     if (a)
         …
     else
         if (b)
             …
    
     // 好
     if (a) {
         …
     } else {
         if (b)
             …
     }
    ```

- 条件语句的主体为空时使用花括号

  ```c++
  // 坏
  while (a);
  
  // 好
  while (a) {}
  ```

### 小括号

- 使用小括号将表达式分组

  ```c++
  // 坏
  if (a && b || c)
  
  // 好
  if ((a && b) || c)
  
  // 坏
  a + b & c
  
  // 好
  (a + b) & c
  ```

### 切换语句（switch）

- `case`标签与开关在同一列中

- 每个`case`的末尾都必须有一个`break`（或`return`）语句，或者Q_FALLTHROUGH()用来表明没有故意的中断，除非立即发生另一`case`。

  ```c++
  switch (myEnum) {
  case Value1:
    doSomething();
    break;
  case Value2:
  case Value3:
    doSomethingElse();
    Q_FALLTHROUGH();
  default:
    defaultHandling();
    break;
  }
  ```

### 跳转语句（return、break、continue）

- 不要在跳转语句后加上`else`

  ```c++
  // 坏
  if (thisOrThat)
      return;
  else
      somethingElse();
  
  // 好
  if (thisOrThat)
      return;
  somethingElse();
  ```

### Lambda 表达式

- **禁止在 Qt 的槽函数使用 lambda 表达式写复杂的业务逻辑!!!(**因为崩溃不好分析, ut 不好覆盖) ，简单的场景允许使用（理论上不超过 10 行）

- lambda 表达式对形参和函数体的格式化和其他函数一致; 捕获列表同理, 表项用逗号隔开

- lambda 尽量不要写太长

  ```c++
  int x = 0;
  auto add_to_x = [&x](int n) { x += n; };
  
  std::set<int> blacklist = {7, 8, 9};
  std::vector<int> digits = {3, 9, 1, 8, 4, 7, 1};
  digits.erase(std::remove_if(digits.begin(), digits.end(), [&blacklist](int i) {
                 return blacklist.find(i) != blacklist.end();
               }),
               digits.end());
  ```

  



## 其他 C++ 特性

均参考 [google C++ style-6](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/others/#id12)