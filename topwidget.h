#ifndef TOPWIDGET_H
#define TOPWIDGET_H

#include <QtGui/QWidget>
#include <QDate>

class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

class TopWidget : public QWidget
{
    Q_OBJECT

public:
    TopWidget(QWidget *parent = 0);
    ~TopWidget();

public slots:
    void getCurDateTime();
    void getUptime();
    void getLoadAverage();
    void getCpuStates();
    void getMemInfo();
    void writeRecord();
    void readRecord();
    void foldBack();
    void deletFile();
    void displayPrePage();
    void displayNextPage();
    void displayPreHour();
    void displayNextHour();

private:
    void doWriteRecord();
    void expandLayout();
    void shrinkLayout();
    void displayRecords();

private:
    QTimer *timer;

    QLabel *uptime;
    QLabel *curtime;
    QDate m_date;
    QTime m_curtime;

    QLabel *loadavg;
    QLabel *cpusage;
    quint16 m_cpusage;

    QLabel *memfree;
    QLabel *memcached;
    quint32 m_memfree;
    quint32 m_memcached;

    QPushButton *btRead;
    QPushButton *btFoldBack;
    QPushButton *btDelete;

    QPushButton *btPre;
    QPushButton *btNext;
    QPushButton *btPreHour;
    QPushButton *btNextHour;
    QLabel *m_content;

    quint16 m_streamVersion;
    quint32 m_totalRecords;
    quint32 m_totalPages;
    quint32 m_curPage;

    QVBoxLayout *statesLayout1;
    QHBoxLayout *statesLayout2;
    QVBoxLayout *mainLayout;
};

#endif // TOPWIDGET_H
