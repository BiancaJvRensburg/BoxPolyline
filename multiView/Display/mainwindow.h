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
    QDockWidget *editFragmentDockWidget;
    void initDisplayDockWidgets();

    // File menu
    QActionGroup *fileActionGroup;
    void initFileMenu();
    void initToolBars();
    void initFileActions();
    void initEditMenu();
    void initEditFragmentsMenu();

    void readJSON(const QJsonObject &json, Viewer *v);
    void openJSON(Viewer* v);

private Q_SLOTS:
    void openMandJSON();
    void openFibJSON();
    void setFragRadios();
    void enableFragmentEditing();
    void disableFragmentEditing();
    void displayEditMenu();
    void displayEditFragmentMenu();
    void displayFragmentMenuButton();
    void hideFragmentMenuButton();

private:
    int sliderMax = 100;
    int fibulaOffsetMax;
    QRadioButton *radioFrag1, *radioFrag2, *radioFrag3;
    QGroupBox *groupRadioBox;
    QPushButton *editPlaneButton, *editMenuButton, *editFragmentMenuButton, *toggleDrawMeshButton;
};

#endif // MAINWINDOW_H
