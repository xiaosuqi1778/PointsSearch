#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PointsSearch.h"
#include<string>
#include<vector>
#include<algorithm>


// ��ṹ��
struct Point{
    int id;
    double x;//����
    double y;//γ��
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

    double distance(Point a, Point b);//����ŷ����þ���
    static bool compareX(Point a, Point b);//�Ƚ�x����Ĵ�С
    bool compareY(Point a, Point b);//�Ƚ�y����Ĵ�С
    Point closestPoint(std::vector< Point>& points, int left, int right, Point target);//���η����ҵ�
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
