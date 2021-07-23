/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../Sparky/src/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[88];
    char stringdata0[1465];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 15), // "connectionError"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 3), // "msg"
QT_MOC_LITERAL(4, 32, 17), // "toggleLineView_P1"
QT_MOC_LITERAL(5, 50, 17), // "toggleLineView_P2"
QT_MOC_LITERAL(6, 68, 17), // "toggleLineView_P3"
QT_MOC_LITERAL(7, 86, 15), // "onViewYAxisData"
QT_MOC_LITERAL(8, 102, 17), // "onDataTypeChanged"
QT_MOC_LITERAL(9, 120, 14), // "onHighSelected"
QT_MOC_LITERAL(10, 135, 14), // "onFullSelected"
QT_MOC_LITERAL(11, 150, 13), // "onMidSelected"
QT_MOC_LITERAL(12, 164, 13), // "onLowSelected"
QT_MOC_LITERAL(13, 178, 14), // "readMasterPipe"
QT_MOC_LITERAL(14, 193, 14), // "isUserInputYes"
QT_MOC_LITERAL(15, 208, 18), // "injectionPumpRates"
QT_MOC_LITERAL(16, 227, 15), // "injectionBucket"
QT_MOC_LITERAL(17, 243, 13), // "injectionMark"
QT_MOC_LITERAL(18, 257, 15), // "injectionMethod"
QT_MOC_LITERAL(19, 273, 27), // "onActionPressureSensorSlope"
QT_MOC_LITERAL(20, 301, 22), // "onActionReadMasterPipe"
QT_MOC_LITERAL(21, 324, 12), // "onMinRefTemp"
QT_MOC_LITERAL(22, 337, 12), // "onMaxRefTemp"
QT_MOC_LITERAL(23, 350, 15), // "onInjectionTemp"
QT_MOC_LITERAL(24, 366, 8), // "onXDelay"
QT_MOC_LITERAL(25, 375, 7), // "onYFreq"
QT_MOC_LITERAL(26, 383, 7), // "onZTemp"
QT_MOC_LITERAL(27, 391, 18), // "onActionMainServer"
QT_MOC_LITERAL(28, 410, 19), // "onActionLocalServer"
QT_MOC_LITERAL(29, 430, 19), // "onIntervalSmallPump"
QT_MOC_LITERAL(30, 450, 17), // "onIntervalBigPump"
QT_MOC_LITERAL(31, 468, 12), // "onLoopNumber"
QT_MOC_LITERAL(32, 481, 17), // "onActionMinMaster"
QT_MOC_LITERAL(33, 499, 17), // "onActionMaxMaster"
QT_MOC_LITERAL(34, 517, 19), // "onActionDeltaMaster"
QT_MOC_LITERAL(35, 537, 24), // "onActionDeltaMasterFinal"
QT_MOC_LITERAL(36, 562, 13), // "onActionWater"
QT_MOC_LITERAL(37, 576, 11), // "onActionOil"
QT_MOC_LITERAL(38, 588, 16), // "onActionSettings"
QT_MOC_LITERAL(39, 605, 12), // "onActionSync"
QT_MOC_LITERAL(40, 618, 13), // "onActionStart"
QT_MOC_LITERAL(41, 632, 12), // "onActionStop"
QT_MOC_LITERAL(42, 645, 19), // "onActionStopPressed"
QT_MOC_LITERAL(43, 665, 12), // "onActionSkip"
QT_MOC_LITERAL(44, 678, 13), // "onActionPause"
QT_MOC_LITERAL(45, 692, 21), // "onActionStopInjection"
QT_MOC_LITERAL(46, 714, 22), // "onActionStartInjection"
QT_MOC_LITERAL(47, 737, 19), // "onMasterPipeToggled"
QT_MOC_LITERAL(48, 757, 12), // "runInjection"
QT_MOC_LITERAL(49, 770, 10), // "runTempRun"
QT_MOC_LITERAL(50, 781, 15), // "stopCalibration"
QT_MOC_LITERAL(51, 797, 14), // "updateFileList"
QT_MOC_LITERAL(52, 812, 16), // "createInjectFile"
QT_MOC_LITERAL(53, 829, 16), // "startCalibration"
QT_MOC_LITERAL(54, 846, 15), // "onRtuPortActive"
QT_MOC_LITERAL(55, 862, 16), // "changeSerialPort"
QT_MOC_LITERAL(56, 879, 17), // "createTempRunFile"
QT_MOC_LITERAL(57, 897, 22), // "initializeToolbarIcons"
QT_MOC_LITERAL(58, 920, 13), // "clearMonitors"
QT_MOC_LITERAL(59, 934, 20), // "updateRequestPreview"
QT_MOC_LITERAL(60, 955, 18), // "updateRegisterView"
QT_MOC_LITERAL(61, 974, 13), // "enableHexView"
QT_MOC_LITERAL(62, 988, 17), // "sendModbusRequest"
QT_MOC_LITERAL(63, 1006, 17), // "onSendButtonPress"
QT_MOC_LITERAL(64, 1024, 16), // "pollForDataOnBus"
QT_MOC_LITERAL(65, 1041, 12), // "aboutQModBus"
QT_MOC_LITERAL(66, 1054, 17), // "onCheckBoxChecked"
QT_MOC_LITERAL(67, 1072, 17), // "onCheckBoxClicked"
QT_MOC_LITERAL(68, 1090, 11), // "resetStatus"
QT_MOC_LITERAL(69, 1102, 14), // "setStatusError"
QT_MOC_LITERAL(70, 1117, 19), // "onReadButtonPressed"
QT_MOC_LITERAL(71, 1137, 20), // "onWriteButtonPressed"
QT_MOC_LITERAL(72, 1158, 23), // "onEquationButtonPressed"
QT_MOC_LITERAL(73, 1182, 8), // "loadFile"
QT_MOC_LITERAL(74, 1191, 11), // "loadCsvFile"
QT_MOC_LITERAL(75, 1203, 15), // "loadCsvTemplate"
QT_MOC_LITERAL(76, 1219, 16), // "onUploadEquation"
QT_MOC_LITERAL(77, 1236, 18), // "onDownloadEquation"
QT_MOC_LITERAL(78, 1255, 17), // "onUpdateRegisters"
QT_MOC_LITERAL(79, 1273, 23), // "onDownloadButtonChecked"
QT_MOC_LITERAL(80, 1297, 11), // "saveCsvFile"
QT_MOC_LITERAL(81, 1309, 22), // "onEquationTableChecked"
QT_MOC_LITERAL(82, 1332, 20), // "onLockFactoryDefault"
QT_MOC_LITERAL(83, 1353, 22), // "onUnlockFactoryDefault"
QT_MOC_LITERAL(84, 1376, 29), // "onUpdateFactoryDefaultPressed"
QT_MOC_LITERAL(85, 1406, 19), // "updatePipeStability"
QT_MOC_LITERAL(86, 1426, 18), // "readJsonConfigFile"
QT_MOC_LITERAL(87, 1445, 19) // "writeJsonConfigFile"

    },
    "MainWindow\0connectionError\0\0msg\0"
    "toggleLineView_P1\0toggleLineView_P2\0"
    "toggleLineView_P3\0onViewYAxisData\0"
    "onDataTypeChanged\0onHighSelected\0"
    "onFullSelected\0onMidSelected\0onLowSelected\0"
    "readMasterPipe\0isUserInputYes\0"
    "injectionPumpRates\0injectionBucket\0"
    "injectionMark\0injectionMethod\0"
    "onActionPressureSensorSlope\0"
    "onActionReadMasterPipe\0onMinRefTemp\0"
    "onMaxRefTemp\0onInjectionTemp\0onXDelay\0"
    "onYFreq\0onZTemp\0onActionMainServer\0"
    "onActionLocalServer\0onIntervalSmallPump\0"
    "onIntervalBigPump\0onLoopNumber\0"
    "onActionMinMaster\0onActionMaxMaster\0"
    "onActionDeltaMaster\0onActionDeltaMasterFinal\0"
    "onActionWater\0onActionOil\0onActionSettings\0"
    "onActionSync\0onActionStart\0onActionStop\0"
    "onActionStopPressed\0onActionSkip\0"
    "onActionPause\0onActionStopInjection\0"
    "onActionStartInjection\0onMasterPipeToggled\0"
    "runInjection\0runTempRun\0stopCalibration\0"
    "updateFileList\0createInjectFile\0"
    "startCalibration\0onRtuPortActive\0"
    "changeSerialPort\0createTempRunFile\0"
    "initializeToolbarIcons\0clearMonitors\0"
    "updateRequestPreview\0updateRegisterView\0"
    "enableHexView\0sendModbusRequest\0"
    "onSendButtonPress\0pollForDataOnBus\0"
    "aboutQModBus\0onCheckBoxChecked\0"
    "onCheckBoxClicked\0resetStatus\0"
    "setStatusError\0onReadButtonPressed\0"
    "onWriteButtonPressed\0onEquationButtonPressed\0"
    "loadFile\0loadCsvFile\0loadCsvTemplate\0"
    "onUploadEquation\0onDownloadEquation\0"
    "onUpdateRegisters\0onDownloadButtonChecked\0"
    "saveCsvFile\0onEquationTableChecked\0"
    "onLockFactoryDefault\0onUnlockFactoryDefault\0"
    "onUpdateFactoryDefaultPressed\0"
    "updatePipeStability\0readJsonConfigFile\0"
    "writeJsonConfigFile"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      85,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  439,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,  442,    2, 0x08 /* Private */,
       5,    1,  445,    2, 0x08 /* Private */,
       6,    1,  448,    2, 0x08 /* Private */,
       7,    1,  451,    2, 0x08 /* Private */,
       8,    0,  454,    2, 0x08 /* Private */,
       9,    0,  455,    2, 0x08 /* Private */,
      10,    0,  456,    2, 0x08 /* Private */,
      11,    0,  457,    2, 0x08 /* Private */,
      12,    0,  458,    2, 0x08 /* Private */,
      13,    0,  459,    2, 0x08 /* Private */,
      14,    2,  460,    2, 0x08 /* Private */,
      15,    0,  465,    2, 0x08 /* Private */,
      16,    0,  466,    2, 0x08 /* Private */,
      17,    0,  467,    2, 0x08 /* Private */,
      18,    0,  468,    2, 0x08 /* Private */,
      19,    0,  469,    2, 0x08 /* Private */,
      20,    0,  470,    2, 0x08 /* Private */,
      21,    0,  471,    2, 0x08 /* Private */,
      22,    0,  472,    2, 0x08 /* Private */,
      23,    0,  473,    2, 0x08 /* Private */,
      24,    0,  474,    2, 0x08 /* Private */,
      25,    0,  475,    2, 0x08 /* Private */,
      26,    0,  476,    2, 0x08 /* Private */,
      27,    0,  477,    2, 0x08 /* Private */,
      28,    0,  478,    2, 0x08 /* Private */,
      29,    0,  479,    2, 0x08 /* Private */,
      30,    0,  480,    2, 0x08 /* Private */,
      31,    0,  481,    2, 0x08 /* Private */,
      32,    0,  482,    2, 0x08 /* Private */,
      33,    0,  483,    2, 0x08 /* Private */,
      34,    0,  484,    2, 0x08 /* Private */,
      35,    0,  485,    2, 0x08 /* Private */,
      36,    0,  486,    2, 0x08 /* Private */,
      37,    0,  487,    2, 0x08 /* Private */,
      38,    0,  488,    2, 0x08 /* Private */,
      39,    0,  489,    2, 0x08 /* Private */,
      40,    0,  490,    2, 0x08 /* Private */,
      41,    0,  491,    2, 0x08 /* Private */,
      42,    0,  492,    2, 0x08 /* Private */,
      43,    0,  493,    2, 0x08 /* Private */,
      44,    0,  494,    2, 0x08 /* Private */,
      45,    0,  495,    2, 0x08 /* Private */,
      46,    0,  496,    2, 0x08 /* Private */,
      47,    1,  497,    2, 0x08 /* Private */,
      48,    0,  500,    2, 0x08 /* Private */,
      49,    0,  501,    2, 0x08 /* Private */,
      50,    0,  502,    2, 0x08 /* Private */,
      51,    3,  503,    2, 0x08 /* Private */,
      52,    5,  510,    2, 0x08 /* Private */,
      53,    0,  521,    2, 0x08 /* Private */,
      54,    1,  522,    2, 0x08 /* Private */,
      55,    1,  525,    2, 0x08 /* Private */,
      56,    5,  528,    2, 0x08 /* Private */,
      57,    0,  539,    2, 0x08 /* Private */,
      58,    0,  540,    2, 0x08 /* Private */,
      59,    0,  541,    2, 0x08 /* Private */,
      60,    0,  542,    2, 0x08 /* Private */,
      61,    0,  543,    2, 0x08 /* Private */,
      62,    0,  544,    2, 0x08 /* Private */,
      63,    0,  545,    2, 0x08 /* Private */,
      64,    0,  546,    2, 0x08 /* Private */,
      65,    0,  547,    2, 0x08 /* Private */,
      66,    1,  548,    2, 0x08 /* Private */,
      67,    1,  551,    2, 0x08 /* Private */,
      68,    0,  554,    2, 0x08 /* Private */,
      69,    1,  555,    2, 0x08 /* Private */,
      70,    0,  558,    2, 0x08 /* Private */,
      71,    0,  559,    2, 0x08 /* Private */,
      72,    0,  560,    2, 0x08 /* Private */,
      73,    1,  561,    2, 0x08 /* Private */,
      74,    0,  564,    2, 0x08 /* Private */,
      75,    0,  565,    2, 0x08 /* Private */,
      76,    0,  566,    2, 0x08 /* Private */,
      77,    0,  567,    2, 0x08 /* Private */,
      78,    1,  568,    2, 0x08 /* Private */,
      79,    1,  571,    2, 0x08 /* Private */,
      80,    0,  574,    2, 0x08 /* Private */,
      81,    1,  575,    2, 0x08 /* Private */,
      82,    0,  578,    2, 0x08 /* Private */,
      83,    0,  579,    2, 0x08 /* Private */,
      84,    0,  580,    2, 0x08 /* Private */,
      85,    2,  581,    2, 0x08 /* Private */,
      86,    0,  586,    2, 0x08 /* Private */,
      87,    0,  587,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool, QMetaType::QString, QMetaType::QString,    2,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Int,    2,    2,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString,    2,    2,    2,    2,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::Int,    2,    2,    2,    2,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    2,    2,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connectionError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->toggleLineView_P1((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->toggleLineView_P2((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->toggleLineView_P3((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->onViewYAxisData((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->onDataTypeChanged(); break;
        case 6: _t->onHighSelected(); break;
        case 7: _t->onFullSelected(); break;
        case 8: _t->onMidSelected(); break;
        case 9: _t->onLowSelected(); break;
        case 10: _t->readMasterPipe(); break;
        case 11: { bool _r = _t->isUserInputYes((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 12: _t->injectionPumpRates(); break;
        case 13: _t->injectionBucket(); break;
        case 14: _t->injectionMark(); break;
        case 15: _t->injectionMethod(); break;
        case 16: _t->onActionPressureSensorSlope(); break;
        case 17: _t->onActionReadMasterPipe(); break;
        case 18: _t->onMinRefTemp(); break;
        case 19: _t->onMaxRefTemp(); break;
        case 20: _t->onInjectionTemp(); break;
        case 21: _t->onXDelay(); break;
        case 22: _t->onYFreq(); break;
        case 23: _t->onZTemp(); break;
        case 24: _t->onActionMainServer(); break;
        case 25: _t->onActionLocalServer(); break;
        case 26: _t->onIntervalSmallPump(); break;
        case 27: _t->onIntervalBigPump(); break;
        case 28: _t->onLoopNumber(); break;
        case 29: _t->onActionMinMaster(); break;
        case 30: _t->onActionMaxMaster(); break;
        case 31: _t->onActionDeltaMaster(); break;
        case 32: _t->onActionDeltaMasterFinal(); break;
        case 33: _t->onActionWater(); break;
        case 34: _t->onActionOil(); break;
        case 35: _t->onActionSettings(); break;
        case 36: _t->onActionSync(); break;
        case 37: _t->onActionStart(); break;
        case 38: _t->onActionStop(); break;
        case 39: _t->onActionStopPressed(); break;
        case 40: _t->onActionSkip(); break;
        case 41: _t->onActionPause(); break;
        case 42: _t->onActionStopInjection(); break;
        case 43: _t->onActionStartInjection(); break;
        case 44: _t->onMasterPipeToggled((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 45: _t->runInjection(); break;
        case 46: _t->runTempRun(); break;
        case 47: _t->stopCalibration(); break;
        case 48: _t->updateFileList((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2])),(*reinterpret_cast< const int(*)>(_a[3]))); break;
        case 49: _t->createInjectFile((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const QString(*)>(_a[5]))); break;
        case 50: _t->startCalibration(); break;
        case 51: _t->onRtuPortActive((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 52: _t->changeSerialPort((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 53: _t->createTempRunFile((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const int(*)>(_a[5]))); break;
        case 54: _t->initializeToolbarIcons(); break;
        case 55: _t->clearMonitors(); break;
        case 56: _t->updateRequestPreview(); break;
        case 57: _t->updateRegisterView(); break;
        case 58: _t->enableHexView(); break;
        case 59: _t->sendModbusRequest(); break;
        case 60: _t->onSendButtonPress(); break;
        case 61: _t->pollForDataOnBus(); break;
        case 62: _t->aboutQModBus(); break;
        case 63: _t->onCheckBoxChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 64: _t->onCheckBoxClicked((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 65: _t->resetStatus(); break;
        case 66: _t->setStatusError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 67: _t->onReadButtonPressed(); break;
        case 68: _t->onWriteButtonPressed(); break;
        case 69: _t->onEquationButtonPressed(); break;
        case 70: _t->loadFile((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 71: _t->loadCsvFile(); break;
        case 72: _t->loadCsvTemplate(); break;
        case 73: _t->onUploadEquation(); break;
        case 74: _t->onDownloadEquation(); break;
        case 75: _t->onUpdateRegisters((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 76: _t->onDownloadButtonChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 77: _t->saveCsvFile(); break;
        case 78: _t->onEquationTableChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 79: _t->onLockFactoryDefault(); break;
        case 80: _t->onUnlockFactoryDefault(); break;
        case 81: _t->onUpdateFactoryDefaultPressed(); break;
        case 82: _t->updatePipeStability((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const bool(*)>(_a[2]))); break;
        case 83: _t->readJsonConfigFile(); break;
        case 84: _t->writeJsonConfigFile(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MainWindow::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::connectionError)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 85)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 85;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 85)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 85;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::connectionError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
