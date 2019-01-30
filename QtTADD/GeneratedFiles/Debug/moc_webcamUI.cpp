/****************************************************************************
** Meta object code from reading C++ file 'webcamUI.h'
**
** Created: Thu 29. May 14:28:01 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../webcamUI.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'webcamUI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_webcamUI[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      36,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x09,
      24,    9,    9,    9, 0x09,
      40,    9,    9,    9, 0x09,
      60,    9,    9,    9, 0x09,
      75,    9,    9,    9, 0x09,
      90,    9,    9,    9, 0x09,
     105,    9,    9,    9, 0x09,
     120,    9,    9,    9, 0x09,
     135,    9,    9,    9, 0x09,
     150,    9,    9,    9, 0x09,
     164,    9,    9,    9, 0x09,
     181,    9,    9,    9, 0x09,
     198,    9,    9,    9, 0x09,
     223,    9,    9,    9, 0x09,
     236,    9,    9,    9, 0x09,
     249,    9,    9,    9, 0x09,
     257,    9,    9,    9, 0x09,
     274,    9,    9,    9, 0x09,
     287,    9,    9,    9, 0x09,
     304,    9,    9,    9, 0x09,
     317,    9,    9,    9, 0x09,
     330,    9,    9,    9, 0x09,
     348,    9,    9,    9, 0x09,
     364,    9,    9,    9, 0x09,
     375,    9,    9,    9, 0x09,
     386,    9,    9,    9, 0x09,
     404,  400,    9,    9, 0x09,
     424,    9,    9,    9, 0x09,
     442,  436,    9,    9, 0x09,
     463,  436,    9,    9, 0x09,
     479,  436,    9,    9, 0x09,
     492,  436,    9,    9, 0x09,
     505,  436,    9,    9, 0x09,
     520,    9,    9,    9, 0x09,
     533,    9,    9,    9, 0x09,
     545,    9,    9,    9, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_webcamUI[] = {
    "webcamUI\0\0pauseAction()\0captureAction()\0"
    "captureAction_NEW()\0class1Action()\0"
    "class2Action()\0class3Action()\0"
    "class4Action()\0class5Action()\0"
    "class6Action()\0trainAction()\0"
    "trainAllAction()\0classifyAction()\0"
    "removeBackgroundAction()\0BGs1Action()\0"
    "BGs2Action()\0Reset()\0previousAction()\0"
    "nextAction()\0nextAction_NEW()\0"
    "editAction()\0saveAction()\0saveBGBigAction()\0"
    "saveBigAction()\0saveData()\0loadData()\0"
    "groundTruth()\0txt\0nameChange(QString)\0"
    "loadImage()\0value\0updateThreshold(int)\0"
    "updateArea(int)\0scrollX(int)\0scrollY(int)\0"
    "scaleslot(int)\0openFolder()\0logButton()\0"
    "logFolderButton()\0"
};

const QMetaObject webcamUI::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_webcamUI,
      qt_meta_data_webcamUI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &webcamUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *webcamUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *webcamUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_webcamUI))
        return static_cast<void*>(const_cast< webcamUI*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int webcamUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: pauseAction(); break;
        case 1: captureAction(); break;
        case 2: captureAction_NEW(); break;
        case 3: class1Action(); break;
        case 4: class2Action(); break;
        case 5: class3Action(); break;
        case 6: class4Action(); break;
        case 7: class5Action(); break;
        case 8: class6Action(); break;
        case 9: trainAction(); break;
        case 10: trainAllAction(); break;
        case 11: classifyAction(); break;
        case 12: removeBackgroundAction(); break;
        case 13: BGs1Action(); break;
        case 14: BGs2Action(); break;
        case 15: Reset(); break;
        case 16: previousAction(); break;
        case 17: nextAction(); break;
        case 18: nextAction_NEW(); break;
        case 19: editAction(); break;
        case 20: saveAction(); break;
        case 21: saveBGBigAction(); break;
        case 22: saveBigAction(); break;
        case 23: saveData(); break;
        case 24: loadData(); break;
        case 25: groundTruth(); break;
        case 26: nameChange((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 27: loadImage(); break;
        case 28: updateThreshold((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 29: updateArea((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 30: scrollX((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 31: scrollY((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 32: scaleslot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 33: openFolder(); break;
        case 34: logButton(); break;
        case 35: logFolderButton(); break;
        default: ;
        }
        _id -= 36;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
