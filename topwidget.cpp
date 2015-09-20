#include "topwidget.h"
#include <QCoreApplication>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QDebug>

#define MAXROWCOUNT 6
#define MAGICNUMBER 0x32

TopWidget::TopWidget(QWidget *parent)
    : QWidget(parent)
{
    timer = new QTimer(this);
    m_curPage = 0;

    curtime = new QLabel;
    uptime = new QLabel;
    loadavg = new QLabel;
    cpusage = new QLabel;
    memfree = new QLabel;
    memcached = new QLabel;

    statesLayout1 = new QVBoxLayout;
    statesLayout1->addWidget(curtime);
    statesLayout1->addWidget(uptime);
    statesLayout1->addWidget(loadavg);
    statesLayout1->addWidget(cpusage);
    statesLayout1->addWidget(memfree);
    statesLayout1->addWidget(memcached);

    QVBoxLayout *runtimeLayout = new QVBoxLayout;
    runtimeLayout->addWidget(curtime);
    runtimeLayout->addWidget(uptime);
    runtimeLayout->addWidget(loadavg);
    QVBoxLayout *cpmemLayout = new QVBoxLayout;
    cpmemLayout->addWidget(cpusage);
    cpmemLayout->addWidget(memfree);
    cpmemLayout->addWidget(memcached);

    statesLayout2 = new QHBoxLayout;
    statesLayout2->addLayout(runtimeLayout);
    statesLayout2->addLayout(cpmemLayout);

    btRead = new QPushButton;
    btRead->setText(tr("Read records >>"));

    btFoldBack = new QPushButton;
    btFoldBack->setText(tr("Fold back <<"));
    btFoldBack->hide();

    btDelete = new QPushButton;
    btDelete->setText(tr("Delete logs"));
    btDelete->hide();

    QHBoxLayout *btLayout = new QHBoxLayout;
    btLayout->addWidget(btRead);
    btLayout->addWidget(btFoldBack);
    btLayout->addWidget(btDelete);

    btPre = new QPushButton;
    btPre->setText(tr("Pre"));
    btPre->setEnabled(false);
    btPre->hide();

    btNext = new QPushButton;
    btNext->setText(tr("Next"));
    btNext->setEnabled(false);
    btNext->hide();

    btPreHour = new QPushButton;
    btPreHour->setText(tr("PreHour"));
    btPreHour->setEnabled(false);
    btPreHour->hide();

    btNextHour = new QPushButton;
    btNextHour->setText(tr("NextHour"));
    btNextHour->setEnabled(false);
    btNextHour->hide();

    QVBoxLayout *browseLayout = new QVBoxLayout;
    browseLayout->addWidget(btPre, 0, Qt::AlignLeft);
    browseLayout->addWidget(btPreHour, 0, Qt::AlignLeft);
    browseLayout->addWidget(btNextHour, 0, Qt::AlignLeft);
    browseLayout->addWidget(btNext, 0, Qt::AlignLeft);

    m_content = new QLabel;
    m_content->setAlignment(Qt::AlignLeft);
    m_content->setAlignment(Qt::AlignTop);
    m_content->hide();

    QHBoxLayout *displayLayout = new QHBoxLayout;
    displayLayout->addLayout(browseLayout, 0);
    displayLayout->addWidget(m_content);

    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addLayout(statesLayout1);
    mainLayout->addLayout(btLayout);
    mainLayout->addLayout(displayLayout);
    setLayout(mainLayout);

    getCurDateTime();
    getUptime();
    getLoadAverage();
    getCpuStates();
    getMemInfo();
    doWriteRecord();

    connect(timer, SIGNAL(timeout()), this, SLOT(getCurDateTime()));
    connect(timer, SIGNAL(timeout()), this, SLOT(getUptime()));
    connect(timer, SIGNAL(timeout()), this, SLOT(getLoadAverage()));
    connect(timer, SIGNAL(timeout()), this, SLOT(getCpuStates()));
    connect(timer, SIGNAL(timeout()), this, SLOT(getMemInfo()));
    connect(timer, SIGNAL(timeout()), this, SLOT(writeRecord()));

    connect(btRead, SIGNAL(clicked()), this, SLOT(readRecord()));
    connect(btFoldBack, SIGNAL(clicked()), this, SLOT(foldBack()));
    connect(btDelete, SIGNAL(clicked()), this, SLOT(deletFile()));

    connect(btPre, SIGNAL(clicked()), this, SLOT(displayPrePage()));
    connect(btNext, SIGNAL(clicked()), this, SLOT(displayNextPage()));
    connect(btPreHour, SIGNAL(clicked()), this, SLOT(displayPreHour()));
    connect(btNextHour, SIGNAL(clicked()), this, SLOT(displayNextHour()));

    timer->start(5000);
}

TopWidget::~TopWidget()
{

}

void TopWidget::getCurDateTime()
{
    m_date = QDate::currentDate();
    m_curtime = QTime::currentTime();
    curtime->setText( tr("Curtime:  ") + m_curtime.toString("HH:mm:ss") );
}

void TopWidget::getUptime()
{
    double time;
    int updays, uphours, upminutes;
    QString days;
    QString hours;
    QString minutes;    
    QString tm;

    QFile file;
    file.setFileName("/proc/uptime");
    if ( file.exists() )
    {
        if ( !file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << tr("Fail to open file %1 for reading.").arg(file.fileName());
            return;
        }
        QTextStream in(&file);
        QString str = in.readAll();
        QStringList strlist = str.split(' ', QString::SkipEmptyParts);
        time = QString(strlist.at(0)).toDouble();

        updays = (int)(time / (60*60*24));
        upminutes = (int)(time / 60);
        uphours = (upminutes / 60) % 24;
        upminutes %= 60;

        if ( updays )
            days.sprintf("%d day%s ", updays, (updays != 1) ? "s" : "");

        if ( uphours )
            hours.sprintf("%d hour%s ", uphours, (uphours != 1) ? "s" : "");

        minutes.sprintf("%d min%s", upminutes, (upminutes > 1) ? "s" : "");

        tm = days + hours + minutes;
        uptime->setText( tr("Uptime:  ") + tm );
    }
}

void TopWidget::getLoadAverage()
{
    QFile file;
    file.setFileName("/proc/loadavg");
    if ( file.exists() )
    {
        if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            qDebug() << tr("Fail to open file %1 for reading.").arg(file.fileName());
            return;
        }
        QTextStream in(&file);
        QString str = in.readAll();
        QStringList strlist = str.split(' ',QString::SkipEmptyParts);

        QString loadavg1 = strlist.at(0);
        QString loadavg5 = strlist.at(1);
        QString loadavg15 = strlist.at(2);
        loadavg->setText( tr("LoadAvg:  ") + loadavg1 + " " + loadavg5 + " " + loadavg15 );
    }
}

void TopWidget::getMemInfo()
{
    QFile file;
    file.setFileName("/proc/meminfo");
    if ( file.exists() )
    {
        if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            qDebug() << tr("Fail to open file %1 for reading.").arg(file.fileName());
            return;
        }

        QTextStream in(&file);

        in.readLine(); // skip one line
        QString str2 = in.readLine();
        QStringList strlist2 = str2.split(' ', QString::SkipEmptyParts);
        m_memfree = QString(strlist2.at(1)).toUInt();
        memfree->setText( tr("MFree:  ") + QString("%1 KB").arg(m_memfree) );

        in.readLine(); // skip one line
        QString str4 = in.readLine();
        QStringList strlist4 = str4.split(' ', QString::SkipEmptyParts);
        m_memcached = QString(strlist4.at(1)).toUInt();
        memcached->setText( tr("MCached:  ") + QString("%1 KB").arg(m_memcached) );
    }
}

#define NCPUSTAETS 9

void TopWidget::getCpuStates()
{
    static ulong cp_old[NCPUSTAETS];
    ulong cp_time[NCPUSTAETS];
    long cp_diff[NCPUSTAETS];
    double cpu_states[NCPUSTAETS];

    QFile file;
    file.setFileName("/proc/stat");
    if ( file.exists() )
    {
        if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            qDebug() << tr("Fail to open file %1 for reading.").arg(file.fileName());
            return;
        }
        QTextStream in(&file);
        QString str = in.readLine();
        QStringList strlist = str.split(' ', QString::SkipEmptyParts);
        for (int i = 0; i < NCPUSTAETS; i++)
            cp_time[i] = QString(strlist.at(i+1)).toULong();

        long change = 0;
        long total_change = 0;

        /* calculate changes for each state and the overall change */
        for (int i = 0; i < NCPUSTAETS; i++)
        {
            change = cp_time[i] - cp_old[i];
            total_change += (cp_diff[i] = change);
            cp_old[i] = cp_time[i];
        }

        /* avoid divide by zero potential */
        if (total_change == 0)
        {
            total_change = 1;
        }

//        for (int i =0; i < NCPUSTAETS; i++)
//        {
//            cpu_states[i] = (double)cp_diff[i] / total_change * 1000;
//        }

        cpu_states[3] = (double)cp_diff[3] / total_change * 1000;
        m_cpusage = (1000 - cpu_states[3]) + 0.5;

        cpusage->setText( tr("Cpusage:  ") + QString("%1%").arg((m_cpusage / 10.0), 0, 'f', 1) );
    }
}

void TopWidget::writeRecord()
{
    static int i = 0;
    i++;
    if ( i == 4 )
    {
        /* sleep for some time: 5ms */
        QTime t;
        t.start();
        while ( t.elapsed() < 5 )
            QCoreApplication::processEvents();

        doWriteRecord();
        i = 0;
    }
}

void TopWidget::doWriteRecord()
{
    QFile file;
    QDataStream out;
    file.setFileName("/tmp/states.log");
    if ( file.exists() )
    {
        if ( !file.open(QIODevice::WriteOnly | QIODevice::Append) )
        {
            qDebug() << tr("Fail to write file %1").arg(file.fileName());
            return;
        }
        out.setDevice(&file);
        out << m_date << m_curtime \
                << m_cpusage << m_memfree << m_memcached;
    }
    else
    {
        if ( !file.open(QIODevice::WriteOnly) )
        {
            qDebug() << tr("Fail to write file %1").arg(file.fileName());
            return;
        }
        out.setDevice(&file);
        out << (quint32)MAGICNUMBER << (quint16)out.version();
        out << m_date << m_curtime \
                << m_cpusage << m_memfree << m_memcached;
    }
}

void TopWidget::readRecord()
{
    QFile file;
    file.setFileName("/tmp/states.log");
    if ( file.exists() )
    {
        if ( !file.open(QIODevice::ReadOnly) )
        {
            qDebug() << tr("Fail to open file %1 for reading.").arg(file.fileName());
            return;
        }
        quint32 magic;
        quint16 version;
        QDataStream in(&file);
        in >> magic >> version;
        if ( magic != MAGICNUMBER )
        {
            QMessageBox::warning(this, tr("File error"), tr("File is not reconized by the application."));
            return;
        }
        else if ( version > in.version() )
        {
            QMessageBox::warning(this, tr("File error"), tr("File is from a more recent version of the application."));
            return;
        }
        m_streamVersion = version;

        quint32 filesize;
        filesize  = file.size();
        m_totalRecords = (filesize - 6) / 18; // file's header size is 6 bytes, every record's size is 18 bytes
        if ( m_totalRecords % MAXROWCOUNT )
            m_totalPages = ( m_totalRecords / MAXROWCOUNT ) + 1;
        else
            m_totalPages = m_totalRecords / MAXROWCOUNT;

        if ( m_totalPages > (m_curPage + 1) )
            btNext->setEnabled(true);
        if ( (m_totalPages - m_curPage) > 30 )
            btNextHour->setEnabled(true);
        if ( m_curPage >= 30 )
            btPreHour->setEnabled(true);
        else if ( m_curPage > 0 )
            btPre->setEnabled(true);

        expandLayout();
        btRead->setEnabled(false);
        btDelete->setEnabled(true);
        btFoldBack->show();
        btDelete->show();
        btPre->show();
        btNext->show();
        btPreHour->show();
        btNextHour->show();
        m_content->show();
    }
    else
        QMessageBox::warning(this, tr("File error"), tr("There is no logs\nThe log file may have been deleted."));

    displayRecords();
}

void TopWidget::foldBack()
{
    btRead->setEnabled(true);
    btFoldBack->hide();
    btDelete->hide();
    btPre->hide();
    btNext->hide();
    btPreHour->hide();
    btNextHour->hide();
    m_content->hide();
    shrinkLayout();
}

void TopWidget::expandLayout()
{
    mainLayout->removeItem(statesLayout1);
    statesLayout1->setParent(0);
    mainLayout->insertLayout(0, statesLayout2);
}

void TopWidget::shrinkLayout()
{
    mainLayout->removeItem(statesLayout2);
    statesLayout2->setParent(0);
    mainLayout->insertLayout(0, statesLayout1);
}

void TopWidget::deletFile()
{
    int ret = QMessageBox::question(this, tr("Top"), tr("Do you want to delete the log file ?"),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if ( ret == QMessageBox::Yes )
    {
        QFile file("/tmp/states.log");
        if ( file.exists() )
            file.remove();
        QMessageBox::information(this, tr("Success"), tr("The log file has been deleted."));

        m_curPage = 0;
        m_content->clear();
        btPre->setEnabled(false);
        btNext->setEnabled(false);
        btPreHour->setEnabled(false);
        btNextHour->setEnabled(false);
        btDelete->setEnabled(false);
    }
    else
        QMessageBox::information(this, tr("You choose No"), tr("Nothing is changed."));
}

void TopWidget::displayRecords()
{
    m_content->clear();

    QFile file;
    file.setFileName("/tmp/states.log");
    if ( file.exists() )
    {
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);
        in.setVersion(m_streamVersion);

        QDate date;
        QTime time;
        QString datetime;
        quint16 cpusage;
        quint32 memfree;
        quint32 memcached;

        quint32 rowOffset = m_curPage * MAXROWCOUNT;
        quint32 fileOffset = rowOffset * 18 + 6; // file header size is 6 bytes, every record size is 18 bytes.
        file.seek(fileOffset);

        QStringList headerList;
        headerList << QString("    DateTime") << QString("Cpusage") \
                << QString("MFree") << QString("  MCached\n");
        QString header = headerList.join("    ");

        QString text;
        for ( int i = 0; i < MAXROWCOUNT; i++)
        {
            quint32 recordNo = rowOffset + i + 1;
            if ( recordNo <= m_totalRecords )
            {
                in >> date >> time;
                in >> cpusage >> memfree >> memcached;
                datetime = date.toString("MM/dd") + " " + time.toString("HH:mm:ss");

                QStringList recordList;
                recordList << datetime;
                recordList << QString("%1%").arg((cpusage / 10.0), 0, 'f', 1);
                recordList << QString("%1 KB").arg(memfree);
                recordList << QString("%1 KB").arg(memcached);
                QString record = recordList.join("    ");

                text += record;
                text += "\n";
            }
            else
                break;
        }
        text = text.trimmed();
        text.prepend(header);

        m_content->setText(text);
    }
}

void TopWidget::displayPrePage()
{
    m_curPage--;

    btNext->setEnabled(true);

    if ( (m_totalPages - m_curPage) > 30 )
        btNextHour->setEnabled(true);

    if ( m_curPage < 30 )
        btPreHour->setEnabled(false);

    if ( !m_curPage )
        btPre->setEnabled(false);

    displayRecords();
}

void TopWidget::displayNextPage()
{
    m_curPage++;

    btPre->setEnabled(true);

    if ( m_curPage >= 30 )
        btPreHour->setEnabled(true);

    if ( (m_totalPages - m_curPage) <= 30 )
        btNextHour->setEnabled(false);

    if ( (m_curPage + 1) == m_totalPages )
        btNext->setEnabled(false);

    displayRecords();
}

void TopWidget::displayPreHour()
{
    m_curPage -= 30;

    btNext->setEnabled(true);
    btNextHour->setEnabled(true);

    if ( m_curPage < 30 )
        btPreHour->setEnabled(false);

    if ( !m_curPage )
        btPre->setEnabled(false);

    displayRecords();
}

void TopWidget::displayNextHour()
{
    m_curPage += 30;

    btPre->setEnabled(true);
    btPreHour->setEnabled(true);

    if ( (m_totalPages - m_curPage) <= 30 )
        btNextHour->setEnabled(false);

    if ( (m_curPage + 1) == m_totalPages )
        btNext->setEnabled(false);

    displayRecords();
}
