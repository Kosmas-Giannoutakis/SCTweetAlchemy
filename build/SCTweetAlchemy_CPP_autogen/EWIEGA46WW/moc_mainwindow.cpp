/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../mainwindow.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "onTweetSelectionChanged",
        "",
        "QListWidgetItem*",
        "current",
        "previous",
        "onSearchTextChanged",
        "text",
        "onSearchNavigateKey",
        "QKeyEvent*",
        "event",
        "focusSearchField",
        "toggleCurrentTweetFavorite",
        "onTweetItemDoubleClicked",
        "item",
        "onTweetListContextMenuRequested",
        "pos",
        "applyAllFilters",
        "handleRepositoryLoadError",
        "title",
        "message",
        "handleTweetsLoaded",
        "count",
        "handleFavoritesChanged",
        "handleTweetsModified",
        "onFileNewTweet",
        "onFileSaveAllChanges",
        "onEditTweet",
        "onEditDeleteTweet",
        "onEditCopyCode",
        "onHelpAbout"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onTweetSelectionChanged'
        QtMocHelpers::SlotData<void(QListWidgetItem *, QListWidgetItem *)>(1, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 3, 5 },
        }}),
        // Slot 'onSearchTextChanged'
        QtMocHelpers::SlotData<void(const QString &)>(6, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Slot 'onSearchNavigateKey'
        QtMocHelpers::SlotData<void(QKeyEvent *)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
        // Slot 'focusSearchField'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleCurrentTweetFavorite'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTweetItemDoubleClicked'
        QtMocHelpers::SlotData<void(QListWidgetItem *)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 3, 14 },
        }}),
        // Slot 'onTweetListContextMenuRequested'
        QtMocHelpers::SlotData<void(const QPoint &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 16 },
        }}),
        // Slot 'applyAllFilters'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleRepositoryLoadError'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 19 }, { QMetaType::QString, 20 },
        }}),
        // Slot 'handleTweetsLoaded'
        QtMocHelpers::SlotData<void(int)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 22 },
        }}),
        // Slot 'handleFavoritesChanged'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleTweetsModified'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFileNewTweet'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFileSaveAllChanges'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEditTweet'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEditDeleteTweet'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEditCopyCode'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onHelpAbout'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onTweetSelectionChanged((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[2]))); break;
        case 1: _t->onSearchTextChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->onSearchNavigateKey((*reinterpret_cast< std::add_pointer_t<QKeyEvent*>>(_a[1]))); break;
        case 3: _t->focusSearchField(); break;
        case 4: _t->toggleCurrentTweetFavorite(); break;
        case 5: _t->onTweetItemDoubleClicked((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 6: _t->onTweetListContextMenuRequested((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 7: _t->applyAllFilters(); break;
        case 8: _t->handleRepositoryLoadError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->handleTweetsLoaded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->handleFavoritesChanged(); break;
        case 11: _t->handleTweetsModified(); break;
        case 12: _t->onFileNewTweet(); break;
        case 13: _t->onFileSaveAllChanges(); break;
        case 14: _t->onEditTweet(); break;
        case 15: _t->onEditDeleteTweet(); break;
        case 16: _t->onEditCopyCode(); break;
        case 17: _t->onHelpAbout(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    return _id;
}
QT_WARNING_POP
