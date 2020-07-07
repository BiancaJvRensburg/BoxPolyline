#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include "Display/viewerfibula.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    // Main viewers
    Viewer *skullViewer;
    ViewerFibula *fibulaViewer;
    QDockWidget *skullDockWidget;
    QDockWidget *editMenuWidget;
    void initDisplayDockWidgets();

    // File menu
    QActionGroup *fileActionGroup;
    void initFileMenu();
    void initToolBars();
    void initFileActions();


    void readJSON(const QJsonObject &json, Viewer *v);
    void openJSON(Viewer* v);

private Q_SLOTS:
    void openMandJSON();
    void openFibJSON();
    void initEditMenu();
    void setFragRadios();
    void enableFragmentEditing();
    void disableFragmentEditing();
    /*void transitionUncutToCut();
    void transitionCutToUncut();
    void transitionUncutToEdit();
    void transitionCutToEdit();
    void transitionEditToUncut();
    void transitionEditToCut();*/

private:
    void uncutStage();
    void cutStage();
    void editStage();
    int sliderMax = 100;
    int fibulaOffsetMax;
    QRadioButton *radioFrag1, *radioFrag2, *radioFrag3;
    QGroupBox *groupRadioBox;
};

#endif // MAINWINDOW_H
