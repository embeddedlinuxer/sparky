#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGroupBox>
#include <QProgressBar>
#include <QFuture>
#include <QLineEdit>
#include <QComboBox>
#include <QLCDNumber>
#include <QCheckBox>
#include <QIntValidator>
#include <QChartView>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QProgressDialog>
#include "modbus.h"
#include "ui_about.h"
#include "modbus-rtu.h"
#include "modbus.h"

#define RELEASE_VERSION             "0.1.6"

#define RAZ                         0 
#define EEA                         1 
#define SERIES_WATERCUT             0
#define SERIES_RP                   1 

#define MD_FLOAT					0
#define MD_INT						1
#define MD_COIL						2
#define MD_WRITE                   	3 
#define MD_READ                     4 
	
#define STABILITY_CHECK				true
#define NO_STABILITY_CHECK			false

#define PHASE_OIL					0
#define PHASE_WATER					1
#define PHASE_ERROR					2

/// sub system	
#define CONTROLBOX_SLAVE 	        100
#define MODBUS_TIMER_SLAVE 			101
#define OIL_MICROMOTION_SLAVE 	 	102
#define WATER_MICROMOTION_SLAVE  	103

#define COIL_OIL_PUMP				60
#define COIL_WATER_PUMP				61

#define EEA_ID_SN_PIPE  			40001 
#define EEA_ID_WATERCUT  			11
#define EEA_ID_SALINITY 			21
#define EEA_ID_OIL_ADJUST  			23
#define EEA_ID_TEMPERATURE  		15
#define EEA_ID_WATER_ADJUST  		25
#define EEA_ID_FREQ  				111 
#define EEA_ID_OIL_RP  				115 
#define EEA_ID_PRESSURE  			1005

#define RAZ_ID_SN_PIPE  			201 
#define RAZ_ID_WATERCUT  			3
#define RAZ_ID_TEMPERATURE  		33 /// REG_TEMP_USER
#define RAZ_ID_SALINITY  			9 
#define RAZ_ID_OIL_ADJUST  			15
#define RAZ_ID_WATER_ADJUST  		17
#define RAZ_ID_FREQ  				19
#define RAZ_ID_OIL_RP  				61

#define FCT_RAZ_TEMP_ADJ    		781

/// pipe cal status
#define DONE						0
#define ENABLED						1	
#define DISABLED					2		

/// master pipe
#define MASTER_WATERCUT             3
#define MASTER_WRITE                1
#define MASTER_READ                 0

/// progress bar type
#define F_BAR                       1
#define T_BAR                       0
  
#define BLANK						"                                                 " 
/// calibration file names
#define HIGH_EEA                    "\\HIGHCUT\\HC"
#define FULL_EEA                    "\\FULLCUT\\FC" 
#define MID_EEA                     "\\MIDCUT\\MC"
#define LOW_EEA                     "\\LOWCUT\\LC"
#define HIGH_RAZ                    "\\HIGHCUT_RAZ\\HC"
#define FULL_RAZ                    "\\FULLCUT_RAZ\\FC" 
#define MID_RAZ                     "\\MIDCUT_RAZ\\MC"
#define LOW_RAZ                     "\\LOWCUT_RAZ\\LC"

/// header lines
#define HEADER3                     "Time From  Water  Osc  Tune Tuning            Incident Reflected              Injection  Master    Master      Master      Master      Master       Master   Master Pipe";
#define HEADER4                     "Run Start   Cut   Band Type Voltage Frequency  Power     Power   Temperature    Time    Pressure Temperature Oil Adjust   Frequency   Watercut      Oil Rp   Phase  Watercut";
#define HEADER5                     "========= ======= ==== ==== ======= ========= ======== ========= =========== ========== ======== =========== =========== =========== =========== =========== ====== ========";

/// loop
#define L1                          0

/// pipe
#define P1                          0
#define P2                          1
#define P3                          2
#define ALL                         3

#define EEA_INJECTION_FILE          "EEA INJECTION FILE"
#define RAZ_INJECTION_FILE          "RAZOR INJECTION FILE"

#define MAIN_SERVER                 "MainServer"
#define LOCAL_SERVER                "LocalServer"

/// stage update
#define ORANGE						"color: rgba(232, 126, 4, 1);"
#define RED							"color: red;"
#define GREEN						"color: green;"
#define BLACK						"color: black;"
#define BLUE						"color: blue;"

/// LOOP.runMode
#define TEMPRUN_MIN					"TEMPRUN MIN"	
#define TEMPRUN_HIGH				"TEMPRUN HIGH"	
#define TEMPRUN_INJECT				"TEMPRUN INJ"	
#define TEMPRUN_ONLY				"TEMPRUN ONLY"	
#define INJECT_OIL					"INJECT OIL"	
#define INJECT_WATER				"INJECT WATER"	
#define INJECT_STANDBY				"INJECT STANDBY"	
#define SIMULATION_RUN				"SIMULATION"	
#define STOP_CALIBRATION			"STOP"	
#define READ_MASTERPIPE				"READ MASTER PIPE"	
#define TEMP_IN_SYNC				"TEMP IN SYNC"	
#define WATER_RUN					"WATER RUN"	
#define OIL_RUN						"OIL RUN"	

//////////////////////////
/////// JSON KEYS ////////
//////////////////////////

#define LOOP_OIL_PUMP_RATE            "LOOP.OilPumpRate"
#define LOOP_WATER_PUMP_RATE          "LOOP.WaterPumpRate"
#define LOOP_SMALL_WATER_PUMP_RATE    "LOOP.SmallWaterPumpRate"
#define LOOP_BUCKET                   "LOOP.Bucket"
#define LOOP_MARK                     "LOOP.Mark"
#define LOOP_METHOD                   "LOOP.Method"
#define LOOP_PRESSURE                 "LOOP.PresssureSensorSlope"
#define LOOP_MIN_TEMP                 "LOOP.MinRefTemp"
#define LOOP_MAX_TEMP                 "LOOP.MaxRefTemp"
#define LOOP_INJECTION_TEMP           "LOOP.InjectionTemp"
#define LOOP_X_DELAY                  "LOOP.XDelay"
#define LOOP_Y_FREQ                   "LOOP.YFreq"
#define LOOP_Z_TEMP                   "LOOP.ZTemp"
#define LOOP_INTERVAL_SMALL_PUMP      "LOOP.IntervalSmallPump"
#define LOOP_INTERVAL_BIG_PUMP  	  "LOOP.IntervalBigPump"
#define LOOP_INTERVAL_OIL_PUMP  	  "LOOP.IntervalOilPump"
#define LOOP_NUMBER  	  			  "LOOP.LoopNumber"
#define LOOP_MASTER_MIN  			  "LOOP.MasterMin"
#define LOOP_MASTER_MAX  			  "LOOP.MasterMax"
#define LOOP_MASTER_DELTA  			  "LOOP.MasterDelta"
#define LOOP_MASTER_DELTA_FINAL		  "LOOP.MasterDeltaFinal"
#define LOOP_MAX_INJECTION_WATER   	  "LOOP.MaxInjectionWater"
#define LOOP_MAX_INJECTION_OIL   	  "LOOP.MaxInjectionOil"
#define LOOP_PORT_INDEX    	          "LOOP.PortIndex"

#define FILE_LIST                   "Filelist.LST"

#define TIMER_DELAY         6000
#define NO_FILE             0
#define S_CALIBRAT         	1 
#define S_ADJUSTED         	2 
#define S_ROLLOVER          3 
#define S_FILELIST          4 

#define SLEEP_TIME          300 // 300 milisecond
#define SLAVE_CALIBRATION   0xFA
#define FUNC_READ_FLOAT     0x04
#define FUNC_READ_INT       0x03 
#define FUNC_READ_COIL      0x01 
#define FUNC_WRITE_FLOAT    0x10
#define FUNC_WRITE_INT      0x06
#define FUNC_WRITE_COIL     0x05
#define BYTE_READ_FLOAT     2
#define BYTE_READ_INT       1
#define BYTE_READ_COIL      1
#define FLOAT_R             0
#define FLOAT_W             1
#define INT_R               2
#define INT_W               3
#define COIL_R              4
#define COIL_W              5
#define ADDR_OFFSET         1

#define RAZ_MEAS_AI         173
#define RAZ_TRIM_AI         175

#define RESET_SERIES		-10000

QT_CHARTS_USE_NAMESPACE

class AboutDialog : public QDialog, public Ui::AboutDialog
{
public:
    AboutDialog( QWidget * _parent ) :
        QDialog( _parent )
    {
        setupUi( this );
        aboutTextLabel->setText(aboutTextLabel->text().arg( "0.3.0" ) );
    }
};

typedef struct PIPE_OBJECT 
{
	bool isStartFreq;
    int osc;
    int tempStability;
    int freqStability;
	int status;
	int rolloverTracker;
	QString calFile;
    QString mainDirPath;
    QString localDirPath;
    QString pipeId;
    QFile file;
    QFile fileCalibrate;
    QFile fileAdjusted;
    QFile fileRollover;
    QLineEdit * slave; 
    QSplineSeries * series;
    QSplineSeries * series_2;
    QElapsedTimer * etimer;
    QCheckBox * checkBox;
    QCheckBox * lineView; 
    QLineEdit * wc;
    QLineEdit * startFreq;
    QLineEdit * freq;
    QLineEdit * temp;
    QLineEdit * reflectedPower;
	QProgressBar * freqProgress;
	QProgressBar * tempProgress;

	double watercut;
	double temperature;
    double temperature_prev;
    double frequency;
    double frequency_prev;
    double frequency_start;
    double oilrp;
    double measai;
    double trimai;

	QPen pen;
	QPen pen2;

	PIPE_OBJECT() : isStartFreq(true), osc(0), tempStability(0), freqStability(0), status(ENABLED), rolloverTracker(0), calFile(""),  mainDirPath(""), localDirPath(""), pipeId(""), file(""), fileCalibrate("CALIBRATE"), fileAdjusted("ADJUSTED"), fileRollover("ROLLOVER"), slave(new QLineEdit), series(new QSplineSeries), series_2(new QSplineSeries), etimer(new QElapsedTimer), lineView(new QCheckBox), checkBox(new QCheckBox), wc(new QLineEdit), startFreq(new QLineEdit), freq(new QLineEdit), temp(new QLineEdit), reflectedPower(new QLineEdit), freqProgress(new QProgressBar), tempProgress(new QProgressBar), watercut(0), temperature(0), frequency(0), temperature_prev(0), frequency_prev(0), frequency_start(0), oilrp(0), measai(0), trimai(0), pen (Qt::green, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), pen2 (Qt::green, 3, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin)  {}

    //This is the destructor.  Will delete the array of vertices, if present.
    ~PIPE_OBJECT()
    {
        if (slave) delete slave;
        if (series) delete series;
        if (etimer) delete etimer;
        if (checkBox) delete checkBox;
        if (lineView) delete lineView; 
        if (wc) delete wc;
        if (startFreq) delete startFreq;
        if (freq) delete freq;
        if (temp) delete temp;
        if (reflectedPower) delete reflectedPower;
		if (freqProgress) delete freqProgress;
		if (tempProgress) delete tempProgress;
    }

} PIPES;


typedef struct LOOP_OBJECT 
{
	bool ignore;
	bool isPhase;
	bool isWaterRun;
	bool isOilRun;
	bool isPause;
	bool isTempRunSkip;
	bool isInject;
	bool isMaster;
    bool isCal;
	bool isEEA;
	bool isInitTempRun;
	bool isInitInject;
	bool isTempRunOnly;
	QString cut;
	double masterMin;
	double masterMax;
	double masterDelta;
	double masterDeltaFinal;
    double watercut;
	double injectionOilPumpRate;
    double injectionWaterPumpRate;
    double injectionSmallWaterPumpRate;
    double injectionBucket;
    double injectionMark;
    double injectionMethod;
    double pressureSensorSlope;
    double minTemp;
    double maxTemp;
    double injectTemp;
    QString currentTemp;
    QString targetTemp;
	int phaseRolloverCounter;
    int xDelay;
	int loopNumber;
	int maxInjectionWater;
	int maxInjectionOil;
	int portIndex;
	int maxGraphDataPoint;
	int salinityIndex;
    double yFreq;
    double zTemp;
	double intervalOilPump;
	double intervalBigPump;
	double intervalSmallPump;
	double intervalRollover;
	QString runMode;
	QString filExt;
	QString calExt;
	QString adjExt;
	QString rolExt;
	QString simExt;
	QString operatorName;
    
	/// register address for calibration
    int ID_SN_PIPE;
    int ID_WATERCUT;
    int ID_TEMPERATURE;
    int ID_SALINITY;
    int ID_OIL_ADJUST;
    int ID_WATER_ADJUST;
    int ID_FREQ;
    int ID_OIL_RP;
    int ID_PRESSURE;

	int ID_MASTER_WATERCUT;
    int ID_MASTER_SALINITY;
    int ID_MASTER_OIL_ADJUST;
    int ID_MASTER_OIL_RP;
    int ID_MASTER_TEMPERATURE;
    int ID_MASTER_FREQ;
    int ID_MASTER_PHASE;
    int ID_MASTER_PRESSURE;

    QLineEdit * loopVolume;
	QComboBox * saltStart;
	QComboBox * saltStop;
    QComboBox * oilTemp;
	QLineEdit * waterRunStart;
	QLineEdit * waterRunStop;
	QLineEdit * oilRunStart;
	QLineEdit * oilRunStop;
	double masterWatercut;
	double masterSalinity;
	double masterOilAdj;
	double masterOilRp;
	double masterFreq;
	double masterTemp;
	double masterPhase;
	double masterPressure;

	modbus_t * modbus;
    modbus_t * serialModbus;
    QChart * chart;
    QChartView * chartView;
    QValueAxis * axisX;
    QValueAxis * axisY;
    QValueAxis * axisY2;

	LOOP_OBJECT() : ignore(false), isPhase(true), isWaterRun(false), isOilRun(false), isPause(false), isTempRunSkip(false), isInject(false), isTempRunOnly(false), isMaster(true), isCal(true), isEEA(false), isInitTempRun(1), isInitInject(1), cut(MID_EEA), masterMin(0), masterMax(0),masterDelta(0), masterDeltaFinal(0), watercut(0), injectionOilPumpRate(0), injectionWaterPumpRate(0), injectionSmallWaterPumpRate(0), injectionBucket(0), injectionMark(0), injectionMethod(0), pressureSensorSlope(0), minTemp(0), maxTemp(0),currentTemp("0"), targetTemp("0"), injectTemp(0), phaseRolloverCounter(0), xDelay(0), loopNumber(0), maxInjectionWater(80), maxInjectionOil(200), portIndex(0), maxGraphDataPoint(0), salinityIndex(0), yFreq(0), zTemp(0), intervalOilPump(0.25), intervalBigPump(1), intervalSmallPump(0.25), runMode(""), filExt(""), calExt(""), adjExt(""),rolExt(""),  simExt(".SIM"), operatorName(""), ID_SN_PIPE(0), ID_WATERCUT(0), ID_TEMPERATURE(0), ID_SALINITY(0), ID_OIL_ADJUST(0), ID_WATER_ADJUST(0), ID_FREQ(0), ID_OIL_RP(0), ID_PRESSURE(0), ID_MASTER_WATERCUT(11), ID_MASTER_SALINITY(21), ID_MASTER_OIL_ADJUST(23), ID_MASTER_OIL_RP(115), ID_MASTER_TEMPERATURE(15),ID_MASTER_FREQ(111),ID_MASTER_PHASE(17),ID_MASTER_PRESSURE(1005), loopVolume(new QLineEdit), saltStart(new QComboBox), saltStop(new QComboBox), oilTemp(new QComboBox), waterRunStart(new QLineEdit), waterRunStop(new QLineEdit), oilRunStart(new QLineEdit), oilRunStop(new QLineEdit), masterWatercut(0), masterSalinity(0), masterOilAdj(0), masterOilRp(0), masterFreq(0), masterTemp(0), masterPhase(1),masterPressure(1), modbus(NULL), serialModbus(NULL), chart(new QChart), chartView(new QChartView), axisX(new QValueAxis), axisY(new QValueAxis), axisY2(new QValueAxis) {};

	~LOOP_OBJECT()
	{
		if (chart) delete chart;
		if (chartView) delete chartView;
        if (loopVolume) delete loopVolume;
        if (saltStart) delete saltStart;
        if (saltStop) delete saltStop;
        if (oilTemp) delete oilTemp;
        if (waterRunStart) delete waterRunStart;
        if (waterRunStop) delete waterRunStop;
        if (oilRunStart) delete oilRunStart;
        if (oilRunStop) delete oilRunStop;
	}
    
} LOOPS;

namespace Ui
{
    class MainWindowClass;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget * parent = 0 );
    ~MainWindow();

    void delay(int);
    modbus_t*  modbus() { return LOOP.serialModbus; }

    int setupModbusPort();

	void initTempRun();
	double read_request(int, int, int, int, uint8_t *, uint16_t *, bool);
	void write_request(int, int, double, int, bool);
	void createDataStream(const int pipe, QString & data_stream);
	void startTempRun();
	void updateCurrentStage(const QString, const QString);
	void updateLoopStatus(const double, const double, const double, const double);
	void readPipe(const int, const bool);
	void inject(const int, const bool);
	void readLoopConfiguration();
    void masterPipe(int, QString, bool);
    void setFileNameForNextStage(const int, const QString);
    void writeToCalFile(int, QString);
    void closeCalibrationFile(int, int, double);
    void changeModbusInterface(const QString &port, char parity);
    void releaseSerialModbus();
	void setValidators();
    void initializeGraph();
    void initializePipeObjects();
    void initializeLoopObjects();
    bool validateSerialNumber(modbus_t *);   
    void displayPipeReading(const int, const double, const double, const double, const double, const double); 
	void updateMasterPipeStatus(const double, const double, const double, const double, const double, const double);
    bool informUser(const QString, const QString, const QString);
    void busMonitorAddItem( bool isRequest,uint16_t slave,uint8_t func,uint16_t addr,uint16_t nb,uint16_t expectedCRC,uint16_t actualCRC );
    static void stBusMonitorAddItem( modbus_t * modbus,uint8_t isOut, uint16_t slave, uint8_t func, uint16_t addr,uint16_t nb, uint16_t expectedCRC, uint16_t actualCRC );
    static void stBusMonitorRawData( modbus_t * modbus, uint8_t * data,uint8_t dataLen, uint8_t addNewline );
    void busMonitorRawData( uint8_t * data, uint8_t dataLen, bool addNewline );
    void connectSerialPort();
    void connectActions();
    void connectModbusMonitor();
    void connectTimers();
    void connectRegisters();
    void connectProfiler();
    void connectToolbar();
    void connectRadioButtons();
    void connectLineView();
    void connectReturnPressed();
    void connectProductBtnPressed();
	void connectMasterPipe();
	void connectCheckbox();
    void setupModbusPorts();
    void updateLoopTabIcon(const bool);
    bool prepareCalibration();
    bool enablePipes();
    void initializeTabIcons();
    float toFloat(QByteArray arr);
    void initializeModbusMonitor();
    void onFunctionCodeChanges();
    void updateChart(QSplineSeries *, double, double, double, double, double, double, double, double);
    void updateGraph(const int, const double, const double, const int);

protected:
	void mousePressEvent(QMouseEvent *event) override;

private slots:

	/// graph 
	void toggleLineView_P1(bool); 
    void toggleLineView_P2(bool); 
    void toggleLineView_P3(bool); 
	void onViewYAxisData(bool);

    /// config menu
	void onDataTypeChanged();
    void onHighSelected();
    void onFullSelected();
    void onMidSelected();
    void onLowSelected();
	void readMasterPipe();
    bool isUserInputYes(const QString, const QString);
    void injectionPumpRates();
    void injectionBucket();
    void injectionMark();
    void injectionMethod();
    void onActionPressureSensorSlope();
    void onActionReadMasterPipe();
	void onMinRefTemp();
	void onMaxRefTemp();
	void onInjectionTemp();
    void onXDelay();
    void onYFreq();
    void onZTemp();
    void onActionMainServer();
    void onActionLocalServer();
    void onIntervalSmallPump();
    void onIntervalBigPump();
    void onLoopNumber();
	void onActionMinMaster();
	void onActionMaxMaster();
	void onActionDeltaMaster();
	void onActionDeltaMasterFinal();
	void onActionWater();
	void onActionOil();
    void onActionSettings();
    void onActionSync();
    void onActionStart();
    void onActionStop();
    void onActionStopPressed();
    void onActionSkip();
    void onActionPause();
    void onActionStopInjection();
    void onActionStartInjection();
	void onMasterPipeToggled(const bool);
    void runInjection();
    void runTempRun();
    void stopCalibration();
	void updateFileList(const QString, const int, const int);
	void createInjectFile(const int, const QString, const QString, const QString, const QString);
    void startCalibration();
    void onRtuPortActive(bool);
    void changeSerialPort(int);
    void createTempRunFile(const int, const QString, const QString, const QString, const int);
    void initializeToolbarIcons(void);
    void clearMonitors( void );
    void updateRequestPreview( void );
    void updateRegisterView( void );
    void enableHexView( void );
    void sendModbusRequest( void );
    void onSendButtonPress( void );
    void pollForDataOnBus( void );
    void aboutQModBus( void );
    void onCheckBoxChecked(bool);
	void onCheckBoxClicked(const bool);
    void resetStatus( void );
    void setStatusError(const QString &msg);    
    void onReadButtonPressed();
    void onWriteButtonPressed();
    void onEquationButtonPressed();
	void loadFile(QString);
    void loadCsvFile();
    void loadCsvTemplate();
    void onUploadEquation();
    void onDownloadEquation();
    void onUpdateRegisters(const bool);
    void onDownloadButtonChecked(bool);
    void saveCsvFile();
    void onEquationTableChecked(bool);
	void onLockFactoryDefault();
	void onUnlockFactoryDefault();
    void onUpdateFactoryDefaultPressed();
    void updatePipeStability(const int, const bool);
    void readJsonConfigFile();
    void writeJsonConfigFile();

signals:
    void connectionError(const QString &msg);

private:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    Ui::MainWindowClass * ui;

    /// versioning
    QString PROJECT_VERSION = RELEASE_VERSION;
    QString PROJECT         = "Sparky ";
    QString PHASEDYNAMICS   = " - Phase Dynamics";
    QString SPARKY          = PROJECT + PROJECT_VERSION + PHASEDYNAMICS;

    /// calibration file location
    QString m_localServer;
	QString m_mainServer;

	/// connection
    modbus_t * m_modbus_snipping;
    QIntValidator *serialNumberValidator;
    QWidget * m_statusInd;
    QLabel * m_statusText;
    QTimer * m_pollTimer;
    QTimer * m_statusTimer;
    bool m_tcpActive;
    bool m_poll;
	bool isModbusTransmissionFailed;

	/// loop objects
	LOOPS LOOP;

	/// pipe objects
	PIPES PIPE [3];
};

#endif // MAINWINDOW_H
