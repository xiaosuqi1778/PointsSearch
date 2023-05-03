#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PointsSearch.h"
#include<string>
#include<vector>
#include<algorithm>


// 点结构体
struct Point{
    int id;
    double x;//经度
    double y;//纬度
};


class PointsSearch : public QMainWindow{
    Q_OBJECT

public:
    PointsSearch(QWidget *parent = nullptr);
    ~PointsSearch();

    void startMainWindow();
    void readShpFile();
    bool readCsvFile();
    void warningMessage(QString msg);

    double distance(Point a, Point b);//计算欧几里得距离
    static bool compareX(Point a, Point b);//比较x坐标的大小
    bool compareY(Point a, Point b);//比较y坐标的大小
    Point closestPoint(std::vector< Point>& points, int left, int right, Point target);//分治法查找点
    void showPointsList(Point& pt);

private:
    Ui::PointsSearchClass ui;
    QString shpFilePath="..\\..\\MemPnts\\MemPnts.shp";
    std::string csvFilePath = "output.csv";
    std::vector<Point> allPoints;
    std::vector<Point> allPointsBackup;
    std::vector<Point> nearPoints;
    std::vector<bool> cor = { false,false };
    Point originTarget{-1, -1, -1};
    bool loadCSV=false;
};
