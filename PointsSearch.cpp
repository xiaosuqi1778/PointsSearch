#include "PointsSearch.h"
#include <QFileDialog>
#include<qmessagebox.h>
#include<iostream>
#include <fstream>
#include<string>
#include "gdal.h"
#include "ogr_api.h"
#include "ogrsf_frmts.h"

using std::string;
using std::vector;
using std::endl;
using std::ifstream;
using std::ofstream;

PointsSearch::PointsSearch(QWidget *parent)
    : QMainWindow(parent)
{
    //初始化窗口
    ui.setupUi(this);
    startMainWindow();

    //信号槽部分
    //选择shp文件
    connect(ui.chooseShpFile,&QPushButton::clicked, this, [&]() {
        shpFilePath = QFileDialog::getOpenFileName(NULL, "Select a shp file","..", "*.shp");
        readShpFile();
        warningMessage("\nThe shp file information has saved.\n");
        loadCSV=readCsvFile();
    });
    //输入查询点坐标
    connect(ui.inputX, &QLineEdit::textChanged, this, [&]() {
        originTarget.x = ui.inputX->text().toDouble();
        cor[0] = true;
    });
    connect(ui.inputY, &QLineEdit::textChanged, this, [&]() {
        originTarget.y = ui.inputY->text().toDouble();
        cor[1] = true;
    });
    //查询并展示结果
    connect(ui.searchBtn, &QPushButton::clicked, this, [&]() {
        if (!cor[0]||!cor[1]) {
            warningMessage("\nPlease input target coordinate!\n");
            return;
        }
        if (!loadCSV) {
            warningMessage("Read CSV File Error!");
            return;
        }
        allPointsBackup = allPoints;
        Point theClosestPoint = closestPoint(allPoints, 0, allPoints.size() - 1, originTarget);
        showPointsList(theClosestPoint);
        allPoints = allPointsBackup;
    });
}

PointsSearch::~PointsSearch(){}

void PointsSearch::startMainWindow()
{
    ui.pointsView->setColumnCount(3);//设置列数
    ui.pointsView->setRowCount(40);//设置行数
    //ui.pointsView->setWindowTitle("QTableWidget");
    ui.pointsView->verticalHeader()->setVisible(true);//纵向表头可视化
    ui.pointsView->horizontalHeader()->setVisible(true);//横向表头可视化
    ui.pointsView->setEditTriggers(QAbstractItemView::NoEditTriggers);//设置编辑方式：禁止编辑表格
    ui.pointsView->setSelectionBehavior(QAbstractItemView::SelectRows);//设置表格选择方式：设置表格为整行选中
    ui.pointsView->setSelectionMode(QAbstractItemView::SingleSelection);//选择目标方式
    ui.pointsView->setStyleSheet("selection-background-color:pink");//设置选中颜色：粉色
    ui.pointsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);//设置垂直滚动条
    //设置行和列的大小
    ui.pointsView->setColumnWidth(0, 80);
    ui.pointsView->setColumnWidth(1,110);
    ui.pointsView->setColumnWidth(2, 110);
    //设置行列标签
    QStringList HStrList;
    HStrList.push_back(QString("ID"));
    HStrList.push_back(QString("X(经度)"));
    HStrList.push_back(QString("Y(纬度)"));
    ui.pointsView->setHorizontalHeaderLabels(HStrList);
}

void PointsSearch::readShpFile()
{
    // 注册GDAL/OGR驱动程序
    GDALAllRegister();
    OGRRegisterAll();

    // 打开shp文件
    std::string str = shpFilePath.toStdString();
    const char* sfp = str.c_str();
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(sfp, GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == NULL) {
        warningMessage("Open File Error.");
        return;
    }

    // 获取第一层图层
    OGRLayer* poLayer = poDS->GetLayer(0);
    // 获取图层目标空间参考
    OGRSpatialReference* poSR = poLayer->GetSpatialRef();
    // 获取图层范围
    OGREnvelope adfExtent;
    poLayer->GetExtent(&adfExtent);

    // 创建CSV文件并写入表头
    std::ofstream outfile("output.csv");
    outfile << "CSVFORMA:TNAME,PointCoordinates" << std::endl;
    outfile << "ATTRIBUTES,Id,X,Y" << std::endl;
    outfile << "DATATYPE,Table" << std::endl;
    outfile << "SPATIALREF," << poSR->GetAttrValue("AUTHORITY", 1) << ",";
    const char* pszEPSG = poSR->GetAttrValue("AUTHORITY", 1);
    if (pszEPSG != NULL)
        outfile << pszEPSG << std::endl;
    else
        outfile << "Unknown EPSG code" << std::endl;
    outfile << "LayerRange," << adfExtent.MinX << "," << adfExtent.MinY << ",";
    outfile << adfExtent.MaxX << "," << adfExtent.MaxY << std::endl;

    // 遍历图层并写入坐标
    double x, y;
    int id = 1;
    poLayer->ResetReading();
    OGRFeature* poFeature;
    while ((poFeature = poLayer->GetNextFeature()) != NULL){
        // 获取要素的几何体
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        // 如果是点则获取其坐标并写入CSV
        if (wkbFlatten(poGeometry->getGeometryType()) == wkbPoint){
            OGRPoint* poPoint = (OGRPoint*)poGeometry;
            x = poPoint->getX();
            y = poPoint->getY();
            outfile << id << ',' << x << ',' << y << std::endl;
            id++;
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    // 关闭文件、释放内存并关闭数据集
    outfile.close();
    GDALClose(poDS);

}

bool PointsSearch::readCsvFile()
{
    // 打开csv文件
    std::ifstream infile(csvFilePath);
    if (!infile.is_open()) {
        return false;
    }

    // 读取文件头
    ui.targetView->append(QString::fromStdString("CSV Header Information:"));
    std::string header1, header2;
    getline(infile, header1); 
    ui.targetView->append(QString::fromStdString(header1));
    getline(infile, header1); 
    ui.targetView->append(QString::fromStdString(header1));
    getline(infile, header1);
    ui.targetView->append(QString::fromStdString(header1));
    getline(infile, header1);
    ui.targetView->append(QString::fromStdString(header1));
    getline(infile, header2); 
    ui.targetView->append(QString::fromStdString(header2));

    // 分割表头行
    std::vector<std::string> headers;
    std::string field;
    for (int i = 0; i < header2.length(); i++){
        if (header2[i] == ','){
            headers.push_back(field);
            field.clear();
        }
        else{
            field.push_back(header2[i]);
        }
    }
    headers.push_back(field);

    // 读取数据
    string line;
    while (getline(infile, line)){
        // 分割数据行
        string id;
        string x;
        string y;
        int cnt = 0;
        for (int i = 0; i < line.length(); i++){
            if (line[i] == ','){
                cnt++;
                continue;
            }
            if (cnt == 0){
                id.push_back(line[i]);
            }
            else if (cnt == 1){
                x.push_back(line[i]);
            }
            else{
                y.push_back(line[i]);
            }
        }
        // 保存数据
        Point p;
        p.id = stoi(id);
        p.x = stod(x);
        p.y = stod(y);
        allPoints.push_back(p);
    }
    // 关闭文件
    infile.close();
    return true;
}

void PointsSearch::warningMessage(QString msg)
{
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.exec();
}

double PointsSearch::distance(Point a, Point b)
{
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

bool PointsSearch::compareX(Point a, Point b)
{
    return a.x < b.x;
}

bool PointsSearch::compareY(Point a, Point b)
{
    return a.y < b.y;
}

Point PointsSearch::closestPoint(std::vector<Point>& points, int left, int right, Point target)
{
    // 如果只有一个点，直接返回
    if (left == right){
        return points[left];
    }

    // 如果有两个点，比较它们和目标点的距离，返回较近的那个
    if (left + 1 == right){
        if (distance(points[left], target) < distance(points[right], target)){
            return points[left];
        }
        else{
            return points[right];
        }
    }

    // 如果有多于两个点，先按照x坐标排序，然后找到中间位置
    sort(points.begin() + left, points.begin() + right + 1, compareX);
    int mid = (left + right) / 2;

    // 如果目标点的x坐标小于中间位置的x坐标，说明目标点在左半部分，递归查找左半部分的最近点
    if (target.x < points[mid].x){
        return closestPoint(points, left, mid, target);
    }
    // 如果目标点的x坐标大于中间位置的x坐标，说明目标点在右半部分，递归查找右半部分的最近点
    else if (target.x > points[mid].x){
        return closestPoint(points, mid + 1, right, target);
    }
    // 如果目标点的x坐标等于中间位置的x坐标，说明目标点可能在左半部分或右半部分，需要比较两边的最近点
    else{
        // 递归查找左半部分和右半部分的最近点
        Point leftClosest = closestPoint(points, left, mid - 1, target);
        Point rightClosest = closestPoint(points, mid + 1, right, target);

        // 比较两个最近点和中间位置的点和目标点的距离，返回最小的那个
        double d1 = distance(leftClosest, target);
        double d2 = distance(rightClosest, target);
        double d3 = distance(points[mid], target);

        if (d1 <= d2 && d1 <= d3){
            return leftClosest;
        }
        else if (d2 <= d1 && d2 <= d3){
            return rightClosest;
        }
        else{
            return points[mid];
        }
    }
}

void PointsSearch::showPointsList(Point& pt)
{
    ui.pointsView->clearContents();
    int startID = (pt.id - 20 < 1) ? 1 : pt.id - 20;//开始的点
    int endID= (allPointsBackup.size() < pt.id + 20) ? allPointsBackup.size() : pt.id + 20;//结束的点
    for (int i = 0; i <= (endID-startID); i++) {
        if (allPointsBackup[startID + i - 1].id == pt.id)continue;
        nearPoints.push_back(allPointsBackup[startID + i-1]);
    }
    for (unsigned int i = 0; i < nearPoints.size(); i++) {
        ui.pointsView->setItem(i, 0, new QTableWidgetItem(QString::number(nearPoints[i].id)));
        ui.pointsView->setItem(i, 1, new QTableWidgetItem(QString::number(nearPoints[i].x)));
        ui.pointsView->setItem(i, 2, new QTableWidgetItem(QString::number(nearPoints[i].y)));
    }
    QString msg = QString("\nThe nearest(or target) point information:")
        .append("\ntargetID:").append(QString::number(pt.id))
        .append("\ntargetX:").append(QString::number(pt.x))
        .append("\ntargetY:").append(QString::number(pt.y))
        .append("\ndistance:").append(QString::number(sqrt(distance(pt,originTarget))));
    ui.targetView->append(msg);
    nearPoints.clear();
}