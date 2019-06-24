#include "TrainWidget.h"
#include "ui_TrainWidget.h"
#include <QResizeEvent>
#include <QApplication>
#include "PMTPixmapWidget.h"
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
TrainWidget::TrainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TrainWidget)
  ,m_trainType(PMT::TestType1)
  ,m_hspan(15)
  ,m_vspan(15)
  ,m_currentClicked(nullptr)
{
    ui->setupUi(this);
    ui->labelIllustration->hide();
    ui->pushButtonNo->hide();
    ui->pushButtonOK->hide();
    setHspan(15);
    setVspan(15);
    setPicSize(calcPrefectSize());
    resetXYNum(m_controller.getXnum(),m_controller.getYnum());
    connect(&m_controller,&TrainController::saveResultExcelErr
            ,this,&TrainWidget::onSaveResultExcelErr);
    connect(&m_controller,&TrainController::finish
            ,this,&TrainWidget::onFinish);
}

TrainWidget::~TrainWidget()
{
    delete ui;
}

void TrainWidget::setMatchingNum(const QString &v)
{
    m_controller.setMatchingNum(v);
}

void TrainWidget::setExpNum(const QString &v)
{
    m_controller.setExpNum(v);
}

void TrainWidget::setAge(const QString &v)
{
    m_controller.setAge(v);
}

void TrainWidget::setShortName(const QString &v)
{
    m_controller.setShortName(v);
}

PMT::TrainType TrainWidget::getTrainType() const
{
    return m_trainType;
}

void TrainWidget::setTrainType(const PMT::TrainType &trainType)
{
    m_trainType = trainType;
    int trainPicCount = 3;
    int startTestIndex = 0;
    switch(trainType)
    {
    case PMT::TestType1:
        trainPicCount = 2;
        startTestIndex = 0;
        break;
    case PMT::TestType2:
        trainPicCount = 2;
        startTestIndex = 0;
        break;
    case PMT::FormalType1:
        trainPicCount = 4;
        startTestIndex = 1;
        m_trainOrder.append(4);
        break;
    case PMT::FormalType2:
        trainPicCount = 5;
        startTestIndex = 2;
        m_trainOrder.append(5);
        break;
    case PMT::FormalType3:
        trainPicCount = 6;
        startTestIndex = 3;
        m_trainOrder.append(6);
        break;
    default:
        break;
    }
    m_controller.makeProject(trainType,trainPicCount,startTestIndex);
    //重置图片
    resetPictureInGroup1();
}


void TrainWidget::resetXYNum(int xnum, int ynum)
{
    foreach (PMTPixmapWidget* w, m_picList) {
        delete w;
    }
    m_picList.clear();
    recalcSize();
    int xoffset = m_picSize.width() + getHspan();
    int yoffset = m_picSize.height() + getVspan();
    for(int j=0;j<ynum;++j)
    {
        for(int i=0;i<xnum;++i)
        {
            PMTPixmapWidget* w = new PMTPixmapWidget(this);
            w->setId(i+j*m_controller.getYnum());
            connect(w,&PMTPixmapWidget::finishPictureMem,this,&TrainWidget::onFinishPictureMem);
            connect(w,&PMTPixmapWidget::clicked,this,&TrainWidget::onClicked);
            w->setGeometry(m_topleftPoint.x()+xoffset*i,m_topleftPoint.y()+yoffset*j,m_picSize.width(),m_picSize.height());
            m_picList.append(w);
        }

    }
}


void TrainWidget::resizeEvent(QResizeEvent *e)
{
    setPicSize(calcPrefectSize());
    recalcSize();
    int xoffset = m_picSize.width() + getHspan();
    int yoffset = m_picSize.height() + getVspan();
    for(int j=0;j<m_controller.getYnum();++j)
    {
        for(int i=0;i<m_controller.getXnum();++i)
        {
            PMTPixmapWidget* w = m_picList[i+j*m_controller.getYnum()];
            w->setGeometry(m_topleftPoint.x()+xoffset*i,m_topleftPoint.y()+yoffset*j,m_picSize.width(),m_picSize.height());
        }

    }
    //设置说明label
    int lh = 50;
    ui->labelIllustration->setGeometry(5,height()/2-lh/2,width()-10,lh);
    int btnH = 30;
    int btnW = 100;
    ui->pushButtonOK->setGeometry(width()-10-btnW,height()/2-btnH-20,btnW,btnH);
    ui->pushButtonNo->setGeometry(width()-10-btnW,height()/2+btnH+20,btnW,btnH);
    QWidget::resizeEvent(e);
}

/**
 * @brief 设置图片的显示
 * @param visible
 */
void TrainWidget::setPictureVisible(bool visible)
{
    for(int i=0;i<m_picList.size();++i)
    {
        m_picList[i]->setVisible(visible);
    }
}

void TrainWidget::recalcSize()
{
    //计算离上下的总间距
    int v = (getPicSize().height() * m_controller.getYnum()) + (getVspan() * (m_controller.getYnum() - 1));
    m_topleftPoint.ry() = (height() - v) / 2;
    int h = (getPicSize().width() * m_controller.getXnum()) + (getHspan() * (m_controller.getXnum()-1));
    m_topleftPoint.rx() = (width() - h) / 2;

}

QSize TrainWidget::calcPrefectSize() const
{
    int h = height() / m_controller.getYnum();
    int w = width() / m_controller.getXnum();
    int min = qMin(h,w);
    min = min - qMax(m_hspan,m_vspan);
    return QSize(min,min);
}


/**
 * @brief 重新设置图片
 */
void TrainWidget::resetPictureInGroup1()
{
    ui->pushButtonNo->hide();
    ui->pushButtonOK->hide();
    m_nameToPMTWidgetGroup1.clear();
    for(int i=0;i<m_picList.size();++i)
    {
        m_picList[i]->setPixmap(m_controller.getPixmap(i),m_controller.getPixmapName(i));
        m_picList[i]->showDelayDisplayBlankSpace(2000);
        m_picList[i]->setClickActionMode(PMTPixmapWidget::ClickShowPicture);
        m_picList[i]->setSelected(false);
        m_nameToPMTWidgetGroup1[m_controller.getPixmapName(i)] = m_picList[i];
    }
}

void TrainWidget::resetPictureInGroup2()
{
    m_nameToPMTWidgetGroup2.clear();
    for(int i=0;i<m_picList.size();++i)
    {
        m_picList[i]->setPixmap(m_controller.getPixmap(i,TrainController::Group2),m_controller.getPixmapName(i,TrainController::Group2));
        m_picList[i]->setShowPicture();
        m_picList[i]->setClickActionMode(PMTPixmapWidget::ClickSelect);
        m_picList[i]->setSelected(false);
        m_nameToPMTWidgetGroup2[m_controller.getPixmapName(i,TrainController::Group2)] = m_picList[i];
    }
}

void TrainWidget::resetPictureInGroup3()
{
    m_locationTestValues = m_controller.getLocationTestValues();
    Q_ASSERT(m_locationTestValues.size() > 0);
    m_currentLocationTestIndex = 0;
    showTestLocation();
}

void TrainWidget::showTestLocation()
{
    for(int i=0;i<m_picList.size();++i)
    {
        m_picList[i]->setShowBrush();
        m_picList[i]->setSelected(false);
        m_picList[i]->setClickActionMode(PMTPixmapWidget::ClickNothing);
        m_picList[i]->repaint();
    }
    LocationTestValue v = m_locationTestValues[m_currentLocationTestIndex];
    PMTPixmapWidget* w = m_picList[v.showLocation];
    w->setShowPicture();
    w->setPixmap(m_controller.getPixmap(v.picName),v.picName);
    w->setClickActionMode(PMTPixmapWidget::ClickNothing);
    qDebug() << "v.showLocation：" << v.showLocation;
}
/**
 * @brief 重置为测试模式
 */
void TrainWidget::resetTestType()
{
    for(int i=0;i<m_picList.size();++i)
    {
        m_picList[i]->setClickActionMode(PMTPixmapWidget::ClickShowPicture);
    }
}

void TrainWidget::setToOrderMemTestType()
{
    resetPictureInGroup2();
    emit showTooltip(tr("顺序记忆测试"),3000);
}

void TrainWidget::setToLocationMemTestType()
{
    resetPictureInGroup3();
    emit showTooltip(tr("位置记忆测试"),3000);
}


/**
 * @brief 根据PMTPixmapWidget查找索引
 * @param w
 * @return
 */
int TrainWidget::findIndexByPMTPixmapWidget(PMTPixmapWidget *w)
{
    return m_picList.indexOf(w);
}

/**
 * @brief pixmap被点击触发的槽
 * @param name
 */
void TrainWidget::onFinishPictureMem(const QString &name,QDateTime clickedInTime,QDateTime clickedOutTime)
{
    if(!m_nameToPMTWidgetGroup1.contains(name))
    {
        qDebug() << "can not find widget!";
        return;
    }
    PMTPixmapWidget* w = m_nameToPMTWidgetGroup1[name];

    if (!m_controller.isFinishSelPic())
    {
        //把其余设置为可选，把这个设置为不可选
        w->setClickActionMode(PMTPixmapWidget::ClickNothing);
        for(int i=0;i<m_picList.size();++i)
        {
            m_picList[i]->setClickActionMode(PMTPixmapWidget::ClickShowPicture);
        }
        m_currentClicked = nullptr;
        PMTTestSelRecord ptr;
        ptr.picName = name;
        ptr.location = findIndexByPMTPixmapWidget(w);
        ptr.testRange = m_controller.getSelPicCount();
        ptr.picShowTime = clickedInTime;
        ptr.testPicCount = m_controller.getTotalTrainPicCount();
        ptr.picDisappearTime = clickedOutTime;
        ptr.testRecordName = QStringLiteral("%1-%2").arg(ptr.testPicCount).arg(ptr.testRange);
        m_controller.appendOneSelRecord(ptr);
        if(m_controller.isFinishSelPic())
        {
            //如果此时选图模式结束，就进入突击测试阶段
            showIllustration(tr("突 击 测 试"),2000,[this](){
                this->setToOrderMemTestType();
            });
            m_currentClicked = nullptr;
        }
    }

//    if(!m_controller.isFinishOrderMemTest())
//    {
//        //进入顺序测试阶段
//    }
}

void TrainWidget::onClicked(const QString& name)
{
    if (!m_controller.isFinishSelPic())
    {
        if(!m_nameToPMTWidgetGroup1.contains(name))
        {
            qDebug() << "can not find widget!";
            return;
        }
        PMTPixmapWidget* w = m_nameToPMTWidgetGroup1[name];
        //还未完成选图，
        if(nullptr == m_currentClicked)
        {
            m_currentClicked = w;
            //把其余设置为不可选
            for(int i=0;i<m_picList.size();++i)
            {
               if(m_picList[i] != w)
                   m_picList[i]->setClickActionMode(PMTPixmapWidget::ClickNothing);
            }
        }
    }
    else if(!m_controller.isFinishOrderMemTest())
    {
        if(!m_nameToPMTWidgetGroup2.contains(name))
        {
            qDebug() << "can not find widget!";
            return;
        }
        PMTPixmapWidget* w = m_nameToPMTWidgetGroup2[name];
        //进入顺序测试阶段
        //记录选中内容m_orderMemSelName
        m_currentClicked = w;
        if(w->isSelected())
        {
            m_controller.appendOrderMemTestRecord(name);
            if(m_controller.isFinishOrderMemTest())
            {
                QString des;
                QList<bool> ores = m_controller.orderMemTestResult();
                for(int i=0;i<ores.size();++i)
                {
                    des += tr("%1、结果:%2 ; ").arg(i+1).arg(ores[i] ? tr("正确") : tr("错误"));
                }
                qDebug() << des;
                //训练模式下把操作结果展示
                if(PMT::TestType1 == m_trainType || PMT::TestType2 == m_trainType)
                {
                    showIllustration(des,2000,[this](){
                        this->setToLocationMemTestType();
                        ui->pushButtonNo->show();
                        ui->pushButtonOK->show();
                    });
                }
                else
                {
                    setToLocationMemTestType();
                    ui->pushButtonNo->show();
                    ui->pushButtonOK->show();
                }
            }
        }
        else
        {
            if(!m_controller.orderMemTestRecordRef().isEmpty())
            {
                if(m_controller.orderMemTestRecordRef().last() == name)
                {
                    m_controller.popOrderMemTestRecord();
                }
            }
        }
    }
    else if (!m_controller.isFinishLocationMemTest()) {

    }
}

void TrainWidget::onSaveResultExcelErr()
{
    emit showTooltip(tr("结果文件存入excel失败，请检查excel是否被占用"),2000);
    QMessageBox::StandardButton btn = QMessageBox::Retry;
    int rt = 0;
    while(btn == QMessageBox::Retry)
    {
        if(rt >3)
        {
            QMessageBox::information(this,tr("警告"),tr("重试失败"));
            return;
        }
        btn = QMessageBox::warning(this,tr("警告"),tr("结果文件存入excel失败，请检查excel是否被占用")
                             ,QMessageBox::Retry|QMessageBox::Cancel,QMessageBox::Retry);
        if(btn == QMessageBox::Retry)
        {
            m_controller.saveResult();
            ++rt;
        }
    }
}

void TrainWidget::onFinish()
{
    if(PMT::FormalType1 == getTrainType()
            ||
       PMT::FormalType2 == getTrainType()
            ||
       PMT::FormalType2 == getTrainType())
    {
        m_controller.saveResult();
        m_controller.savePicTestOrder(m_trainOrder);
    }
}

int TrainWidget::getVspan() const
{
    return m_vspan;
}

void TrainWidget::setVspan(int vspan)
{
    m_vspan = vspan;
}

/**
 * @brief 显示说明
 * 显示说明阶段会把所有隐藏
 * @param str 说明内容
 * @param ms 说明内容持续时间
 */
void TrainWidget::showIllustration(const QString &str, int ms, std::function<void(void)> fn)
{
    setPictureVisible(false);//先把图片隐藏
    ui->labelIllustration->setText(str);
    ui->labelIllustration->show();
    QTimer::singleShot(ms,this,[this,fn](){
        ui->labelIllustration->hide();
        setPictureVisible(true);
        if(fn != nullptr)
        {
            fn();
        }
    });
}



int TrainWidget::getHspan() const
{
    return m_hspan;
}

void TrainWidget::setHspan(int hspan)
{
    m_hspan = hspan;
}

QSize TrainWidget::getPicSize() const
{
    return m_picSize;
}

void TrainWidget::setPicSize(const QSize &picSize)
{
    m_picSize = picSize;
}



void TrainWidget::on_pushButtonOK_clicked()
{
    if(m_currentLocationTestIndex >= m_locationTestValues.size())
    {
        qDebug() << "click too more";
        return;
    }
    m_controller.appendLocationMemTestRecord(true);
    if(m_controller.isFinishLocationMemTest())
    {
        ui->pushButtonNo->hide();
        ui->pushButtonOK->hide();
        emit finish(getTrainType());
    }
    else
    {
        ++m_currentLocationTestIndex;
        showTestLocation();
    }
}

void TrainWidget::on_pushButtonNo_clicked()
{
    if(m_currentLocationTestIndex >= m_locationTestValues.size())
    {
        qDebug() << "click too more";
        return;
    }
    m_controller.appendLocationMemTestRecord(false);
    if(m_controller.isFinishLocationMemTest())
    {
        ui->pushButtonNo->hide();
        ui->pushButtonOK->hide();
        emit finish(getTrainType());
    }
    else
    {
        ++m_currentLocationTestIndex;
        showTestLocation();
    }
}