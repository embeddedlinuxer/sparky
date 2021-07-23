#include <QRegExp>
#include <QtConcurrent>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QScrollBar>
#include <QTime>
#include <QGroupBox>
#include <QFileDialog>
#include <QThread>
#include <errno.h>
#include <QSignalMapper>
#include <QListWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QInputDialog>
#include <QProgressDialog>
#include "mainwindow.h"
#include "modbus.h"
#include "modbus-private.h"
#include "modbus-rtu.h"
#include "ui_mainwindow.h"
#include "qextserialenumerator.h"

QT_CHARTS_USE_NAMESPACE
#define MAX_PHASE_CHECKING		5

const QString SALINITY[] = {"0.02", "0.10", "0.20", "0.30", "0.40", "0.50", "1.00", "1.50", "2.00", "3.00", "5.00", "8.00", "11.00", "20.00", "25.00", "28.00"};

const int DataTypeColumn = 0;
const int AddrColumn = 1;
const int DataColumn = 2;

static QString HIGH = "0";
static QString FULL = "0";
static QString MID = "0";
static QString LOW = "0";

extern MainWindow * globalMainWin;

MainWindow::MainWindow( QWidget * _parent ) :
    QMainWindow( _parent ),
    ui( new Ui::MainWindowClass ),
    m_modbus_snipping( NULL ),
    m_poll(false),
    isModbusTransmissionFailed(false)
{
    ui->setupUi(this);

    /// versioning
    setWindowTitle(SPARKY);

    readJsonConfigFile();
    initializeToolbarIcons();
    initializeTabIcons();
    updateRegisterView();
    updateRequestPreview();
    enableHexView();
    setupModbusPorts();
    initializePipeObjects();
    initializeLoopObjects();
    initializeGraph();
    initializeModbusMonitor();
    setValidators();
	onMidSelected();

    /// hide main configuration panel at start
    ui->groupBox_18->hide();
    ui->groupBox_35->hide();
    ui->groupBox_34->hide();
    ui->groupBox_33->hide();
    ui->groupBox_32->hide();
    ui->groupBox_31->hide();
    ui->groupBox_30->hide();
    ui->groupBox_29->hide();

    /// set stop calibration
    updateLoopTabIcon(false);
    stopCalibration();

    ui->regTable->setColumnWidth( 0, 150 );
    m_statusInd = new QWidget;
    m_statusInd->setFixedSize( 16, 16 );
    m_statusText = new QLabel;
    ui->statusBar->addWidget( m_statusInd );
    ui->statusBar->addWidget( m_statusText, 10 );
    resetStatus();

    /// connections
    connectCheckbox();
    connectProductBtnPressed();
    connectRadioButtons();
    connectSerialPort();
    connectActions();
    connectModbusMonitor();
    connectTimers();
    connectProfiler();
    connectToolbar();
    connectLineView();
    connectMasterPipe();

    /// clear connection at start
    updateLoopTabIcon(false);

    /// match pipe colors with chart
    PIPE[0].slave->setStyleSheet("color: red;");
    PIPE[0].wc->setStyleSheet("color: red;");
    PIPE[0].startFreq->setStyleSheet("color: red;");
    PIPE[0].freq->setStyleSheet("color: red;");
    PIPE[0].temp->setStyleSheet("color: red;");
    PIPE[0].reflectedPower->setStyleSheet("color: red;");

    PIPE[1].slave->setStyleSheet("color: blue;");
    PIPE[1].wc->setStyleSheet("color: blue;");
    PIPE[1].startFreq->setStyleSheet("color: blue;");
    PIPE[1].freq->setStyleSheet("color: blue;");
    PIPE[1].temp->setStyleSheet("color: blue;");
    PIPE[1].reflectedPower->setStyleSheet("color: blue;");

    PIPE[2].slave->setStyleSheet("color: black;");
    PIPE[2].wc->setStyleSheet("color: black;");
    PIPE[2].startFreq->setStyleSheet("color: black;");
    PIPE[2].freq->setStyleSheet("color: black;");
    PIPE[2].temp->setStyleSheet("color: black;");
    PIPE[2].reflectedPower->setStyleSheet("color: black;");

	displayPipeReading(ALL,0,0,0,0,0);
	updateCurrentStage(RED,STOP_CALIBRATION);
}


MainWindow::~MainWindow()
{
	inject(COIL_OIL_PUMP,false);
	inject(COIL_WATER_PUMP,false);
	releaseSerialModbus();
    delete m_statusInd;
    delete m_statusText;
    delete ui;
}


void
MainWindow::
setValidators()
{
    ui->lineEdit->setValidator( new QDoubleValidator(0, 1000000, 2, this) ); // loopVolume
    ui->lineEdit_37->setValidator( new QDoubleValidator(0, 1000000, 2, this) ); // waterRunStart
    ui->lineEdit_38->setValidator( new QDoubleValidator(0, 1000000, 2, this) ); // waterRunStop
    ui->lineEdit_39->setValidator( new QDoubleValidator(0, 1000000, 2, this) ); // oilRunStart
    ui->lineEdit_40->setValidator( new QDoubleValidator(0, 1000000, 2, this) ); // oilRunStop
    ui->lineEdit_2->setValidator( new QDoubleValidator(0, 1000000, 2, this) ); // oilRunStart
    ui->lineEdit_7->setValidator( new QDoubleValidator(0, 1000000, 2, this) ); // oilRunStart
    ui->lineEdit_13->setValidator( new QDoubleValidator(0, 1000000, 2, this) ); // oilRunStart
    ui->lineEdit_109->setValidator( new QDoubleValidator(-100000, 1000000, 5, this) ); // float modbus monitor
    ui->lineEdit_111->setValidator( new QDoubleValidator(-100000, 1000000, 5, this) ); // integer modbus monitor
}


void
MainWindow::
delay(int msec = 2000)
{
    QTime dieTime= QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}


void
MainWindow::
initializeModbusMonitor()
{
    ui->groupBox_103->setEnabled(TRUE);
    ui->groupBox_106->setEnabled(FALSE);
    ui->groupBox_107->setEnabled(FALSE);
    ui->functionCode->setCurrentIndex(3);
}


void
MainWindow::
initializeToolbarIcons() {

    ui->toolBar->addAction(ui->actionOpen);
    ui->toolBar->addAction(ui->actionSave);

    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionStart);
    ui->toolBar->addAction(ui->actionPause);
	ui->actionStart->setVisible(true);
	ui->actionPause->setVisible(false);
    ui->toolBar->addAction(ui->actionSkip);
    ui->toolBar->addAction(ui->actionStop);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionReadMasterPipe);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionSync);
	ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionStopInjection);
	ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionSettings);
}

void
MainWindow::
keyPressEvent(QKeyEvent* event)
{
    if( event->key() == Qt::Key_Control )
    {
        //set flag to request polling
        if( LOOP.modbus != NULL )	m_poll = true;
        if( ! m_pollTimer->isActive() )	ui->sendBtn->setText( tr("Poll") );
    }
}

void
MainWindow::
keyReleaseEvent(QKeyEvent* event)
{
    if( event->key() == Qt::Key_Control )
    {
        m_poll = false;
        if( ! m_pollTimer->isActive() )	ui->sendBtn->setText( tr("Send") );
    }
}

void MainWindow::onSendButtonPress( void )
{
    // if already polling then stop
    if( m_pollTimer->isActive() )
    {
        m_pollTimer->stop();
        ui->sendBtn->setText( tr("Send") );
    }
    else
    {
        // if polling requested then enable timer
        if( m_poll )
        {
            m_pollTimer->start( 1000 );
            ui->sendBtn->setText( tr("Stop") );
        }

        if (ui->repeatSending->isChecked()) 
		{
			while (ui->repeatSending->isChecked()) 
			{
				sendModbusRequest();
				delay();
			}
		}
		else sendModbusRequest();
    }
}

void MainWindow::busMonitorAddItem( bool isRequest,
                    uint16_t slave,
                    uint8_t func,
                    uint16_t addr,
                    uint16_t nb,
                    uint16_t expectedCRC,
                    uint16_t actualCRC )
{
    QTableWidget * bm = ui->busMonTable;
    const int rowCount = bm->rowCount();
    bm->setRowCount( rowCount+1 );

    QTableWidgetItem * numItem;
    QTableWidgetItem * ioItem = new QTableWidgetItem( isRequest ? tr( "Req >>" ) : tr( "<< Resp" ) );
    QTableWidgetItem * slaveItem = new QTableWidgetItem( QString::number( slave ) );
    QTableWidgetItem * funcItem = new QTableWidgetItem( QString::number( func ) );
    QTableWidgetItem * addrItem = new QTableWidgetItem( QString::number( addr ) );
    (ui->radioButton_181->isChecked()) ? numItem  = new QTableWidgetItem( QString::number( 2 ) ) : numItem = new QTableWidgetItem( QString::number( 1 ) );
    QTableWidgetItem * crcItem = new QTableWidgetItem;

    if( func > 127 )
    {
        addrItem->setText( QString() );
        numItem->setText( QString() );
        funcItem->setText( tr( "Exception (%1)" ).arg( func-128 ) );
        funcItem->setForeground( Qt::red );
    }
    else
    {
        if( expectedCRC == actualCRC )
        {
            crcItem->setText( QString().sprintf( "%.4x", actualCRC ) );
        }
        else
        {
            crcItem->setText( QString().sprintf( "%.4x (%.4x)", actualCRC, expectedCRC ) );
            crcItem->setForeground( Qt::red );
        }
    }
    ioItem->setFlags( ioItem->flags() & ~Qt::ItemIsEditable );
    slaveItem->setFlags( slaveItem->flags() & ~Qt::ItemIsEditable );
    funcItem->setFlags( funcItem->flags() & ~Qt::ItemIsEditable );
    addrItem->setFlags( addrItem->flags() & ~Qt::ItemIsEditable );
    numItem->setFlags( numItem->flags() & ~Qt::ItemIsEditable );
    crcItem->setFlags( crcItem->flags() & ~Qt::ItemIsEditable );
    bm->setItem( rowCount, 0, ioItem );
    bm->setItem( rowCount, 1, slaveItem );
    bm->setItem( rowCount, 2, funcItem );
    bm->setItem( rowCount, 3, addrItem );
    bm->setItem( rowCount, 4, numItem );
    bm->setItem( rowCount, 5, crcItem );
    bm->verticalScrollBar()->setValue( bm->verticalScrollBar()->maximum() );
}


void MainWindow::busMonitorRawData( uint8_t * data, uint8_t dataLen, bool addNewline )
{
    if( dataLen > 0 )
    {
        QString dump = ui->rawData->toPlainText();
        for( int i = 0; i < dataLen; ++i )
        {
            dump += QString().sprintf( "%.2x ", data[i] );
        }
        if( addNewline )
        {
            dump += "\n";
        }
        ui->rawData->setPlainText( dump );
        ui->rawData->verticalScrollBar()->setValue( 100000 );
        ui->rawData->setLineWrapMode( QPlainTextEdit::NoWrap );
    }
}

// static
void MainWindow::stBusMonitorAddItem( modbus_t * modbus, uint8_t isRequest, uint16_t slave, uint8_t func, uint16_t addr, uint16_t nb, uint16_t expectedCRC, uint16_t actualCRC )
{
    Q_UNUSED(modbus);
    globalMainWin->busMonitorAddItem( isRequest, slave, func, addr+1, nb, expectedCRC, actualCRC );
}

// static
void MainWindow::stBusMonitorRawData( modbus_t * modbus, uint8_t * data, uint8_t dataLen, uint8_t addNewline )
{
    Q_UNUSED(modbus);
    globalMainWin->busMonitorRawData( data, dataLen, addNewline != 0 );
}

static QString descriptiveDataTypeName( int funcCode )
{
    switch( funcCode )
    {
        case MODBUS_FC_READ_COILS:
        case MODBUS_FC_WRITE_SINGLE_COIL:
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
            return "Coil (binary)";
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            return "Discrete Input (binary)";
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            return "Holding Register (16 bit)";
        case MODBUS_FC_READ_INPUT_REGISTERS:
            return "Input Register (16 bit)";
        default:
            break;
    }
    return "Unknown";
}


static inline QString embracedString( const QString & s )
{
    return s.section( '(', 1 ).section( ')', 0, 0 );
}


static inline int stringToHex( QString s )
{
    return s.replace( "0x", "" ).toInt( NULL, 16 );
}


void MainWindow::updateRequestPreview( void )
{
    const int slave = ui->slaveID->value();
    const int func = stringToHex( embracedString(ui->functionCode->currentText() ) );
    const int addr = ui->startAddr->value()-1;
    const int num = ui->numCoils->value();
    if( func == MODBUS_FC_WRITE_SINGLE_COIL || func == MODBUS_FC_WRITE_SINGLE_REGISTER )
    {
        ui->requestPreview->setText(
            QString().sprintf( "%.2x  %.2x  %.2x %.2x ",
                    slave,
                    func,
                    addr >> 8,
                    addr & 0xff ) );
    }
    else
    {
        ui->requestPreview->setText(
            QString().sprintf( "%.2x  %.2x  %.2x %.2x  %.2x %.2x",
                    slave,
                    func,
                    addr >> 8,
                    addr & 0xff,
                    num >> 8,
                    num & 0xff ) );
    }
}




void MainWindow::updateRegisterView( void )
{
    const int func = stringToHex( embracedString(ui->functionCode->currentText() ) );
    const QString funcType = descriptiveDataTypeName( func );
    const int addr = ui->startAddr->value();

    int rowCount = 0;
    switch( func )
    {
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        case MODBUS_FC_WRITE_SINGLE_COIL:
            ui->numCoils->setEnabled( false );
            rowCount = 1;
            break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            rowCount = ui->numCoils->value();
        default:
            ui->numCoils->setEnabled( true );
            break;
    }

    ui->regTable->setRowCount( rowCount );
    for( int i = 0; i < rowCount; ++i )
    {
        QTableWidgetItem * dtItem = new QTableWidgetItem( funcType );
        QTableWidgetItem * addrItem = new QTableWidgetItem( QString::number( addr+i ) );
        QTableWidgetItem * dataItem = new QTableWidgetItem( QString::number( 0 ) );

        dtItem->setFlags( dtItem->flags() & ~Qt::ItemIsEditable	);
        addrItem->setFlags( addrItem->flags() & ~Qt::ItemIsEditable );
        ui->regTable->setItem( i, DataTypeColumn, dtItem );
        ui->regTable->setItem( i, AddrColumn, addrItem );
        ui->regTable->setItem( i, DataColumn, dataItem );
    }

    ui->regTable->setColumnWidth( 0, 150 );
}


void MainWindow::enableHexView( void )
{
    const int func = stringToHex( embracedString(
                    ui->functionCode->currentText() ) );

    bool b_enabled =
        func == MODBUS_FC_READ_HOLDING_REGISTERS ||
        func == MODBUS_FC_READ_INPUT_REGISTERS;

    //ui->checkBoxHexData->setEnabled( b_enabled );
}


void MainWindow::sendModbusRequest( void )
{
    // UPDATE m_modbus_snipping WITH THE CURRENT
    m_modbus_snipping = LOOP.modbus;

    if( m_modbus_snipping == NULL )
    {
        setStatusError( tr("Not configured!") );
        return;
    }

    const int slave = ui->slaveID->value();
    const int func = stringToHex(embracedString(ui->functionCode->currentText()));
    const int addr = ui->startAddr->value()-1;
    int num = ui->numCoils->value();
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;

    memset( dest, 0, 1024 );

    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( func );

    modbus_set_slave( m_modbus_snipping, slave );

    switch( func )
    {
        case MODBUS_FC_READ_COILS:
            ret = modbus_read_bits( m_modbus_snipping, addr, num, dest );
            break;
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            ret = modbus_read_input_bits( m_modbus_snipping, addr, num, dest );
            break;
        case MODBUS_FC_READ_HOLDING_REGISTERS:
            ret = modbus_read_registers( m_modbus_snipping, addr, num, dest16 );
            is16Bit = true;
            break;
        case MODBUS_FC_READ_INPUT_REGISTERS:
            ret = modbus_read_input_registers(m_modbus_snipping, addr, num, dest16 );
            is16Bit = true;
            break;
        case MODBUS_FC_WRITE_SINGLE_COIL:
            //ret = modbus_write_bit( m_modbus_snipping, addr,ui->regTable->item( 0, DataColumn )->text().toInt(0, 0) ? 1 : 0 );
            ret = modbus_write_bit( m_modbus_snipping, addr,ui->radioButton_184->isChecked() ? 1 : 0 );
            writeAccess = true;
            num = 1;
            break;
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
            //ret = modbus_write_register( m_modbus_snipping, addr,ui->regTable->item( 0, DataColumn )->text().toInt(0, 0) );
            ret = modbus_write_register( m_modbus_snipping, addr,ui->lineEdit_111->text().toInt(0, 0) );
            writeAccess = true;
            num = 1;
            break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        {
            uint8_t * data = new uint8_t[num];
            for( int i = 0; i < num; ++i ) data[i] = ui->regTable->item( i, DataColumn )->text().toInt(0, 0);
            ret = modbus_write_bits( m_modbus_snipping, addr, num, data );
            delete[] data;
            writeAccess = true;
            break;
        }
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        {
            float value;
            QString qvalue = ui->lineEdit_109->text();
            QTextStream floatTextStream(&qvalue);
            floatTextStream >> value;
            quint16 (*reg)[2] = reinterpret_cast<quint16(*)[2]>(&value);
            uint16_t * data = new uint16_t[2];
            data[0] = (*reg)[1];
            data[1] = (*reg)[0];
            ret = modbus_write_registers( m_modbus_snipping, addr, 2, data );
            delete[] data;
            writeAccess = true;
            break;
        }
        default:
            break;
    }

    if( ret == num  )
    {
        isModbusTransmissionFailed = false;

        if( writeAccess )
        {
            m_statusText->setText(tr( "Values successfully sent" ) );
            m_statusInd->setStyleSheet( "background: #0b0;" );
            m_statusTimer->start( 2000 );
        }
        else
        {
            bool b_hex = false; //is16Bit && ui->checkBoxHexData->checkState() == Qt::Checked;
            QString qs_num;
            QString qs_output = "0x";
            bool ok = false;

            ui->regTable->setRowCount( num );
            for( int i = 0; i < num; ++i )
            {
                int data = is16Bit ? dest16[i] : dest[i];
                QString qs_tmp;

                QTableWidgetItem * dtItem = new QTableWidgetItem( funcType );
                QTableWidgetItem * addrItem = new QTableWidgetItem(QString::number( ui->startAddr->value()+i ) );
                qs_num.sprintf( b_hex ? "0x%04x" : "%d", data);
                qs_tmp.sprintf("%04x", data);
                qs_output.append(qs_tmp);
                QTableWidgetItem * dataItem = new QTableWidgetItem( qs_num );
                dtItem->setFlags( dtItem->flags() & ~Qt::ItemIsEditable );
                addrItem->setFlags( addrItem->flags() & ~Qt::ItemIsEditable );
                dataItem->setFlags( dataItem->flags() & ~Qt::ItemIsEditable );
                ui->regTable->setItem( i, DataTypeColumn, dtItem );
                ui->regTable->setItem( i, AddrColumn, addrItem );
                ui->regTable->setItem( i, DataColumn, dataItem );
                if (ui->radioButton_182->isChecked()) ui->lineEdit_111->setText(QString::number(data));
                else if (ui->radioButton_183->isChecked())
                {
                    (data) ? ui->radioButton_184->setChecked(true) : ui->radioButton_185->setChecked(true);
                }
            }

            QByteArray array = QByteArray::fromHex(qs_output.toLatin1());
            const float d = toFloat(array);

            if (ui->radioButton_181->isChecked())
            {
                (b_hex) ? ui->lineEdit_109->setText(qs_output) : ui->lineEdit_109->setText(QString::number(d,'f',10)) ;
            }
        }
    }
    else
    {
        QString err;

        if( ret < 0 )
        {
            if(
#ifdef WIN32
                    errno == WSAETIMEDOUT ||
#endif
                    errno == EIO
                                                                    )
            {
                err += tr( "I/O error" );
                err += ": ";
                err += tr( "did not receive any data from slave." );
            }
            else
            {
                err += tr( "Protocol error" );
                err += ": ";
                err += tr( "Slave threw a exception '" );
                err += modbus_strerror( errno );
                err += tr( "' or function not implemented." );
            }
        }
        else
        {
            err += tr( "Protocol error" );
            err += ": ";
            err += tr( "Number of registers returned does not "
                    "match number of registers requested!" );
        }

        if( err.size() > 0 )
            setStatusError( err );

        isModbusTransmissionFailed = true;
    }
}

void MainWindow::resetStatus( void )
{
    m_statusText->setText( tr( "Ready" ) );
    m_statusInd->setStyleSheet( "background: #aaa;" );
}

void MainWindow::pollForDataOnBus( void )
{
    if( LOOP.modbus )
    {
        modbus_poll( LOOP.modbus );
    }
}

void MainWindow::aboutQModBus( void )
{
    AboutDialog( this ).exec();
}

void MainWindow::onRtuPortActive(bool active)
{
    if (active) {
        LOOP.modbus = this->modbus();

        //LOOP[loop].modbus = this->modbus();
        if (LOOP.modbus) {
            modbus_register_monitor_add_item_fnc(LOOP.modbus, MainWindow::stBusMonitorAddItem);
            modbus_register_monitor_raw_data_fnc(LOOP.modbus, MainWindow::stBusMonitorRawData);
        }
    }
    else LOOP.modbus = NULL;
}


void
MainWindow::
setStatusError(const QString &msg)
{
    m_statusText->setText( msg );
    m_statusInd->setStyleSheet( "background: red;" );
    m_statusTimer->start( 2000 );
}


void
MainWindow::
initializePipeObjects()
{
    /// label
    PIPE[0].pipeId = "P1";
    PIPE[1].pipeId = "P2";
    PIPE[2].pipeId = "P3";

    /// slave
    PIPE[0].slave = ui->lineEdit_2;
    PIPE[1].slave = ui->lineEdit_7;
    PIPE[2].slave = ui->lineEdit_13;

    /// on/off graph line view
    PIPE[0].lineView = ui->checkBox_19;
    PIPE[1].lineView = ui->checkBox_20;
    PIPE[2].lineView = ui->checkBox_21;

    /// on/off pipe switch
    PIPE[0].checkBox = ui->checkBox;
    PIPE[1].checkBox = ui->checkBox_2;
    PIPE[2].checkBox = ui->checkBox_3;

    /// lcdWatercut
    PIPE[0].wc = ui->lineEdit_3;
    PIPE[1].wc = ui->lineEdit_8;
    PIPE[2].wc = ui->lineEdit_14;

    /// lcdStartFreq
    PIPE[0].startFreq = ui->lineEdit_4;
    PIPE[1].startFreq = ui->lineEdit_9;
    PIPE[2].startFreq = ui->lineEdit_15;

    /// lcdFreq
    PIPE[0].freq = ui->lineEdit_5;
    PIPE[1].freq = ui->lineEdit_10;
    PIPE[2].freq = ui->lineEdit_16;

    /// lcdTemp
    PIPE[0].temp = ui->lineEdit_6;
    PIPE[1].temp = ui->lineEdit_11;
    PIPE[2].temp = ui->lineEdit_17;

    /// lcd
    PIPE[0].reflectedPower = ui->lineEdit_12;
    PIPE[1].reflectedPower = ui->lineEdit_19;
    PIPE[2].reflectedPower = ui->lineEdit_18;

    /// stability progressbar
    PIPE[0].freqProgress = ui->progressBar;
    PIPE[0].tempProgress = ui->progressBar_2;
    PIPE[1].freqProgress = ui->progressBar_4;
    PIPE[1].tempProgress = ui->progressBar_3;
    PIPE[2].freqProgress = ui->progressBar_6;
    PIPE[2].tempProgress = ui->progressBar_5;
}


void
MainWindow::
initializeLoopObjects()
{
    /// loop volume
    LOOP.loopVolume = ui->lineEdit;

    /// saltStart
    LOOP.saltStart = ui->comboBox_31;

    /// saltStop
    LOOP.saltStop = ui->comboBox_33;

    /// oilTemp
    LOOP.oilTemp = ui->comboBox_32;

    /// waterRunStart
    LOOP.waterRunStart = ui->lineEdit_37;

    /// waterRunStop
    LOOP.waterRunStop = ui->lineEdit_38;

    /// oilRunStart
    LOOP.oilRunStart = ui->lineEdit_39;

    /// oilRunStop
    LOOP.oilRunStop = ui->lineEdit_40;
}


void
MainWindow::
initializeGraph()
{
    ui->gridLayout_5->addWidget(LOOP.chartView,0,0);

    /// setup chart
    LOOP.chart->legend()->hide();
    LOOP.chartView->setChart(LOOP.chart);
    LOOP.chartView->setRubberBand(QChartView::HorizontalRubberBand);

    /// set pen color
    PIPE[0].pen.setColor(Qt::red);
    PIPE[0].pen2.setColor(Qt::red);
    PIPE[1].pen.setColor(Qt::blue);
    PIPE[1].pen2.setColor(Qt::blue);
    PIPE[2].pen.setColor(Qt::black);
    PIPE[2].pen2.setColor(Qt::black);

    /// setPen and addSeries
    for (int pipe=0; pipe<3; pipe++)
    {
        PIPE[pipe].series->setPen(PIPE[pipe].pen);
        PIPE[pipe].series_2->setPen(PIPE[pipe].pen2);
        LOOP.chart->addSeries(PIPE[pipe].series);
        LOOP.chart->addSeries(PIPE[pipe].series_2);
    }

    /// addAxis
    LOOP.chart->addAxis(LOOP.axisX, Qt::AlignBottom);
    LOOP.chart->addAxis(LOOP.axisY, Qt::AlignLeft);
    LOOP.chart->addAxis(LOOP.axisY2, Qt::AlignRight);

    /// axisX
    LOOP.axisX->setRange(0,1000);
    LOOP.axisX->setTickCount(21);
    LOOP.axisX->setTickInterval(100);
    LOOP.axisX->setLabelFormat("%i");
    LOOP.axisX->setTitleText("Frequency (Mhz)");

    /// axisY
    LOOP.axisY->setRange(0,100);
    LOOP.axisY->setTickCount(11);
    LOOP.axisY->setLabelFormat("%i");
    LOOP.axisY->setTitleText("Watercut (%)");

    /// axisY2
    LOOP.axisY2->setRange(0,5);
    LOOP.axisY2->setTickCount(11);
    LOOP.axisY2->setLabelFormat("%.1f");
    LOOP.axisY2->setTitleText("Reflected Power (V)");

    for (int pipe=0; pipe<3; pipe++)
    {
        PIPE[pipe].series->attachAxis(LOOP.axisX);
        PIPE[pipe].series->attachAxis(LOOP.axisY);
        PIPE[pipe].series_2->attachAxis(LOOP.axisX);
        PIPE[pipe].series_2->attachAxis(LOOP.axisY2);
    }

    for (int pipe=0; pipe<3; pipe++)(PIPE[pipe].lineView) ? PIPE[pipe].series->show() : PIPE[pipe].series->hide();
    for (int pipe=0; pipe<3; pipe++)(PIPE[pipe].lineView) ? PIPE[pipe].series_2->show() : PIPE[pipe].series_2->hide();
}


void
MainWindow::
updateGraph(const int pipe, const double x, const double y, const int Series)
{
	
	if ((x == RESET_SERIES) && (y == RESET_SERIES))
	{
		if (pipe == ALL)
		{
			for (int i=0; i<3; i++)
			{
				PIPE[i].series->remove(0);
				PIPE[i].series_2->remove(0);
			}
			return;
		}

		PIPE[pipe].series->remove(0);
		PIPE[pipe].series_2->remove(0);
		return;
	}

    if (Series == SERIES_WATERCUT) PIPE[pipe].series->append(x,y);
    else PIPE[pipe].series_2->append(x,y);
}

void
MainWindow::
connectTimers()
{
    QTimer * t = new QTimer( this );
    connect( t, SIGNAL(timeout()), this, SLOT(pollForDataOnBus()));
    t->start( 5 );

    m_pollTimer = new QTimer( this );
    connect( m_pollTimer, SIGNAL(timeout()), this, SLOT(sendModbusRequest()));

    m_statusTimer = new QTimer( this );
    connect( m_statusTimer, SIGNAL(timeout()), this, SLOT(resetStatus()));
    m_statusTimer->setSingleShot(true);
}


void
MainWindow::
connectRadioButtons()
{   
	/// cut (high, full, mid, low)
	connect(ui->radioButton_3, SIGNAL(pressed()), this, SLOT(onHighSelected()));
    connect(ui->radioButton_4, SIGNAL(pressed()), this, SLOT(onFullSelected()));
    connect(ui->radioButton_5, SIGNAL(pressed()), this, SLOT(onMidSelected()));
    connect(ui->radioButton_6, SIGNAL(pressed()), this, SLOT(onLowSelected()));

	// product
    connect(ui->radioButton_2, SIGNAL(toggled(bool)), ui->radioButton_15, SLOT(setEnabled(bool)));
    connect(ui->radioButton, SIGNAL(toggled(bool)), ui->radioButton_16, SLOT(setChecked(bool)));

    connect(ui->radioButton_187, SIGNAL(pressed()), this, SLOT(onReadButtonPressed()));
    connect(ui->radioButton_186, SIGNAL(pressed()), this, SLOT(onWriteButtonPressed()));
	
    connect(ui->radioButton_181, SIGNAL(pressed()), this, SLOT(onDataTypeChanged()));
    connect(ui->radioButton_182, SIGNAL(pressed()), this, SLOT(onDataTypeChanged()));
    connect(ui->radioButton_183, SIGNAL(pressed()), this, SLOT(onDataTypeChanged()));
}



void
MainWindow::
connectSerialPort()
{
    connect( ui->groupBox_18, SIGNAL(toggled(bool)), this, SLOT(onCheckBoxChecked(bool)));
    connect( this, SIGNAL(connectionError(const QString&)), this, SLOT(setStatusError(const QString&)));
}


void
MainWindow::
connectActions()
{
    connect( ui->actionAbout_QModBus, SIGNAL( triggered() ),this, SLOT( aboutQModBus() ) );
    connect( ui->functionCode, SIGNAL( currentIndexChanged( int ) ),this, SLOT( enableHexView() ) );
}


void
MainWindow::
connectModbusMonitor()
{
    connect( ui->slaveID, SIGNAL( valueChanged( int ) ),this, SLOT( updateRequestPreview() ) );
    connect( ui->functionCode, SIGNAL( currentIndexChanged( int ) ),this, SLOT( updateRequestPreview() ) );
    connect( ui->startAddr, SIGNAL( valueChanged( int ) ),this, SLOT( updateRequestPreview() ) );
    connect( ui->numCoils, SIGNAL( valueChanged( int ) ),this, SLOT( updateRequestPreview() ) );
    connect( ui->functionCode, SIGNAL( currentIndexChanged( int ) ),this, SLOT( updateRegisterView() ) );
    connect( ui->numCoils, SIGNAL( valueChanged( int ) ),this, SLOT( updateRegisterView() ) );
    connect( ui->startAddr, SIGNAL( valueChanged( int ) ),this, SLOT( updateRegisterView() ) );
    connect( ui->sendBtn, SIGNAL(pressed()),this, SLOT( onSendButtonPress() ) );
    connect( ui->groupBox_105, SIGNAL( toggled(bool)), this, SLOT( onEquationTableChecked(bool)));
}


void
MainWindow::
connectToolbar()
{
    connect(ui->actionSave, SIGNAL(triggered()),this,SLOT(saveCsvFile()));
    connect(ui->actionOpen, SIGNAL(triggered()),this,SLOT(loadCsvFile()));
    connect(ui->actionSettings, SIGNAL(triggered()),this,SLOT(onActionSettings()));
    connect(ui->actionSync, SIGNAL(triggered()),this,SLOT(onActionSync()));
    connect(ui->actionStart, SIGNAL(triggered()),this,SLOT(onActionStart()));
    connect(ui->actionStop, SIGNAL(triggered()),this,SLOT(onActionStopPressed()));
    connect(ui->actionPause, SIGNAL(triggered()),this,SLOT(onActionPause()));
    connect(ui->actionReadMasterPipe, SIGNAL(triggered()),this,SLOT(onActionReadMasterPipe()));
    connect(ui->actionStopInjection, SIGNAL(triggered()),this,SLOT(onActionStopInjection()));
    connect(ui->actionSkip, SIGNAL(triggered()),this,SLOT(onActionSkip()));

    /// injection pump rates
    connect(ui->actionInjection_Pump_Rates, SIGNAL(triggered()),this,SLOT(injectionPumpRates()));

    /// injection bucket
    connect(ui->actionInjection_Bucket, SIGNAL(triggered()),this,SLOT(injectionBucket()));

    /// injection mark
    connect(ui->actionInjection_Mark, SIGNAL(triggered()),this, SLOT(injectionMark()));

    /// injection method
    connect(ui->actionInjection_Method, SIGNAL(triggered()),this, SLOT(injectionMethod()));

    /// pressure sensor slope
    connect(ui->actionPressure_Sensor_Slope, SIGNAL(triggered()),this, SLOT(onActionPressureSensorSlope()));

    /// Minimum reference temperature
    connect(ui->actionMin_Ref_Temp, SIGNAL(triggered()), this, SLOT(onMinRefTemp()));

    /// Maximum reference temperature
    connect(ui->actionMax_Ref_Temp, SIGNAL(triggered()),this, SLOT(onMaxRefTemp()));

    /// injection temperature
    connect(ui->actionInjection_Temp, SIGNAL(triggered()),this, SLOT(onInjectionTemp()));

    /// Delta Stability X seconds
    connect(ui->actionX_seconds, SIGNAL(triggered()),this, SLOT(onXDelay()));

    /// Delta Stability Frequency
    connect(ui->actionY_KHz, SIGNAL(triggered()),this, SLOT(onYFreq()));

    /// Delta Stability Temperature
    connect(ui->actionZ_C, SIGNAL(triggered()),this, SLOT(onZTemp()));

    /// Interval - calibration
    connect(ui->actionCalibration, SIGNAL(triggered()),this, SLOT(onIntervalSmallPump()));

    /// Interval - calibration
    connect(ui->actionRollover, SIGNAL(triggered()),this, SLOT(onIntervalBigPump()));

    /// Loop Number
    connect(ui->actionLOOP_NUMBER, SIGNAL(triggered()),this, SLOT(onLoopNumber()));

    /// master pipe
    connect(ui->actionMin_Watercut, SIGNAL(triggered()),this, SLOT(onActionMinMaster()));
    connect(ui->actionMax_Watercut, SIGNAL(triggered()),this, SLOT(onActionMaxMaster()));
    connect(ui->actionDelta_Master, SIGNAL(triggered()),this, SLOT(onActionDeltaMaster()));
    connect(ui->actionFinal_Delta_Watercut, SIGNAL(triggered()),this, SLOT(onActionDeltaMasterFinal()));

    /// max injection time
    connect(ui->actionWater, SIGNAL(triggered()),this, SLOT(onActionWater()));
    connect(ui->actionOil, SIGNAL(triggered()),this, SLOT(onActionOil()));

    /// file folders
    connect(ui->actionMain_Server, SIGNAL(triggered()),this,SLOT(onActionMainServer()));
    connect(ui->actionLocal_Server, SIGNAL(triggered()),this,SLOT(onActionLocalServer()));
}


void
MainWindow::
connectCheckbox()
{
    connect(ui->checkBox, SIGNAL(clicked(bool)),this, SLOT(onCheckBoxClicked(bool)));
    connect(ui->checkBox_2, SIGNAL(clicked(bool)),this, SLOT(onCheckBoxClicked(bool)));
    connect(ui->checkBox_3, SIGNAL(clicked(bool)),this, SLOT(onCheckBoxClicked(bool)));
}


void
MainWindow::
onCheckBoxClicked(const bool isChecked)
{
    (ui->checkBox->isChecked()) ?  PIPE[0].status = ENABLED : PIPE[0].status = DONE;
    (ui->checkBox_2->isChecked()) ?  PIPE[1].status = ENABLED : PIPE[1].status = DONE;
    (ui->checkBox_3->isChecked()) ?  PIPE[2].status = ENABLED : PIPE[2].status = DONE;
}


/// hide/show graph line
void
MainWindow::
toggleLineView_P1(bool b)
{
   (b) ? PIPE[0].series->show() : PIPE[0].series->hide();
   (b) ? PIPE[0].series_2->show() : PIPE[0].series_2->hide();
}

void
MainWindow::
toggleLineView_P2(bool b)
{
   (b) ? PIPE[1].series->show() : PIPE[1].series->hide();
   (b) ? PIPE[1].series_2->show() : PIPE[1].series_2->hide();
}

void
MainWindow::
toggleLineView_P3(bool b)
{
   (b) ? PIPE[2].series->show() : PIPE[2].series->hide();
   (b) ? PIPE[2].series_2->show() : PIPE[2].series_2->hide();
}

void
MainWindow::
onViewYAxisData(const bool isTrue)
{
    if (ui->radioButton_17->isChecked())
    {
        for (int pipe=0; pipe<3; pipe++) PIPE[pipe].series->show();
        for (int pipe=0; pipe<3; pipe++) PIPE[pipe].series_2->show();
    }

    else if (ui->radioButton_18->isChecked())
    {
        for (int pipe=0; pipe<3; pipe++) PIPE[pipe].series->show();
        for (int pipe=0; pipe<3; pipe++) PIPE[pipe].series_2->hide();
    }
    else
    {
        for (int pipe=0; pipe<3; pipe++) PIPE[pipe].series->hide();
        for (int pipe=0; pipe<3; pipe++) PIPE[pipe].series_2->show();
    }
}


void
MainWindow::
connectLineView()
{
    connect(ui->checkBox_19, SIGNAL(clicked(bool)), this, SLOT(toggleLineView_P1(bool)));
    connect(ui->checkBox_20, SIGNAL(clicked(bool)), this, SLOT(toggleLineView_P2(bool)));
    connect(ui->checkBox_21, SIGNAL(clicked(bool)), this, SLOT(toggleLineView_P3(bool)));

    connect(ui->radioButton_17, SIGNAL(toggled(bool)), this, SLOT(onViewYAxisData(bool)));
    connect(ui->radioButton_18, SIGNAL(toggled(bool)), this, SLOT(onViewYAxisData(bool)));
    connect(ui->radioButton_19, SIGNAL(toggled(bool)), this, SLOT(onViewYAxisData(bool)));
}


void
MainWindow::
connectMasterPipe()
{
    connect(ui->radioButton_11, SIGNAL(toggled(bool)), this, SLOT(onMasterPipeToggled(bool)));
}

void
MainWindow::
onMasterPipeToggled(const bool isEnabled)
{
    (isEnabled) ? LOOP.isMaster = true : LOOP.isMaster = false;
}

void
MainWindow::
readJsonConfigFile()
{
    bool ok = false;
    QFile file("./sparky.json");
    file.open(QIODevice::ReadOnly);
    QString jsonString = file.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonString.toUtf8()));
    QJsonObject jsonObj = jsonDoc.object();
    QVariantMap json = jsonObj.toVariantMap();

    /// file server
    m_mainServer = json[MAIN_SERVER].toString();
    m_localServer = json[LOCAL_SERVER].toString();

    /// calibration control variables
    LOOP.injectionOilPumpRate = json[LOOP_OIL_PUMP_RATE].toDouble();
    LOOP.injectionWaterPumpRate = json[LOOP_WATER_PUMP_RATE].toDouble();
    LOOP.injectionSmallWaterPumpRate = json[LOOP_SMALL_WATER_PUMP_RATE].toDouble();
    LOOP.injectionBucket = json[LOOP_BUCKET].toDouble();
    LOOP.injectionMark = json[LOOP_MARK].toDouble();
    LOOP.injectionMethod = json[LOOP_METHOD].toDouble();
    LOOP.pressureSensorSlope = json[LOOP_PRESSURE].toDouble();
    LOOP.minTemp = json[LOOP_MIN_TEMP].toInt();
    LOOP.maxTemp = json[LOOP_MAX_TEMP].toInt();
    LOOP.injectTemp = json[LOOP_INJECTION_TEMP].toInt();
    LOOP.xDelay = json[LOOP_X_DELAY].toInt();
    LOOP.yFreq = json[LOOP_Y_FREQ].toDouble();
    LOOP.zTemp = json[LOOP_Z_TEMP].toDouble();
    LOOP.intervalSmallPump = json[LOOP_INTERVAL_SMALL_PUMP].toDouble();
    LOOP.intervalBigPump = json[LOOP_INTERVAL_BIG_PUMP].toDouble();
    LOOP.intervalOilPump = json[LOOP_INTERVAL_OIL_PUMP].toDouble();
    LOOP.loopNumber = json[LOOP_NUMBER].toInt();
    LOOP.masterMin = json[LOOP_MASTER_MIN].toDouble();
    LOOP.masterMax = json[LOOP_MASTER_MAX].toDouble();
    LOOP.masterDelta = json[LOOP_MASTER_DELTA].toDouble();
    LOOP.masterDeltaFinal = json[LOOP_MASTER_DELTA_FINAL].toDouble();
    LOOP.maxInjectionWater = json[LOOP_MAX_INJECTION_WATER].toInt();
    LOOP.maxInjectionOil = json[LOOP_MAX_INJECTION_OIL].toInt();
    LOOP.portIndex = json[LOOP_PORT_INDEX].toInt();

    /// main configuration panel
    ui->lineEdit_27->setText(QString::number(LOOP.injectionOilPumpRate));
    ui->lineEdit_28->setText(QString::number(LOOP.injectionWaterPumpRate));
    ui->lineEdit_82->setText(QString::number(LOOP.injectionSmallWaterPumpRate));

    ui->lineEdit_65->setText(QString::number(LOOP.maxInjectionOil));
    ui->lineEdit_66->setText(QString::number(LOOP.maxInjectionWater));

    ui->lineEdit_67->setText(QString::number(LOOP.minTemp));
    ui->lineEdit_68->setText (QString::number(LOOP.maxTemp));
    ui->lineEdit_69->setText(QString::number(LOOP.injectTemp));

    ui->lineEdit_72->setText(QString::number(LOOP.xDelay));
    ui->lineEdit_70->setText(QString::number(LOOP.yFreq));
    ui->lineEdit_71->setText(QString::number(LOOP.zTemp));

    ui->lineEdit_73->setText(QString::number(LOOP.intervalSmallPump));
    ui->lineEdit_74->setText(QString::number(LOOP.intervalBigPump));
    ui->lineEdit_79->setText(QString::number(LOOP.intervalOilPump));

    ui->lineEdit_75->setText(QString::number(LOOP.masterMin));
    ui->lineEdit_76->setText(QString::number(LOOP.masterMax));
    ui->lineEdit_77->setText(QString::number(LOOP.masterDelta));
    ui->lineEdit_78->setText(QString::number(LOOP.masterDeltaFinal));

    /// done. close file.
    file.close();
}


void
MainWindow::
writeJsonConfigFile(void)
{
    QFile file(QStringLiteral("sparky.json"));
    file.open(QIODevice::WriteOnly);

    QJsonObject json;

    LOOP.injectionOilPumpRate = ui->lineEdit_27->text().toDouble();
    LOOP.injectionWaterPumpRate = ui->lineEdit_28->text().toDouble();
    LOOP.injectionSmallWaterPumpRate = ui->lineEdit_82->text().toDouble();

    LOOP.maxInjectionOil = ui->lineEdit_65->text().toDouble();
    LOOP.maxInjectionWater = ui->lineEdit_66->text().toDouble();

    LOOP.minTemp = ui->lineEdit_67->text().toDouble();
    LOOP.maxTemp = ui->lineEdit_68->text().toDouble();
    LOOP.injectTemp = ui->lineEdit_69->text().toDouble();

    LOOP.xDelay =  ui->lineEdit_72->text().toDouble();
    LOOP.yFreq = ui->lineEdit_70->text().toDouble();
    LOOP.zTemp = ui->lineEdit_71->text().toDouble();

    LOOP.intervalSmallPump = ui->lineEdit_73->text().toDouble();
    LOOP.intervalBigPump = ui->lineEdit_74->text().toDouble();
    LOOP.intervalOilPump = ui->lineEdit_79->text().toDouble();

    LOOP.masterMin = ui->lineEdit_75->text().toDouble();
    LOOP.masterMax =  ui->lineEdit_76->text().toDouble();
    LOOP.masterDelta =  ui->lineEdit_77->text().toDouble();
    LOOP.masterDeltaFinal = ui->lineEdit_78->text().toDouble();

    /// calibration control variables
    json[LOOP_OIL_PUMP_RATE] = QString::number(LOOP.injectionOilPumpRate);
    json[LOOP_WATER_PUMP_RATE] = QString::number(LOOP.injectionWaterPumpRate);
    json[LOOP_SMALL_WATER_PUMP_RATE] = QString::number(LOOP.injectionSmallWaterPumpRate);
    json[LOOP_BUCKET] = QString::number(LOOP.injectionBucket);
    json[LOOP_MARK] = QString::number(LOOP.injectionMark);
    json[LOOP_METHOD] = QString::number(LOOP.injectionMethod);
    json[LOOP_PRESSURE] = QString::number(LOOP.pressureSensorSlope);
    json[LOOP_MIN_TEMP] = QString::number(LOOP.minTemp);
    json[LOOP_MAX_TEMP] = QString::number(LOOP.maxTemp);
    json[LOOP_INJECTION_TEMP] = QString::number(LOOP.injectTemp);
    json[LOOP_X_DELAY] = QString::number(LOOP.xDelay);
    json[LOOP_Y_FREQ] = QString::number(LOOP.yFreq);
    json[LOOP_Z_TEMP] = QString::number(LOOP.zTemp);
    json[LOOP_INTERVAL_SMALL_PUMP] = QString::number(LOOP.intervalSmallPump);
    json[LOOP_INTERVAL_BIG_PUMP] = QString::number(LOOP.intervalBigPump);
    json[LOOP_INTERVAL_OIL_PUMP] = QString::number(LOOP.intervalOilPump);
    json[LOOP_NUMBER] = QString::number(LOOP.loopNumber);
    json[LOOP_MASTER_MIN] = QString::number(LOOP.masterMin);
    json[LOOP_MASTER_MAX] = QString::number(LOOP.masterMax);
    json[LOOP_MASTER_DELTA] = QString::number(LOOP.masterDelta);
    json[LOOP_MASTER_DELTA_FINAL] = QString::number(LOOP.masterDeltaFinal);
    json[LOOP_MAX_INJECTION_WATER] = QString::number(LOOP.maxInjectionWater);
    json[LOOP_MAX_INJECTION_OIL] = QString::number(LOOP.maxInjectionOil);
    json[LOOP_PORT_INDEX] = QString::number(LOOP.portIndex);

    /// file server
    json[MAIN_SERVER] = m_mainServer;
    json[LOCAL_SERVER] = m_localServer;

    file.write(QJsonDocument(json).toJson());
    file.close();
}

void
MainWindow::
injectionPumpRates()
{
    bool ok;
    LOOP.injectionOilPumpRate = QInputDialog::getDouble(this,tr("Injection Oil Pump Rate"),tr("Enter Injection Oil Pump Rate [mL/min]"),LOOP.injectionOilPumpRate , -10000, 10000, 2, &ok,Qt::WindowFlags(), 1);
    LOOP.injectionWaterPumpRate = QInputDialog::getDouble(this,tr("Injection Water Pump Rate"),tr("Enter Injection Water Pump Rate [mL/min]"), LOOP.injectionWaterPumpRate, -10000, 10000, 2, &ok,Qt::WindowFlags(), 1);
    LOOP.injectionSmallWaterPumpRate = QInputDialog::getDouble(this,tr("Injection Small Water Pump Rate"),tr("Enter Injection Small Water Pump Rate [mL/min]"),LOOP.injectionSmallWaterPumpRate , -10000, 10000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
injectionBucket()
{
    bool ok;
    LOOP.injectionBucket = QInputDialog::getDouble(this, tr("Injection Bucket"),tr("Enter Injection Bucket [L]"),LOOP.injectionBucket , -10000, 10000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
injectionMark()
{
    bool ok;
    LOOP.injectionMark = QInputDialog::getDouble(this,tr("Injection Mark"),tr("Enter Injection Mark [L]"), LOOP.injectionMark, -10000, 10000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
injectionMethod()
{
    bool ok;
    LOOP.injectionMethod = QInputDialog::getDouble(this,tr("Injection Method"),tr("Enter Injection Method: "), LOOP.injectionMethod, -10000, 10000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
onActionPressureSensorSlope()
{
    bool ok;
    LOOP.pressureSensorSlope = QInputDialog::getDouble(this,tr("Pressure Sensor Slope"),tr("Enter Pressure Sensor Slope: "), LOOP.pressureSensorSlope, -10000, 10000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
onMinRefTemp()
{
    bool ok;
    LOOP.minTemp = QInputDialog::getInt(this,tr("Minimum Reference Temperature"),tr("Enter Minimum Reference Temperature: "), LOOP.minTemp, 0, 1000, 2, &ok);

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
onMaxRefTemp()
{
    bool ok;
    LOOP.maxTemp = QInputDialog::getInt(this,tr("Maximum Reference Temperature"),tr("Enter Maximum Reference Temperature: "), LOOP.maxTemp, 0, 1000, 2, &ok);

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
onInjectionTemp()
{
    bool ok;
    LOOP.injectTemp = QInputDialog::getInt(this,tr("Injection Temperature"),tr("Enter Injection Temperature: "), LOOP.injectTemp, 0, 1000, 2, &ok);

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
onXDelay()
{
    bool ok;
    LOOP.xDelay = QInputDialog::getInt(this,tr("X Delay"),tr("Enter Delay Peroid (seconds): "), LOOP.xDelay, 0, 3600, 2, &ok);

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
onYFreq()
{
    bool ok;
    LOOP.yFreq = QInputDialog::getDouble(this,tr("Y Delta Frequency"),tr("Enter Y Delta Fequency (KHz): 1000000"), LOOP.yFreq, 0, 1000000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
onZTemp()
{
    bool ok;
    LOOP.zTemp = QInputDialog::getDouble(this,tr("Z Delta Temperature"),tr("Enter Z Delta Temperature (°C): 0.1"), LOOP.zTemp, 0, 1000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
onIntervalSmallPump()
{
    bool ok;
    LOOP.intervalSmallPump = QInputDialog::getDouble(this,tr("Small Pump Interval [ % ]"),tr("Enter Small Pump Interval (%): 0.25"), LOOP.intervalSmallPump, 0, 1000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
onIntervalBigPump()
{
    bool ok;
    LOOP.intervalBigPump = QInputDialog::getDouble(this,tr("Big Pump Interval [ % ]"),tr("Enter Big Pump Interval (%): 1.0"), LOOP.intervalBigPump, 0, 1000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
onLoopNumber()
{
    bool ok;
    LOOP.loopNumber = QInputDialog::getInt(this,tr("Loop Number"),tr("Enter Loop Number : "), LOOP.loopNumber, 0, 999999, 2, &ok);

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
onActionMinMaster()
{
    bool ok;
    LOOP.masterMin = QInputDialog::getDouble(this,tr("Master Pipe"),tr("Enter Minimum Master Watercut (%): "), LOOP.masterMin, -1000, 1000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
onActionMaxMaster()
{
    bool ok;
    LOOP.masterMax = QInputDialog::getDouble(this,tr("Master Pipe"),tr("Enter Maximum Master Watercut (%): +0.15"), LOOP.masterMax, -1000, 1000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
onActionDeltaMaster()
{
    bool ok;
    LOOP.masterDelta = QInputDialog::getDouble(this,tr("Master Pipe"),tr("Enter Initial Delta Master Watercut (%): +0.15"), LOOP.masterDelta, -1000, 1000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
onActionDeltaMasterFinal()
{
    bool ok;
    LOOP.masterDeltaFinal = QInputDialog::getDouble(this,tr("Master Pipe"),tr("Enter Final Delta Master Watercut (%): +0.15"), LOOP.masterDeltaFinal, -1000, 1000, 2, &ok,Qt::WindowFlags(), 1);

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
onActionMainServer()
{
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Directory"),m_mainServer,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dirName.isEmpty() && !dirName.isNull()) m_mainServer = dirName;

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
onActionLocalServer()
{
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Directory"),m_localServer,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dirName.isEmpty() && !dirName.isNull()) m_localServer = dirName;

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
onActionWater()
{
    bool ok;
    LOOP.maxInjectionWater = QInputDialog::getInt(this,tr("Loop Number"),tr("Enter Max Injection Time for Water [ S ]: "), LOOP.maxInjectionWater, 0, 9999, 2, &ok);

    /// update json config file
    writeJsonConfigFile();
}

void
MainWindow::
onActionOil()
{
    bool ok;
    LOOP.maxInjectionOil = QInputDialog::getInt(this,tr("Loop Number"),tr("Enter Max Injection Time for Oil : [ S ]"), LOOP.maxInjectionOil, 0, 9999, 2, &ok);

    /// update json config file
    writeJsonConfigFile();
}


void
MainWindow::
onEquationTableChecked(bool isTable)
{
    if (!isTable) ui->tableWidget->setRowCount(0);
}

void
MainWindow::
write_request(int func, int address, double dval, int ival, bool bval)
{
    //////// address offset /////////
    int addr = address - ADDR_OFFSET;
    /////////////////////////////////

    switch( func )
    {
        case FUNC_WRITE_COIL:
            modbus_write_bit( LOOP.serialModbus, addr, bval);
			break;
        case FUNC_WRITE_INT	:
            modbus_write_register( LOOP.serialModbus, addr, ival);
			break;
        case FUNC_WRITE_FLOAT:
        {
            float value;
            QString qvalue = QString::number(dval);
            QTextStream floatTextStream(&qvalue);
            floatTextStream >> value;
            quint16 (*reg)[2] = reinterpret_cast<quint16(*)[2]>(&value);
            uint16_t * data = new uint16_t[2];
            data[0] = (*reg)[1];
            data[1] = (*reg)[0];
            modbus_write_registers( LOOP.serialModbus, addr, 2, data );
            delete[] data;
			break;
        }
        default:
			break;
    }
}


double
MainWindow::
read_request(int func, int address, int num, int ret, uint8_t * dest, uint16_t * dest16, bool is16Bit)
{
    //////// address offset /////////
    int addr = address - ADDR_OFFSET;
    /////////////////////////////////

    switch( func )
    {
        case FUNC_READ_COIL:
            ret = modbus_read_bits( LOOP.serialModbus, addr, num, dest );
            is16Bit = false;
            break;
		case FUNC_READ_INT:
		case FUNC_READ_FLOAT:
            ret = modbus_read_input_registers(LOOP.serialModbus, addr, num, dest16 );
            is16Bit = true;
            break;
        default:
            break;
    }

    if( ret == num  )
    {
        isModbusTransmissionFailed = false;

        {
            QString qs_num;
            QString qs_output = "0x";
            bool ok = false;
            float d;

            for( int i = 0; i < num; ++i )
            {
                int data = is16Bit ? dest16[i] : dest[i];
                QString qs_tmp;

                qs_num.sprintf("%d", data);
                qs_tmp.sprintf("%04x", data);
                qs_output.append(qs_tmp);

                if (func == FUNC_READ_INT) return data;
                else if (func == FUNC_READ_COIL) return (data) ? 1 : 0;
            }

            if (func == FUNC_READ_FLOAT)
            {
                QByteArray array = QByteArray::fromHex(qs_output.toLatin1());
                d = toFloat(array);
                return (double) d;
            }
        }
    }
    else
    {
        isModbusTransmissionFailed = true;
		//informUser("MODBUS Failed","Modbus Transmission Failed" ,QString("Address: ").append(QString::number(address)));
		return 0;
    }
}

void
MainWindow::
saveCsvFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save Equation"), "",tr("CSV file (*.csv);;All Files (*)"));

    if (fileName.isEmpty()) return;
    QFile file(fileName);
    QTextStream out(&file);

    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"),file.errorString());
        return;
    }

    for ( int i = 0; i < ui->tableWidget->rowCount(); i++ )
    {
        QString dataStream;

        for (int j=0; j < ui->tableWidget->item(i,6)->text().toInt()+7; j++)
        {
             dataStream.append(ui->tableWidget->item(i,j)->text()+",");
        }

        out << dataStream << endl;
    }

    file.close();
}


void
MainWindow::
onActionStartInjection()
{
	LOOP.isInject = true;
	LOOP.isTempRunSkip = false;

    if (LOOP.isOilRun) 
	{
		updateCurrentStage(BLACK,INJECT_WATER);
		inject(COIL_WATER_PUMP,LOOP.isInject);
	}
    else if (LOOP.isWaterRun) 
	{
		updateCurrentStage(BLACK,INJECT_OIL);
		inject(COIL_OIL_PUMP,LOOP.isInject);
	}
}


void
MainWindow::
onActionStopInjection()
{
	if (LOOP.isInject) updateCurrentStage(BLACK,INJECT_STANDBY);
	LOOP.isInject = false;
	LOOP.isTempRunSkip = false;

    inject(COIL_WATER_PUMP,LOOP.isInject);
    inject(COIL_OIL_PUMP,LOOP.isInject);
}

void
MainWindow::
onActionSkip()
{
	//if ((LOOP.runMode != TEMPRUN_MIN) && (LOOP.runMode != TEMPRUN_HIGH) && (LOOP.runMode != TEMPRUN_INJECT)) return;
	if (isUserInputYes("Skip Current Stage", "Do You Want To Skip and Go To Next Stage?")) LOOP.isTempRunSkip = true;
	if (LOOP.isPause == true)
	{
		ui->actionStart->setVisible(false);
		ui->actionPause->setVisible(true);
	
		LOOP.isPause = false;
	}
}

void
MainWindow::
onActionPause()
{
	ui->actionStart->setVisible(true);
	ui->actionPause->setVisible(false);

	LOOP.isPause = true;
	LOOP.isTempRunSkip = false;

	return;
}

void
MainWindow::
onActionStart()
{
	ui->actionStart->setVisible(false);
	ui->actionPause->setVisible(true);

	/// update configuration file with the latest select 
    writeJsonConfigFile();

	/// hide settings screen
    ui->groupBox_18->hide();
    ui->groupBox_35->hide();
    ui->groupBox_34->hide();
    ui->groupBox_33->hide();
    ui->groupBox_32->hide();
    ui->groupBox_31->hide();
    ui->groupBox_30->hide();
    ui->groupBox_29->hide();

	/// enable main screen
    ui->groupBox_12->show();
    ui->groupBox_20->show();

	/// graph display only
    ui->tabWidget->setTabEnabled(1,false);
    ui->tabWidget->setTabEnabled(2,false);
    ui->tabWidget->setTabEnabled(3,false);
    ui->tabWidget->setTabEnabled(4,false);

    updateLoopTabIcon(true);

    ui->groupBox_13->setEnabled(false);
    ui->groupBox_11->setEnabled(false);
    ui->groupBox_113->setEnabled(false);
    ui->groupBox_114->setEnabled(false);
    ui->groupBox_5->setEnabled(false);
    ui->groupBox_9->setEnabled(false);
    ui->groupBox_10->setEnabled(false);
    ui->groupBox_6->setEnabled(false);
    ui->groupBox_65->setEnabled(false);

	/// flipping trigger
    LOOP.isCal = !LOOP.isCal;

    if (!LOOP.isCal)
    {
        onActionStop();
        return;
    }

    /// start calibration
    startCalibration();
}

void
MainWindow::
onActionStopPressed()
{
	if (LOOP.runMode == STOP_CALIBRATION) return;
	if (isUserInputYes("Cancel Calibration.", "Do You Want To Cancel?")) onActionStop();
}


void
MainWindow::
onActionStop()
{
	updateCurrentStage(RED,STOP_CALIBRATION);
    updateLoopTabIcon(false);
	LOOP.isTempRunSkip = false;
	LOOP.isPause = false;

    ui->tabWidget->setTabEnabled(1,true);
    ui->tabWidget->setTabEnabled(2,true);
    ui->tabWidget->setTabEnabled(3,true);
    ui->tabWidget->setTabEnabled(4,true);

    ui->groupBox_13->setEnabled(true);
    ui->groupBox_11->setEnabled(true);
    ui->groupBox_113->setEnabled(true);
    ui->groupBox_114->setEnabled(true);
    ui->groupBox_5->setEnabled(true);
    ui->groupBox_9->setEnabled(true);
    ui->groupBox_10->setEnabled(true);
    ui->groupBox_6->setEnabled(true);
    ui->groupBox_65->setEnabled(true);

	ui->actionStart->setVisible(true);
	ui->actionPause->setVisible(false);

    if (LOOP.isInject) onActionStopInjection();

    stopCalibration();

	displayPipeReading(ALL,0,0,0,0,0);
}


void
MainWindow::
onActionReadMasterPipe()
{
	updateCurrentStage(BLACK,READ_MASTERPIPE);
	if (!LOOP.isCal) readMasterPipe();
	else if (LOOP.isPause) readMasterPipe();
	updateCurrentStage(RED,STOP_CALIBRATION);
}

void
MainWindow::
onActionSync()
{
	if (LOOP.isEEA)	
	{
		informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),"FEATURE NO SUPPORT FOR EEA   " ,"Temperature Synchronization Is Not Supported in EEA");
		return;
	}

	if (LOOP.isCal) 
	{
		informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),"UNABLE TO SYNCHRONIZE   " ,"Unable To Synchronize During Calibration!");
		return;
	}

    if (PIPE[0].slave->text().isEmpty() && PIPE[1].slave->text().isEmpty() && PIPE[2].slave->text().isEmpty()) 
	{
		informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("LOOP ")+QString::number(LOOP.loopNumber).append(BLANK),"No Valid Serial Number Found!");
		return;
	}

	if (ui->lineEdit_29->text().isEmpty())
	{
		informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("LOOP ")+QString::number(LOOP.loopNumber).append(BLANK),"Read Master Pipe And Get Valid Temperature First!");
		return;
	}

	for (int pipe=0; pipe<3; pipe++)
    {
        if (!PIPE[pipe].slave->text().isEmpty())
		{
			int sn;
			double temp, fctAdjTemp, masterTemp;
            uint8_t dest[1024];
            uint16_t * dest16 = (uint16_t *) dest;
            int ret = -1;
            bool is16Bit = false;
            bool writeAccess = false;
            //const QString funcType = descriptiveDataTypeName( FUNC_READ_INT );

            /// set slave
            memset( dest, 0, 1024 );
            modbus_set_slave( LOOP.serialModbus, PIPE[pipe].slave->text().toInt());

            /// read pipe serial number
   			sn = read_request(FUNC_READ_INT, RAZ_ID_SN_PIPE, BYTE_READ_INT, ret, dest, dest16, is16Bit);
			delay();

			//if (ui->lineEdit_111->text() != PIPE[pipe].slave->text())
			if (QString::number(sn) != PIPE[pipe].slave->text())
            {
        		informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("LOOP ")+QString::number(LOOP.loopNumber).append(BLANK),"Invalid Seial Number!");
				onActionStop();
                return;
            }

			updateCurrentStage(BLACK,TEMP_IN_SYNC);

			/// get master temp
			masterTemp = ui->lineEdit_29->text().toDouble();

			/// get target temp
   			temp = read_request(FUNC_READ_FLOAT, RAZ_ID_TEMPERATURE, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
			delay();
   			while (temp != temp) 
			{
				temp = read_request(FUNC_READ_FLOAT, RAZ_ID_TEMPERATURE, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
				delay();
			}

			/// get PDI_TEMP_ADJ
			fctAdjTemp = read_request(FUNC_READ_FLOAT, FCT_RAZ_TEMP_ADJ, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
			delay();
			while (fctAdjTemp != fctAdjTemp) 
			{
				fctAdjTemp = read_request(FUNC_READ_FLOAT, FCT_RAZ_TEMP_ADJ, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
				delay();
			}

			/// unlock fct
			write_request(FUNC_WRITE_COIL, 999, 0, 0, true);
			delay();

			/// update PDI_TEMP_ADJ
			while (abs(temp-masterTemp) > 0.1)
			{
				write_request(FUNC_WRITE_FLOAT, FCT_RAZ_TEMP_ADJ, fctAdjTemp + masterTemp - temp, 0, true);
				delay();

   				temp = read_request(FUNC_READ_FLOAT, RAZ_ID_TEMPERATURE, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
				delay();

			}

			/// display final temp
			PIPE[pipe].temp->setText(QString::number(temp));
		}
	}
			
	updateCurrentStage(RED,STOP_CALIBRATION);
}

void
MainWindow::
onActionSettings()
{
    static bool on = true;

    if (on)
    {
        /// calibration configuration
        readJsonConfigFile();
        ui->groupBox_18->show();
        ui->groupBox_35->show();
        ui->groupBox_34->show();
        ui->groupBox_33->show();
        ui->groupBox_32->show();
        ui->groupBox_31->show();
        ui->groupBox_30->show();
        ui->groupBox_29->show();

        /// main loop configuration
        ui->groupBox_12->hide();
        ui->groupBox_20->hide();
    }
    else
    {
        /// calibration configuration
        writeJsonConfigFile();
        ui->groupBox_18->hide();
        ui->groupBox_35->hide();
        ui->groupBox_34->hide();
        ui->groupBox_33->hide();
        ui->groupBox_32->hide();
        ui->groupBox_31->hide();
        ui->groupBox_30->hide();
        ui->groupBox_29->hide();

        /// main loop configuration
        ui->groupBox_12->show();
        ui->groupBox_20->show();
    }

    on = !on;
}

void
MainWindow::
loadFile(QString fileName)
{
	QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) return;

    QTextStream str(&file);

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    while (!str.atEnd()) {

        QString s = "";
		while (s.size() == 0) 
		{
			s = str.readLine();
			if (str.atEnd()) break;
		}

        QStringList valueList = s.split(',');

        if (!valueList[0].contains("*") && !valueList[0].contains("#"))
        {
            // insert a new row
            ui->tableWidget->insertRow( ui->tableWidget->rowCount() );

            // insert columns
            while (ui->tableWidget->columnCount() < valueList[6].toInt()+7) ui->tableWidget->insertColumn(ui->tableWidget->columnCount());

            // fill the cells with property values 
			for (int j=0; j<7; j++) ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, j, new QTableWidgetItem(valueList[j])); 

            // fill the cells withg actual value list
            for (int j = 0; j < valueList[6].toInt(); j++)
            {
                QString cellData = valueList[7+j];
                if (valueList[3].contains("int")) cellData = cellData.mid(0, cellData.indexOf("."));

                ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, j+7, new QTableWidgetItem(cellData));
            }

            // enable uploadEquationButton
            ui->startEquationBtn->setEnabled(1);
        }
    }

    // set column width
    ui->tableWidget->setColumnWidth(0,120); // Name
    ui->tableWidget->setColumnWidth(1,30);  // Slave
    ui->tableWidget->setColumnWidth(2,50);  // Address
    ui->tableWidget->setColumnWidth(3,40);  // Type
    ui->tableWidget->setColumnWidth(4,30);  // Scale
    ui->tableWidget->setColumnWidth(5,30);  // RW
    ui->tableWidget->setColumnWidth(6,30);  // Qty

    // close file
    file.close();
}


void
MainWindow::
loadCsvTemplate()
{
    QFile file;
    QString razorTemplatePath = QCoreApplication::applicationDirPath()+"/razor.csv";
    QString eeaTemplatePath = QCoreApplication::applicationDirPath()+"/eea.csv";

    if (ui->radioButton_190->isChecked()) loadFile(eeaTemplatePath);
	else loadFile(razorTemplatePath);
}

void
MainWindow::
loadCsvFile()
{
    QString fileName = QFileDialog::getOpenFileName( this, tr("Open CSV file"), QDir::currentPath(), tr("CSV files (*.csv)") );
	loadFile(fileName);
}

void
MainWindow::
onEquationButtonPressed()
{
    ui->startEquationBtn->setEnabled(false);
    ui->startEquationBtn->setText( tr("Loading") );

    if (ui->radioButton_188->isChecked())
    {
        if( m_pollTimer->isActive() )
        {
            m_pollTimer->stop();
            ui->startEquationBtn->setText( tr("Loading") );
        }
        else
        {
            // if polling requested then enable timer
            if( m_poll )
            {
                m_pollTimer->start( 1000 );
                ui->sendBtn->setText( tr("Loading") );
            }

            onUploadEquation();
        }
    }
    else
    {
        if( m_pollTimer->isActive() )
        {
            m_pollTimer->stop();
            ui->startEquationBtn->setText( tr("Loading") );
        }
        else
        {
            // if polling requested then enable timer
            if( m_poll )
            {
                m_pollTimer->start( 1000 );
                ui->sendBtn->setText( tr("Loading") );
            }

            ui->tableWidget->clearContents();
            ui->tableWidget->setRowCount(0);

            onDownloadEquation();
        }
    }

    ui->startEquationBtn->setText(tr("Start"));
    ui->startEquationBtn->setEnabled(true);
}


void
MainWindow::
onDownloadButtonChecked(bool isChecked)
{
    if (isChecked)
    {
        ui->startEquationBtn->setEnabled(true);
    }
    else {
        ui->startEquationBtn->setEnabled(true);
    }
}


void
MainWindow::
onUploadEquation()
{
	if (ui->lineEdit_32->text().isEmpty()) 
	{
       	informUser("SLAVE ID MISSING","No Slave ID                ","No Valid Slave ID Exists!");
		return;
	}

   	uint8_t dest[1024];
   	uint16_t * dest16 = (uint16_t *) dest;
   	int ret = -1;
   	bool is16Bit = false;
   	bool writeAccess = false;

	int value = 0;
    int rangeMax = 0;
    QMessageBox msgBox;
    isModbusTransmissionFailed = false;

   	/// set slave
   	memset( dest, 0, 1024 );
   	modbus_set_slave( LOOP.serialModbus, ui->lineEdit_32->text().toInt());

	/// read pipe serial number
   	int sn = read_request(FUNC_READ_INT, RAZ_ID_SN_PIPE, BYTE_READ_INT, ret, dest, dest16, is16Bit);
	delay();

	if (QString::number(sn) != ui->lineEdit_32->text())
    {
   		informUser("Invalid Serial Number","Invalid Serial Number",ui->lineEdit_32->text());
        return;
    }

    /// get rangeMax of progressDialog
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) rangeMax+=ui->tableWidget->item(i,6)->text().toInt();

    QProgressDialog progress("Uploading...", "Abort", 0, rangeMax, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(true);
    progress.setAutoReset(true);

	/// unlcok FCT
	write_request(FUNC_WRITE_COIL, 999, 0, 0, true);
    delay();

    if (isModbusTransmissionFailed)
    {
        isModbusTransmissionFailed = false;
        msgBox.setText("Modbus Transmission Failed.");
        msgBox.setInformativeText("Do you want to continue with next item?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::Yes:
                break;
            case QMessageBox::No:
            default: return;
        }
    }

   for (int i = 0; i < ui->tableWidget->rowCount(); i++)
   {
        int regAddr = ui->tableWidget->item(i,2)->text().toInt();
        if (ui->tableWidget->item(i,3)->text().contains("float"))
        {
            for (int x=0; x < ui->tableWidget->item(i,6)->text().toInt(); x++)
            {
                QString val = ui->tableWidget->item(i,7+x)->text(); // read value
                if (progress.wasCanceled()) return;
                if (ui->tableWidget->item(i,6)->text().toInt() > 1) progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"["+QString::number(x+1)+"]"+"\""+","+" \""+val+"\"");
                else progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"\""+","+" \""+val+"\"");
                progress.setValue(value++);

				/// send modbus request
				write_request(FUNC_WRITE_FLOAT, regAddr, val.toInt(), 0, true);
                delay();
                regAddr += 2; // update reg address

                if (isModbusTransmissionFailed)
                {
                    isModbusTransmissionFailed = false;
                    if (ui->tableWidget->item(i,6)->text().toInt() > 1) msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text()+"["+QString::number(x+1)+"]");
                    else msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text());
                    msgBox.setInformativeText("Do you want to continue with next item?");
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    msgBox.setDefaultButton(QMessageBox::No);
                    int ret = msgBox.exec();
                    switch (ret) {
                        case QMessageBox::Yes: break;
                        case QMessageBox::No:
                        default: return;
                    }
                }
            }
        }
        else if (ui->tableWidget->item(i,3)->text().contains("int"))
        {
            QString val = ui->tableWidget->item(i,7)->text(); // read value

            if (progress.wasCanceled()) return;
            progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"\""+","+" \""+val+"\"");
            progress.setValue(value++);

			/// send modbus request
			write_request(FUNC_WRITE_INT, regAddr, 0, val.toInt(), true);
            delay();

            if (isModbusTransmissionFailed)
            {
                isModbusTransmissionFailed = false;
                msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text());
                msgBox.setInformativeText("Do you want to continue with next item?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                int ret = msgBox.exec();
                switch (ret) {
                    case QMessageBox::Yes: break;
                    case QMessageBox::No:
                    default: return;
                }
            }
        }
        else
        {
			bool val;
			(ui->tableWidget->item(i,7)->text().toInt()) ? val = true : val = false; // read value

            progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"\""+","+"\""+val+"\"");
            //if (val) progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"\""+","+"\""+val+"\"");
            //else progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"\""+","+" \"0\"");

            if (progress.wasCanceled()) return;
            progress.setValue(value++);

			/// send modbus request
			write_request(FUNC_WRITE_COIL, regAddr, 0, 0, val);
            delay();

            if (isModbusTransmissionFailed)
            {
                isModbusTransmissionFailed = false;
                msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text());
                msgBox.setInformativeText("Do you want to continue with next item?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                int ret = msgBox.exec();
                switch (ret) {
                    case QMessageBox::Yes: break;
                    case QMessageBox::No:
                    default: return;
                }
            }
        }
    }

	write_request(FUNC_WRITE_COIL, 999, 0, 0, true); // COIL_UNLOCKED_FACTORY_DEFAULT 
	delay();
	write_request(FUNC_WRITE_COIL, 9999, 0, 0, true); // COIL_UPDATE_FACTORY_DEFAULT 
	delay();
}


void
MainWindow::
onDownloadEquation()
{
	if (ui->lineEdit_32->text().isEmpty()) 
	{
       	informUser("SLAVE ID MISSING","No Slave ID                ","No Valid Slave ID Exists!");
		return;
	}

   	uint8_t dest[1024];
   	uint16_t * dest16 = (uint16_t *) dest;
   	int ret = -1;
   	bool is16Bit = false;
   	bool writeAccess = false;
    int value = 0;
    int rangeMax = 0;
	char lcdModelCode[] = "INCDYNAMICSPHASE";

    QMessageBox msgBox;
    isModbusTransmissionFailed = false;

   	/// set slave
   	memset( dest, 0, 1024 );
   	modbus_set_slave( LOOP.serialModbus, ui->lineEdit_32->text().toInt());

	/// read pipe serial number
   	int sn = read_request(FUNC_READ_INT, RAZ_ID_SN_PIPE, BYTE_READ_INT, ret, dest, dest16, is16Bit);
	delay();

	if (QString::number(sn) != ui->lineEdit_32->text())
    {
   		informUser("Invalid Serial Number","Invalid Serial Number",ui->lineEdit_32->text());
        return;
    }

    // load empty equation file
    loadCsvTemplate();

    /// get rangeMax of progressDialog
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) rangeMax+=ui->tableWidget->item(i,6)->text().toInt();

    QProgressDialog progress("Downloading...", "Abort", 0, rangeMax, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(true);
    progress.setAutoReset(true);

    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
		int regAddr = ui->tableWidget->item(i,2)->text().toInt();
		is16Bit = false;

        if (ui->tableWidget->item(i,3)->text().contains("float"))
        {
            for (int x=0; x < ui->tableWidget->item(i,6)->text().toInt(); x++)
            {
				is16Bit = true;
                if (progress.wasCanceled()) return;
                if (ui->tableWidget->item(i,6)->text().toInt() > 1) progress.setLabelText("Downloading \""+ui->tableWidget->item(i,0)->text()+"\" "+QString::number(x+1));
                else progress.setLabelText("Downloading \""+ui->tableWidget->item(i,0)->text()+"\"");
                progress.setValue(value++);
  				double val = read_request(FUNC_READ_FLOAT, regAddr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
                delay();
				if (isModbusTransmissionFailed)
                {
                    isModbusTransmissionFailed = false;
                    if (ui->tableWidget->item(i,6)->text().toInt() > 1) msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text()+"["+QString::number(x+1)+"]");
                    else msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text());
                    msgBox.setInformativeText("Do you want to read again?");
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    msgBox.setDefaultButton(QMessageBox::No);
                    int ret = msgBox.exec();
                    switch (ret) {
                        case QMessageBox::Yes: 
							x--;
							break;
                        case QMessageBox::No:
                        default: return;
                    }
                }
				else
				{
					ui->tableWidget->item(i, x+7)->setText(QString("%1").arg(val,10,'f',4,' '));
            		regAddr += 2; // update reg address
				}
            }
        }
        else if (ui->tableWidget->item(i,3)->text().contains("int") || ui->tableWidget->item(i,3)->text().contains("long") )
        {
            if (progress.wasCanceled()) return;
            progress.setLabelText("Downloading \""+ui->tableWidget->item(i,0)->text()+"\"");
            progress.setValue(value++);

			quint32 val; 

			if (regAddr == 219)
			{
    			for (int k=0;k<4;k++)
    			{
					val = read_request(FUNC_READ_INT, regAddr+k, BYTE_READ_INT, ret, dest, dest16, is16Bit);
				    for (int j=0; j<4; j++) lcdModelCode[k*4+j] = (val >> j*8) & 0xFF;
    			}
			}		
			else
			{
				if (ui->tableWidget->item(i,3)->text().contains("int")) val = read_request(FUNC_READ_INT, regAddr, BYTE_READ_INT, ret, dest, dest16, is16Bit);
				else val = read_request(FUNC_READ_INT, regAddr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
			}

            delay();

			while (isModbusTransmissionFailed)
            {
            	isModbusTransmissionFailed = false;
                msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text());
                msgBox.setInformativeText("Do you want to read again?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                int ret = msgBox.exec();
                switch (ret) {
                    case QMessageBox::Yes: 
						if (ui->tableWidget->item(i,3)->text().contains("int")) val = read_request(FUNC_READ_INT, regAddr, BYTE_READ_INT, ret, dest, dest16, is16Bit);
						else val = read_request(FUNC_READ_INT, regAddr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
            			delay();
						break;
                    case QMessageBox::No:
                    default: return;
                }
            }

			if (regAddr == 219) ui->tableWidget->item(i, 7)->setText(QString::fromUtf8(lcdModelCode, 16));
			else ui->tableWidget->item(i, 7)->setText(QString::number(val));
        }
        else
        {
            if (progress.wasCanceled()) return;
            progress.setLabelText("Downloading \""+ui->tableWidget->item(i,0)->text()+"\"");
            progress.setValue(value++);

			bool val = read_request(FUNC_READ_COIL, regAddr, BYTE_READ_COIL, ret, dest, dest16, is16Bit);
            delay();
			while (isModbusTransmissionFailed)
            {
            	isModbusTransmissionFailed = false;
                msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text());
                msgBox.setInformativeText("Do you want to read again?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                int ret = msgBox.exec();
                switch (ret) {
                    case QMessageBox::Yes: 
						val = read_request(FUNC_READ_COIL, regAddr, BYTE_READ_COIL, ret, dest, dest16, is16Bit);
            			delay();
						break;
                    case QMessageBox::No:
                    default: return;
                }
            }

			ui->tableWidget->item(i, 7)->setText(QString::number(val));
        }
	}
}


bool
MainWindow::
informUser(const QString t1, const QString t2, const QString t3 = "")
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(t1);
    msgBox.setText(t2);
    msgBox.setInformativeText(t3);
    msgBox.setStandardButtons(QMessageBox::Ok);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Ok: return true;
        default: return true;
    }
}

bool
MainWindow::
isUserInputYes(const QString t1, const QString t2)
{
    QMessageBox msgBox;
    msgBox.setText(t1);
    msgBox.setInformativeText(t2);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes: return true;
        case QMessageBox::Cancel: return false;
        default: return true;
    }
}


void
MainWindow::
onUnlockFactoryDefault()
{
    ui->numCoils->setValue(1);                      // 1 byte
    ui->radioButton_183->setChecked(true);          // coil type
    ui->radioButton_186->setChecked(true);          // write 
    ui->functionCode->setCurrentIndex(4);           // function code
    ui->radioButton_184->setChecked(true);          // set value
    ui->startAddr->setValue(999);                   // address 999
    onSendButtonPress();
    delay();
}


void
MainWindow::
onLockFactoryDefault()
{
    ui->numCoils->setValue(1);                      // 1 byte
    ui->radioButton_183->setChecked(true);          // coil type
    ui->radioButton_186->setChecked(true);          // write 
    ui->functionCode->setCurrentIndex(4);           // function code
    ui->radioButton_185->setChecked(true);          // set value
    ui->startAddr->setValue(999);                   // address 999
    onSendButtonPress();
    delay();
}


void
MainWindow::
onUpdateFactoryDefaultPressed()
{
    if ( ui->radioButton_192->isChecked()) return;

    QMessageBox msgBox;

    msgBox.setText("Factory default values will be permanently changed.");
    msgBox.setInformativeText("Are you sure you want to do this?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes: break;
        case QMessageBox::No:
        default: return;
    }

    ui->numCoils->setValue(1);                      // 1 byte
    ui->radioButton_183->setChecked(TRUE);          // coil type
    ui->functionCode->setCurrentIndex(4);           // function code
    ui->radioButton_184->setChecked(true);          // set value

    /// update factory default registers
    ui->startAddr->setValue(9999);                  // address 99999
    onSendButtonPress();
    delay();
}


void
MainWindow::
connectProfiler()
{
    connect(ui->radioButton_193, SIGNAL(pressed()), this, SLOT(onUnlockFactoryDefault()));
    connect(ui->radioButton_192, SIGNAL(pressed()), this, SLOT(onLockFactoryDefault()));
    connect(ui->pushButton_2, SIGNAL(pressed()), this, SLOT(onUpdateFactoryDefaultPressed()));
    connect(ui->startEquationBtn, SIGNAL(pressed()), this, SLOT(onEquationButtonPressed()));
    connect(ui->radioButton_189, SIGNAL(toggled(bool)), this, SLOT(onDownloadButtonChecked(bool)));
}


void
MainWindow::
setupModbusPorts()
{
    setupModbusPort();
}


int
MainWindow::
setupModbusPort()
{
    QSettings s;

    int portIndex = LOOP.portIndex;
    int i = 0;
    ui->comboBox->disconnect();
    ui->comboBox->clear();
    foreach( QextPortInfo port, QextSerialEnumerator::getPorts() )
    {
        ui->comboBox->addItem( port.friendName );

        if( port.friendName == s.value( "serialinterface" ) )
        {
            portIndex = i;
        }
        ++i;
    }
    ui->comboBox->setCurrentIndex( portIndex );
    ui->comboBox_2->setCurrentIndex(0);
    ui->comboBox_3->setCurrentIndex(0);
    ui->comboBox_4->setCurrentIndex(0);
    ui->comboBox_5->setCurrentIndex(0);

    connect( ui->comboBox, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort( int ) ) );
    connect( ui->comboBox_2, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort( int ) ) );
    connect( ui->comboBox_3, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort( int ) ) );
    connect( ui->comboBox_4, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort( int ) ) );
    connect( ui->comboBox_5, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort( int ) ) );

    changeSerialPort( portIndex );
    return portIndex;
}


void
MainWindow::
releaseSerialModbus()
{
    modbus_close( LOOP.serialModbus );
    modbus_free( LOOP.serialModbus );
    LOOP.serialModbus = NULL;
    updateLoopTabIcon(false);
}


void
MainWindow::
changeSerialPort( int )
{
    const int iface = ui->comboBox->currentIndex();
    LOOP.portIndex = iface;
    writeJsonConfigFile();

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    if( !ports.isEmpty() )
    {
        QSettings settings;
        settings.setValue( "serialinterface", ports[iface].friendName );
        settings.setValue( "serialbaudrate", ui->comboBox_2->currentText() );
        settings.setValue( "serialparity", ui->comboBox_3->currentText() );
        settings.setValue( "serialdatabits", ui->comboBox_4->currentText() );
        settings.setValue( "serialstopbits", ui->comboBox_5->currentText() );
        QString port = ports[iface].portName;

        // is it a serial port in the range COM1 .. COM9?
        if ( port.startsWith( "COM" ) )
        {
            // use windows communication device name "\\.\COMn"
            port = "\\\\.\\" + port;
        }

        char parity;
        switch( ui->comboBox_3->currentIndex() )
        {
            case 1: parity = 'O'; break;
            case 2: parity = 'E'; break;
            default:
            case 0: parity = 'N'; break;
        }

        changeModbusInterface(port, parity);
        onRtuPortActive(true);
    }
    else emit connectionError( tr( "No serial port found at Loop_1" ) );
}


void
MainWindow::
changeModbusInterface(const QString& port, char parity)
{
    releaseSerialModbus();
    LOOP.serialModbus = modbus_new_rtu( port.toLatin1().constData(),ui->comboBox_2->currentText().toInt(),parity,ui->comboBox_3->currentText().toInt(),ui->comboBox_4->currentText().toInt() );

    if( modbus_connect( LOOP.serialModbus ) == -1 )
    {
        emit connectionError( tr( "Could not connect serial port at LOOP " )+QString::number(0) );
        releaseSerialModbus();
    }
    else
        updateLoopTabIcon(true);
}


void
MainWindow::
onCheckBoxChecked(bool checked)
{
    clearMonitors();

    if (checked)
    {
        setupModbusPort();
        updateLoopTabIcon(true);
    }
    else updateLoopTabIcon(false);

    onRtuPortActive(checked);
}


void
MainWindow::
clearMonitors()
{
    ui->rawData->clear();
    ui->regTable->setRowCount(0);
    ui->busMonTable->setRowCount(0);
}


void
MainWindow::
updatePipeStability(const int pipe, const bool checkStability)
{
    if (checkStability)
    {
        /// check temp stability
        if (PIPE[pipe].tempStability < 5)
        {
            if (abs(PIPE[pipe].temperature - PIPE[pipe].temperature_prev) <= LOOP.zTemp) PIPE[pipe].tempStability++;
            else PIPE[pipe].tempStability = 0;

            PIPE[pipe].temperature_prev = PIPE[pipe].temperature;
            PIPE[pipe].tempProgress->setValue(PIPE[pipe].tempStability*20);
        }
        else PIPE[pipe].tempStability = 5;

        /// check freq stability
        if (PIPE[pipe].freqStability < 5)
        {
            if (abs(PIPE[pipe].frequency - PIPE[pipe].frequency_prev) <= LOOP.yFreq) PIPE[pipe].freqStability++;
            else PIPE[pipe].freqStability = 0;

            PIPE[pipe].frequency_prev = PIPE[pipe].frequency;
            PIPE[pipe].freqProgress->setValue(PIPE[pipe].freqStability*20);
        }
        else PIPE[pipe].freqStability = 5;
    }
    else
    {
        if (pipe == ALL)
        {
            for (int i=0; i<3; i++)
            {
                PIPE[i].freqStability = 0;
                PIPE[i].tempStability = 0;
                PIPE[i].freqProgress->setValue(PIPE[i].freqStability);
                PIPE[i].tempProgress->setValue(PIPE[i].tempStability);
            }
        }
        else
        {
            PIPE[pipe].freqStability = 0;
            PIPE[pipe].tempStability = 0;
            PIPE[pipe].freqProgress->setValue(PIPE[pipe].freqStability);
            PIPE[pipe].tempProgress->setValue(PIPE[pipe].tempStability);
        }
    }
}

void
MainWindow::
onHighSelected()
{
   	LOOP.waterRunStart->setText("99");
   	LOOP.waterRunStop->setText("60");

   	LOOP.oilRunStart->setText("0.0");
   	LOOP.oilRunStop->setText("0.0");
}


void
MainWindow::
onFullSelected()
{
   	LOOP.waterRunStart->setText("99");
   	LOOP.waterRunStop->setText("60");

   	LOOP.oilRunStart->setText("0.0");
   	LOOP.oilRunStop->setText("0.0");
}


void
MainWindow::
onMidSelected()
{
   	LOOP.waterRunStart->setText("0.0");
   	LOOP.waterRunStop->setText("0.0");

   	LOOP.oilRunStart->setText("0.0");
   	LOOP.oilRunStop->setText("78");
}

void
MainWindow::
onLowSelected()
{
   	LOOP.waterRunStart->setText("0.0");
   	LOOP.waterRunStop->setText("0.0");

   	LOOP.oilRunStart->setText("0.0");
   	LOOP.oilRunStop->setText("78");
}

void
MainWindow::
connectProductBtnPressed()
{
    connect(ui->radioButton, SIGNAL(toggled(bool)), this, SLOT(onUpdateRegisters(bool)));
}


void
MainWindow::
updateLoopTabIcon(const bool connected)
{
    QIcon icon(QLatin1String(":/green.ico"));
    QIcon icoff(QLatin1String(":/red.ico"));
    (connected) ? ui->tabWidget_2->setTabIcon(0,icon) : ui->tabWidget_2->setTabIcon(0,icoff);
}


void
MainWindow::
initializeTabIcons()
{
    QIcon icon(QLatin1String(":/red.ico"));
    ui->tabWidget_2->setTabIcon(0,icon);
}


void
MainWindow::
createTempRunFile(const int sn, const QString startValue, const QString stopValue, const QString saltValue, const int pipe)
{
    /// set file names
    QDateTime currentDataTime = QDateTime::currentDateTime();

    /// headers
    QString header0;
    LOOP.isEEA ? header0 = EEA_INJECTION_FILE : header0 = RAZ_INJECTION_FILE;
    QString header1("SN"+QString::number(sn)+" | "+LOOP.cut.split("\\").at(1) +" | "+currentDataTime.toString()+" | L"+QString::number(LOOP.loopNumber)+PIPE[pipe].pipeId+" | "+PROJECT+RELEASE_VERSION);
    QString header2("INJECTION:  "+startValue+" % "+"to "+stopValue+" % "+"Watercut at "+saltValue+" % "+"Salinity\n");
    if ((LOOP.cut == LOW) || (!LOOP.isEEA)) header2 = "TEMPERATURE:  "+startValue+" °C "+"to "+stopValue+" °C\n";
    QString header3 = HEADER3;
    QString header4 = HEADER4;
    QString header5 = HEADER5;

    /// stream
    QTextStream stream(&PIPE[pipe].file);

    /// open file
    PIPE[pipe].file.open(QIODevice::WriteOnly | QIODevice::Text);

    /// write headers to stream
    stream << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';

    /// close file
    PIPE[pipe].file.close();

    /// update file list
    updateFileList(QFileInfo(PIPE[pipe].file).fileName(), sn, pipe);
}


void
MainWindow::
createInjectFile(const int pipe, const QString startValue, const QString stopValue, const QString saltValue, const QString filename)
{
    /// headers
	int sn = PIPE[pipe].slave->text().toInt();
    QDateTime currentDataTime = QDateTime::currentDateTime();
    QString header0;
    LOOP.isEEA ? header0 = EEA_INJECTION_FILE : header0 = RAZ_INJECTION_FILE;
    QString header1("SN"+QString::number(sn)+" | "+LOOP.cut.split("\\").at(1) +" | "+currentDataTime.toString()+" | L"+QString::number(LOOP.loopNumber)+PIPE[pipe].pipeId+" | "+PROJECT+RELEASE_VERSION);
    QString header2("INJECTION:  "+startValue+" % "+"to "+stopValue+" % "+"Watercut at "+saltValue+" % "+"Salinity\n");
    QString header3 = HEADER3;
    QString header4 = HEADER4;
    QString header5 = HEADER5;
    QString header21; // LOWCUT ONLY
    QString header22; // LOWCUT ONLY

    /// CALIBRAT, ADJUSTED, ROLLOVER (LOWCUT ONLY)
    if (LOOP.cut == LOW)
    {
        /// create headers
        header2 = "TEMPERATURE:  "+startValue+" °C "+"to "+stopValue+" °C\n";
        header21 = "INJECTION:  "+LOOP.oilRunStart->text()+" % "+"to "+LOOP.oilRunStop->text()+" % Watercut\n";
        header22 = "ROLLOVER:  "+QString::number(LOOP.watercut)+" % "+"to "+"rollover\n";

        /// set filenames
        PIPE[pipe].fileCalibrate.setFileName(PIPE[pipe].mainDirPath+"\\"+QString("CALIBRAT").append(LOOP.calExt));
        PIPE[pipe].fileAdjusted.setFileName(PIPE[pipe].mainDirPath+"\\"+QString("ADJUSTED").append(LOOP.adjExt));
        PIPE[pipe].fileRollover.setFileName(PIPE[pipe].mainDirPath+"\\"+QString("ROLLOVER").append(LOOP.rolExt));

        /// update PIPE object
        if (filename == "CALIBRAT")
        {
            /// CALIBRAT
            if (!QFileInfo(PIPE[pipe].fileCalibrate).exists())
            {
                QTextStream streamCalibrate(&PIPE[pipe].fileCalibrate);
                PIPE[pipe].fileCalibrate.open(QIODevice::WriteOnly | QIODevice::Text);
                streamCalibrate << header0 << '\n' << header1 << '\n' << header21 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
                PIPE[pipe].fileCalibrate.close();

                /// update file list
                updateFileList(QFileInfo(PIPE[pipe].fileCalibrate).fileName(), sn, pipe);
            }
        }
        else if (filename == "ADJUSTED")
        {
            if (!QFileInfo(PIPE[pipe].fileAdjusted).exists())
            {
                QTextStream streamAdjusted(&PIPE[pipe].fileAdjusted);
                PIPE[pipe].fileAdjusted.open(QIODevice::WriteOnly | QIODevice::Text);
                streamAdjusted << header0 << '\n' << header1 << '\n' << header21 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
                PIPE[pipe].fileAdjusted.close();

                /// update file list
                updateFileList(QFileInfo(PIPE[pipe].fileAdjusted).fileName(), sn, pipe);
            }
        }
        else if (filename == "ROLLOVER")
        {
            /// ROLLOVER
            if (!QFileInfo(PIPE[pipe].fileRollover).exists())
            {
                QTextStream streamRollover(&PIPE[pipe].fileRollover);
                PIPE[pipe].fileRollover.open(QIODevice::WriteOnly | QIODevice::Text);
                streamRollover << header0 << '\n' << header1 << '\n' << header22 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
                PIPE[pipe].fileRollover.close();

                /// update file list
                updateFileList(QFileInfo(PIPE[pipe].fileRollover).fileName(), sn, pipe);
            }
        }
    }
    else if (LOOP.cut == MID)
    {
        /// OIL_INJECTION TEMP
        if (!QFileInfo(PIPE[pipe].file).exists())
        {
            QTextStream stream(&PIPE[pipe].file);
            PIPE[pipe].file.open(QIODevice::WriteOnly | QIODevice::Text);
            stream << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
            PIPE[pipe].file.close();

            /// update file list
            updateFileList(QFileInfo(PIPE[pipe].file).fileName(), sn, pipe);
        }
    }
	else if (LOOP.cut == FULL)
    {
        /// OIL_INJECTION TEMP
        if (!QFileInfo(PIPE[pipe].file).exists())
        {
            QTextStream stream(&PIPE[pipe].file);
            PIPE[pipe].file.open(QIODevice::WriteOnly | QIODevice::Text);
            stream << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
            PIPE[pipe].file.close();

            /// update file list
            updateFileList(QFileInfo(PIPE[pipe].file).fileName(), sn, pipe);
        }
    }
	else if (LOOP.cut == HIGH)
    {
        /// OIL_INJECTION TEMP
        if (!QFileInfo(PIPE[pipe].file).exists())
        {
            QTextStream stream(&PIPE[pipe].file);
            PIPE[pipe].file.open(QIODevice::WriteOnly | QIODevice::Text);
            stream << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
            PIPE[pipe].file.close();

            /// update file list
            updateFileList(QFileInfo(PIPE[pipe].file).fileName(), sn, pipe);
        }
    }
}


void
MainWindow::
updateFileList(const QString fileName, const int sn, const int pipe)
{
    QString header0;
    QFile file;
    QTextStream streamList(&file);
    QDateTime currentDataTime = QDateTime::currentDateTime();

    LOOP.isEEA ? header0 = EEA_INJECTION_FILE : header0 = RAZ_INJECTION_FILE;
    QString header1("SN"+QString::number(sn)+" | "+LOOP.cut.split("\\").at(1) +" | "+currentDataTime.toString()+" | L"+QString::number(LOOP.loopNumber)+PIPE[pipe].pipeId+" | "+PROJECT+RELEASE_VERSION);

    if (!QFileInfo(file).exists()) file.setFileName(PIPE[pipe].mainDirPath+"\\"+FILE_LIST);
    file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    /// write header to streamList
    if (QFileInfo(file).exists())
    {
        if (QFileInfo(file).size() == 0) streamList << header0 << '\n' << header1 << '\n' << '\n' << fileName << '\n';
        else streamList << fileName << '\n';
    }

    file.close();
}

bool
MainWindow::
validateSerialNumber(modbus_t * serialModbus)
{
    for (int pipe=0; pipe<3; pipe++)
    {
        if (PIPE[pipe].status == ENABLED)
        {
            uint8_t dest[1024];
            uint16_t * dest16 = (uint16_t *) dest;
            int ret = -1;
    		int num = ui->numCoils->value();
            bool is16Bit = false;
            bool writeAccess = false;
            const QString funcType = descriptiveDataTypeName( FUNC_READ_INT );

            /// set slave
            memset( dest, 0, 1024 );
            modbus_set_slave( serialModbus, PIPE[pipe].slave->text().toInt());

            /// read pipe serial number
            read_request(FUNC_READ_INT, LOOP.ID_SN_PIPE, BYTE_READ_INT, ret, dest, dest16, is16Bit);

            /// verify if serial number matches with pipe
            if (*dest16 != PIPE[pipe].slave->text().toInt())
            {
        		informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("LOOP ")+QString::number(LOOP.loopNumber).append(BLANK),"Invalid Seial Number!");

                /// cancel calibration
                LOOP.isCal = false;
                PIPE[pipe].status = DISABLED;

                /// sn is valid but serial port invalid then it's an error
                return false;
            }
            else
            {
                PIPE[pipe].status = ENABLED;
                PIPE[pipe].checkBox->setChecked(true);
            }
        }
        else
        {
            PIPE[pipe].status = DISABLED;
            PIPE[pipe].checkBox->setChecked(false);
        }
    }

    return true;
}


void
MainWindow::
readLoopConfiguration()
{
    /// reset triggers
    LOOP.isInitTempRun = false;
    LOOP.isInitInject = false;
    LOOP.isInject = false;
	LOOP.isPause = false;
    LOOP.isTempRunSkip = false;
	LOOP.isTempRunOnly = false;
	LOOP.isWaterRun = false;
	LOOP.isOilRun = false;
	LOOP.salinityIndex = 0;

    /// set product
    LOOP.isEEA = ui->radioButton->isChecked();

    /// set register ids
    onUpdateRegisters(LOOP.isEEA);

    /// set master pipe
    LOOP.isMaster = ui->radioButton_11->isChecked();

	/// set file path
	if (LOOP.isEEA)
	{
		HIGH = HIGH_EEA;
		FULL = FULL_EEA;
		MID = MID_EEA;
		LOW = LOW_EEA;
	}
	else
	{
		HIGH = HIGH_RAZ;
		FULL = FULL_RAZ;
		MID = MID_RAZ;
		LOW = LOW_RAZ;
	}

    /// set cut
    if (ui->radioButton_3->isChecked()) 
	{
		LOOP.cut = HIGH;
		LOOP.isInitInject = true;
		LOOP.isWaterRun = true;
		LOOP.isOilRun = false;
	}
    else if (ui->radioButton_4->isChecked()) 
	{
		LOOP.cut = FULL;
		LOOP.isInitInject = true;
		LOOP.isWaterRun = true;
		LOOP.isOilRun = false;
	}
    else if (ui->radioButton_5->isChecked()) 
	{
		LOOP.cut = MID;	
		LOOP.isWaterRun = false;
		LOOP.isOilRun = true;

		if (LOOP.isEEA) 
		{
			LOOP.isInitTempRun = false;
			LOOP.isInitInject = true;
		}
		else 
		{
			LOOP.isInitTempRun = true;
			LOOP.isInitInject = false;
		}

	}
    else if (ui->radioButton_6->isChecked()) 
	{
		LOOP.cut = LOW;
		LOOP.isInitTempRun = true;
		LOOP.isInitInject = false;
		LOOP.isWaterRun = false;
		LOOP.isOilRun = true;
	}

    /// set run mode
	if (LOOP.isEEA)
	{   /// eea
    	if (ui->radioButton_14->isChecked()) LOOP.runMode = SIMULATION_RUN;
    	else
		{
			if (LOOP.cut == LOW) 
			{
				LOOP.isOilRun = true;
				LOOP.isWaterRun = false;
				LOOP.runMode = TEMPRUN_MIN;
			}
			else if (LOOP.cut == MID) 
			{
				LOOP.isOilRun = true;
				LOOP.isWaterRun = false;
				LOOP.runMode = INJECT_WATER;
			}
			else 
			{
				LOOP.isOilRun = false;
				LOOP.isWaterRun = true;
				LOOP.runMode = INJECT_OIL;
			}
		}
	}
	else
	{   /// razor
		LOOP.isOilRun = true;
		LOOP.isWaterRun = false;

    	if (ui->radioButton_14->isChecked()) 
		{
			LOOP.runMode = SIMULATION_RUN;
			LOOP.isInitInject = true;
		}
		else 
		{
			LOOP.runMode = TEMPRUN_MIN;
			if (ui->radioButton_15->isChecked()) LOOP.isTempRunOnly = true;
		}
	}

    /// set file extensions
    if (LOOP.cut == HIGH)
    {
        LOOP.filExt = ".HCI";
        LOOP.calExt = ".HCI";
        LOOP.adjExt = ".HCI";
        LOOP.rolExt = ".HCR";
    }
    else if (LOOP.cut == FULL)
    {
        LOOP.filExt = ".FCI";
        LOOP.calExt = ".FCI";
        LOOP.adjExt = ".FCI";
        LOOP.rolExt = ".FCR";
    }
    else if (LOOP.cut == MID)
    {
        LOOP.filExt = ".MCI";
        LOOP.calExt = ".MCI";
        LOOP.adjExt = ".MCI";
        LOOP.rolExt = ".MCR";
    }
    else if (LOOP.cut == LOW)
    {
        LOOP.filExt = ".LCT";
        LOOP.calExt = ".LCI";
        LOOP.adjExt = ".LCI";
        LOOP.rolExt = ".LCR";
    }
}


bool 
MainWindow::
enablePipes()
{
	/// disable empty pipes
    for (int pipe=0; pipe<3; pipe++)
    {
        if (PIPE[pipe].slave->text().isEmpty()) PIPE[pipe].status = DISABLED;
        else PIPE[pipe].status = ENABLED;
    }

	/// cancle calibration if pipes are all empty
    if (PIPE[0].slave->text().isEmpty() && PIPE[1].slave->text().isEmpty() && PIPE[2].slave->text().isEmpty())
    {
        onActionStop();
        informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("LOOP ")+QString::number(LOOP.loopNumber).append(BLANK),"No Valid Serial Number Exists!");
        return false;
    }

	return true;
}


bool
MainWindow::
prepareCalibration()
{
	if (!enablePipes()) return false;

    /// validatae loop volume
    if (LOOP.loopVolume->text().toDouble() < 1)
    {
        onActionStop();
        informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("LOOP ")+QString::number(LOOP.loopNumber).append(BLANK),"No Valid Loop Volume Exists!");
        return false;
    }

    /// validate serial port
    if (LOOP.modbus == NULL)
    {
        /// update tab icon
        onActionStop();
        informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("LOOP ")+QString::number(LOOP.loopNumber).append(BLANK),"Bad Serial Connection");
        return false;
    }

    /// LOOP configuration
    readLoopConfiguration();

    /// reset stability progressbar
    updatePipeStability(ALL,NO_STABILITY_CHECK);

    /// validate serial numbers
    if (!validateSerialNumber(LOOP.serialModbus))
    {
        /// stop calibration
        onActionStop();
        return false;
    }

    /// set file path
    for (int pipe = 0; pipe < 3; pipe++)
    {
		if (PIPE[pipe].status == ENABLED)
		{
        	PIPE[pipe].etimer->restart();
        	PIPE[pipe].mainDirPath = m_mainServer+LOOP.cut+QString::number(((int)(PIPE[pipe].slave->text().toInt()/100))*100).append("'s").append("\\")+LOOP.cut.split("\\").at(2)+PIPE[pipe].slave->text();

        	if (ui->radioButton_7->isChecked()) PIPE[pipe].osc = 1;
        	else if (ui->radioButton_8->isChecked()) PIPE[pipe].osc = 2;
        	else if (ui->radioButton_9->isChecked()) PIPE[pipe].osc = 3;
        	else PIPE[pipe].osc = 4;
		}
    }

    return true;
}


void
MainWindow::
startCalibration()
{
    /// scan calibration variables
    if (!prepareCalibration()) return;

    /// LOOP calibration on
    if (LOOP.isCal)
    {
        for (int pipe = 0; pipe < 3; pipe++)
        {
            if (PIPE[pipe].status == ENABLED)
            {
                PIPE[pipe].isStartFreq = true;
                QDir dir;
                int fileCounter = 2;

                /// corresponding folder should exist before starting SIMULATION
                if (LOOP.runMode == SIMULATION_RUN) // simulation
                {
                    if (!dir.exists(PIPE[pipe].mainDirPath))
                    {
                        informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("No Simulation Directory Exists!!                     "),PIPE[pipe].mainDirPath);
                        onActionStop();
                        return;
                    }
                }
                else // calibration
                {
					/// set folder
                    if (!dir.exists(PIPE[pipe].mainDirPath)) dir.mkpath(PIPE[pipe].mainDirPath);
                    else
                    {
                        while (1)
                        {
                            if (!dir.exists(PIPE[pipe].mainDirPath+"_"+QString::number(fileCounter)))
                            {
                                dir.rename(PIPE[pipe].mainDirPath,PIPE[pipe].mainDirPath + "_"+QString::number(fileCounter) ); // rename existing folder"
                                dir.mkpath(PIPE[pipe].mainDirPath); // create new folder

                                break;
                            }
                            else fileCounter++;
                        }
                    }
                }

                /// set file name
				if (LOOP.isEEA)
				{
               		if ((LOOP.cut == HIGH) || (LOOP.cut == FULL))
					{
						/// get salinity index
						LOOP.salinityIndex = 0;
						while (SALINITY[LOOP.salinityIndex] != LOOP.saltStart->currentText()) LOOP.salinityIndex++;

						/// set file name
						if (LOOP.runMode == SIMULATION_RUN) PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+ QString::number(LOOP.saltStart->currentText().toDouble()*100).append("_100").append(LOOP.simExt));
						else PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+ QString::number(LOOP.saltStart->currentText().toDouble()*100).append("_100").append(LOOP.filExt));

					}
					else if (LOOP.cut == MID)
					{
						if (LOOP.runMode == SIMULATION_RUN) PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+ QString("OIL_").append(QString::number(LOOP.injectTemp)).append(LOOP.simExt));
						else PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+ QString("OIL_").append(QString::number(LOOP.injectTemp)).append(LOOP.filExt));
					}
					else
					{
						if (LOOP.runMode == SIMULATION_RUN) PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+ QString("OIL_").append(QString::number(LOOP.injectTemp)).append(LOOP.simExt));
						else PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+ QString("AMB").append("_").append(QString::number(LOOP.minTemp)).append(LOOP.filExt));
					}
				}
				else // razor
				{
					if (LOOP.runMode == SIMULATION_RUN) PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+ QString("CALIBRAT__").append(QString::number(LOOP.injectTemp)).append(LOOP.simExt));
					else PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+ QString("AMB").append("_").append(QString::number(LOOP.minTemp)).append(LOOP.filExt));
				}
        	}
		}

		///////////////////////////
		///////////////////////////
        /// start calibration
		///////////////////////////
		///////////////////////////

        while (LOOP.isCal)
        {
			if (LOOP.isPause)
			{
				updateCurrentStage(RED,"PAUSE");
			}
			else
			{
				updateCurrentStage(BLACK,LOOP.runMode);

            	if ((LOOP.runMode == TEMPRUN_ONLY) || (LOOP.runMode == TEMPRUN_MIN) || (LOOP.runMode == TEMPRUN_HIGH) || (LOOP.runMode == TEMPRUN_INJECT)) runTempRun();
            	else if ((LOOP.runMode == INJECT_OIL) || (LOOP.runMode == INJECT_WATER) || (LOOP.runMode == SIMULATION_RUN)) runInjection();
            	else
				{
					onActionStop();
					return;
				}
			}

            delay(LOOP.xDelay*1000);
        }
    }
    else
    {
        onActionStop();
        informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),BLANK,"Calibration cancelled!");
    }
}


void
MainWindow::
updateCurrentStage(const QString color, const QString label)
{
	ui->runModeStatus->setStyleSheet(color);
	ui->runModeStatus->setText(label); 	
}


void
MainWindow::
stopCalibration()
{
    int i;

	LOOP.isOilRun = false;
	LOOP.isWaterRun = false;
    LOOP.isCal = false;
    LOOP.isMaster = false;
    LOOP.isEEA = false;
    LOOP.isInitTempRun = false;
    LOOP.isInitInject = false;
	LOOP.salinityIndex = 0;
	LOOP.phaseRolloverCounter = 0;
	LOOP.isPhase = true;
    LOOP.runMode = STOP_CALIBRATION;

    for (i=0;i<3;i++)
    {
        PIPE[i].freqProgress->setValue(0);
        PIPE[i].tempProgress->setValue(0);
        PIPE[i].status = DISABLED;
        PIPE[i].checkBox->setChecked(false);
        PIPE[i].isStartFreq = true;
        PIPE[i].tempStability = 0;
        PIPE[i].freqStability = 0;
    }

	LOOP.axisY->setTitleText("Watercut (%)");
    updateGraph(ALL, RESET_SERIES, RESET_SERIES, SERIES_WATERCUT); // reset chart

    return;
}


void
MainWindow::
readPipe(const int pipe, const bool checkStability)
{
    double val;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );

    /// reset connection
    memset( dest, 0, 1024 );
    modbus_set_slave( LOOP.serialModbus, PIPE[pipe].slave->text().toInt() );

    /// watercut
    if (LOOP.runMode == SIMULATION_RUN) PIPE[pipe].watercut = read_request(FUNC_READ_FLOAT, LOOP.ID_WATERCUT, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    delay(SLEEP_TIME);

    /// temperature
    val = read_request(FUNC_READ_FLOAT, LOOP.ID_TEMPERATURE, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    if ((val<100) && (val>-100)) PIPE[pipe].temperature = val;
    delay(SLEEP_TIME);

    /// get frequency
    val = read_request(FUNC_READ_FLOAT, LOOP.ID_FREQ, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    if ((val<1000) && (val>0)) PIPE[pipe].frequency = val;
    delay(SLEEP_TIME);

    /// get oil_rp
    val = read_request(FUNC_READ_FLOAT, LOOP.ID_OIL_RP, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    if ((val<100) && (val>0)) PIPE[pipe].oilrp = val;
    delay(SLEEP_TIME);

    /// get measured ai
    val = read_request(FUNC_READ_FLOAT, RAZ_MEAS_AI, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    if ((val<100) && (val>-100)) PIPE[pipe].measai = val;
    delay(SLEEP_TIME);

    /// get trimmed ai
    val = read_request(FUNC_READ_FLOAT, RAZ_TRIM_AI, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    if ((val<100) && (val>-100)) PIPE[pipe].trimai = val;
    delay(SLEEP_TIME);

    /// update display
    if (PIPE[pipe].status != DISABLED)
    {
        if ((LOOP.runMode != TEMPRUN_MIN) && (LOOP.runMode != TEMPRUN_HIGH) && (LOOP.runMode != TEMPRUN_INJECT)) displayPipeReading(pipe, PIPE[pipe].watercut, PIPE[pipe].frequency_start, PIPE[pipe].frequency, PIPE[pipe].temperature, PIPE[pipe].oilrp);
        else displayPipeReading(pipe, LOOP.watercut, PIPE[pipe].frequency_start, PIPE[pipe].frequency, PIPE[pipe].temperature, PIPE[pipe].oilrp);
        delay(SLEEP_TIME);
    }

    /// update chart
    if ((LOOP.runMode == INJECT_WATER) || (LOOP.runMode == INJECT_OIL) || (LOOP.runMode == SIMULATION_RUN))
    {
        updateGraph(pipe, PIPE[pipe].frequency, LOOP.masterWatercut, SERIES_WATERCUT);
        updateGraph(pipe, PIPE[pipe].frequency, PIPE[pipe].oilrp, SERIES_RP);
        delay(SLEEP_TIME);
    }
    else
    {
        updateGraph(pipe, PIPE[pipe].frequency, PIPE[pipe].temperature, SERIES_WATERCUT);
        updateGraph(pipe, PIPE[pipe].frequency, PIPE[pipe].oilrp, SERIES_RP);
        delay(SLEEP_TIME);
    }

    updatePipeStability(pipe,checkStability);
}


void
MainWindow::
readMasterPipe()
{
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );

    /// reset connection
    memset( dest, 0, 1024 );
    modbus_set_slave( LOOP.serialModbus, CONTROLBOX_SLAVE);

    /// watercut
    LOOP.masterWatercut = read_request(FUNC_READ_FLOAT, LOOP.ID_MASTER_WATERCUT, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    delay(SLEEP_TIME);

    /// salinity
    LOOP.masterSalinity = read_request(FUNC_READ_FLOAT, LOOP.ID_MASTER_SALINITY, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    delay(SLEEP_TIME);

    /// oil adjust
    LOOP.masterOilAdj = read_request(FUNC_READ_FLOAT, LOOP.ID_MASTER_OIL_ADJUST, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    delay(SLEEP_TIME);

    /// oil rp
    LOOP.masterOilRp = read_request(FUNC_READ_FLOAT, LOOP.ID_MASTER_OIL_RP, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    delay(SLEEP_TIME);

    /// temp
    LOOP.masterTemp = read_request(FUNC_READ_FLOAT, LOOP.ID_MASTER_TEMPERATURE, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    delay(SLEEP_TIME);

    /// freq
    LOOP.masterFreq = read_request(FUNC_READ_FLOAT, LOOP.ID_MASTER_FREQ, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    delay(SLEEP_TIME);

    /// phase
    LOOP.masterPhase = read_request(FUNC_READ_FLOAT, LOOP.ID_MASTER_PHASE, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    delay(SLEEP_TIME);

	/// pressure 
    LOOP.masterPressure = read_request(FUNC_READ_FLOAT, LOOP.ID_MASTER_PRESSURE, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit);
    delay(SLEEP_TIME);

    /// update master pipe display
    updateMasterPipeStatus(LOOP.masterWatercut,LOOP.masterFreq,LOOP.masterTemp,LOOP.masterPhase,LOOP.masterOilAdj,LOOP.masterSalinity);
    delay(SLEEP_TIME);
}


void
MainWindow::
initTempRun()
{
	bool ok;
	/// popup questions
   	if (!isUserInputYes(QString("LOOP ")+QString::number(LOOP.loopNumber),"Fill The Water Container To The Mark."))
   	{
       	onActionStop();
       	return;
   	}

   	LOOP.operatorName = QInputDialog::getText(this, QString("LOOP ")+QString::number(LOOP.loopNumber)+QString(" "),tr(qPrintable("Enter Operator's Name.")), QLineEdit::Normal," ", &ok);
   	LOOP.oilRunStart->setText(QInputDialog::getText(this, QString("LOOP ")+QString::number(LOOP.loopNumber)+QString(" "),tr(qPrintable("Enter Measured Initial Watercut.")), QLineEdit::Normal,"0.0", &ok));
   	LOOP.watercut = LOOP.oilRunStart->text().toDouble();

  	/// master pipe validation
   	if (LOOP.isMaster)
   	{
      	if (LOOP.masterWatercut > LOOP.masterMax)
       	{
           	if (!isUserInputYes(QString("Master Pipe Raw watercut Value Is Greater Than ")+QString::number(LOOP.masterMax), "Do You Want To Continue?"))
           	{
               	onActionStop();
               	return;
           	}
       	}

       	if (LOOP.masterWatercut < LOOP.masterMin)
       	{
           	if (!isUserInputYes(QString("Master Pipe Raw watercut Value Is Less Than ")+QString::number(LOOP.masterMin), "Do You Want To Continue?"))
           	{
               	onActionStop();
               	return;
           	}
       	}
                   
		if (abs(LOOP.masterWatercut - LOOP.oilRunStart->text().toDouble()) > LOOP.masterDelta)
      	{
          	if (!isUserInputYes(QString("The difference between master watercut and measured initial watercut is greater than ")+QString::number(LOOP.masterDelta), "Do You Want To Continue?"))
          	{
               	onActionStop();
              	return;
           	}
     	}
   	}
}


void
MainWindow::
runTempRun()
{
    QString data_stream;

    if (LOOP.isCal)
    {
		/// read master pipe no matter what
        readMasterPipe();

		/// variable initialization 
        if (LOOP.isInitTempRun)
        {
        	LOOP.isInitTempRun = false;
            updatePipeStability(ALL,NO_STABILITY_CHECK);
            LOOP.axisY->setTitleText("Temperature (°C)");

			/// set target temperature
        	if (LOOP.runMode == TEMPRUN_MIN) 
			{
				initTempRun(); /// get initial user inputs
				LOOP.currentTemp = "AMB";
				LOOP.targetTemp = QString::number(LOOP.minTemp);
			}
       		else if (LOOP.runMode == TEMPRUN_HIGH)
			{
				LOOP.currentTemp = QString::number(LOOP.minTemp);
				LOOP.targetTemp = QString::number(LOOP.maxTemp);
			}
       		else if (LOOP.runMode == TEMPRUN_INJECT) 
			{
				LOOP.currentTemp = QString::number(LOOP.maxTemp);
				LOOP.targetTemp = QString::number(LOOP.injectTemp);
			}
			
			/// ask op to set heat exchanger temperature
            if (!isUserInputYes(QString("Set The Heat Exchanger Temperature (°C)"), LOOP.targetTemp))
            {
                onActionStop();
                return;
            }

            /// create files
            for (int pipe = 0; pipe < 3; pipe++)
            {
                if (PIPE[pipe].status == ENABLED)
                {
          			setFileNameForNextStage(pipe, LOOP.currentTemp+"_"+LOOP.targetTemp+LOOP.filExt);
                    if (!QFileInfo(PIPE[pipe].file).exists()) createTempRunFile(PIPE[pipe].slave->text().toInt(), LOOP.currentTemp, LOOP.targetTemp, "1", pipe);
                }
            }

			/// reset chart
    		updateGraph(ALL, RESET_SERIES, RESET_SERIES, SERIES_WATERCUT);

		} //---- end LOOP.isInitTempRun here ---//

        /// start reading values and write to file
        for (int pipe = 0; pipe < 3; pipe++)
        {
			/// skip to next temprun stage?
			if (LOOP.isTempRunSkip)
			{
				if ((PIPE[pipe].status == ENABLED) && PIPE[pipe].checkBox->isChecked()) 
				{
					PIPE[pipe].tempStability = 5;
					PIPE[pipe].freqStability = 5;
				}
			}

            /// if not stable yet && read pipe and update temprun file
            if ((PIPE[pipe].status == ENABLED) && PIPE[pipe].checkBox->isChecked() && ((PIPE[pipe].tempStability != 5) || (PIPE[pipe].freqStability != 5)))
            {
            	(abs(LOOP.targetTemp.toDouble() - PIPE[pipe].temperature) < 2.0) ? readPipe(pipe, STABILITY_CHECK) : readPipe(pipe, NO_STABILITY_CHECK);
                createDataStream(pipe,data_stream);
                writeToCalFile(pipe, data_stream);
            }
            else /// now stable
            {
                if (PIPE[pipe].status == ENABLED) /// "ENABLED" pipe is now stable 
                {
					PIPE[pipe].tempStability = 0;
					PIPE[pipe].freqStability = 0;
                    PIPE[pipe].status = DONE; /// set status DONE 

					/// all pipes are finished with current stage
                    if ((PIPE[0].status != ENABLED) && (PIPE[1].status != ENABLED) && (PIPE[2].status != ENABLED)) 
					{
						LOOP.isInitTempRun = true;
						LOOP.isTempRunSkip = false;

						/// change mode
						if (LOOP.runMode == TEMPRUN_MIN) 
						{
							LOOP.runMode = TEMPRUN_HIGH;
							for (int x=0; x<3; x++) if (PIPE[x].status == DONE) PIPE[x].status = ENABLED;
						}
						else if (LOOP.runMode == TEMPRUN_HIGH) 
						{
							LOOP.runMode = TEMPRUN_INJECT;
							for (int x=0; x<3; x++) if (PIPE[x].status == DONE) PIPE[x].status = ENABLED;
						}
						else
						{
							if (LOOP.isTempRunOnly) 
							{
								informUser("Temp Run Has Finished.", "Calibration Stopped" ); // TEMPRUN_ONLY mode
								onActionStop();
							}
                        	else /// temprun is over. moving to inject_run
							{
								for (int pipe=0; pipe<3; pipe++)	
								{
                    				if (PIPE[pipe].status == DONE)
									{
										if (LOOP.cut == LOW) setFileNameForNextStage(pipe,"CALIBRAT.LCI");
                        				else setFileNameForNextStage(pipe,QString("OIL__").append(LOOP.targetTemp).append(".MCI"));
                        				PIPE[pipe].frequency_start = PIPE[pipe].frequency;
										PIPE[pipe].status = ENABLED;
									}
								}

								informUser("Temp Run Has Finished.", "Injection Will Get Started." );
								LOOP.runMode = INJECT_WATER;
								LOOP.isOilRun = true;
								LOOP.isWaterRun = false;
								LOOP.isInitInject = true;
								LOOP.isInitTempRun = false;
								LOOP.isPhase = true;
							}
						}
					}
                }
            }
        }
    }
}


void
MainWindow::
runInjection()
{
	if (LOOP.runMode == STOP_CALIBRATION) return;

    static double injectionTime = 0;
    static double totalInjectionTime = 0;
    static double accumulatedInjectionTime_prev = 0;
    static double correctedWatercut = 0;
    static double measuredWatercut = 0;
    static double totalInjectionVolume = 0;
    QString data_stream;

    if ((LOOP.runMode == INJECT_OIL) || (LOOP.runMode == INJECT_WATER) || (LOOP.runMode == SIMULATION_RUN))
    {
		/// enable pipes in simulation mode
		if (LOOP.runMode == SIMULATION_RUN)
		{
			if (!enablePipes()) return;
		}

        if (LOOP.isInitInject)
        {
 			/// user notice
 			LOOP.ignore = false;           
			bool ok;
  
         	/// chage y axis and reset series
            LOOP.axisY->setTitleText("Watercut (%)");
    		updateGraph(ALL, RESET_SERIES, RESET_SERIES, SERIES_WATERCUT);

			if (LOOP.isWaterRun)
			{
				/// confirm salinity
				if (!isUserInputYes(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("Fill The Loop With Water At ").append(SALINITY[LOOP.salinityIndex]).append(" \% Salinity")))
				{
					onActionStop();
               		return;
				}

				/// confirm enough oil
				if (!isUserInputYes(QString("WATER_RUN : OIL INJECTION WILL START")+QString::number(LOOP.loopNumber),"Please Make Sure There Is Enough Oil In The Injection Bucket"))
           		{
               		onActionStop();
               		return;
           		}

				/// confirm temperature
               	if (!isUserInputYes(QString("Set The Heat Exchanger Temperature"), "38 °C  (100 °F)"))
				{
					onActionStop();
               		return;
				}

				/* no need to ask initial watercut - 99% always */
           		LOOP.watercut = LOOP.waterRunStart->text().toDouble();
           		LOOP.phaseRolloverCounter = 0;
				LOOP.isOilRun = false;
				LOOP.isPhase = true;
			}
			else if (LOOP.isOilRun) // lowcut, miduct, razor
			{
				/// confirm salinity
				if (!isUserInputYes(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("Fill The Loop With Oil At ").append("1 \% Salinity")))
				{
					onActionStop();
               		return;
				}

				if (!isUserInputYes(QString("OIL_RUN : WATER INJECTION WILL START")+QString::number(LOOP.loopNumber),"Please Make Sure There Is Enough Water In The Injection Bucket"))
           		{
               		onActionStop();
               		return;
           		}

				/// confirm temperature
				if (LOOP.isEEA)
				{
               		if (!isUserInputYes(QString("Set The Heat Exchanger Temperature"), "60 °C  (140 °F)"))
					{
						onActionStop();
               			return;
					}
				}
				else
				{
					if (!isUserInputYes(QString("Set The Heat Exchanger Temperature"), "38 °C  (100 °F)"))
					{
						onActionStop();
               			return;
					}
				}

				/// get initial watercut
           		LOOP.oilRunStart->setText(QInputDialog::getText(this, QString("LOOP ")+QString::number(LOOP.loopNumber),tr(qPrintable("Enter Measured Initial Watercut")), QLineEdit::Normal,"0.0", &ok));
           		LOOP.watercut = LOOP.oilRunStart->text().toDouble();
           		LOOP.phaseRolloverCounter = 0;
				LOOP.isWaterRun = false;
				LOOP.isPhase = true;
			}

			/// create inject file	
			for (int pipe = 0; pipe < 3; pipe++)
            {
                if (PIPE[pipe].status == ENABLED)
                {
                    if (LOOP.cut == LOW) createInjectFile(pipe, LOOP.oilRunStart->text(), LOOP.oilRunStop->text(), "1", "CALIBRAT");
                    else if (LOOP.cut == MID) createInjectFile(pipe, LOOP.oilRunStart->text(), LOOP.oilRunStop->text(), "1", "MID");
					else if (LOOP.cut == HIGH) createInjectFile(pipe,LOOP.waterRunStart->text() , LOOP.waterRunStop->text(), SALINITY[LOOP.salinityIndex], "ANY");
                    else if (LOOP.cut == FULL)
					{
						if (LOOP.isWaterRun) createInjectFile(pipe, LOOP.waterRunStart->text(), LOOP.waterRunStop->text(), SALINITY[LOOP.salinityIndex], "ANY");
						else if (LOOP.isOilRun) createInjectFile(pipe, LOOP.oilRunStart->text(),LOOP.oilRunStop->text(), "1", "ANY");
					}
                }
            }

			LOOP.isInitInject = false;
        	updateGraph(ALL, RESET_SERIES, RESET_SERIES, SERIES_WATERCUT); // reset chart

        } //---- end LOOP.isInitInject ---//

        // 
        if ((( LOOP.isOilRun && (LOOP.oilRunStop->text().toDouble() >= LOOP.watercut)) || 
			( LOOP.isWaterRun && (LOOP.waterRunStop->text().toDouble() <= LOOP.watercut))) && LOOP.isPhase)
        {
			//// update calibration file with the latest reads
            for (int pipe = 0; pipe < 3; pipe++)
            {
                if ((PIPE[pipe].status == ENABLED) && PIPE[pipe].checkBox->isChecked())
                {
                    /// read data
                    readPipe(pipe, NO_STABILITY_CHECK);

                    /// write to calibration file
                    createDataStream(pipe,data_stream);
                    writeToCalFile(pipe, data_stream);
                }
            }

            /// in master pipe mode
            if (LOOP.isMaster)
            {
				if (LOOP.runMode == STOP_CALIBRATION) return;

                int masterInjectionStartTime = PIPE[0].etimer->elapsed()/1000;

				/// start injecting
   		 		onActionStartInjection();

				/// compare with master pipe
                while (1)
                {
					/// phase condition checking for injection stop 
					if (((LOOP.isOilRun) && ((int)LOOP.masterPhase == PHASE_OIL)) || ((LOOP.isWaterRun) && ((int)LOOP.masterPhase == PHASE_WATER)))
					{
   	                	LOOP.phaseRolloverCounter = 0;
						LOOP.isPhase = true;
					}
					else
   	            	{
   	                	LOOP.phaseRolloverCounter++;

   	                	if (LOOP.phaseRolloverCounter > MAX_PHASE_CHECKING)
   	                	{
   	                		LOOP.phaseRolloverCounter = 0;
   	                    	if (LOOP.masterPhase == PHASE_WATER) informUser("WATER Phase", QString("Master Pipe Is In WATER Phase Now : Watercut ").append(QString::number(LOOP.masterWatercut)).append(" %"));
   	                    	else if (LOOP.masterPhase == PHASE_OIL) informUser("OIL Phase", QString("Master Pipe Is In OIL Phase Now : Watercut ").append(QString::number(LOOP.masterWatercut)).append(" %"));
   	                    	else informUser("ERROR Phase",QString("Master Pipe Is In ERROR Phase : Phase Value Is ").append(QString::number(LOOP.masterPhase)));
			
							LOOP.isPhase = false;
   	                	}
					}

                    /// check max injection time
                    int masterInjectionTime = PIPE[0].etimer->elapsed()/1000;
                    if ((masterInjectionTime - masterInjectionStartTime) > LOOP.maxInjectionWater)
                    {
                        if (!LOOP.ignore)
                        {
							LOOP.ignore = true;

                            /// stop water injection?
                            if (!isUserInputYes(QString("Injection Time ")+QString::number(masterInjectionTime - masterInjectionStartTime)+QString(" Is Greater Than Max Water Injection Time ")+QString::number(LOOP.maxInjectionWater), "Do You Want To Continue?"))
                            {
    							onActionStopInjection();
                                onActionStop();
                                return;
                            }
                        }
                    }

					/// read master pipe
                    readMasterPipe();

					/// update current loop status in waterrun mode
            		if (LOOP.isWaterRun) updateLoopStatus(LOOP.watercut, QString(SALINITY[LOOP.salinityIndex]).toDouble(),PIPE[0].etimer->elapsed()/1000-masterInjectionStartTime, (PIPE[0].etimer->elapsed()/1000-masterInjectionStartTime)*LOOP.injectionWaterPumpRate/60);

					/// check if current target watercut is reached
					if ((LOOP.isOilRun && (LOOP.masterWatercut >= LOOP.watercut)) || 
					    (LOOP.isWaterRun && (LOOP.masterWatercut <= LOOP.watercut))) break;
                }

                /// stop water injection
    			onActionStopInjection();

				/// update injection results
                int masterInjectionEndTime = PIPE[0].etimer->elapsed()/1000;
                totalInjectionVolume += (masterInjectionEndTime-masterInjectionStartTime)*LOOP.injectionWaterPumpRate/60;
                totalInjectionTime += masterInjectionEndTime-masterInjectionStartTime;
 
                /// set next watercut target
                if (LOOP.cut == LOW) LOOP.watercut += LOOP.intervalSmallPump; 
				else if (LOOP.cut == MID) LOOP.watercut += LOOP.intervalBigPump;
				else if (LOOP.cut == FULL) 
				{
					if (LOOP.isWaterRun) LOOP.watercut -= LOOP.intervalBigPump;
					else LOOP.watercut += LOOP.intervalBigPump;
				}
				else if (LOOP.cut == HIGH) LOOP.watercut -= LOOP.intervalBigPump;
            }

            ///////////////////////////////////////
            //// IN PUMP RATE MODE
            ///////////////////////////////////////

            else 
            {
				if (LOOP.runMode == STOP_CALIBRATION) return;

                /// next injection time and update totalInjectionTime
                double accumulatedInjectionTime = -(LOOP.loopVolume->text().toDouble()/(LOOP.injectionWaterPumpRate/60))*log((1-(LOOP.watercut - LOOP.oilRunStart->text().toDouble())/100));
                injectionTime = accumulatedInjectionTime - accumulatedInjectionTime_prev;
                totalInjectionTime += injectionTime;
                totalInjectionVolume = totalInjectionTime*LOOP.injectionWaterPumpRate/60;
                accumulatedInjectionTime_prev = accumulatedInjectionTime;

                /// validate injection time
                if (injectionTime > LOOP.maxInjectionWater)
                {
                    /// stop water injection?
                    if (!isUserInputYes(QString("Injection Time ")+QString::number(injectionTime)+QString(" Is Greater Than Max Water Injection Time ")+QString::number(LOOP.maxInjectionWater), "Do You Want To Continue?"))
                    {
                        onActionStop();
    					onActionStopInjection();
                        return;
                    }
                }

                /// inject water to the pipe for "injectionTime" seconds
    			onActionStartInjection();
                delay(injectionTime*1000);
    			onActionStopInjection();

                /// set next watercut
                if (LOOP.cut == LOW) LOOP.watercut += LOOP.intervalSmallPump; 
				else if (LOOP.cut == MID) LOOP.watercut += LOOP.intervalBigPump;
				else if (LOOP.cut == FULL) 
				{
					if (LOOP.isWaterRun) LOOP.watercut -= LOOP.intervalBigPump;
					else LOOP.watercut += LOOP.intervalBigPump;
				}
				else if (LOOP.cut == HIGH) LOOP.watercut -= LOOP.intervalBigPump;
            }

           	if (LOOP.isWaterRun) updateLoopStatus(LOOP.watercut, QString(SALINITY[LOOP.salinityIndex]).toDouble(), injectionTime, injectionTime*LOOP.injectionWaterPumpRate/60);
        }
        else /// finish
        {
            /// enter measured watercut and injected volume
            bool ok;
            measuredWatercut = QInputDialog::getDouble(this, QString("LOOP ")+QString::number(LOOP.loopNumber),tr(qPrintable("Enter Measured Watercut [%]")), 0.0, 0, 100, 2, &ok,Qt::WindowFlags(), 1);

            if (LOOP.isMaster)
            {
                if (abs(LOOP.masterWatercut - measuredWatercut) > LOOP.masterDeltaFinal)
                {
                    /// stop water injection?
                    if (!isUserInputYes(QString("MASTER PIPE ")+QString::number(LOOP.loopNumber)+QString(" Difference between measured watercut and master watercut is greater than ")+QString::number(LOOP.masterDeltaFinal), "Do You Want To Continue?"))
                    {
                        onActionStop();
                        return;
                    }
                }
            }

            /// finalize and close
            QDateTime currentDataTime = QDateTime::currentDateTime();
            QString data_stream   = QString("Total injection time   = %1 s").arg(totalInjectionTime, 10, 'g', -1, ' ');
            QString data_stream_2 = QString("Total injection volume = %1 mL").arg(totalInjectionVolume, 10, 'g', -1, ' ');
            QString data_stream_3 = QString("Initial loop volume    = %1 mL").arg(LOOP.loopVolume->text().toDouble(), 10, 'g', -1, ' ');
            QString data_stream_4 = QString("Measured watercut      = %1 %").arg(measuredWatercut, 10, 'f', 2, ' ');
            QString data_stream_5 = QString("[%1] [%2]").arg(currentDataTime.toString()).arg(LOOP.operatorName);

            for (int pipe=0; pipe<3; pipe++)
            {
                if ((PIPE[pipe].status == ENABLED) && (QFileInfo(PIPE[pipe].file).exists()))
                {
                    QTextStream stream(&PIPE[pipe].file);
                    PIPE[pipe].file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
                    stream << '\n' << '\n' << data_stream << '\n' << data_stream_2 << '\n' << data_stream_3 << '\n' << data_stream_4 << '\n' << data_stream_5 << '\n';
                    PIPE[pipe].file.close();
                }
            }

			/// stop if razor
			if ((!LOOP.isEEA) || (LOOP.cut == MID))
            {
                /// finish calibration or simulation 
                if (LOOP.runMode == SIMULATION_RUN) informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),BLANK,"Simulation has finished successfully.");
                else informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),BLANK,"Calibration has finished successfully.");
                onActionStop();
                return;
            }
			else if ((LOOP.cut == HIGH) || (LOOP.cut == FULL))
            {
                if (LOOP.runMode == SIMULATION_RUN)
				{
					/// finish calibration if razor
                	informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),BLANK,"Simulation has finished successfully.");
                	onActionStop();
                	return;
				}

                informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),("Calibration has finished at "), QString(SALINITY[LOOP.salinityIndex]).append(" \% Salinity."));

				if (LOOP.isWaterRun) 
				{
					/// finish if reached at stop_salinity
					if (SALINITY[LOOP.salinityIndex] == LOOP.saltStop->currentText())
					{
        				informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),QString("Water Run Has Finished Successfully.      "),"Oil Run Will Get Started.");
				
						/// update run triggers	
						LOOP.isOilRun = true;
						LOOP.isWaterRun = false;
						LOOP.isInitInject = true;
						LOOP.salinityIndex = 0;
						LOOP.isPhase = true;

						/// set oil run file name
						for (int pipe=0; pipe<3; pipe++)
						{
							if (PIPE[pipe].status == ENABLED) PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+ QString("OIL__140").append(LOOP.filExt));
						}
					}
					else // otherwise, go to next salinity
					{
						LOOP.isInitInject = true;
						LOOP.salinityIndex++; // go to next salinity
						for (int pipe=0; pipe<3; pipe++) if (PIPE[pipe].status == ENABLED) PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+ QString::number(SALINITY[LOOP.salinityIndex].toDouble()*100).append("_100").append(LOOP.filExt));
					}
				}
				else // oil run needs to stop here
				{	/// entire calibration has finished after oilRun finished
					onActionStop(); 
                	informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),BLANK,"Calibration has finished successfully.");
				}

               	return;
            }

			//---------- LOWCUT starts here -------------//

            /// ADJUSTED.LCI
            if (abs(totalInjectionVolume - (LOOP.injectionWaterPumpRate/60)*totalInjectionTime) > 0)
            {
                for (int pipe=0; pipe<3; pipe++)
                {
                    if ((PIPE[pipe].status == ENABLED) && PIPE[pipe].fileCalibrate.open(QIODevice::ReadOnly))
                    {
                        int i = 0;
                        PIPE[pipe].fileAdjusted.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
                        QTextStream in(&PIPE[pipe].fileCalibrate);
                        QTextStream out(&PIPE[pipe].fileAdjusted);
                        while (!in.atEnd())
                        {
                            QString line = in.readLine();
                            if (i > 6)
                            {
                                /// process string and correct watercut
                                QStringList data = line.split(" ");
                                QString newData[data.size()];
                                int k = 0;
                                int x = 0;

                                for (int j=0; j<data.size(); j++)
                                {
                                    if (data[j] != "")
                                    {
                                        newData[k] = data[j];
                                        if (k==1) x = j; /// saving index for watercut
                                        if (k==12)
                                        {
                                            correctedWatercut = (LOOP.oilRunStart->text().toDouble() + 100) - (100*exp(-(LOOP.injectionWaterPumpRate/60)*QString(newData[k]).toDouble()/LOOP.loopVolume->text().toDouble()));
                                            data[x] = QString("%1").arg(correctedWatercut,7,'f',2,' ');
                                        }
                                        k++;
                                    }
                                }

                                /// re-create a corrected data_stream
                                line = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16 %17 %18 %19").arg(newData[0].toDouble(), 9, 'g', -1, ' ').arg(correctedWatercut,7,'f',2,' ').arg(newData[2].toDouble(), 4, 'g', -1, ' ').arg(" INT").arg(1, 7, 'g', -1, ' ').arg(newData[5].toDouble(),9,'f',3,' ').arg(0,8,'f',2,' ').arg(newData[7].toDouble(),9,'f',2,' ').arg(newData[8].toDouble(),11,'f',2,' ').arg(0,8,'f',2,' ').arg(newData[10].toDouble(),12,'f',2,' ').arg(newData[11].toDouble(),12,'f',2,' ').arg(0,10,'f',2,' ').arg(newData[13].toDouble(), 11,'f',2,' ').arg(newData[14].toDouble(), 11,'f',2,' ').arg(newData[15].toDouble(), 11,'f',2,' ').arg(newData[16].toDouble(), 11,'f',2,' ').arg(newData[17].toDouble(), 11,'f',2,' ').arg(0,12,'f',2,' ');
                            }

                            out << line << '\n';
                            i++;
                        }

                        PIPE[pipe].fileCalibrate.close();
                        PIPE[pipe].fileAdjusted.close();
                    }
                }
            }

            /// finalize current file
            for (int pipe=0; pipe<3; pipe++)
            {
                if ((PIPE[pipe].status == ENABLED) && QFileInfo(PIPE[pipe].fileCalibrate).exists())
                {
                    QTextStream stream(&PIPE[pipe].fileCalibrate);
                    PIPE[pipe].fileCalibrate.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
                    stream << '\n' << '\n' << data_stream << '\n' << data_stream_2 << '\n' << data_stream_3 << '\n' << data_stream_4 << '\n' << data_stream_5 << '\n';
                    PIPE[pipe].fileCalibrate.close();
                }

                if (QFileInfo(PIPE[pipe].fileAdjusted).exists())
                {
                    QTextStream stream(&PIPE[pipe].fileAdjusted);
                    PIPE[pipe].fileAdjusted.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
                    stream << '\n' << '\n' << data_stream << '\n' << data_stream_2 << '\n' << data_stream_3 << '\n' << data_stream_4 << '\n' << data_stream_5 << '\n';
                    PIPE[pipe].fileAdjusted.close();
                }
            }

            /// prepare for ROLLOVER.LCR
            informUser(QString("LOOP ")+QString::number(LOOP.loopNumber),BLANK,"Please Switch The Injection Pump.");
            LOOP.watercut += LOOP.intervalBigPump;
            for (int pipe = 0; pipe < 3; pipe++)
            {
                if (PIPE[pipe].status == ENABLED)
                {
                    setFileNameForNextStage(pipe,"ROLLOVER.LCR");
                    LOOP.watercut = correctedWatercut;
                    PIPE[pipe].rolloverTracker = 0;
                }
            }
        }
    }
    else if ((QFileInfo(PIPE[0].file).fileName() == "ROLLOVER.LCR") ||
             (QFileInfo(PIPE[1].file).fileName() == "ROLLOVER.LCR") ||
             (QFileInfo(PIPE[2].file).fileName() == "ROLLOVER.LCR")) // ROLLOVER.LCR
    {
        for (int pipe = 0; pipe < 3; pipe++)
        {
            if ((PIPE[pipe].status == ENABLED) && PIPE[pipe].checkBox->isChecked())
            {
                readPipe(pipe, NO_STABILITY_CHECK);

                if (PIPE[pipe].frequency < PIPE[pipe].frequency_prev)
                {
                    if (PIPE[pipe].rolloverTracker > 2) PIPE[pipe].status == DONE;
                    else PIPE[pipe].rolloverTracker++;
                    PIPE[pipe].frequency_prev = PIPE[pipe].frequency;
                }
                else PIPE[pipe].rolloverTracker = 0;

                /// read data
                createDataStream(pipe,data_stream);

                /// create a new file if needed
                if (!QFileInfo(PIPE[pipe].file).exists())
                {
                    /// re-read data
                    createInjectFile(pipe, "Rollver", QString::number(LOOP.watercut), "1", "ROLLOVER");
                    displayPipeReading(pipe, LOOP.watercut, PIPE[pipe].frequency_start, PIPE[pipe].frequency, PIPE[pipe].temperature, PIPE[pipe].oilrp);
                }

                writeToCalFile(pipe, data_stream);
            }
        }

        if (LOOP.isMaster)
        {
            int masterInjectionStartTime = PIPE[0].etimer->elapsed()/1000;

            while (LOOP.masterWatercut < LOOP.watercut)
            {
                int masterInjectionTime = PIPE[0].etimer->elapsed()/1000;

				/// start injection
    			onActionStartInjection();

                /// validate injection time
                if ((masterInjectionTime - masterInjectionStartTime) > LOOP.maxInjectionWater)
                {
                    if (!isUserInputYes(QString("Injection Time")+QString::number(masterInjectionTime - masterInjectionStartTime)+QString(" Is Greater Than Max Water Injection Time ")+QString::number(LOOP.maxInjectionWater), "Do You Want To Continue?"))
                    {
                    	/// stop water injection
    					onActionStopInjection();
                        onActionStop();
                        return;
                    }
                }
            }

            /// stop water injection
    		onActionStopInjection();

            /// set next watercut
            LOOP.watercut += LOOP.intervalBigPump;
        }
        else
        {
            /// next injection time and update totalInjectionTime
            double accumulatedInjectionTime = -(LOOP.loopVolume->text().toDouble()/(LOOP.injectionWaterPumpRate/60))*log((1-(LOOP.watercut - LOOP.oilRunStart->text().toDouble())/100));
            injectionTime = accumulatedInjectionTime - accumulatedInjectionTime_prev;
            totalInjectionTime += injectionTime;
            totalInjectionVolume = totalInjectionTime*LOOP.injectionWaterPumpRate/60;
            accumulatedInjectionTime_prev = accumulatedInjectionTime;

            /// validate injection time
            if (injectionTime > LOOP.maxInjectionWater)
            {
                if (!isUserInputYes(QString("Injection Time ")+QString::number(injectionTime)+QString(" Is Greater Than Max Water Injection Time ")+QString::number(LOOP.maxInjectionWater), "Do You Want To Continue?"))
                {
                    onActionStop();
                    return;
                }
            }

            /// inject water to the pipe for "injectionTime" seconds
            onActionStartInjection();
            delay(injectionTime*1000);
            onActionStopInjection();

            /// set next watercut
            LOOP.watercut += LOOP.intervalBigPump;
        }
    }
    else
    {
        /// finalize and close
        QDateTime currentDataTime = QDateTime::currentDateTime();
        QString data_stream   = QString("Total injection time   = %1 s").arg(totalInjectionTime, 10, 'g', -1, ' ');
        QString data_stream_2 = QString("Total injection volume = %1 mL").arg(totalInjectionVolume, 10, 'g', -1, ' ');
        QString data_stream_3 = QString("Initial loop volume    = %1 mL").arg(LOOP.loopVolume->text().toDouble(), 10, 'g', -1, ' ');
        QString data_stream_4 = QString("[%1] [%2]").arg(currentDataTime.toString()).arg(LOOP.operatorName);

        for (int pipe=0; pipe<3; pipe++)
        {
            if ((PIPE[pipe].status == DONE) && QFileInfo(PIPE[pipe].fileRollover).exists())
            {
                QTextStream stream(&PIPE[pipe].fileRollover);
                PIPE[pipe].fileRollover.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
                stream << '\n' << '\n' << data_stream << '\n' << data_stream_2 << '\n' << data_stream_3 << '\n' << data_stream_4 << '\n';
                PIPE[pipe].fileRollover.close();
                PIPE[pipe].checkBox->setChecked(false);
            }
        }
   }

    return;
}


void
MainWindow::
inject(const int coil,const bool value)
{
    uint8_t dest[1024];

    /// set slave
    memset( dest, 0, 1024 );
    modbus_set_slave(LOOP.serialModbus, CONTROLBOX_SLAVE);
    modbus_write_bit(LOOP.serialModbus,coil-ADDR_OFFSET, value);
}


void
MainWindow::
setFileNameForNextStage(const int pipe, const QString nextFileId)
{
    PIPE[pipe].file.setFileName(PIPE[pipe].mainDirPath+"\\"+nextFileId);
    PIPE[pipe].freqStability = 0;
    PIPE[pipe].tempStability = 0;
}


void
MainWindow::
writeToCalFile(int pipe, QString data_stream)
{
    /// write to file
    QTextStream stream(&PIPE[pipe].file);
    PIPE[pipe].file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    stream << data_stream << '\n' ;
    PIPE[pipe].file.close();
}


void
MainWindow::
onFunctionCodeChanges()
{
    if (ui->radioButton_181->isChecked()) // float
    {
        if (ui->radioButton_187->isChecked()) // read
        {
            ui->functionCode->setCurrentIndex(3);
        }
        else
        {
            ui->functionCode->setCurrentIndex(7);
        }
    }
    else if (ui->radioButton_182->isChecked()) // integer
    {
        if (ui->radioButton_187->isChecked()) // read
        {
            ui->functionCode->setCurrentIndex(3);
        }
        else
        {
            ui->functionCode->setCurrentIndex(5);
        }
    }
    else // coil
    {
        if (ui->radioButton_187->isChecked()) // read
        {
            ui->functionCode->setCurrentIndex(0);
        }
        else
        {
            ui->functionCode->setCurrentIndex(4);
        }
    }
}


void
MainWindow::
onReadButtonPressed()
{
	ui->groupBox_103->setEnabled(false);
   	ui->groupBox_106->setEnabled(false);
   	ui->groupBox_107->setEnabled(false);

    if (ui->radioButton_181->isChecked()) ui->functionCode->setCurrentIndex(3); // float
    else if (ui->radioButton_182->isChecked()) ui->functionCode->setCurrentIndex(3); // integer
    else ui->functionCode->setCurrentIndex(0); // coil
}


void
MainWindow::
onWriteButtonPressed()
{
    ui->groupBox_103->setEnabled(true);
    ui->groupBox_106->setEnabled(true);
    ui->groupBox_107->setEnabled(true);

	if (ui->radioButton_181->isChecked()) ui->functionCode->setCurrentIndex(7); // float
    else if (ui->radioButton_182->isChecked()) ui->functionCode->setCurrentIndex(5); // integer
    else ui->functionCode->setCurrentIndex(4); // coil
}


float
MainWindow::
toFloat(QByteArray f)
{
    bool ok;
    int sign = 1;

    f = f.toHex(); // Convert to Hex

    f = QByteArray::number(f.toLongLong(&ok, 16), 2);    // Convert hex to binary

    if(f.length() == 32) {
        if(f.at(0) == '1') sign =-1;     // If bit 0 is 1 number is negative
        f.remove(0,1);                   // Remove sign bit
    }

    QByteArray fraction = f.right(23);  // Get the fractional part
    double mantissa = 0;
    for(int i = 0; i < fraction.length(); i++){  // Iterate through the array to claculate the fraction as a decimal.
        if(fraction.at(i) == '1')
            mantissa += 1.0 / (pow(2, i+1));
    }

    int exponent = f.left(f.length() - 23).toLongLong(&ok, 2) - 127;     // Calculate the exponent

    return (sign * pow(2, exponent) * (mantissa + 1.0));
}


void
MainWindow::
onUpdateRegisters(const bool isEEA)
{
    if (isEEA)
    {
        LOOP.ID_SN_PIPE = EEA_ID_SN_PIPE;
        LOOP.ID_WATERCUT = EEA_ID_WATERCUT;
        LOOP.ID_SALINITY = EEA_ID_SALINITY;
        LOOP.ID_OIL_ADJUST = EEA_ID_OIL_ADJUST;
        LOOP.ID_TEMPERATURE = EEA_ID_TEMPERATURE;
        LOOP.ID_WATER_ADJUST = EEA_ID_WATER_ADJUST;
        LOOP.ID_FREQ = EEA_ID_FREQ;
        LOOP.ID_OIL_RP = EEA_ID_OIL_RP;
		LOOP.ID_PRESSURE = EEA_ID_PRESSURE;
    }
    else
    {
		LOOP.ID_SN_PIPE = RAZ_ID_SN_PIPE;
        LOOP.ID_WATERCUT = RAZ_ID_WATERCUT;
        LOOP.ID_SALINITY = RAZ_ID_SALINITY;
        LOOP.ID_OIL_ADJUST = RAZ_ID_OIL_ADJUST;
        LOOP.ID_TEMPERATURE = RAZ_ID_TEMPERATURE;
        LOOP.ID_WATER_ADJUST = RAZ_ID_WATER_ADJUST;
        LOOP.ID_FREQ = RAZ_ID_FREQ;
        LOOP.ID_OIL_RP = RAZ_ID_OIL_RP;
		LOOP.ID_PRESSURE = EEA_ID_PRESSURE;
    }
}


void
MainWindow::
displayPipeReading(const int pipe, const double watercut, const double startfreq, const double freq, const double temp, const double rp)
{
    if ((PIPE[pipe].status == ENABLED) && (!isModbusTransmissionFailed))
    {
        PIPE[pipe].wc->setText(QString("%1").arg(watercut,7,'f',2,' '));
        if (PIPE[pipe].isStartFreq) PIPE[pipe].startFreq->setText(QString("%1").arg(freq,7,'f',3,' '));
        PIPE[pipe].freq->setText(QString("%1").arg(freq,7,'f',3,' '));
        PIPE[pipe].temp->setText(QString("%1").arg(temp,7,'f',2,' '));
        PIPE[pipe].reflectedPower->setText(QString("%1").arg(rp,7,'f',3,' '));
        if (freq > 100) PIPE[pipe].isStartFreq = false;
    }
	else if (pipe == ALL)
    {
		for (int i=0; i<ALL; i++)
		{ 
			PIPE[i].wc->setText("0");
        	PIPE[i].startFreq->setText("0");
        	PIPE[i].freq->setText("0");
        	PIPE[i].temp->setText("0");
        	PIPE[i].reflectedPower->setText("0");
		}
    }
}

void
MainWindow::
updateMasterPipeStatus(const double watercut, const double freq, const double temp, const double phase, const double adj, const double salt)
{
    if (!isModbusTransmissionFailed)
    {
		if ((int)phase == PHASE_OIL) ui->lineEdit_31->setText(QString("OIL"));
        else if ((int)phase == PHASE_WATER) ui->lineEdit_31->setText(QString("WATER"));
        else if ((int)phase == PHASE_ERROR) ui->lineEdit_31->setText(QString("ERROR")); 
        ui->lineEdit_20->setText(QString("%1").arg(watercut,7,'f',2,' '));
        ui->lineEdit_22->setText(QString("%1").arg(salt,7,'f',2,' '));
        ui->lineEdit_21->setText(QString("%1").arg(adj,7,'f',2,' '));
        ui->lineEdit_29->setText(QString("%1").arg(temp,7,'f',2,' '));
        ui->lineEdit_30->setText(QString("%1").arg(freq,7,'f',3,' '));
        //ui->lineEdit_31->setText(QString("%1").arg(phase,7,'f',3,' '));
    }
}

void
MainWindow::
updateLoopStatus(const double watercut, const double salt, const double injectionTime, const double injectionVol)
{
    if (!isModbusTransmissionFailed)
    {
        ui->lineEdit_26->setText(QString("%1").arg(watercut,7,'f',2,' '));
        ui->lineEdit_25->setText(QString("%1").arg(salt,7,'f',2,' '));
        ui->lineEdit_24->setText(QString::number(injectionTime));
        ui->lineEdit_23->setText(QString::number(injectionVol));
    }
}

void
MainWindow::
createDataStream(const int pipe, QString & data_stream)
{
    if (LOOP.isMaster)
    {
        if (LOOP.runMode == SIMULATION_RUN) data_stream = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16 %17 %18").arg(PIPE[pipe].etimer->elapsed()/1000, 9, 'g', -1, ' ').arg(LOOP.masterWatercut,7,'f',2,' ').arg(PIPE[pipe].osc, 4, 'g', -1, ' ').arg(" INT").arg(1, 7, 'g', -1, ' ').arg(PIPE[pipe].frequency,9,'f',3,' ').arg(0,8,'f',2,' ').arg(PIPE[pipe].oilrp,9,'f',2,' ').arg(PIPE[pipe].temperature,11,'f',2,' ').arg(0,10,'f',2,' ').arg(LOOP.masterPressure,8,'f',2,' ').arg(LOOP.masterTemp, 11,'f',2,' ').arg(LOOP.masterOilAdj, 11,'f',2,' ').arg(LOOP.masterFreq, 11,'f',2,' ').arg(LOOP.masterWatercut, 11,'f',2,' ').arg(LOOP.masterOilRp, 11,'f',2,' ').arg(LOOP.masterPhase, 6,'f',1,' ').arg(PIPE[pipe].watercut,8,'f',2,' ');

        else data_stream = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16 %17 %18").arg(PIPE[pipe].etimer->elapsed()/1000, 9, 'g', -1, ' ').arg(LOOP.masterWatercut,7,'f',2,' ').arg(PIPE[pipe].osc, 4, 'g', -1, ' ').arg(" INT").arg(1, 7, 'g', -1, ' ').arg(PIPE[pipe].frequency,9,'f',3,' ').arg(0,8,'f',2,' ').arg(PIPE[pipe].oilrp,9,'f',2,' ').arg(PIPE[pipe].temperature,11,'f',2,' ').arg(0,10,'f',2,' ').arg(LOOP.masterPressure,8,'f',2,' ').arg(LOOP.masterTemp, 11,'f',2,' ').arg(LOOP.masterOilAdj, 11,'f',2,' ').arg(LOOP.masterFreq, 11,'f',2,' ').arg(LOOP.masterWatercut, 11,'f',2,' ').arg(LOOP.masterOilRp, 11,'f',2,' ').arg(LOOP.masterPhase, 6,'f',1,' ').arg(0,8,'f',2,' ');
    }

    else
    {
        if (LOOP.runMode == SIMULATION_RUN) data_stream = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16 %17 %18").arg(PIPE[pipe].etimer->elapsed()/1000, 9, 'g', -1, ' ').arg(LOOP.watercut,7,'f',2,' ').arg(PIPE[pipe].osc, 4, 'g', -1, ' ').arg(" INT").arg(1, 7, 'g', -1, ' ').arg(PIPE[pipe].frequency,9,'f',3,' ').arg(0,8,'f',2,' ').arg(PIPE[pipe].oilrp,9,'f',2,' ').arg(PIPE[pipe].temperature,11,'f',2,' ').arg(0,10,'f',2,' ').arg(LOOP.masterPressure,8,'f',2,' ').arg(LOOP.masterTemp, 11,'f',2,' ').arg(LOOP.masterOilAdj, 11,'f',2,' ').arg(LOOP.masterFreq, 11,'f',2,' ').arg(LOOP.masterWatercut, 11,'f',2,' ').arg(LOOP.masterOilRp, 11,'f',2,' ').arg(LOOP.masterPhase, 6,'f',1,' ').arg(PIPE[pipe].watercut,8,'f',2,' ');

        else data_stream = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16 %17 %18").arg(PIPE[pipe].etimer->elapsed()/1000, 9, 'g', -1, ' ').arg(LOOP.watercut,7,'f',2,' ').arg(PIPE[pipe].osc, 4, 'g', -1, ' ').arg(" INT").arg(1, 7, 'g', -1, ' ').arg(PIPE[pipe].frequency,9,'f',3,' ').arg(0,8,'f',2,' ').arg(PIPE[pipe].oilrp,9,'f',2,' ').arg(PIPE[pipe].temperature,11,'f',2,' ').arg(0,10,'f',2,' ').arg(LOOP.masterPressure,8,'f',2,' ').arg(LOOP.masterTemp, 11,'f',2,' ').arg(LOOP.masterOilAdj, 11,'f',2,' ').arg(LOOP.masterFreq, 11,'f',2,' ').arg(LOOP.masterWatercut, 11,'f',2,' ').arg(LOOP.masterOilRp, 11,'f',2,' ').arg(LOOP.masterPhase, 6,'f',1,' ').arg(0,8,'f',2,' ');
    }

    delay(SLEEP_TIME);
}


void
MainWindow::
mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) LOOP.chart->zoomReset();
}


void
MainWindow::
onDataTypeChanged()
{
	delay(SLEEP_TIME);

	/// number of objects
	if (ui->radioButton_181->isChecked()) ui->numCoils->setValue(2); // float
	else ui->numCoils->setValue(1); // int or coil

	onFunctionCodeChanges();
}

