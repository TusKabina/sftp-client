/****************************************************************************
** Meta object code from reading C++ file 'SimpleMenu.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainGui.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SimpleMenu.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ThreadPool_t {
    QByteArrayData data[4];
    char stringdata0[49];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ThreadPool_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ThreadPool_t qt_meta_stringdata_ThreadPool = {
    {
QT_MOC_LITERAL(0, 0, 10), // "ThreadPool"
QT_MOC_LITERAL(1, 11, 8), // "WorkDone"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 27) // "EventFromThreadPoolReceived"

    },
    "ThreadPool\0WorkDone\0\0EventFromThreadPoolReceived"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ThreadPool[] = {

    // content:
          7,       // revision
          0,       // classname
          0,    0, // classinfo
          2,   14, // methods
          0,    0, // properties
          0,    0, // enums/sets
          0,    0, // constructors
          0,       // flags
          1,       // signalCount

          // signals: name, argc, parameters, tag, flags
                1,    1,   24,    2, 0x06 /* Public */,

                // slots: name, argc, parameters, tag, flags
                      3,    1,   27,    2, 0x0a /* Public */,

                      // signals: parameters
                         QMetaType::Void, QMetaType::Int,    2,

                         // slots: parameters
                            QMetaType::Void, QMetaType::Int,    2,

                               0        // eod
};

void ThreadPool::qt_static_metacall(QObject* _o, QMetaObject::Call _c, int _id, void** _a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ThreadPool* _t = static_cast<ThreadPool*>(_o);
        Q_UNUSED(_t)
            switch (_id) {
            case 0: _t->WorkDone((*reinterpret_cast<int(*)>(_a[1]))); break;
            case 1: _t->EventFromThreadPoolReceived((*reinterpret_cast<int(*)>(_a[1]))); break;
            default:;
            }
    }
    else if (_c == QMetaObject::IndexOfMethod) {
        int* result = reinterpret_cast<int*>(_a[0]);
        void** func = reinterpret_cast<void**>(_a[1]);
        {
            typedef void (ThreadPool::* _t)(int);
            if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&ThreadPool::WorkDone)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject ThreadPool::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ThreadPool.data,
      qt_meta_data_ThreadPool,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject* ThreadPool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void* ThreadPool::qt_metacast(const char* _clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ThreadPool.stringdata0))
        return static_cast<void*>(const_cast<ThreadPool*>(this));
    return QWidget::qt_metacast(_clname);
}

int ThreadPool::qt_metacall(QMetaObject::Call _c, int _id, void** _a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ThreadPool::WorkDone(int _t1)
{
    void* _a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_MyThread_t {
    QByteArrayData data[3];
    char stringdata0[19];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MyThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MyThread_t qt_meta_stringdata_MyThread = {
    {
QT_MOC_LITERAL(0, 0, 8), // "MyThread"
QT_MOC_LITERAL(1, 9, 8), // "WorkDone"
QT_MOC_LITERAL(2, 18, 0) // ""

    },
    "MyThread\0WorkDone\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MyThread[] = {

    // content:
          7,       // revision
          0,       // classname
          0,    0, // classinfo
          1,   14, // methods
          0,    0, // properties
          0,    0, // enums/sets
          0,    0, // constructors
          0,       // flags
          1,       // signalCount

          // signals: name, argc, parameters, tag, flags
                1,    1,   19,    2, 0x06 /* Public */,

                // signals: parameters
                   QMetaType::Void, QMetaType::Int,    2,

                      0        // eod
};

void MyThread::qt_static_metacall(QObject* _o, QMetaObject::Call _c, int _id, void** _a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MyThread* _t = static_cast<MyThread*>(_o);
        Q_UNUSED(_t)
            switch (_id) {
            case 0: _t->WorkDone((*reinterpret_cast<int(*)>(_a[1]))); break;
            default:;
            }
    }
    else if (_c == QMetaObject::IndexOfMethod) {
        int* result = reinterpret_cast<int*>(_a[0]);
        void** func = reinterpret_cast<void**>(_a[1]);
        {
            typedef void (MyThread::* _t)(int);
            if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&MyThread::WorkDone)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject MyThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_MyThread.data,
      qt_meta_data_MyThread,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject* MyThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void* MyThread::qt_metacast(const char* _clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MyThread.stringdata0))
        return static_cast<void*>(const_cast<MyThread*>(this));
    return QThread::qt_metacast(_clname);
}

int MyThread::qt_metacall(QMetaObject::Call _c, int _id, void** _a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void MyThread::WorkDone(int _t1)
{
    void* _a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_TreeView_t {
    QByteArrayData data[5];
    char stringdata0[46];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TreeView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TreeView_t qt_meta_stringdata_TreeView = {
    {
QT_MOC_LITERAL(0, 0, 8), // "TreeView"
QT_MOC_LITERAL(1, 9, 16), // "RightClickAction"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 12), // "QMouseEvent*"
QT_MOC_LITERAL(4, 40, 5) // "event"

    },
    "TreeView\0RightClickAction\0\0QMouseEvent*\0"
    "event"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TreeView[] = {

    // content:
          7,       // revision
          0,       // classname
          0,    0, // classinfo
          1,   14, // methods
          0,    0, // properties
          0,    0, // enums/sets
          0,    0, // constructors
          0,       // flags
          1,       // signalCount

          // signals: name, argc, parameters, tag, flags
                1,    1,   19,    2, 0x06 /* Public */,

                // signals: parameters
                   QMetaType::Void, 0x80000000 | 3,    4,

                      0        // eod
};

void TreeView::qt_static_metacall(QObject* _o, QMetaObject::Call _c, int _id, void** _a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        TreeView* _t = static_cast<TreeView*>(_o);
        Q_UNUSED(_t)
            switch (_id) {
            case 0: _t->RightClickAction((*reinterpret_cast<QMouseEvent * (*)>(_a[1]))); break;
            default:;
            }
    }
    else if (_c == QMetaObject::IndexOfMethod) {
        int* result = reinterpret_cast<int*>(_a[0]);
        void** func = reinterpret_cast<void**>(_a[1]);
        {
            typedef void (TreeView::* _t)(QMouseEvent*);
            if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&TreeView::RightClickAction)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject TreeView::staticMetaObject = {
    { &QTreeView::staticMetaObject, qt_meta_stringdata_TreeView.data,
      qt_meta_data_TreeView,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject* TreeView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void* TreeView::qt_metacast(const char* _clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TreeView.stringdata0))
        return static_cast<void*>(const_cast<TreeView*>(this));
    return QTreeView::qt_metacast(_clname);
}

int TreeView::qt_metacall(QMetaObject::Call _c, int _id, void** _a)
{
    _id = QTreeView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void TreeView::RightClickAction(QMouseEvent* _t1)
{
    void* _a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_TreeWidget_t {
    QByteArrayData data[5];
    char stringdata0[48];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TreeWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TreeWidget_t qt_meta_stringdata_TreeWidget = {
    {
QT_MOC_LITERAL(0, 0, 10), // "TreeWidget"
QT_MOC_LITERAL(1, 11, 16), // "RightClickAction"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 12), // "QMouseEvent*"
QT_MOC_LITERAL(4, 42, 5) // "event"

    },
    "TreeWidget\0RightClickAction\0\0QMouseEvent*\0"
    "event"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TreeWidget[] = {

    // content:
          7,       // revision
          0,       // classname
          0,    0, // classinfo
          1,   14, // methods
          0,    0, // properties
          0,    0, // enums/sets
          0,    0, // constructors
          0,       // flags
          1,       // signalCount

          // signals: name, argc, parameters, tag, flags
                1,    1,   19,    2, 0x06 /* Public */,

                // signals: parameters
                   QMetaType::Void, 0x80000000 | 3,    4,

                      0        // eod
};

void TreeWidget::qt_static_metacall(QObject* _o, QMetaObject::Call _c, int _id, void** _a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        TreeWidget* _t = static_cast<TreeWidget*>(_o);
        Q_UNUSED(_t)
            switch (_id) {
            case 0: _t->RightClickAction((*reinterpret_cast<QMouseEvent * (*)>(_a[1]))); break;
            default:;
            }
    }
    else if (_c == QMetaObject::IndexOfMethod) {
        int* result = reinterpret_cast<int*>(_a[0]);
        void** func = reinterpret_cast<void**>(_a[1]);
        {
            typedef void (TreeWidget::* _t)(QMouseEvent*);
            if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&TreeWidget::RightClickAction)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject TreeWidget::staticMetaObject = {
    { &QTreeWidget::staticMetaObject, qt_meta_stringdata_TreeWidget.data,
      qt_meta_data_TreeWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject* TreeWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void* TreeWidget::qt_metacast(const char* _clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TreeWidget.stringdata0))
        return static_cast<void*>(const_cast<TreeWidget*>(this));
    return QTreeWidget::qt_metacast(_clname);
}

int TreeWidget::qt_metacall(QMetaObject::Call _c, int _id, void** _a)
{
    _id = QTreeWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void TreeWidget::RightClickAction(QMouseEvent* _t1)
{
    void* _a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_TreeViewWidget_t {
    QByteArrayData data[13];
    char stringdata0[213];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TreeViewWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TreeViewWidget_t qt_meta_stringdata_TreeViewWidget = {
    {
QT_MOC_LITERAL(0, 0, 14), // "TreeViewWidget"
QT_MOC_LITERAL(1, 15, 17), // "OnClickedTreeView"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 5), // "index"
QT_MOC_LITERAL(4, 40, 22), // "OnConnectButtonClicked"
QT_MOC_LITERAL(5, 63, 20), // "OnRightClickedAction"
QT_MOC_LITERAL(6, 84, 12), // "QMouseEvent*"
QT_MOC_LITERAL(7, 97, 5), // "event"
QT_MOC_LITERAL(8, 103, 30), // "OnRightClickedActionTreeWidget"
QT_MOC_LITERAL(9, 134, 28), // "ProcessTreeWidgetItemClicked"
QT_MOC_LITERAL(10, 163, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(11, 180, 4), // "item"
QT_MOC_LITERAL(12, 185, 27) // "EventFromThreadPoolReceived"

    },
    "TreeViewWidget\0OnClickedTreeView\0\0"
    "index\0OnConnectButtonClicked\0"
    "OnRightClickedAction\0QMouseEvent*\0"
    "event\0OnRightClickedActionTreeWidget\0"
    "ProcessTreeWidgetItemClicked\0"
    "QTreeWidgetItem*\0item\0EventFromThreadPoolReceived"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TreeViewWidget[] = {

    // content:
          7,       // revision
          0,       // classname
          0,    0, // classinfo
          6,   14, // methods
          0,    0, // properties
          0,    0, // enums/sets
          0,    0, // constructors
          0,       // flags
          0,       // signalCount

          // slots: name, argc, parameters, tag, flags
                1,    1,   44,    2, 0x0a /* Public */,
                4,    0,   47,    2, 0x0a /* Public */,
                5,    1,   48,    2, 0x0a /* Public */,
                8,    1,   51,    2, 0x0a /* Public */,
                9,    2,   54,    2, 0x0a /* Public */,
               12,    1,   59,    2, 0x0a /* Public */,

               // slots: parameters
                  QMetaType::Void, QMetaType::QModelIndex,    3,
                  QMetaType::Void,
                  QMetaType::Void, 0x80000000 | 6,    7,
                  QMetaType::Void, 0x80000000 | 6,    7,
                  QMetaType::Void, 0x80000000 | 10, QMetaType::Int,   11,    3,
                  QMetaType::Void, QMetaType::Int,    2,

                     0        // eod
};

void TreeViewWidget::qt_static_metacall(QObject* _o, QMetaObject::Call _c, int _id, void** _a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        TreeViewWidget* _t = static_cast<TreeViewWidget*>(_o);
        Q_UNUSED(_t)
            switch (_id) {
            case 0: _t->OnClickedTreeView((*reinterpret_cast<const QModelIndex(*)>(_a[1]))); break;
            case 1: _t->OnConnectButtonClicked(); break;
            case 2: _t->OnRightClickedAction((*reinterpret_cast<QMouseEvent * (*)>(_a[1]))); break;
            case 3: _t->OnRightClickedActionTreeWidget((*reinterpret_cast<QMouseEvent * (*)>(_a[1]))); break;
            case 4: _t->ProcessTreeWidgetItemClicked((*reinterpret_cast<QTreeWidgetItem * (*)>(_a[1])), (*reinterpret_cast<int(*)>(_a[2]))); break;
            case 5: _t->EventFromThreadPoolReceived((*reinterpret_cast<int(*)>(_a[1]))); break;
            default:;
            }
    }
}

const QMetaObject TreeViewWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_TreeViewWidget.data,
      qt_meta_data_TreeViewWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject* TreeViewWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void* TreeViewWidget::qt_metacast(const char* _clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TreeViewWidget.stringdata0))
        return static_cast<void*>(const_cast<TreeViewWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int TreeViewWidget::qt_metacall(QMetaObject::Call _c, int _id, void** _a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
